#ifndef __MESH_GENERATOR_H
#define __MESH_GENERATOR_H

#include "renderer/mesh/mesh_types.h"

// 
mesh_handle_t generate_cube_mesh();
mesh_handle_t generate_skybox_cube_mesh();
mesh_handle_t generate_uv_sphere_mesh(float _radius=1.0f, uint32_t _sectors=36, uint32_t _stacks=18);
mesh_handle_t generate_plane_mesh(float _size, uint32_t _subdivisions);




#endif // __MESH_GENERATOR_H
