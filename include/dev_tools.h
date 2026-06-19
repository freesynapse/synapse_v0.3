#ifndef __DEV_TOOLS_H
#define __DEV_TOOLS_H

#include <vector>
#include <string>

#include "event/key_codes.h"

// 
class dev_tools_t
{
public:
    void handle_input();
    void show_help_window();

    // 
    void load_help_file(const std::string &_file_path);
    void set_fullscreen_hotkey(int _key) { m_fullscreen_toggle_key = _key; };

    // 
    const std::vector<std::string> &get_help_content() { return m_help_content; }
    
    
private:
    int m_fullscreen_toggle_key = SYN_KEY_F11;
    std::vector<std::string> m_help_content;
};


#endif // __DEV_TOOLS_H
