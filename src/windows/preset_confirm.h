#pragma once

#include <pebble.h>

int preset_num;
char text[20];

void preset_confirm_window_push();
char* get_name(int num, bool return_desc);