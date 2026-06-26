#ifndef __SCATTER_PLOT_2D_H
#define __SCATTER_PLOT_2D_H

#include "renderer/shader/shader.h"
#include "mplc/canvas/canvas_2d.h"


// data update
void scatter_finalize_data(canvas_2d_t &_c);
void scatter_data(canvas_2d_t &_c, const std::vector<float> &_y);
void scatter_data(canvas_2d_t &_c, const std::vector<float> &_x, const std::vector<float> &_y);

// 
void scatter_redraw(canvas_2d_t &_c, const axes_t &_axes);
void scatter_render(const canvas_2d_t &_c, shader_t &_shader);


#endif // __SCATTER_PLOT_2D_H
