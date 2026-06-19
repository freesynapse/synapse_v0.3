#ifndef __MESH_GENERATOR_H
#define __MESH_GENERATOR_H

#include "renderer/mesh/mesh_types.h"

//
class mesh_generator_t
{
public:
    mesh_handle_t create_cube_mesh();
    mesh_handle_t create_skybox_cube_mesh();
    mesh_handle_t create_uv_sphere_mesh(float _radius=1.0f, uint32_t _sectors=36, uint32_t _stacks=18);
    mesh_handle_t create_plane_mesh(float _size, uint32_t _subdivisions);
    mesh_handle_t create_cylinder_mesh(float _radius=1.0f, float _height=2.0f, uint32_t _sectors=32);
    mesh_handle_t create_cone_mesh(float _radius=1.0f, float _height=2.0f, uint32_t _sectors=32);
    mesh_handle_t create_torus_mesh(float _outer_radius=1.0f, float _inner_radius=0.3f, uint32_t _sectors=36, uint32_t _sides=18);

private:
    mesh_handle_t _create_cylinder_or_cone(const std::string &_name, float _bottom_radius, float _top_radius, float _height, uint32_t _sectors);

};



#endif // __MESH_GENERATOR_H
