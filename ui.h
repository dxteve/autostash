#ifndef UI_H
#define UI_H

void show_menu();
void add_folder();
void remove_folder();
void show_settings();
void ensure_log_terminal(); // New function to check/restart terminal
void ui_log(const char *color, const char *format, ...);

#endif