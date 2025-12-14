#include "windows/kana.h"
#include "windows/settings.h"
#include "windows/flashcards.h"
#include "windows/card_num.h"
#include "windows/presets.h"
#include "windows/warning.h"
#include "windows/help.h"
#include "sizes.h"

#define NUM_WINDOWS 3

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

void get_persistent_data();
bool is_kana_selected();

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return NUM_WINDOWS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  #if PBL_DISPLAY_HEIGHT == 228 
    return MENU_CELL_BASIC_HEADER_HEIGHT + HEADER_HEIGHT_EMERY_ADD;
  #elif PBL_DISPLAY_HEIGHT == 180 
    return MENU_CELL_BASIC_HEADER_HEIGHT;
  #else
    return MENU_CELL_BASIC_HEADER_HEIGHT;
  #endif
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  #if PBL_DISPLAY_HEIGHT == 228 
    menu_cell_title_draw(ctx, cell_layer, "Koishi - Main Menu");
  #else
    menu_cell_basic_header_draw(ctx, cell_layer, "Koishi - Main Menu");
  #endif
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  switch (cell_index->row)
  {
  case 0:
    menu_cell_basic_draw(ctx, cell_layer, "Start Deck", "Use current settings.", NULL);
    break;
  case 1:
    menu_cell_basic_draw(ctx, cell_layer, "Deck Settings", NULL, NULL);
    break;
  case 2:
    menu_cell_basic_draw(ctx, cell_layer, "About", NULL, NULL);
    break;
  default:
    break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  #if PBL_DISPLAY_HEIGHT == 228 
    return WINDOW_CELL_HEIGHT_EMERY;
  #elif PBL_DISPLAY_HEIGHT == 180 
    return menu_layer_is_index_selected(menu_layer, cell_index) ? MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT;
  #else
    return WINDOW_CELL_HEIGHT;
  #endif
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  switch (cell_index->row)
  {
  case 0:
    if (is_kana_selected())
    {
      flashcards_window_push();
    }
    else
    {
      warning_window_push();
    }
    break;
  case 1:
    settings_window_push();
    break;
  case 2:
    help_window_push();
    break;
  default:
    break;
  }
}

static void window_load(Window *window)
{
  get_persistent_data();

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  const GEdgeInsets status_insets = {.top = STATUS_BAR_LAYER_HEIGHT};
  bounds = grect_inset(bounds, status_insets);

  s_status_layer = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_layer, GColorWhite, GColorBlack);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
      .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL),
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));
}

static void window_unload(Window *window)
{
  menu_layer_destroy(s_menu_layer);
  status_bar_layer_destroy(s_status_layer);
}

static void init()
{
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){
      .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit()
{
  window_destroy(s_main_window);
}

int main()
{
  init();
  app_event_loop();
  deinit();
}

/* SETUP FUNCTIONS */
void get_persistent_data()
{
  // Resource Icon List
  uint32_t hiragana_select_icon[15] = {5}; // Set the icons here
  uint32_t katakana_select_icon[15] = {5}; // Set the icons here

  // Hiragana Selected
  storage_hiragana_selected = 0;
  if (persist_exists(storage_hiragana_selected))
  {
    persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
  }
  else
  {
    for (int i = 0; i < 16; i++)
    {
      hiragana_struct.selected_kana[i] = true;
      hiragana_struct.icon = hiragana_select_icon[i];
    }
    persist_write_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
  }

  // Katakana Selected
  storage_katakana_selected = 1;
  if (persist_exists(storage_katakana_selected))
  {
    persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
  }
  else
  {
    for (int i = 0; i < 16; i++)
    {
      katakana_struct.selected_kana[i] = true;
      katakana_struct.icon = hiragana_select_icon[i];
    }
    persist_write_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
  }

  // Card Number
  storage_card_number = 2;
  if (persist_exists(storage_card_number))
  {
    card_num = persist_read_int(storage_card_number);
  }
  else
  {
    card_num = 20;
    persist_write_int(storage_card_number, card_num);
  }
}

bool is_kana_selected()
{
  for (int i = 0; i < 16; i++)
  {
    if (hiragana_struct.selected_kana[i] || katakana_struct.selected_kana[i])
    {
      return true;
    }
  }
  return false;
}