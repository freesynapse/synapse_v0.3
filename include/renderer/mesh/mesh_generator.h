#ifndef __MESH_GENERATOR_H
#define __MESH_GENERATOR_H

#include "renderer/mesh/mesh_types.h"

// 
mesh_handle_t generate_cube_mesh();
mesh_handle_t generate_skybox_cube();
mesh_handle_t generate_uv_sphere(float _radius, uint32_t _sectors, uint32_t _stacks);





#endif // __MESH_GENERATOR_H
