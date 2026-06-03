#ifndef __TIMESTEP_H
#define __TIMESTEP_H

// 
#define FPS_ACC_COUNT 20

//
class time_step_t
{
public:
    float  fps_limit                = 60.0f;
    int    fps                      = 0;
    float  dt                       = 0.0f;

private:
    float m_timer_start = 0.0f;
    bool m_is_timing = false;
    
private:
    double current_frame_time_s     = 0.0;
    double last_frame_time_s        = 0.0;
    double end_of_last_frame_s      = 0.0;
    float  acc_dt_s                 = 0.0f;
    float  avg_fps                  = 0.0f;
    float  fps_acc[FPS_ACC_COUNT] = { 0 };
    
public:
    time_step_t() = default;
    ~time_step_t() = default;
    
    void update();
    void calculate_fps();

    void start_timer();
    float timer_dt();
    
};


#endif // __TIMESTEP_H
