#ifndef __MPLC_MANAGER_H
#define __MPLC_MANAGER_H

#include "mplc/figure.h"
#include "mplc/figure_types.h"


// 
#define SYN_MPLC_MAX_FIGURES 32

// 
class mplc_manager_t
{
public:
    friend class figure_handle_t;

public:
    mplc_manager_t() = default;
    ~mplc_manager_t() = default;

    void init();
    void shutdown();

    figure_handle_t create_figure(const glm::vec2 &_size_px);
    void release_figure(figure_handle_t _handle);
    figure_t *get_figure(const figure_handle_t &_handle);

private:
    figure_t m_pool[SYN_MPLC_MAX_FIGURES];
    
};

// 
extern mplc_manager_t mplc;


#endif // __MPLC_MANAGER_H
