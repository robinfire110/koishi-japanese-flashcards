#include "preset_confirm.h"
#include "kana.h"

static Window *s_main_window;
static TextLayer *s_label_layer;
static BitmapLayer *s_icon_layer;
static ActionBarLayer *s_action_bar_layer;

static GBitmap *s_icon_bitmap, *s_tick_bitmap, *s_cross_bitmap;

static void click_accept_handler(ClickRecognizerRef recognizer, void *context)
{
    // Set Preset
    kana_struct new_hiragana_struct;
    kana_struct new_katakana_struct;
    switch (preset_num)
    {
    /* ALL KANA*/
    case 0:
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = true;
            new_katakana_struct.selected_kana[i] = true;
        }

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 1: /* HIRAGANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = true;
            new_katakana_struct.selected_kana[i] = false;
        }

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 2: /* MAIN HIRAGANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = true;
            new_katakana_struct.selected_kana[i] = false;
        }
        new_hiragana_struct.selected_kana[2] = false;
        new_hiragana_struct.selected_kana[4] = false;
        new_hiragana_struct.selected_kana[6] = false;
        new_hiragana_struct.selected_kana[9] = false;
        new_hiragana_struct.selected_kana[10] = false;
        new_hiragana_struct.selected_kana[15] = false;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 3: /* DAKUTEN HIRAGANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = false;
        }
        new_hiragana_struct.selected_kana[2] = true;
        new_hiragana_struct.selected_kana[4] = true;
        new_hiragana_struct.selected_kana[6] = true;
        new_hiragana_struct.selected_kana[9] = true;
        new_hiragana_struct.selected_kana[10] = true;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 4: /* COMBO HIRAGANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = false;
        }
        new_hiragana_struct.selected_kana[15] = true;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 5: /* KATAKANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = true;
        }

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 6: /* MAIN KATAKANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = true;
        }
        new_katakana_struct.selected_kana[2] = false;
        new_katakana_struct.selected_kana[4] = false;
        new_katakana_struct.selected_kana[6] = false;
        new_katakana_struct.selected_kana[9] = false;
        new_katakana_struct.selected_kana[10] = false;
        new_katakana_struct.selected_kana[15] = false;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 7: /* DAKUTEN KATAKANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = false;
        }
        new_katakana_struct.selected_kana[2] = true;
        new_katakana_struct.selected_kana[4] = true;
        new_katakana_struct.selected_kana[6] = true;
        new_katakana_struct.selected_kana[9] = true;
        new_katakana_struct.selected_kana[10] = true;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    case 8: /* COMBO KATAKANA ONLY */
        for (int i = 0; i < 16; i++)
        {
            new_hiragana_struct.selected_kana[i] = false;
            new_katakana_struct.selected_kana[i] = false;
        }
        new_katakana_struct.selected_kana[15] = true;

        persist_write_data(storage_hiragana_selected, &new_hiragana_struct, sizeof(kana_struct));
        persist_read_data(storage_hiragana_selected, &hiragana_struct, sizeof(kana_struct));
        persist_write_data(storage_katakana_selected, &new_katakana_struct, sizeof(kana_struct));
        persist_read_data(storage_katakana_selected, &katakana_struct, sizeof(kana_struct));
        break;
    }

    // Pop back to settings
    window_stack_pop(true);
    window_stack_pop(true);
}

static void click_decline_handler(ClickRecognizerRef recognizer, void *context)
{
    window_stack_pop(true);
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, click_accept_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, click_decline_handler);
}

static void window_load(Window *window)
{
    // Create Prompt
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CONFIRM);

    #if defined(PBL_RECT)
        const GEdgeInsets icon_insets = {.top = 7, .right = ACTION_BAR_WIDTH, .bottom = 56, .left = 0};
    #elif defined(PBL_ROUND)
        const GEdgeInsets icon_insets = {.top = 7, .right = ACTION_BAR_WIDTH / 2, .bottom = 56, .left = 0};
    #endif
    s_icon_layer = bitmap_layer_create(grect_inset(bounds, icon_insets));
    bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
    bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

    #if defined(PBL_RECT)
        const GEdgeInsets label_insets = {.top = 112, .right = ACTION_BAR_WIDTH, .left = 0};
    #elif defined(PBL_ROUND)
        const GEdgeInsets label_insets = {.top = 112, .right = ACTION_BAR_WIDTH / 2, .left = 0};
    #endif
    s_label_layer = text_layer_create(grect_inset(bounds, label_insets));
    static char s_buff[30];
    snprintf(s_buff, sizeof(s_buff), "Apply preset\n%s?", get_name(preset_num, false));
    text_layer_set_text(s_label_layer, s_buff);
    text_layer_set_background_color(s_label_layer, GColorClear);
    text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
    text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

    s_tick_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK);
    s_cross_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CROSS);

    s_action_bar_layer = action_bar_layer_create();
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_tick_bitmap);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_cross_bitmap);
    action_bar_layer_add_to_window(s_action_bar_layer, window);
    action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);
}

static void window_unload(Window *window)
{
    text_layer_destroy(s_label_layer);
    action_bar_layer_destroy(s_action_bar_layer);
    bitmap_layer_destroy(s_icon_layer);

    gbitmap_destroy(s_icon_bitmap);
    gbitmap_destroy(s_tick_bitmap);
    gbitmap_destroy(s_cross_bitmap);

    window_destroy(window);
    s_main_window = NULL;
}

void preset_confirm_window_push()
{
    if (!s_main_window)
    {
        s_main_window = window_create();
        window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorJaegerGreen, GColorWhite));
        window_set_window_handlers(s_main_window, (WindowHandlers){
            .load = window_load,
            .unload = window_unload,
        });
    }
    window_stack_push(s_main_window, true);
}

char *get_name(int num, bool return_desc)
{
    switch (num)
    {
    case 0:
        if (!return_desc)
            return "All Kana";
        else
            return NULL;
        break;
    case 1:
        if (!return_desc)
            return "All Hiragana";
        else
            return "Hiragana Only";
        break;
    case 2:
        if (!return_desc)
            return "Main Hiragana";
        else
            return "No Dakuten/Combo";
        break;
    case 3:
        if (!return_desc)
            return "Dakuten Hiragana";
        else
            return "Dakuten Only";
        break;
    case 4:
        if (!return_desc)
            return "Combo Hiragana";
        else
            return "Combo Only";
        break;
    case 5:
        if (!return_desc)
            return "All Katakana";
        else
            return "Katakana Only";
        break;
    case 6:
        if (!return_desc)
            return "Main Katakana";
        else
            return "No Dakuten/Combo";
        break;
    case 7:
        if (!return_desc)
            return "Dakuten Katakana";
        else
            return "Dakuten Only";
        break;
    case 8:
        if (!return_desc)
            return "Combo Katakana";
        else
            return "Combo Only";
        break;
    }
    return "";
}