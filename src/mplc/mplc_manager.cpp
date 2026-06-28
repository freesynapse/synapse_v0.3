
#include "mplc/mplc_manager.h"


// global instance
mplc_manager_t mplc;

// 
void mplc_manager_t::init()
{
    
}

// 
void mplc_manager_t::shutdown()
{
    for (size_t i = 0; i < SYN_MPLC_MAX_FIGURES; i++) {
        if (m_pool[i].is_valid()) m_pool[i].destroy();
    }
    
}

// 
figure_handle_t mplc_manager_t::create_figure(const glm::vec2 &_size_px)
{
    for (uint32_t i = 0; i < SYN_MPLC_MAX_FIGURES; i++) {
        if (!m_pool[i].is_valid()) {
            std::string fbo_id = "mplc_figure_" + std::to_string(i);
            m_pool[i].init(_size_px, fbo_id);
            return { i + 1 };
        }
    }

    SYN_WARNING("figure pool exhausted.\n");
    return { 0 };
    
}

// 
void mplc_manager_t::release_figure(figure_handle_t _handle)
{
    figure_t *fig = get_figure(_handle);
    if (!fig) return;
    fig->destroy();
    _handle.id = 0;
}

// 
figure_t *mplc_manager_t::get_figure(const figure_handle_t &_handle)
{
    if (!_handle.is_valid()) return nullptr;
    size_t idx = _handle.id - 1;
    if (idx >= SYN_MPLC_MAX_FIGURES) return nullptr;
    if (!m_pool[idx].is_valid()) return nullptr;
    return &m_pool[idx];
    
}
