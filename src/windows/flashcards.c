#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "flashcards.h"
#include "card_num.h"
#include "kana.h"

static Window *s_main_window;
static TextLayer *s_text_layer, *s_number_layer;
static BitmapLayer *s_kana_layer;
static ActionBarLayer *s_action_bar_layer;
static StatusBarLayer *s_status_layer;
static TextLayer *s_end_header_layer, *s_end_text_layer;

static GBitmap *s_kana_bitmap, *s_forward_bitmap, *s_center_bitmap, *s_back_bitmap, *s_restart_bitmap;

static deck_struct current_deck;
static int current_num = 0;
static GRect display_bounds;
static bool end_card_loaded = false;

static void click_forward_handler(ClickRecognizerRef recognizer, void *context)
{
    if (!end_card_loaded)
    {
        // Reset image
        current_num++;
        reset_card();
    }
    else
    {
        // Go back
        window_stack_pop(true);
    }
}

static void click_center_handler(ClickRecognizerRef recognizer, void *context)
{
    if (!end_card_loaded)
    {
        // Middle (flip)
        layer_set_hidden(text_layer_get_layer(s_text_layer), !layer_get_hidden(text_layer_get_layer(s_text_layer)));
    }
    else
    {
        // Create new deck
        reset_deck();
        create_new_deck();
        reset_card();
    }
}

static void click_back_handler(ClickRecognizerRef recognizer, void *context)
{
    if (!end_card_loaded)
    {
        // Reset image
        current_num--;
        reset_card();
    }
    else
    {
        current_num = 0;
        reset_card();
    }
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, click_back_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, click_center_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, click_forward_handler);
}

static void window_load(Window *window)
{
    // Random
    srand(time(NULL));

    // Create Deck
    create_new_deck();

    // Create Prompt
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    #if defined(PBL_RECT)
        const GEdgeInsets status_insets = {.top = STATUS_BAR_LAYER_HEIGHT, .right = ACTION_BAR_WIDTH};
    #elif defined(PBL_ROUND)
        const GEdgeInsets status_insets = {.top = STATUS_BAR_LAYER_HEIGHT, .right = ACTION_BAR_WIDTH / 2};
    #endif
    bounds = grect_inset(bounds, status_insets);
    display_bounds = bounds;

    // Status Bar
    #if defined(PBL_RECT)
        GRect frame = GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT);
    #elif defined(PBL_ROUND)
        GRect frame = GRect(0, 0, bounds.size.w + (ACTION_BAR_WIDTH / 2), STATUS_BAR_LAYER_HEIGHT);
    #endif
    s_status_layer = status_bar_layer_create();
    layer_set_frame(status_bar_layer_get_layer(s_status_layer), frame);
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_layer));

    // Kana Bitmap
    const GEdgeInsets kana_insets = {.top = 7, .right = 0, .bottom = KANA_INSET_BOTTOM, .left = 0};
    s_kana_layer = bitmap_layer_create(grect_inset(bounds, kana_insets));
    bitmap_layer_set_compositing_mode(s_kana_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_kana_layer));

    // Text
    const GEdgeInsets text_insets = {.top = 105, .right = 0, .left = 0};
    s_text_layer = text_layer_create(grect_inset(bounds, text_insets));
    text_layer_set_text(s_text_layer, current_deck.flashcards[0].sound);
    text_layer_set_background_color(s_text_layer, GColorClear);
    text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

    // End Card
    const GEdgeInsets end_header_insets = {.top = 5};
    s_end_header_layer = text_layer_create(grect_inset(bounds, end_header_insets));
    text_layer_set_text(s_end_header_layer, "Deck Complete!");
    text_layer_set_background_color(s_end_header_layer, GColorClear);
    text_layer_set_text_alignment(s_end_header_layer, GTextAlignmentCenter);
    text_layer_set_font(s_end_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_end_header_layer));
    layer_set_hidden(text_layer_get_layer(s_end_header_layer), true);

    const GEdgeInsets end_text_insets = {.top = 70};
    s_end_text_layer = text_layer_create(grect_inset(bounds, end_text_insets));
    text_layer_set_text(s_end_text_layer, "Up: Restart Deck\nCenter: New Deck\nDown: Main Menu");
    text_layer_set_background_color(s_end_text_layer, GColorClear);
    text_layer_set_text_alignment(s_end_text_layer, GTextAlignmentCenter);
    text_layer_set_font(s_end_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_end_text_layer));
    layer_set_hidden(text_layer_get_layer(s_end_text_layer), true);

