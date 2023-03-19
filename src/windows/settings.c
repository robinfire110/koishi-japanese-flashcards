#include "settings.h"
#include "card_num.h"
#include "kana.h"
#include "presets.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return SETTINGS_NUM_WINDOWS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Settings");
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  char str[4];
  switch (cell_index->row)
  {
  case 0:
    snprintf(str, sizeof(str), "%d", card_num);
    menu_cell_basic_draw(ctx, cell_layer, "Card Number", str, NULL);
    break;
  case 1:
    menu_cell_basic_draw(ctx, cell_layer, "Presets", "Load a deck preset", NULL);
    // Here you can add a check for if there is a deck in perseitant storage
    break;
  case 2:
    menu_cell_basic_draw(ctx, cell_layer, "Hiragana Setup", hiragana_desc, NULL);
    break;
  case 3:
    menu_cell_basic_draw(ctx, cell_layer, "Katakana Setup", katakana_desc, NULL);
    break;
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  return PBL_IF_ROUND_ELSE(menu_layer_is_index_selected(menu_layer, cell_index) ? MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT, CHECKBOX_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  switch (cell_index->row)
  {
  case 0:
    card_num_window_push();
    break;
  case 1:
    presets_window_push();
    break;
  case 2:
    current_kana = hiragana;
    kana_window_push();
    break;
  case 3:
    current_kana = katakana;
    kana_window_push();
    break;
  }
}

static void window_load(Window *window)
{
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
  window_destroy(window);
  s_main_window = NULL;
}

static void window_appear(Window *window)
{
  set_kana_description();
  menu_layer_reload_data(s_menu_layer);
}

void settings_window_push()
{
  if (!s_main_window)
  {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers){
      .load = window_load,
      .unload = window_unload,
      .appear = window_appear
    });
  }
  window_stack_push(s_main_window, true);
}

void set_kana_description()
{
  // Clear Current Strings
  memset(hiragana_desc, 0, strlen(hiragana_desc));
  memset(katakana_desc, 0, strlen(katakana_desc));

  // Get Currently Selected Kana
  static char kana_list[16][30] = {"a", "ka", "ga", "sa", "za", "ta", "da", "na", "ha", "ba", "pa", "ma", "ya", "ra", "wa", "Combo Kana"};
  int h_count = 0;
  int k_count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (h_count < 8 && hiragana_struct.selected_kana[i] == true)
    {
      if (strcmp(hiragana_desc, "") != 0)
        strcat(hiragana_desc, ", ");
      strcat(hiragana_desc, kana_list[i]);
      h_count++;
    }

    if (k_count < 8 && katakana_struct.selected_kana[i] == true)
    {
      if (strcmp(katakana_desc, "") != 0)
        strcat(katakana_desc, ", ");
      strcat(katakana_desc, kana_list[i]);

      k_count++;
    }
  }

  // If empty
  if (strcmp(hiragana_desc, "") == 0)
    strcat(hiragana_desc, "None");
  if (strcmp(katakana_desc, "") == 0)
    strcat(katakana_desc, "None");
}