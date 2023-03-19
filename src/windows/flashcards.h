#pragma once

#include <pebble.h>

#define KANA_INSET_BOTTOM 35

void flashcards_window_push();
void create_new_deck();
void reset_card();
void reset_deck();
void randomize();
void swap();

typedef struct dakuten_position_struct{
    
} dakuten_position_struct;

typedef struct flashcard_struct{
    uint32_t kana;
    char sound[4];
} flashcard_struct;

typedef struct deck_struct{
    flashcard_struct* flashcards;
    int deck_size;
} deck_struct;