#include "presets.h"
#include "preset_confirm.h"
#include "c/sizes.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

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
    menu_cell_title_draw(ctx, cell_layer, "Load preset");
  #else
    menu_cell_basic_header_draw(ctx, cell_layer, "Load preset");
  #endif
}

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return PRESETS_WINDOW_NUM_ROWS;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  menu_cell_basic_draw(ctx, cell_layer, get_name(cell_index->row, false), get_name(cell_index->row, true), NULL);
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
  preset_num = cell_index->row;
  preset_confirm_window_push();
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
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){.get_num_rows = get_num_rows_callback, .draw_row = draw_row_callback, .get_cell_height = get_cell_height_callback, .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL), .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL), .select_click = select_callback});
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

void presets_window_push()
{
  if (!s_main_window)
  {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers){
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}
