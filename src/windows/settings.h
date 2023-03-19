#pragma once

#include <pebble.h>

#define SETTINGS_NUM_WINDOWS 4
#define CHECKBOX_WINDOW_CELL_HEIGHT 44

static char hiragana_desc[38];
static char katakana_desc[38];

void settings_window_push();
void set_kana_description();