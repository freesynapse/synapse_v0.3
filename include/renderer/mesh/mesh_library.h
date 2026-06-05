#ifndef __MESH_LIBRARY_H
#define __MESH_LIBRARY_H

#include <string>
#include <unordered_map>

#include "renderer/mesh/mesh_types.h"
#include "renderer/buffers/vertex_array.h"

// 
#define SYN_MAX_MESH_COUNT 128

//
class mesh_library_t
{
public:
    mesh_library_t() = default;
    ~mesh_library_t() = default;

    mesh_handle_t load_mesh(const std::string &_name, 
                            const vertex_data_t *_vertices, 
                            size_t _vertex_count,
                            const uint32_t *_indices,
                            size_t _index_count);

    // set directly from a vao, permitting alternate buffer layouts (i.e. vertex attrubutes)
    mesh_handle_t load_mesh_from_vao(const std::string &_name, vertex_array_t _vao, void *_vertex_data=NULL);

    mesh_handle_t load_mesh_from_file(const std::string &_filename);
    
    mesh_internal_t *get_mesh(mesh_handle_t handle);
    mesh_handle_t get_handle_by_name(const std::string &name);

    void shutdown();
    
private:
    mesh_internal_t m_meshes[SYN_MAX_MESH_COUNT];
    size_t m_mesh_count = 0;
    std::unordered_map<std::string, mesh_handle_t> m_name_to_handle_map;
    
};


#endif // __MESH_LIBRARY_H