// Card Number
#if defined(PBL_RECT)
    const GEdgeInsets number_insets = {.top = 1, .left = 5};
#elif defined(PBL_ROUND)
    const GEdgeInsets number_insets = {.top = 5, .right = 0};
#endif
    s_number_layer = text_layer_create(grect_inset(bounds, number_insets));
    text_layer_set_background_color(s_number_layer, GColorClear);
    text_layer_set_text_alignment(s_number_layer, PBL_IF_RECT_ELSE(GTextAlignmentLeft, GTextAlignmentCenter));
    text_layer_set_font(s_number_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(s_number_layer));

    // Action Bar
    s_forward_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_FORWARD);
    s_center_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_CENTER);
    s_back_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_BACK);
    s_restart_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ICON_RESTART);
    s_action_bar_layer = action_bar_layer_create();
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_UP, s_back_bitmap);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_center_bitmap);
    action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_DOWN, s_forward_bitmap);
    action_bar_layer_add_to_window(s_action_bar_layer, window);
    action_bar_layer_set_click_config_provider(s_action_bar_layer, click_config_provider);

    // Set Card
    reset_card(window);
}

static void window_unload(Window *window)
{
    text_layer_destroy(s_text_layer);
    text_layer_destroy(s_number_layer);
    text_layer_destroy(s_end_header_layer);
    text_layer_destroy(s_end_text_layer);
    action_bar_layer_destroy(s_action_bar_layer);
    bitmap_layer_destroy(s_kana_layer);
    status_bar_layer_destroy(s_status_layer);

    gbitmap_destroy(s_kana_bitmap);
    s_kana_bitmap = NULL;
    gbitmap_destroy(s_forward_bitmap);
    gbitmap_destroy(s_center_bitmap);
    gbitmap_destroy(s_back_bitmap);
    gbitmap_destroy(s_restart_bitmap);

    reset_deck();

    window_destroy(window);
    s_main_window = NULL;
}

void flashcards_window_push()
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

