#include "kana.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

static GBitmap *s_tick_black_bitmap, *s_tick_white_bitmap;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return KANA_WINDOW_NUM_ROWS;
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  if (current_kana == hiragana)
  {
    menu_cell_basic_header_draw(ctx, cell_layer, "Select Hiragana");
  }
  else
  {
    menu_cell_basic_header_draw(ctx, cell_layer, "Select Katakana");
  }
  
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  // Choice item
  static char s_buff[26];
  #if defined(PBL_RECT)
    static char kana_list[16][30] = {"  a", "  ka", "  ga", "  sa", "  za", "  ta", "  da", "  na", "  ha", "  ba", "  pa", "  ma", "  ya", "  ra", "  wa", "  Combo"};
  #elif defined(PBL_ROUND)
    static char kana_list[16][30] = {"a", "ka", "ga", "sa", "za", "ta", "da", "na", "ha", "ba", "pa", "ma", "ya", "ra", "wa", "Combo"};
  #endif
  snprintf(s_buff, sizeof(s_buff), "%s", kana_list[(int)cell_index->row]);

  // Create Icon Bitmaps
  uint32_t first_resource;
  if (current_kana == hiragana)
  {
    first_resource = RESOURCE_ID_ICON_H_A;
  }
  else
  {
    first_resource = RESOURCE_ID_ICON_K_A;
  }
  GBitmap *kana_source = gbitmap_create_with_resource(first_resource + ((int)cell_index->row));
  GBitmap *kana_black = gbitmap_create_as_sub_bitmap(kana_source, GRect(0, 0, 30, 30));
  GBitmap *kana_white = gbitmap_create_as_sub_bitmap(kana_source, GRect(30, 0, 30, 30));

  // Selected?
  GBitmap *ptr = s_tick_black_bitmap;
  GBitmap *kana_icon = kana_black;
  if (menu_cell_layer_is_highlighted(cell_layer))
  {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    ptr = s_tick_white_bitmap;
    kana_icon = kana_white;
  }

  // Draw and destroy bitmaps
  menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, kana_icon);
  gbitmap_destroy(kana_source);
  gbitmap_destroy(kana_black);
  gbitmap_destroy(kana_white);

  GRect bounds = layer_get_bounds(cell_layer);
  GRect bitmap_bounds = gbitmap_get_bounds(ptr);

  // Draw checkbox
  GRect r = GRect(bounds.size.w - (2 * KANA_WINDOW_BOX_SIZE), (bounds.size.h / 2) - (KANA_WINDOW_BOX_SIZE / 2), KANA_WINDOW_BOX_SIZE, KANA_WINDOW_BOX_SIZE);
  graphics_draw_rect(ctx, r);
  if (current_kana == hiragana)
  {
    if (hiragana_struct.selected_kana[cell_index->row])
    {
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, ptr, GRect(r.origin.x, r.origin.y - 3, bitmap_bounds.size.w, bitmap_bounds.size.h));
    }
  }
  else
  {
    if (katakana_struct.selected_kana[cell_index->row])
    {
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, ptr, GRect(r.origin.x, r.origin.y - 3, bitmap_bounds.size.w, bitmap_bounds.size.h));
    }
  }
  
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  return PBL_IF_ROUND_ELSE(menu_layer_is_index_selected(menu_layer, cell_index) ? MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT, KANA_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context)
{
  if (cell_index->row == KANA_WINDOW_NUM_ROWS)
  {
    window_stack_pop(true);
  }
  else
  {
    // Check/uncheck
    int row = cell_index->row;

    if (current_kana == hiragana)
    {
      hiragana_struct.selected_kana[row] = !hiragana_struct.selected_kana[row];
      persist_write_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
      menu_layer_reload_data(menu_layer);
    }
    else
    {
      katakana_struct.selected_kana[row] = !katakana_struct.selected_kana[row];
      persist_write_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
      menu_layer_reload_data(menu_layer);
    }
    
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

  s_tick_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK_BLACK);
  s_tick_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK_WHITE);

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

  gbitmap_destroy(s_tick_black_bitmap);
  gbitmap_destroy(s_tick_white_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

void kana_window_push(void)
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
