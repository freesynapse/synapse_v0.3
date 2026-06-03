
#include <GLFW/glfw3.h>
#include <math.h>
#include <time.h>

#include "utils/time_step.h"

// 
void time_step_t::update()
{
    double current_time_s = glfwGetTime();
    dt = current_time_s - end_of_last_frame_s;
    double limiter_start_s = current_time_s; 
    
    // hybrid sleep and burn
    if (fps_limit > 0.0f) {
        double s_per_frame = 1.0 / (double)fps_limit;
        double remaining_time_s = s_per_frame - dt;
        // sleep, reserving 1.5 ms for later burn
        double sleep_padding_s = 0.0015;
        if (remaining_time_s > sleep_padding_s) {
            double safe_sleep_s = remaining_time_s - sleep_padding_s;
            struct timespec req = { 0 };
            req.tv_sec = (time_t)safe_sleep_s;
            req.tv_nsec = (long)((safe_sleep_s - req.tv_sec) * 1000000000L);
    
            nanosleep(&req, nullptr);
        }
        // burn the remaining time
        while ((glfwGetTime() - end_of_last_frame_s) < s_per_frame) {}
    }
    
    // time for end of this frame, accounting for sleeping
    end_of_last_frame_s = glfwGetTime();
    // adjust dt, accounting for the actual duration of sleep (limiter_start_s + dt)
    dt = end_of_last_frame_s - limiter_start_s + dt;
    
}

// 
void time_step_t::calculate_fps()
{
    acc_dt_s += dt;
    static int index;
    index = (index + 1) % FPS_ACC_COUNT;
    avg_fps -= fps_acc[index];
    fps_acc[index] = dt / FPS_ACC_COUNT;
    avg_fps += fps_acc[index];
    
    if (acc_dt_s > 1.0f) {
        fps = (int)roundf(1.0f / avg_fps);
        acc_dt_s = 0.0f;
    }
}

// 
void time_step_t::start_timer()
{
    m_is_timing = true;
    m_timer_start = glfwGetTime();
}

// 
float time_step_t::timer_dt()
{
    if (m_is_timing) {
        float dt = (glfwGetTime() - m_timer_start) * 1000.0f;
        m_is_timing = false;
        return dt;
    }
    return 0.0f;
}
