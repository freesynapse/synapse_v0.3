
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
        return (mesh_handle_t){ 0 };
    }

    uint32_t slot = (uint32_t)m_mesh_count;
    m_meshes[slot].create(_vertices, _vertex_count, _indices, _index_count);
    m_mesh_count++;
    return (mesh_handle_t){ slot + 1 };
    
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
mesh_handle_t mesh_library_t::load_mesh_from_vao(const std::string &_name, vertex_array_t _vao)
{
    auto it = m_name_to_handle_map.find(_name);
    if (it != m_name_to_handle_map.end()) {
        return it->second;
    }

    if (m_mesh_count >= SYN_MAX_MESH_COUNT) {
        SYN_ERROR("max mesh allocations reached.\n");
        return (mesh_handle_t){ 0 };
    }

    uint32_t slot = (uint32_t)m_mesh_count;
    m_meshes[slot] = _vao;
    m_mesh_count++;
    return (mesh_handle_t){ slot + 1 };
    
}

// 
const vertex_array_t *mesh_library_t::get(mesh_handle_t handle) const
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
        m_meshes[i].destroy();
    }
}

