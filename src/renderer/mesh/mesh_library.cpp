
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "renderer/mesh/mesh_library.h"
#include "utils/log.h"

// 
mesh_handle_t mesh_library_t::load_mesh(const std::string &_name, 
                                        const vertex_data_t *_vertices, 
                                        size_t _vertex_count,
                                        const uint32_t *_indices,
                                        size_t _index_count)
{
    auto it = m_name_to_handle_map.find(_name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }

    if (m_mesh_count >= SYN_MAX_MESH_COUNT) {
        SYN_ERROR("max mesh allocations reached.\n");
        return { 0 };
    }

    mesh_internal_t *mesh = &m_meshes[m_mesh_count];
    mesh_handle_t handle = { (uint32_t)m_mesh_count + 1 };

    //
    mesh->vao.create(_vertices, _vertex_count, _indices, _index_count);

    //
    glm::vec3 min_bounds(FLT_MAX);
    glm::vec3 max_bounds(-FLT_MAX);
    for (size_t i = 0; i < _vertex_count; i++) {
        glm::vec3 pos = _vertices[i].position;
        min_bounds = glm::min(pos, min_bounds);
        max_bounds = glm::max(pos, max_bounds);
    }

    // mesh metadata
    mesh->aabb_min      = min_bounds;
    mesh->aabb_max      = max_bounds;
    mesh->vertex_count  = _vertex_count;
    mesh->index_count   = _index_count;
    mesh->name          = _name;
    mesh->is_active     = true;

    m_mesh_count++;
    m_name_to_handle_map[_name] = handle;
    
    return handle;
    
}

// 
mesh_handle_t mesh_library_t::load_mesh_from_file(const std::string &_filepath)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(_filepath.c_str(), 
        aiProcess_Triangulate |
        // aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SYN_ERROR("assimp error: %s.\n", importer.GetErrorString());
        return { 0 };
    }

    #ifdef DEBUG_MESH_LIBRARY
    SYN_INFO("loading model: %s | meshes found: %d\n", _filepath.c_str(), scene->mNumMeshes);
    #endif

    aiMesh *ai_mesh = scene->mMeshes[0];
    std::vector<vertex_data_t> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++) {
        vertex_data_t vertex;
        vertex.position = { ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z };
        vertex.normal = { ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z };
        if (ai_mesh->mTextureCoords[0]) {
            vertex.uv = { ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y };
        } else {
            vertex.uv = { 0.0f, 0.0f };
        }

        vertex.tangent = { ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z };
        vertex.bitangent = { ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z };
        
        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < ai_mesh->mNumFaces; i++) {
        aiFace face = ai_mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return load_mesh(_filepath, vertices.data(), vertices.size(), indices.data(), indices.size());
}

// 
mesh_handle_t mesh_library_t::load_mesh_from_vao(const std::string &_name, vertex_array_t _vao, void *_vertex_data)
{
    auto it = m_name_to_handle_map.find(_name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }

    if (m_mesh_count >= SYN_MAX_MESH_COUNT) {
        SYN_ERROR("max mesh allocations reached.\n");
        return (mesh_handle_t){ 0 };
    }

    if (_vao.m_buffer_layout.m_element_count == 0) {
        SYN_ERROR("mesh '%s': vertex array contains no buffer layout.\n", _name.c_str());
        return { 0 };
    }
    
    uint32_t slot = (uint32_t)m_mesh_count;
    mesh_internal_t *mesh = &m_meshes[slot];
    if (!mesh) return { 0 };

    // AABB
    glm::vec3 min_bounds = glm::vec3(FLT_MAX);
    glm::vec3 max_bounds = glm::vec3(-FLT_MAX);

    // For vertex_array_t:s with custom buffer element layouts its slightly more 
    // complicated; we need to know the offset and size of the position and stride 
    // of the layout. The vertex positions can then be casted as float *pointers
    // and compared to the bounds.
    // 
    if (!_vertex_data) {
        SYN_WARNING("mesh '%s': no raw vertex data provided, omitting AABB.\n", _name.c_str());
        min_bounds = glm::vec3(0.0f);
        max_bounds = glm::vec3(0.0f);
    }
    else {
        uint32_t position_offset = 0;
        uint32_t position_size = 0;
        bool found_position = false;
        for (size_t i = 0; i < _vao.m_buffer_layout.m_element_count; i++) {
            buffer_element_t e = _vao.m_buffer_layout.m_elements[i];
            if (e.shader_location == VERTEX_ATTRIB_LOCATION_POSITION) {
                position_offset = e.offset;
                position_size = shader_data_type_size(e.type);
                found_position = true;
            }
        }
        
        if (!found_position) {
            SYN_WARNING("mesh '%s': no position attribute, omitting AABB.\n", _name.c_str());
            min_bounds = glm::vec3(0.0f);
            max_bounds = glm::vec3(0.0f);
        } 
        else if (position_size != shader_data_type_size(shader_data_type_t::FLOAT3) &&
            position_size != shader_data_type_size(shader_data_type_t::FLOAT4)) {
            SYN_WARNING("mesh '%s': vertex position data size invalid (size=%d), omitting AABB.\n", _name.c_str(), position_size);
            min_bounds = glm::vec3(0.0f);
            max_bounds = glm::vec3(0.0f);
        } 
        else {
            
            // address of first position takes position_offset into account
            const uint8_t *vertex_base = (uint8_t *)_vertex_data + position_offset;
            const uint8_t *vertex_data = vertex_base;
            uint32_t stride = _vao.m_buffer_layout.m_stride;
            
            for (size_t i = 0; i < _vao.get_vertex_count(); i++) {
                if ((size_t)(vertex_data - vertex_base) > stride * _vao.get_vertex_count()) {
                    SYN_WARNING("mesh '%s': vertex data buffer overflow, omitting AABB.\n", _name.c_str());
                    min_bounds = glm::vec3(0.0f);
                    max_bounds = glm::vec3(0.0f);
                    break;
                }
                
                const float *position = (float *)vertex_data;
                
                if (position_size == 12) {
                    glm::vec3 pos = *(glm::vec3 *)position;
                    min_bounds = glm::min(pos, min_bounds);
                    max_bounds = glm::max(pos, max_bounds);
                } else {
                    glm::vec4 pos_ = *(glm::vec4 *)position;
                    glm::vec3 pos = glm::vec3(pos_.x, pos_.y, pos_.z);
                    min_bounds = glm::min(pos, min_bounds);
                    max_bounds = glm::max(pos, max_bounds);
                }
    
                vertex_data += stride;
            }
        }
    }
    
    
    mesh->vao = _vao;
    mesh->aabb_min = min_bounds;
    mesh->aabb_max = max_bounds;
    mesh->vertex_count = _vao.get_vertex_count();
    mesh->index_count = _vao.get_index_count();
    mesh->name = _name;
    mesh->is_active = true;

    m_mesh_count++;
    
    return { slot + 1 };
    
}

// 
mesh_internal_t *mesh_library_t::get_mesh(mesh_handle_t handle)
{
    uint32_t idx = handle.id - 1;
    if (handle.id == 0 || idx >= m_mesh_count)
        return nullptr;
    return &m_meshes[idx];
}

// 
mesh_handle_t mesh_library_t::get_handle_by_name(const std::string &name)
{
    auto it = m_name_to_handle_map.find(name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }
    return (mesh_handle_t){ 0 };
}

// 
void mesh_library_t::shutdown()
{
    SYN_INFO("deleting %ld meshes...\n", m_mesh_count);
    for (size_t i = 0; i < m_mesh_count; i++) {
        m_meshes[i].vao.destroy();
    }
}

