#ifndef __LINE_PLOT_2D_H
#define __LINE_PLOT_2D_H

#include "mplc/canvas/canvas_2d.h"


// 
void lineplot_finalize_data(canvas_2d_t &_c);
void lineplot_data(canvas_2d_t &_c, const std::vector<float> &_y);
void lineplot_data(canvas_2d_t &_c, const std::vector<float> &_x,
                                    const std::vector<float> &_y);
void lineplot_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_y);
void lineplot_data(canvas_2d_t &_c, const std::vector<std::vector<float>> &_x,
                                    const std::vector<std::vector<float>> &_y);

// 
void lineplot_redraw(canvas_2d_t &_c, const axes_t &_axes);
void lineplot_render(const canvas_2d_t &_c, shader_t &_shader);


#endif // __LINE_PLOT_2D_H