void create_new_deck()
{
    // Create Array
    int *selected_kana_array_original = malloc(sizeof(int) * 208);
    int selected_kana_total = 0;
    int selected_kana_index = 0;
    int selected_kana_group = 0;
    char kana_list[104][4] = {"a", "i", "u", "e", "o", "ka", "ki", "ku", "ke", "ko", "ga", "gi", "gu", "ge", "go", "sa", "shi", "su", "se", "so", "za", "ji", "zu", "ze", "zo", "ta", "chi", "tsu", "te", "to", "da", "ji", "dzu", "de", "do", "na", "ni", "nu", "ne", "no", "ha", "hi", "fu", "he", "ho", "ba", "bi", "bu", "be", "bo", "pa", "pi", "pu", "pe", "po", "ma", "mi", "mu", "me", "mo", "ya", "yu", "yo", "ra", "ri", "ru", "re", "ro", "wa", "wo", "n", "kya", "kyu", "kyo", "gya", "gyu", "gyo", "sha", "shu", "sho", "ja", "ju", "jo", "cha", "chu", "cho", "nya", "nyu", "nyo", "hya", "hyu", "hyo", "bya", "byu", "byo", "pya", "pyu", "pyo", "mya", "myu", "myo", "rya", "ryu", "ryo"};

    // Add all possible cards to selected_kana array
    char *kana = kana_list[0];
    for (int i = 0; i < 104; i++)
    {
        if (i < 5 || selected_kana_group == 15 || (strcmp(kana_list[i], "fu") == 0 || strcmp(kana_list[i], "ji") == 0 || strcmp(kana_list[i], "chi") == 0 || strcmp(kana_list[i], "n") == 0) || kana[0] == kana_list[i][0])
        {
            if (selected_kana_group < 15 || (selected_kana_group == 15 && (hiragana_struct.selected_kana[selected_kana_group] || katakana_struct.selected_kana[selected_kana_group])))
            {
                // Hiragana
                if (hiragana_struct.selected_kana[selected_kana_group])
                {
                    selected_kana_array_original[selected_kana_index] = i;
                    selected_kana_index++;
                    selected_kana_total++;
                }

                // Katakana
                if (katakana_struct.selected_kana[selected_kana_group])
                {
                    selected_kana_array_original[selected_kana_index] = i + 104;
                    selected_kana_index++;
                    selected_kana_total++;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            selected_kana_group++;
            kana = kana_list[i];
            i--;
        }
    }

    // Set array
    randomize(selected_kana_array_original, selected_kana_total);

    // Create Deck
    current_deck.flashcards = (flashcard_struct *)malloc(sizeof(flashcard_struct) * card_num);
    current_deck.deck_size = card_num;

    int add_cards = 0;
    while (add_cards < card_num)
    {
        for (int i = 0; i < selected_kana_total; i++)
        {
            // Check
            if (!(add_cards < card_num))
                break;

            // Create Card
            flashcard_struct card;
            card.kana = RESOURCE_ID_H_A + selected_kana_array_original[i];
            if (selected_kana_array_original[i] < 104)
            {
                strcpy(card.sound, kana_list[selected_kana_array_original[i]]);
            }
            else
            {
                strcpy(card.sound, kana_list[selected_kana_array_original[i] - 104]);
            }

            // Add Card
            current_deck.flashcards[add_cards] = card;
            add_cards++;
            printf("%d Card: %s", i, card.sound);
        }

        if (add_cards < card_num)
        {
            randomize(selected_kana_array_original, selected_kana_total);
        }
    }

    // Free Array
    free(selected_kana_array_original);
}

void reset_card()
{
    if (current_num >= 0 && current_num < card_num)
    {
        // Reset from End Card
        if (end_card_loaded)
        {
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_center_bitmap);
            layer_set_hidden(text_layer_get_layer(s_end_header_layer), true);
            layer_set_hidden(text_layer_get_layer(s_end_text_layer), true);
            end_card_loaded = false;
        }

        // Set Layer Visibility
        layer_set_hidden(text_layer_get_layer(s_number_layer), false);
        layer_set_hidden(bitmap_layer_get_layer(s_kana_layer), false);
        layer_set_hidden(text_layer_get_layer(s_text_layer), true);

        // Destroy and Create Bitmap
        if (gbitmap_get_data(s_kana_bitmap))
        {
            gbitmap_destroy(s_kana_bitmap);
        }
        s_kana_bitmap = gbitmap_create_with_resource(current_deck.flashcards[current_num].kana);
        bitmap_layer_set_bitmap(s_kana_layer, s_kana_bitmap);

        // Set Sound
        text_layer_set_text(s_text_layer, current_deck.flashcards[current_num].sound);

        // Set Card Num
        static char number_text[8];
        snprintf(number_text, sizeof(number_text), "%d/%d", current_num + 1, card_num);
        printf("%s", number_text);
        text_layer_set_text(s_number_layer, number_text);
    }
    else
    {
        // Set Number Back
        if (current_num < 0)
        {
            current_num = 0;
        }
        else
        {
            end_card_loaded = true;
            // Hide Layers
            layer_set_hidden(text_layer_get_layer(s_number_layer), true);
            layer_set_hidden(bitmap_layer_get_layer(s_kana_layer), true);
            layer_set_hidden(text_layer_get_layer(s_text_layer), true);

            // Show Layer
            layer_set_hidden(text_layer_get_layer(s_end_header_layer), false);
            layer_set_hidden(text_layer_get_layer(s_end_text_layer), false);

            // Set Action Bar Icons
            action_bar_layer_set_icon(s_action_bar_layer, BUTTON_ID_SELECT, s_restart_bitmap);

            // Show End Screen
            current_num = card_num;
        }
    }
}

void reset_deck()
{
    free(current_deck.flashcards);
    current_num = 0;
}

// https://www.geeksforgeeks.org/shuffle-a-given-array-using-fisher-yates-shuffle-algorithm/
void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void randomize(int arr[], int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i + 1);

        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}