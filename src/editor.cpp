
#include "editor.h"
#include "utils/math_utils.h"
#include "utils/log.h"

#include "c_api.h"

// 
static void __editor_on_keydown_callback(const event_t &_e) { editor.on_keydown_event(_e); }


// 
void editor_t::init()
{
    events.register_callback(event_type_t::INPUT_KEYDOWN, __editor_on_keydown_callback);
    
}

// 
void editor_t::on_keydown_event(const event_t &_e)
{
    int action = _e.as.keydown.action;
    if (action != SYN_KEY_PRESSED && action != SYN_KEY_REPEAT) return;

    int key = _e.as.keydown.key;
    int mods = _e.as.keydown.mods;

    // 
    if (key == SYN_KEY_A && mods & SYN_MOD_SHIFT) {
        toggle_create_menu(input.mouse_position);
    }

    // 
    if (key == SYN_KEY_ESCAPE) {
        if (window_manager.get_focused_window().id == m_create_window_handle.id) {
            hide_create_menu();
        }
    }

    // 
    if (key == SYN_KEY_DELETE && selected_entity_handle.is_valid()) {
        entity_lib.release_entity(selected_entity_handle);
        selected_entity_handle = { 0 };
    }
    
}

// 
void editor_t::toggle_create_menu(const glm::vec2 &_pos)
{
    window_t *pw = window_manager.get_window(m_create_window_handle);
    if (!pw) return;
    pw->set_visible(!pw->is_visible());
    if (pw->is_visible()) {
        glm::vec2 offset = pw->size * 0.5f;
        glm::vec2 pos = glm::vec2(
            glm::min(_pos.x - offset.x, root_window.get_fwidth() - pw->size.x),
            glm::min(_pos.y - offset.y, root_window.get_fheight() - pw->size.y)
        );
        pw->position = pos;
        window_manager.set_focused_window(m_create_window_handle);
    }
}

// 
void editor_t::hide_create_menu()
{
    window_t *pw = window_manager.get_window(m_create_window_handle);
    if (!pw) return;
    pw->set_visible(false);
}

// 
entity_handle_t editor_t::create_primitive(primitive_type_t _type)
{
    mesh_handle_t mesh;
    std::string name;

    switch (_type) {
        case primitive_type_t::CUBE: {
            mesh = mesh_generator.create_cube_mesh();
            name = "cube";
            break;
        }

        case primitive_type_t::SPHERE_UV: {
            mesh = mesh_generator.create_uv_sphere_mesh();
            name = "sphere";
            break;
        }

        case primitive_type_t::PLANE: {
            mesh = mesh_generator.create_plane_mesh(10.0f, 21);
            name = "plane";
            break;
        }

        case primitive_type_t::CONE: {
            mesh = mesh_generator.create_cone_mesh();
            name = "cone";
            break;
        }
        
        case primitive_type_t::CYLINDER: {
            mesh = mesh_generator.create_cylinder_mesh();
            name = "cylinder";
            break;
        }
        
        case primitive_type_t::TORUS: {
            SYN_INFO("hello.\n");
            mesh = mesh_generator.create_torus_mesh();
            name = "torus";
            break;
        }

        default:
            SYN_WARNING("unknown primitive type %d.\n", (int)_type);
            return { 0 };
    }

    // generate unique name (sphere, spehere.001, sphere.002 etc)
    std::string uname = name;
    uint32_t suffix = 1;
    while (true) {
        bool taken = false;
        for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
            if (entity_lib.m_pool[i].is_active && entity_lib.m_pool[i].name == uname) {
                taken = true;
                break;
            }
        }
        if (!taken) break;
        char buf[32];
        snprintf(buf, sizeof(buf), ".%03d", suffix++);
        uname = name + buf;
    }

    glm::mat4 transform = glm::mat4(1.0f);
    entity_handle_t handle = entity_lib.create_entity(uname, mesh, mat_lib.fallback_material_handle, transform);
    selected_entity_handle = handle;

    // close menu window
    window_t *pw = window_manager.get_window(m_create_window_handle);
    if (pw) pw->set_visible(false);
    
    SYN_INFO("created primitive '%s'.\n", uname.c_str());
    return handle;
    
}

// 
entity_handle_t editor_t::pick_entity(const glm::vec2 &_screen_pos)
{
    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return { 0 };

    glm::vec2 vp_pos  = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();

    if (_screen_pos.x < vp_pos.x || _screen_pos.x > vp_pos.x + vp_size.x ||
        _screen_pos.y < vp_pos.y || _screen_pos.y > vp_pos.y + vp_size.y)
        return { 0 };

    ray_t ray = ray_from_screen(_screen_pos, vp_pos, vp_size, cam.get_view_matrix(), cam.get_projection_matrix());
    entity_handle_t closest_entity = { 0 };
    float closest_t = FLT_MAX;

    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        entity_t *e = entity_lib.get_entity_from_index(i);
        if (!e->is_active) continue;

        mesh_internal_t *mesh = mesh_lib.get_mesh(e->mesh_handle);
        if (!mesh) continue;

        glm::mat4 inv = glm::inverse(e->transform);
        ray_t local_ray;
        local_ray.origin    = glm::vec3(inv * glm::vec4(ray.origin, 1.0f));
        local_ray.direction = glm::normalize(glm::vec3(inv * glm::vec4(ray.direction, 0.0f)));

        float t;
        if (ray_aabb_intersect(local_ray, mesh->aabb_min, mesh->aabb_max, t)) {
            if (t < closest_t) {
                closest_t = t;
                closest_entity = { i + 1 };
            }
        }
    }

    return closest_entity;
}
