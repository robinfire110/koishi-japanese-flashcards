#include "warning.h"
#include "c/sizes.h"

static Window *s_main_window;
static TextLayer *s_label_layer;
static Layer *s_background_layer, *s_icon_layer;

static Animation *s_appear_anim = NULL;

static GBitmap *s_icon_bitmap;

static void click_handler(ClickRecognizerRef recognizer, void *context)
{
    window_stack_pop(true);
}

static void anim_stopped_handler(Animation *animation, bool finished, void *context)
{
    s_appear_anim = NULL;
}

static void background_update_proc(Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorWhite));
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, 0);
}

static void icon_update_proc(Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds(layer);
    GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_icon_bitmap, (GRect){.origin = bounds.origin, .size = bitmap_bounds.size});
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, click_handler);
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    window_set_click_config_provider(window, click_config_provider);

    const GEdgeInsets background_insets = {.top = bounds.size.h};
    s_background_layer = layer_create(grect_inset(bounds, background_insets));
    layer_set_update_proc(s_background_layer, background_update_proc);
    layer_add_child(window_layer, s_background_layer);

    s_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WARNING);
    GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

    s_icon_layer = layer_create(PBL_IF_ROUND_ELSE(
        GRect((bounds.size.w - bitmap_bounds.size.w) / 2, bounds.size.h + WARNING_WINDOW_MARGIN, bitmap_bounds.size.w, bitmap_bounds.size.h),
        GRect(WARNING_WINDOW_MARGIN, bounds.size.h + WARNING_WINDOW_MARGIN, bitmap_bounds.size.w, bitmap_bounds.size.h)));
    layer_set_update_proc(s_icon_layer, icon_update_proc);
    layer_add_child(window_layer, s_icon_layer);

    s_label_layer = text_layer_create(GRect(WARNING_WINDOW_MARGIN, bounds.size.h + WARNING_WINDOW_MARGIN + bitmap_bounds.size.h, bounds.size.w - (2 * WARNING_WINDOW_MARGIN), bounds.size.h));
    text_layer_set_text(s_label_layer, "No Kana Selected!\nSelect Kana in Deck Settings.");
    text_layer_set_background_color(s_label_layer, GColorClear);
    text_layer_set_text_alignment(s_label_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
    #if PBL_DISPLAY_HEIGHT == 228 
        text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    #else
        text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    #endif
    layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void window_unload(Window *window)
{
    layer_destroy(s_background_layer);
    layer_destroy(s_icon_layer);

    text_layer_destroy(s_label_layer);

    gbitmap_destroy(s_icon_bitmap);

    window_destroy(window);
    s_main_window = NULL;
}

static void window_appear(Window *window)
{
    if (s_appear_anim)
    {
        // In progress, cancel
        animation_unschedule(s_appear_anim);
    }

    GRect bounds = layer_get_bounds(window_get_root_layer(window));
    // bounds = grect_inset(bounds, GEdgeInsets(bounds.size.w/2, 0, 0, 0));
    GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

    Layer *label_layer = text_layer_get_layer(s_label_layer);

    GRect start = layer_get_frame(s_background_layer);
    GRect finish = bounds;
    Animation *background_anim = (Animation *)property_animation_create_layer_frame(s_background_layer, &start, &finish);

    start = layer_get_frame(s_icon_layer);
    const GEdgeInsets icon_insets = {
        .top = PBL_IF_ROUND_ELSE(WARNING_WINDOW_MARGIN + WARNING_ROUND_OFFSET, WARNING_WINDOW_MARGIN),
        .left = PBL_IF_ROUND_ELSE((bounds.size.w - bitmap_bounds.size.w) / 2, WARNING_WINDOW_MARGIN)};
    finish = grect_inset(bounds, icon_insets);
    Animation *icon_anim = (Animation *)property_animation_create_layer_frame(s_icon_layer, &start, &finish);

    start = layer_get_frame(label_layer);
    const GEdgeInsets finish_insets = {
        .top = PBL_IF_ROUND_ELSE(WARNING_WINDOW_MARGIN + bitmap_bounds.size.h + 5 + WARNING_ROUND_OFFSET, WARNING_WINDOW_MARGIN + bitmap_bounds.size.h + 5),
        .right = WARNING_WINDOW_MARGIN,
        .left = WARNING_WINDOW_MARGIN};
    finish = grect_inset(bounds, finish_insets);
    Animation *label_anim = (Animation *)property_animation_create_layer_frame(label_layer, &start, &finish);

    s_appear_anim = animation_spawn_create(background_anim, icon_anim, label_anim, NULL);
    animation_set_handlers(s_appear_anim, (AnimationHandlers){.stopped = anim_stopped_handler}, NULL);
    animation_set_delay(s_appear_anim, 50);
    animation_schedule(s_appear_anim);
}

void warning_window_push()
{
    if (!s_main_window)
    {
        s_main_window = window_create();
        window_set_background_color(s_main_window, GColorBlack);
        window_set_window_handlers(s_main_window, (WindowHandlers){
            .load = window_load,
            .unload = window_unload,
            .appear = window_appear
        });
    }
    window_stack_push(s_main_window, true);
}
