#include "card_num.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_layer;

static int amounts[] = {10, 20, 30, 40, 50, 75, 100, -1};
static int current_selected;

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
  return sizeof(amounts) / sizeof(amounts[0]);
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *data)
{
  menu_cell_basic_header_draw(ctx, cell_layer, "Select Card Number");
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
  char str[4];
  snprintf(str, sizeof(str), "%d", amounts[cell_index->row]);
  menu_cell_basic_draw(ctx, cell_layer, str, NULL, NULL);

  GRect bounds = layer_get_bounds(cell_layer);
  GPoint p = GPoint(bounds.size.w - (3 * RADIO_BUTTON_WINDOW_RADIO_RADIUS), (bounds.size.h / 2));

  // Selected?
  if (menu_cell_layer_is_highlighted(cell_layer))
  {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
  }
  else
  {
    graphics_context_set_fill_color(ctx, GColorBlack);
  }

  // Draw radio filled/empty
  graphics_draw_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS);
  if (cell_index->row == current_selected)
  {
    // This is the selection
    graphics_fill_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS - 3);
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  return PBL_IF_ROUND_ELSE(menu_layer_is_index_selected(menu_layer, cell_index) ? MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT, CARD_NUM_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
  card_num = amounts[cell_index->row];
  persist_write_int(storage_card_number, card_num);
  window_stack_pop(true);
}

static void window_load(Window *window)
{
  // Get Current Selected
  for (int i = 0; i < (int)(sizeof(amounts) / sizeof(amounts[0])); i++)
  {
    if (amounts[i] == card_num)
    {
      current_selected = i;
      break;
    }
  }

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

void card_num_window_push()
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
