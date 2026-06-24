#ifndef __EDITOR_COMMAND_H
#define __EDITOR_COMMAND_H

#include <glm/glm.hpp>

#include "renderer/entity/entity_types.h"
#include "renderer/material/material_types.h"


// 
#define SYN_UNDO_STACK_SIZE 64

// 
enum class editor_command_type_t {
    TRANSFORM_CHANGE,
    MATERIAL_CHANGE,
    TEXTURE_CHANGE,
    ENTITY_CREATE,
    ENTITY_DELETE,
    ENTITY_DUPLICATE,
};

// 
struct editor_command_t {
    editor_command_type_t type;
    entity_handle_t entity_handle;

    // transform
    glm::vec3 prev_position;
    glm::vec3 prev_rotation;
    glm::vec3 prev_scale;
    glm::vec3 next_position;
    glm::vec3 next_rotation;
    glm::vec3 next_scale;

    // material
    material_pbr_payload_t prev_pbr;
    material_pbr_payload_t next_pbr;

    // texture
    texture_handle_t prev_texture;
    texture_handle_t next_texture;  
};

// 
struct undo_stack_t {
    editor_command_t commands[SYN_UNDO_STACK_SIZE];
    int top = -1;

    bool is_empty() { return top < 0; }
    
    // 
    void push(const editor_command_t &_cmd)
    {
        top = (top + 1) & SYN_UNDO_STACK_SIZE;
        commands[top] = _cmd;
    }

    // 
    editor_command_t *peek()
    {
        if (top < 0) return nullptr;
        return &commands[top];
    }

    // 
    editor_command_t *pop()
    {
        if (top < 0) return nullptr;
        editor_command_t *cmd = &commands[top];
        top--;
        return cmd;
    }
    
};



#endif // __EDITOR_COMMAND_H
