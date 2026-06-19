
#include "editor.h"
#include "utils/math_utils.h"
#include "utils/log.h"
#include "renderer/mesh/mesh_generator.h"

#include "c_api.h"

// 
static const char *primitive_type_to_str(primitive_type_t _type)
{
    switch (_type) {
        case primitive_type_t::CUBE:      return "CUBE";
        case primitive_type_t::SPHERE_UV: return "SPHERE_UV";
        case primitive_type_t::PLANE:     return "PLANE";
        case primitive_type_t::CONE:      return "CONE";
        case primitive_type_t::CYLINDER:  return "CYLINDER";
        case primitive_type_t::TORUS:     return "TORUS";
        default:                          return "NONE";
    }
}

// 
static void __editor_on_keydown_callback(const event_t &_e) { editor.on_keydown_event(_e); }


// 
void editor_t::init()
{
    events.register_callback(event_type_t::INPUT_KEYDOWN, __editor_on_keydown_callback);
    
}

// 
void editor_t::save_scene(const std::string &_path)
{
    FILE *f = fopen(_path.c_str(), "w");
    if (!f) {
        SYN_ERROR("could not open '%s' for writing.\n", _path.c_str());
        return;
    }

    fprintf(f, "# synapse scene file\n\n");

    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        entity_t &e = entity_lib.m_pool[i];
        if (!e.is_active) continue;

        fprintf(f, "entity %s\n", e.name.c_str());

        // mesh
        if (e.mesh_primitive_type == primitive_type_t::NONE) {
            // asset mesh -- save by mesh name
            entity_handle_t this_handle = { i + 1 };
            std::string asset_name = assets.get_entity_name(this_handle);
            fprintf(f, "    mesh_type   ASSET\n");
            fprintf(f, "    mesh_name   %s\n", asset_name.empty() ? "unknown" : asset_name.c_str());
        } else {
            fprintf(f, "    mesh_type   PRIMITIVE\n");
            fprintf(f, "    mesh_name   %s\n", primitive_type_to_str(e.mesh_primitive_type));
            if (e.mesh_param_count > 0) {
                fprintf(f, "    mesh_params");
                for (uint32_t p = 0; p < e.mesh_param_count; p++)
                    fprintf(f, " %g", e.mesh_params[p]);
                fprintf(f, "\n");
            }
        }

        // transform
        fprintf(f, "    position    %.4f %.4f %.4f\n", e.t_position.x, e.t_position.y, e.t_position.z);
        fprintf(f, "    rotation    %.4f %.4f %.4f\n", e.t_rotation.x, e.t_rotation.y, e.t_rotation.z);
        fprintf(f, "    scale       %.4f %.4f %.4f\n", e.t_scale.x,    e.t_scale.y,    e.t_scale.z);

        // material
        std::string mat_name = assets.get_material_name(e.material_handle);
        if (!mat_name.empty()) {
            fprintf(f, "    manifest_material  %s\n", mat_name.c_str());
        } else {
            material_internal_t *mat = mat_lib.get_material(e.material_handle);
            if (mat) {
                material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;
                fprintf(f, "    material\n");
                fprintf(f, "        albedo      %.4f %.4f %.4f %.4f\n",
                        pbr->albedo_color.r, pbr->albedo_color.g,
                        pbr->albedo_color.b, pbr->albedo_color.a);
                fprintf(f, "        roughness   %.4f\n", pbr->roughness);
                fprintf(f, "        metallic    %.4f\n", pbr->metallic);
                fprintf(f, "        ao          %.4f\n", pbr->ao);
                fprintf(f, "        tiling      %.4f\n", pbr->tiling_factor);
                texture_handle_t tex_h = mat->textures[(uint32_t)texture_map_type_t::ALBEDO];
                texture_internal_t *tex = tex_lib.get_texture(tex_h);
                if (pbr->use_albedo_map > 0.5f && tex && !tex->name.empty())
                    fprintf(f, "        albedo_tex  %s\n", tex->name.c_str());
                else
                    fprintf(f, "        albedo_tex  none\n");
                fprintf(f, "    end_material\n");
            }
        }
        
        fprintf(f, "end_entity\n\n");
    }

    fclose(f);
    SYN_INFO("scene saved to '%s'.\n", _path.c_str());
}

