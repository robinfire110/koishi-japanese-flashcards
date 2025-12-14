#pragma once

#include <pebble.h>

#define RADIO_BUTTON_WINDOW_NUM_ROWS     4
#define RADIO_BUTTON_WINDOW_RADIO_RADIUS 6

void card_num_window_push();

//Storage
uint32_t storage_card_number;
int card_num;