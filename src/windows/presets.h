#pragma once

#include <pebble.h>

#define PRESETS_WINDOW_NUM_ROWS 9
#define PRESETS_WINDOW_CELL_HEIGHT 44
#define PRESETS_WINDOW_MENU_HEIGHT \
    PRESETS_WINDOW_NUM_ROWS * PRESETS_WINDOW_CELL_HEIGHT

void presets_window_push();