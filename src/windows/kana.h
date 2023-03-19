#pragma once

#include <pebble.h>

#define KANA_WINDOW_NUM_ROWS 16
#define KANA_WINDOW_BOX_SIZE 12
#define KANA_WINDOW_CELL_HEIGHT 44

void kana_window_push();

//Kana
enum Kana {
  hiragana,
  katakana
};
enum Kana current_kana;

//Create Data Structs
typedef struct kana_struct{
  bool selected_kana[16];
  uint32_t icon;
} kana_struct;

//Create Vars
uint32_t storage_hiragana_selected;
kana_struct hiragana_struct;
uint32_t storage_katakana_selected;
kana_struct katakana_struct;
