#ifndef __HISTOGRAM_2D_H
#define __HISTOGRAM_2D_H

#include "mplc/canvas/canvas_2d.h"


// 
void histogram_finalize_data(canvas_2d_t &_c);
void histogram_data(canvas_2d_t &_c, const std::vector<float> &_data);

void histogram_redraw(canvas_2d_t &_c, const axes_t &_axes);
void histogram_render(const canvas_2d_t &_c, shader_t &_shader);


#endif // __HISTOGRAM_2D_H