// 
void editor_t::load_scene(const std::string &_path)
{
    FILE *f = fopen(_path.c_str(), "r");
    if (!f) {
        SYN_ERROR("could not open '%s' for reading.\n", _path.c_str());
        return;
    }

    // clear existing scene
    for (uint32_t i = 0; i < SYN_MAX_ENTITY_COUNT; i++) {
        entity_t &e = entity_lib.m_pool[i];
        if (!e.is_active) continue;

        std::string mat_name = assets.get_material_name(e.material_handle);
        if (mat_name.empty())
            mat_lib.release_material(e.material_handle);
        entity_lib.release_entity({ i + 1 });
    }
    selected_entity_handle = { 0 };

    char line[512];

    // per-entity state
    std::string             ent_name;
    std::string             mesh_type_str;
    std::string             mesh_name_str;
    std::string             manifest_mat_name;
    float                   mesh_params[4]  = {};
    uint32_t                mesh_param_count = 0;
    glm::vec3               position        = {};
    glm::vec3               rotation        = {};
    glm::vec3               scale           = { 1.0f, 1.0f, 1.0f };
    material_pbr_payload_t  pbr             = {};
    std::string             albedo_tex_name;
    bool                    in_entity       = false;
    bool                    in_material     = false;

    auto reset = [&]() {
        ent_name.clear();
        mesh_type_str.clear();
        mesh_name_str.clear();
        manifest_mat_name.clear();
        albedo_tex_name.clear();
        memset(mesh_params, 0, sizeof(mesh_params));
        mesh_param_count = 0;
        position  = {};
        rotation  = {};
        scale     = { 1.0f, 1.0f, 1.0f };
        pbr       = {};
        in_entity   = false;
        in_material = false;
    };

    while (fgets(line, sizeof(line), f)) {
        // tokenize
        std::vector<std::string> tokens;
        char *tok = strtok(line, " \t\r\n");
        while (tok) { tokens.push_back(tok); tok = strtok(nullptr, " \t\r\n"); }
        if (tokens.empty() || tokens[0][0] == '#') continue;

        if (tokens[0] == "entity") {
            reset();
            ent_name  = tokens[1];
            in_entity = true;
        }
        else if (in_entity) {
            if      (tokens[0] == "mesh_type") mesh_type_str = tokens[1];
            else if (tokens[0] == "mesh_name") mesh_name_str = tokens[1];
            else if (tokens[0] == "mesh_params") {
                mesh_param_count = (uint32_t)(tokens.size() - 1);
                for (uint32_t p = 0; p < mesh_param_count; p++)
                    mesh_params[p] = std::stof(tokens[p + 1]);
            }
            else if (tokens[0] == "position")
                position = { std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
            else if (tokens[0] == "rotation")
                rotation = { std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
            else if (tokens[0] == "scale")
                scale    = { std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
            else if (tokens[0] == "manifest_material")
                manifest_mat_name = tokens[1];
            else if (tokens[0] == "material")
                in_material = true;
            else if (in_material) {
                if (tokens[0] == "albedo")
                    pbr.albedo_color = { std::stof(tokens[1]), std::stof(tokens[2]),
                                         std::stof(tokens[3]), std::stof(tokens[4]) };
                else if (tokens[0] == "roughness")   pbr.roughness     = std::stof(tokens[1]);
                else if (tokens[0] == "metallic")    pbr.metallic      = std::stof(tokens[1]);
                else if (tokens[0] == "ao")          pbr.ao            = std::stof(tokens[1]);
                else if (tokens[0] == "tiling")      pbr.tiling_factor = std::stof(tokens[1]);
                else if (tokens[0] == "albedo_tex")  albedo_tex_name   = tokens[1];
                else if (tokens[0] == "end_material") in_material = false;
            }
            else if (tokens[0] == "end_entity") {
                // --- resolve mesh ---
                mesh_handle_t    mesh      = { 0 };
                primitive_type_t prim_type = primitive_type_t::NONE;

                if (mesh_type_str == "ASSET") {
                    mesh = assets.get_entity_mesh(mesh_name_str);
                }
                else if (mesh_type_str == "PRIMITIVE") {
                    if (mesh_name_str == "CUBE") {
                        prim_type = primitive_type_t::CUBE;
                        mesh = mesh_generator.create_cube_mesh();
                    }
                    else if (mesh_name_str == "SPHERE_UV") {
                        prim_type = primitive_type_t::SPHERE_UV;
                        mesh = mesh_generator.create_uv_sphere_mesh(
                            1.0f, (uint32_t)mesh_params[0], (uint32_t)mesh_params[1]);
                    }
                    else if (mesh_name_str == "PLANE") {
                        prim_type = primitive_type_t::PLANE;
                        mesh = mesh_generator.create_plane_mesh(mesh_params[0], (uint32_t)mesh_params[1]);
                    }
                    else if (mesh_name_str == "CONE") {
                        prim_type = primitive_type_t::CONE;
                        mesh = mesh_generator.create_cone_mesh(
                            mesh_params[0], mesh_params[1], (uint32_t)mesh_params[2]);
                    }
                    else if (mesh_name_str == "CYLINDER") {
                        prim_type = primitive_type_t::CYLINDER;
                        mesh = mesh_generator.create_cylinder_mesh(
                            mesh_params[0], mesh_params[1], (uint32_t)mesh_params[2]);
                    }
                    else if (mesh_name_str == "TORUS") {
                        prim_type = primitive_type_t::TORUS;
                        mesh = mesh_generator.create_torus_mesh(
                            mesh_params[0], mesh_params[1],
                            (uint32_t)mesh_params[2], (uint32_t)mesh_params[3]);
                    }
                    else {
                        SYN_WARNING("unknown primitive type '%s'.\n", mesh_name_str.c_str());
                    }
                }

                if (!mesh.id) {
                    SYN_ERROR("could not resolve mesh for entity '%s', skipping.\n", ent_name.c_str());
                    reset();
                    continue;
                }

                // --- resolve material ---
                material_handle_t mat = { 0 };

                if (!manifest_mat_name.empty()) {
                    mat = assets.get_material(manifest_mat_name);
                } else {
                    mat = mat_lib.create_material_from(mat_lib.fallback_material_handle);
                    material_internal_t *mat_ptr = mat_lib.get_material(mat);
                    if (mat_ptr) {
                        memcpy(mat_ptr->data, &pbr, sizeof(material_pbr_payload_t));
                        mat_ptr->data_size = sizeof(material_pbr_payload_t);

                        if (albedo_tex_name != "none" && !albedo_tex_name.empty()) {
                            texture_handle_t tex = assets.get_texture(albedo_tex_name);
                            if (tex.id != 0) {
                                mat_ptr->textures[(uint32_t)texture_map_type_t::ALBEDO] = tex;
                                material_pbr_payload_t *pbr_ptr = (material_pbr_payload_t *)mat_ptr->data;
                                pbr_ptr->use_albedo_map = 1.0f;
                            }
                        }
                    }
                }
                

                // --- create entity ---
                glm::mat4 transform = entity_t::make_transform(position, rotation, scale);
                entity_handle_t handle = entity_lib.create_entity(ent_name, mesh, mat, transform);
                entity_t *ent = entity_lib.get_entity(handle);
                if (ent) {
                    ent->t_position            = position;
                    ent->t_rotation            = rotation;
                    ent->t_scale               = scale;
                    ent->mesh_primitive_type   = prim_type;
                    ent->mesh_param_count      = mesh_param_count;
                    memcpy(ent->mesh_params, mesh_params, sizeof(mesh_params));
                    ent->manifest_material_name = manifest_mat_name;
                }

                SYN_INFO("loaded entity '%s'.\n", ent_name.c_str());
                reset();
            }
        }
    }

    fclose(f);
    SYN_INFO("scene loaded from '%s'.\n", _path.c_str());
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
                std::string mat_name = assets.get_material_name(e->material_handle);
                if (mat_name.empty())
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

    // save/load scene
    if (key == SYN_KEY_S && (mods & SYN_MOD_CTRL) && action == SYN_KEY_PRESSED) {
        save_scene("../assets/scene.syn");
    }

    if (key == SYN_KEY_O && (mods & SYN_MOD_CTRL) && action == SYN_KEY_PRESSED) {
        load_scene("../assets/scene.syn");
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

    // store mesh parameters
    entity_t *e = entity_lib.get_entity(handle);
    if (e) {
        e->mesh_primitive_type = _type;
        switch (_type) {
            case primitive_type_t::CUBE: {
                e->mesh_param_count = 0;
                break;
            }
    
            case primitive_type_t::SPHERE_UV: {
                e->mesh_params[0] = 36.0f;
                e->mesh_params[1] = 18.0f;
                e->mesh_param_count = 2;
                break;
            }
    
            case primitive_type_t::PLANE: {
                e->mesh_params[0] = 10.0;
                e->mesh_params[1] = 21.0f;
                e->mesh_param_count = 2;
                break;
            }
    
            case primitive_type_t::CONE:
            case primitive_type_t::CYLINDER: {
                e->mesh_params[0] = 1.0f;
                e->mesh_params[1] = 2.0f;
                e->mesh_params[2] = 32.0f;
                e->mesh_param_count = 3;
                break;
            }
            
            case primitive_type_t::TORUS: {
                e->mesh_params[0] = 1.0f;
                e->mesh_params[1] = 0.3f;
                e->mesh_params[2] = 36.0f;
                e->mesh_params[3] = 18.0f;
                e->mesh_param_count = 4;
                break;
            }

            default: break;
            
        }
    }

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