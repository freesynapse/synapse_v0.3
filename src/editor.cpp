
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

    // delete selected entity
    if (key == SYN_KEY_DELETE && selected_entity_handle.is_valid()) {
        // only delete if the hierarchy or the viewport is selected
        window_handle_t focused = window_manager.get_focused_window();
        if (focused.id != window_manager.get_viewport_window_handle().id && 
            focused.id != window_manager.get_hierarchy_window_handle().id) return;
        
        if (selected_entity_handle.is_valid()) {
            entity_t *e = entity_lib.get_entity(selected_entity_handle);
            if (e) {
                mat_lib.release_material(e->material_handle);
                entity_lib.release_entity(selected_entity_handle);
            }
        }
        selected_entity_handle = { 0 };
    }

    // duplication of selected entity
    if (key == SYN_KEY_D && (mods & SYN_MOD_CTRL) && action == SYN_KEY_PRESSED) {
        if (selected_entity_handle.is_valid()) {
            entity_t *src = entity_lib.get_entity(selected_entity_handle);
            if (src) {
                material_handle_t new_mat = mat_lib.create_material_from(src->material_handle);
                std::string unique_name = entity_lib.generate_unique_name(src->name);

                entity_handle_t new_handle = entity_lib.create_entity(unique_name, src->mesh_handle, new_mat, src->transform);
                selected_entity_handle = new_handle;
            }
        }
    }

    // change ui transform mode
    if (selected_entity_handle.is_valid() && m_grabbed_ui_transform_axis == ui_transform_axis_t::NONE) {
        if (key == SYN_KEY_G) m_ui_transform_mode = ui_transform_mode_t::TRANSLATE;
        if (key == SYN_KEY_R) m_ui_transform_mode = ui_transform_mode_t::ROTATE;
        if (key == SYN_KEY_S) m_ui_transform_mode = ui_transform_mode_t::SCALE;
    }

    // focus camera on entity
    if (key == SYN_KEY_F && action == SYN_KEY_PRESSED) {
        if (selected_entity_handle.is_valid()) {
            entity_t *e = entity_lib.get_entity(selected_entity_handle);
            if (e) {
                mesh_internal_t *mesh = mesh_lib.get_mesh(e->mesh_handle);
                float radius = 1.0f;
                if (mesh) {
                    glm::vec3 extent = (mesh->aabb_max - mesh->aabb_min) * 0.5f;
                    radius = glm::length(extent * e->t_scale);
                }
                float target_distance = radius * 5.5f;
                cam.focus_on(e->t_position, target_distance);
            }
        }
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
void editor_t::open_texture_select()
{
    window_t *pw = window_manager.get_window(m_texture_select_window_handle);
    if (!pw) return;

    widget_t *lw = pw->get_widget_of_type(widget_type_t::LIST);
    if (!lw) return;

    lw->list_widget.items.clear();
    lw->list_widget.selected_index = -1;
    lw->list_widget.items.push_back("(no texture)");

    for (uint32_t i = 0; i < SYN_MAX_TEXTURE_COUNT; i++) {
        texture_internal_t &tex = tex_lib.m_pool[i];
        if (tex.is_active && !tex.name.empty()) {
            lw->list_widget.items.push_back(tex.name);
        }
    }

    pw->set_visible(true);
    window_manager.set_focused_window(m_texture_select_window_handle);
    
}

// 
void editor_t::assign_texture_to_selected(const std::string &_name)
{
    if (!selected_entity_handle.is_valid()) {
        SYN_WARNING("trying to assign texture '%s' but no entity selected.\n", _name.c_str());
        return;
    }

    entity_t *e = entity_lib.get_entity(selected_entity_handle);
    if (!e) return;

    material_internal_t *mat = mat_lib.get_material(e->material_handle);
    if (!mat) return;

    // 
    if (_name == "(no texture)") {
        mat->textures[(uint32_t)texture_map_type_t::ALBEDO] = { 0 };
        material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;
        pbr->use_albedo_map = 0.0f;
    } else {
        for (uint32_t i = 0; i < SYN_MAX_TEXTURE_COUNT; i++) {
            texture_internal_t &tex = tex_lib.m_pool[i];
            if (tex.is_active && tex.name == _name) {
                mat->textures[(uint32_t)texture_map_type_t::ALBEDO] = { i };
                material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;
                pbr->use_albedo_map = 1.0f;
                break;
            }
        }
    }

    // close after select
    window_t *pw = window_manager.get_window(m_texture_select_window_handle);
    pw->set_visible(false);
    
}

// 
void editor_t::set_texture_select_preview(const std::string &_name)
{
    window_t *pw = window_manager.get_window(m_texture_select_window_handle);
    if (!pw) return;
    widget_t *preview = pw->get_widget_of_type(widget_type_t::TEX_QUAD);
    if (!preview) return;

    if (_name == "(no texture)") {
        preview->tex_quad_widget.texture_handle = { 0 };
        return;
    }
    
    for (uint32_t i = 0; i < SYN_MAX_TEXTURE_COUNT; i++) {
        texture_internal_t &tex = tex_lib.m_pool[i];
        if (tex.is_active && tex.name == _name) {
            preview->tex_quad_widget.texture_handle = { i };
            return;
        }
    }
    preview->tex_quad_widget.texture_handle = { 0 };
    
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

    // 
    std::string unique_name = entity_lib.generate_unique_name(name);

    glm::mat4 transform = glm::mat4(1.0f);
    entity_handle_t handle = entity_lib.create_entity(unique_name, mesh, mat_lib.fallback_material_handle, transform);
    selected_entity_handle = handle;

    // close menu window
    window_t *pw = window_manager.get_window(m_create_window_handle);
    if (pw) pw->set_visible(false);
    
    SYN_INFO("created primitive '%s'.\n", unique_name.c_str());
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

// 
ui_transform_axis_t editor_t::pick_ui_transform_axis(const glm::vec2 &_screen_pos)
{
    if (!selected_entity_handle.is_valid()) return ui_transform_axis_t::NONE;

    entity_t *e = entity_lib.get_entity(selected_entity_handle);
    if (!e) return ui_transform_axis_t::NONE;

    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return ui_transform_axis_t::NONE;

    glm::vec2 vp_pos = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();

    glm::mat4 view = cam.get_view_matrix();
    glm::mat4 proj = cam.get_projection_matrix();

    float dist = glm::length(cam.get_position() - e->t_position);
    float axis_length = dist * 0.15f;

    glm::vec3 axes[3];
    if (editor.get_ui_transform_mode() == ui_transform_mode_t::ROTATE) {        
        axes[0] = glm::normalize(glm::vec3(e->transform[0]));
        axes[1] = glm::normalize(glm::vec3(e->transform[1]));
        axes[2] = glm::normalize(glm::vec3(e->transform[2]));
    } else {
        axes[0] = { 1.0f, 0.0f, 0.0f };
        axes[1] = { 0.0f, 1.0f, 0.0f };
        axes[2] = { 0.0f, 0.0f, 1.0f };
    }

    ui_transform_axis_t best_axis = ui_transform_axis_t::NONE;
    float best_dist = 20.0f;    // pixel hit threshold

    glm::vec2 origin_screen = world_to_screen_ui(e->t_position, vp_pos, vp_size, view, proj);

    if (glm::length(_screen_pos - origin_screen) < 15.0f) return best_axis;

    for (int i = 0; i < 3; i++) {
        glm::vec3 tip_world = e->t_position + axes[i] * axis_length;
        glm::vec tip_screen = world_to_screen_ui(tip_world, vp_pos, vp_size, view, proj);

        // point-to-segment distance in screen space
        glm::vec2 seg = tip_screen - origin_screen;
        float seg_len2 = glm::dot(seg, seg);
        float t = seg_len2 > 0.0001f ? glm::clamp(glm::dot(_screen_pos - origin_screen, seg) / seg_len2, 0.0f, 1.0f) : 0.0f;
        glm::vec2 closest = origin_screen + seg * t;
        float d = glm::length(_screen_pos - closest);

        if (d < best_dist) {
            best_dist = d;
            best_axis = (ui_transform_axis_t)(i + 1);   // NONE=0, X=1, Y=2, Z=3
        }
    }

    return best_axis;
    
}

// 
void editor_t::begin_ui_transform_drag()
{
    entity_t *e = entity_lib.get_entity(selected_entity_handle);
    if (!e) return;

    m_grabbed_ui_transform_axis = m_hovered_ui_transform_axis;
    m_drag_start_world = e->t_position;
    m_drag_start_screen = input.mouse_position;

    if (m_ui_transform_mode == ui_transform_mode_t::TRANSLATE) return;

    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return;
    glm::vec2 vp_pos  = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();
    m_drag_origin_ss = world_to_screen_ui(e->t_position,
                                       vp_pos,
                                       vp_size,
                                       cam.get_view_matrix(),
                                       cam.get_projection_matrix());
    m_drag_start_vec = input.mouse_position - m_drag_origin_ss;

    // 
    if (m_ui_transform_mode == ui_transform_mode_t::ROTATE) {
        // Extract rotation from the actual transform matrix, bypassing t_rotation
        glm::mat3 rot_mat = glm::mat3(e->transform);
        // Remove scale
        rot_mat[0] = glm::normalize(rot_mat[0]);
        rot_mat[1] = glm::normalize(rot_mat[1]);
        rot_mat[2] = glm::normalize(rot_mat[2]);
        m_drag_start_quat = glm::quat_cast(rot_mat);
        
        // m_drag_start_quat = glm::quat(glm::radians(e->t_rotation));
        m_drag_start_quat = glm::quat_cast(rot_mat);
    }

    // 
    if (m_ui_transform_mode == ui_transform_mode_t::SCALE) {
        m_drag_start_scale = e->t_scale;
        
    }
    
}

// 
void editor_t::update_ui_transform_drag(glm::vec2 &_screen_pos)
{
    entity_t *e = entity_lib.get_entity(selected_entity_handle);
    if (!e) return;

    window_t *vp = window_manager.get_viewport_window();
    if (!vp) return;
    glm::vec2 vp_pos  = vp->get_content_position();
    glm::vec2 vp_size = vp->get_content_size();

    // 
    if (m_ui_transform_mode == ui_transform_mode_t::TRANSLATE) {
        glm::vec3 axes[3] = {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
        };
        int axis_idx = (int)m_grabbed_ui_transform_axis - 1;
        glm::vec3 axis = axes[axis_idx];
    
        // project axis endpoints into screen space to get screen-space axis direction
        glm::mat4 view = cam.get_view_matrix();
        glm::mat4 proj = cam.get_projection_matrix();
        glm::vec2 origin_ss = world_to_screen_ui(m_drag_start_world, vp_pos, vp_size, view, proj);
        glm::vec2 tip_ss    = world_to_screen_ui(m_drag_start_world + axis, vp_pos, vp_size, view, proj);
        glm::vec2 axis_ss   = tip_ss - origin_ss;
    
        float axis_ss_len = glm::length(axis_ss);
        if (axis_ss_len < 0.0001f) return;   // axis pointing directly at camera
        glm::vec2 axis_ss_dir = axis_ss / axis_ss_len;
    
        // mouse delta from drag start
        glm::vec2 mouse_delta = _screen_pos - m_drag_start_screen;
    
        // project mouse delta onto screen-space axis direction
        float screen_delta = glm::dot(mouse_delta, axis_ss_dir);
    
        // scale: how many world units per pixel?
        // axis_ss_len is pixels per world unit at the entity's distance, so:
        float world_delta = screen_delta / axis_ss_len;
    
        e->t_position = m_drag_start_world + axis * world_delta;
        e->transform  = entity_t::make_transform(e->t_position, e->t_rotation, e->t_scale);
    }

    // 
    else if (m_ui_transform_mode == ui_transform_mode_t::ROTATE) {
        glm::vec2 current_vec = _screen_pos - m_drag_origin_ss;
        if (glm::length(m_drag_start_vec) < 0.0001f || glm::length(current_vec) < 0.0001f) return;

        float angle = glm::degrees(
            atan2f(m_drag_start_vec.x * current_vec.y - m_drag_start_vec.y * current_vec.x,
                   m_drag_start_vec.x * current_vec.x + m_drag_start_vec.y * current_vec.y)
        );

        int axis_idx = (int)m_grabbed_ui_transform_axis - 1;
        glm::vec3 world_axes[3] = {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f },
        };
        glm::vec3 local_axis = m_drag_start_quat * world_axes[axis_idx];
        // glm::vec3 local_axis = glm::normalize(glm::vec3(e->transform[axis_idx]));

        glm::quat delta = glm::angleAxis(glm::radians(angle), local_axis);
        glm::quat result = delta * m_drag_start_quat;

        // convert back to Euler angles -- causes instability so we only update the transform
        // here and e->t_rotation only on mouse button release
        // e->t_rotation = glm::degrees(glm::eulerAngles(result));
        // e->transform = entity_t::make_transform(e->t_position, e->t_rotation, e->t_scale);
        glm::mat4 rot = glm::mat4_cast(result);
        glm::mat4 scale = glm::scale(glm::mat4(1.0f), e->t_scale);
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), e->t_position);
        e->transform = trans * rot * scale;
    }

    // 
    else if (m_ui_transform_mode == ui_transform_mode_t::SCALE) {
        glm::vec2 current_vec = _screen_pos - m_drag_origin_ss;

        float start_len = glm::length(m_drag_start_vec);
        float current_len = glm::length(current_vec);
        if (start_len < 0.0001f) return;

        float scale_factor = current_len / start_len;
        if (input.is_key_down(SYN_KEY_LEFT_SHIFT)) {
            e->t_scale = glm::max(glm::vec3(0.0001f), m_drag_start_scale * scale_factor);    
        } else {
            int axis_idx = (int)m_grabbed_ui_transform_axis - 1;
            e->t_scale[axis_idx] = glm::max(0.0001f, m_drag_start_scale[axis_idx] * scale_factor);    
        }

        e->transform = entity_t::make_transform(e->t_position, e->t_rotation, e->t_scale);
        
    }
    
}