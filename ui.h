#ifndef UI_H
#define UI_H

void show_menu();
void add_item();
void remove_item();
void show_settings();
void change_directory();
void ensure_log_terminal();
void check_terminal_status();
void ui_log(const char *color, const char *format, ...);
void play_sound(int type);

#endif