# Synapse Entity System — API Overview

## Core Concepts

Entities are the fundamental scene objects. Each entity has a mesh, a material, and a transform. All entities live in a flat pool managed by `entity_lib`, accessed via `entity_handle_t`.

---

## Creating Entities

**From a primitive type** (editor-spawned, per-entity material):
```cpp
entity_handle_t e = editor.create_entity_from_primitive(
    primitive_type_t::SPHERE_UV,
    "my_sphere",
    glm::vec3(0.0f, 1.0f, 0.0f));
```

**From a primitive with a manifest material:**
```cpp
entity_handle_t e = editor.create_entity_from_primitive(
    primitive_type_t::CUBE, "my_cube",
    glm::vec3(2.0f, 0.0f, 0.0f), "chrome");
```

**From a manifest asset:**
```cpp
entity_handle_t helmet = editor.create_entity_from_asset(
    "helmet",
    glm::vec3(0.0f, 1.0f, -2.0f), "helmet_mat");
```

**Low-level** (when you need full control):
```cpp
mesh_handle_t    mesh = mesh_generator.create_cube_mesh();
material_handle_t mat = mat_lib.create_material_from(mat_lib.fallback_material_handle);
entity_handle_t    e  = entity_lib.create_entity("my_entity", mesh, mat, glm::mat4(1.0f));
```

---

## Available Primitive Types

```cpp
primitive_type_t::CUBE
primitive_type_t::SPHERE_UV    // params: sectors=36, stacks=18
primitive_type_t::PLANE        // params: size=10, subdivisions=21
primitive_type_t::CONE         // params: radius=1, height=2, sectors=32
primitive_type_t::CYLINDER     // params: radius=1, height=2, sectors=32
primitive_type_t::TORUS        // params: outer=1, inner=0.3, sectors=36, sides=18
```

---

## Accessing and Modifying Entities

```cpp
entity_t *e = entity_lib.get_entity(handle);
if (e) {
    e->t_position = glm::vec3(1.0f, 0.0f, 0.0f);
    e->t_rotation = glm::vec3(0.0f, 45.0f, 0.0f);  // degrees
    e->t_scale    = glm::vec3(2.0f);
    e->transform  = entity_t::make_transform(e->t_position, e->t_rotation, e->t_scale);
}
```

Always call `make_transform()` after modifying `t_position`, `t_rotation`, or `t_scale` to keep the matrix in sync.

---

## Materials

Each editor-spawned entity gets its own material clone — modifying it doesn't affect other entities.

```cpp
material_internal_t *mat = mat_lib.get_material(e->material_handle);
material_pbr_payload_t *pbr = (material_pbr_payload_t *)mat->data;

pbr->albedo_color  = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
pbr->roughness     = 0.3f;
pbr->metallic      = 0.8f;
pbr->ao            = 1.0f;
pbr->tiling_factor = 1.0f;
```

**Assigning a texture:**
```cpp
texture_handle_t tex = assets.get_texture("my_texture");
mat->textures[(uint32_t)texture_map_type_t::ALBEDO] = tex;
pbr->use_albedo_map = 1.0f;
```

Mark the material dirty if it was cloned from a manifest material:
```cpp
e->material_is_dirty = true;
```

---

## Deleting and Duplicating

```cpp
// delete
mat_lib.release_material(e->material_handle);   // only if not a manifest material
entity_lib.release_entity(handle);

// duplicate
entity_handle_t copy = editor.create_primitive(e->mesh_primitive_type);
// then copy transform, material etc manually, or use Ctrl+D in the editor
```

---

## Selection

```cpp
// set selection
selected_entity_handle = handle;

// clear selection
selected_entity_handle = { 0 };

// check validity
if (selected_entity_handle.is_valid()) { ... }
```

---

## Scene Save / Load

```cpp
editor.save_scene("../assets/scene.syn");
editor.load_scene("../assets/scene.syn");
```

Triggered automatically via `Ctrl+S` / `Ctrl+O`. The scene file is human-readable plain text. Manifest materials that haven't been modified save as a name reference and restore perfectly. Modified or per-entity materials save the full PBR payload.

---

## Editor Shortcuts

| Key | Action |
|-----|--------|
| `Shift+A` | Open primitive creation menu |
| `DEL` | Delete selected entity |
| `Ctrl+D` | Duplicate selected entity |
| `F` | Focus camera on selected entity |
| `G` | Translate mode |
| `R` | Rotate mode |
| `S` | Scale mode |
| `Shift` (while scaling) | Uniform scale |
| `Ctrl+S` | Save scene |
| `Ctrl+O` | Load scene |
