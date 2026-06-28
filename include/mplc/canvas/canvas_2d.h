#ifndef __CANVAS_2D_H
#define __CANVAS_2D_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "external/glad/glad.h"

#include "renderer/buffers/vertex_array.h"
#include "renderer/shader/shader.h"

#include "mplc/figure_params.h"
#include "mplc/figure_utils.h"


// forward decls
class figure_t;

// canvas type tag
enum class canvas_type_t {
    SCATTER,
    LINEPLOT,
    HISTOGRAM,
};

// type-specific data structs
// 
struct scatter_data_t {
    std::vector<float> x;
    std::vector<float> y;
};

// 
struct lineplot_data_t {
    std::vector<std::vector<float>> x;
    std::vector<std::vector<float>> y;
    int             row_count = 0;
    vertex_array_t  vao_markers;
    uint32_t        marker_vcount = 0;
    
};

// 
struct histogram_data_t {
    std::vector<float>      data;
    int                     bin_count = -1;
    float                   bins_dx = 0.0f;
    std::map<float, size_t> bins;
};


//---------------------------------------------------------------------------
// canvas_2d_t
// 
struct canvas_2d_t {

    canvas_type_t   type;
    std::string     id;
    figure_params_t params; // per canvas copy
    
    // GPU resources
    vertex_array_t  vao_data;
    GLenum          gl_primitive = GL_TRIANGLES;

    // data limits (in data space)
    glm::vec2       data_lim_x = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };
    glm::vec2       data_lim_y = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };


    // type-specific payload
    scatter_data_t   scatter;
    lineplot_data_t  lineplot;
    histogram_data_t histogram;
    
    // 
    canvas_2d_t() {}
    ~canvas_2d_t() {}

    canvas_2d_t(canvas_2d_t &&_o) = default;
    canvas_2d_t &operator=(canvas_2d_t &&_o) = default;
    canvas_2d_t(const canvas_2d_t &) = delete;
    canvas_2d_t &operator=(const canvas_2d_t &) = delete;
    
};

//---------------------------------------------------------------------------
// dispatch API
void canvas_finalize_data(canvas_2d_t &_c);
void canvas_data(canvas_2d_t &_c, const std::vector<float> &_y);
void canvas_data(canvas_2d_t &_c, const std::vector<float> &_x,
                                  const std::vector<float> &_y);
void canvas_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_y);
void canvas_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_x,
                                  const std::vector<std::vector<float>> &_y);

void canvas_redraw(canvas_2d_t &_c, const axes_t &_axes);
void canvas_render(const canvas_2d_t &_c, shader_t &_shader);
void canvas_destroy(canvas_2d_t &_c);

std::string canvas_resolve_id(const std::vector<std::unique_ptr<canvas_2d_t>> &_canvases,
                              const std::string &_id);


#endif // __CANVAS_2D_H

