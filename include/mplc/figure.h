#ifndef __FIGURE_H
#define __FIGURE_H

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "renderer/buffers/framebuffer.h"
#include "renderer/buffers/vertex_array.h"
#include "renderer/font/font.h"
#include "renderer/shader/shader_types.h"
#include "renderer/UI/window/window_types.h"

#include "mplc/figure_params.h"
#include "mplc/figure_types.h"
#include "mplc/figure_utils.h"
#include "mplc/canvas/canvas_2d.h"
#include "mplc/canvas/canvas_params.h"


// redraw / aux flags
#define FIGURE_REDRAW_DATA          0x01
#define FIGURE_REDRAW_AXES          0x02
#define FIGURE_REDRAW_TICKS         0x04
#define FIGURE_REDRAW_TICK_LABELS   0x08
#define FIGURE_REDRAW_FILL          0x10
#define FIGURE_REDRAW_ALL           0x1f

#define FIGURE_AUX_GRIDLINES        0x01
#define FIGURE_AUX_FILL_X           0x02
#define FIGURE_AUX_FILL_Y           0x04

// 
class figure_t 
{
public:
    friend class mplc_manager_t;

public:
    figure_t() = default;
    ~figure_t() { destroy(); }


    //-------------------------------------------------------------------------
    // canvas creators
    // 
    void scatter(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="", scatter_params_t _params={});
    void scatter(const std::vector<float> &_y, const std::string &_id="", scatter_params_t _params={});
    void lineplot(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id="", lineplot_params_t _params={});
    void lineplot(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="", lineplot_params_t _params={});
    void lineplot(const std::vector<float> &_y, const std::string &_id="", lineplot_params_t _params={});
    void histogram(const std::vector<float> &_data, const std::string &_id="", histogram_params_t _params={});


    //-------------------------------------------------------------------------
    // data update -- replaces data on an existing canvas
    // 
    void data(const std::vector<float> &_y, const std::string &_id="");
    void data(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id="");
    void data(const std::vector<std::vector<float>> &_y, const std::string &_id="");
    void data(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id="");    


    //-------------------------------------------------------------------------
    // render
    // 

    // rebuild dirty GPU geometry, render into FBO. Call each frame inside syn_im_begin/end
    void render();

    // blit figure FBO into a synapse window
    void draw(window_handle_t &_window);


    //-------------------------------------------------------------------------
    // accessors
    // 
    void set_title(const std::string &_title) { m_title = _title; m_redraw_flags |= FIGURE_REDRAW_ALL; }
    void set_x_lim(const glm::vec2 &_lim);
    void set_y_lim(const glm::vec2 &_lim);
    void set_placement(figure_placement_t _p) { m_placement = _p; }
    void set_placement_offset(const glm::vec2 &_offset) { m_placement_offset = _offset; }
    void enable_grid()  { m_aux_flags |=  FIGURE_AUX_GRIDLINES; m_redraw_flags |= FIGURE_REDRAW_ALL; }
    void disable_grid() { m_aux_flags &= ~FIGURE_AUX_GRIDLINES; m_redraw_flags |= FIGURE_REDRAW_ALL; }
    void fill_x(const glm::vec2 &_lim);
    void fill_y(const glm::vec2 &_lim);
    void disable_fill_x();
    void disable_fill_y();

    canvas_2d_t *find_canvas(const std::string &_id);
    const glm::vec2 &size_px() const { return m_size_px; }
    const std::string &title() const { return m_title;   }
    bool is_valid() const { return m_initialized; }

    
    //-------------------------------------------------------------------------
    // private members
    // 
private:
    void init(const glm::vec2 &_size_px,const std::string &_fbo_id);
    void destroy();

    void add_canvas(canvas_2d_t &_canvas);
    void update_data_limits();

    void redraw_axes(const normalized_params_t &_p);
    void redraw_ticks(const normalized_params_t &_p);
    void format_tick_labels(nice_scale_t &_ticks, const std::vector<glm::vec2> &_positions);
    void redraw_tick_labels(const normalized_params_t &_p);
    void redraw_fill(const normalized_params_t &_p);

private:
    bool                    m_initialized           = false;
    std::string             m_title;
    glm::vec2               m_size_px               = { 0.0f, 0.0f };

    figure_params_t         m_params;
    axes_t                  m_axes;

    std::vector<std::unique_ptr<canvas_2d_t>> m_canvases;

    figure_placement_t      m_placement             = figure_placement_t::TOP_LEFT;
    glm::vec2               m_placement_offset      = { 0.0f, 0.0f };
    
    glm::vec2               m_data_lim_x            = { 0.0f, 1.0f };
    glm::vec2               m_data_lim_y            = { 0.0f, 1.0f };
    glm::vec2               m_data_lim_x_prev       = { 0.0f, 1.0f };
    glm::vec2               m_data_lim_y_prev       = { 0.0f, 1.0f };

    vertex_array_t          m_vao_axes;
    vertex_array_t          m_vao_ticks;
    vertex_array_t          m_vao_grid;
    vertex_array_t          m_vao_fill;
    vertex_array_t          m_vao_canvas_clear_rect;

    std::vector<glm::vec2>  m_tick_label_pos_x;
    std::vector<glm::vec2>  m_tick_label_pos_y;

    font_t                 *m_font_tick_label       = nullptr;
    font_t                 *m_font_title            = nullptr;

    framebuffer_handle_t    m_fbo_handle;
    shader_handle_t         m_geom_shader_handle    = { 0 };
    
    uint32_t                m_redraw_flags          = FIGURE_REDRAW_ALL;
    uint32_t                m_aux_flags             = FIGURE_AUX_GRIDLINES;

    glm::vec2               m_fill_lim_x            = { 1.0f, -1.0f };
    glm::vec2               m_fill_lim_y            = { 1.0f, -1.0f };
    
};


#endif // __FIGURE_H
