
#include "mplc/figure_types.h"

#include "mplc/figure.h"
#include "mplc/mplc_manager.h"

// 
figure_t &figure_handle_t::get() const
{
    figure_t *fig = mplc.get_figure(*this);
    SYN_ASSERT(fig != nullptr, "invalid figure_handle_t.\n");
    return *fig;
    
}

void figure_handle_t::scatter(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id, scatter_params_t _p)    { get().scatter(_x, _y, _id, _p); }
void figure_handle_t::scatter(const std::vector<float> &_y, const std::string &_id, scatter_params_t _p)                                  { get().scatter(_y, _id, _p); }
void figure_handle_t::lineplot(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id, lineplot_params_t _p) { get().lineplot(_x, _y, _id, _p); }
void figure_handle_t::lineplot(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id, lineplot_params_t _p)  { get().lineplot(_x, _y, _id, _p); }
void figure_handle_t::lineplot(const std::vector<float> &_y, const std::string &_id, lineplot_params_t _p)                                { get().lineplot(_y, _id, _p); }
void figure_handle_t::histogram(const std::vector<float> &_data, const std::string &_id, histogram_params_t _p)                           { get().histogram(_data, _id, _p); }

void figure_handle_t::data(const std::vector<float> &_y, const std::string &_id)                                                          { get().data(_y, _id); }
void figure_handle_t::data(const std::vector<float> &_x, const std::vector<float> &_y, const std::string &_id)                            { get().data(_x, _y, _id); }
void figure_handle_t::data(const std::vector<std::vector<float>> &_y, const std::string &_id)                                             { get().data(_y, _id); }
void figure_handle_t::data(const std::vector<std::vector<float>> &_x, const std::vector<std::vector<float>> &_y, const std::string &_id)  { get().data(_x, _y, _id); }

void figure_handle_t::render()                                          { get().render(); }
void figure_handle_t::draw(window_handle_t &_window)                    { get().draw(_window); }

void figure_handle_t::set_title(const std::string &_title)              { get().set_title(_title); }
void figure_handle_t::set_x_lim(const glm::vec2 &_lim)                  { get().set_x_lim(_lim); }
void figure_handle_t::set_y_lim(const glm::vec2 &_lim)                  { get().set_y_lim(_lim); }
void figure_handle_t::set_placement(figure_placement_t _p)              { get().set_placement(_p); }
void figure_handle_t::set_placement_offset(const glm::vec2 &_offset)    { get().set_placement_offset(_offset); }
void figure_handle_t::enable_grid()                                     { get().enable_grid(); }
void figure_handle_t::disable_grid()                                    { get().disable_grid(); }
void figure_handle_t::fill_x(const glm::vec2 &_lim)                     { get().fill_x(_lim); }
void figure_handle_t::fill_y(const glm::vec2 &_lim)                     { get().fill_y(_lim); }
void figure_handle_t::disable_fill_x()                                  { get().disable_fill_x(); }
void figure_handle_t::disable_fill_y()                                  { get().disable_fill_y(); }