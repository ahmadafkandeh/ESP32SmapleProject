#include <Arduino.h>
#include <Wire.h>
#define LGFX_AUTODETECT
// #define LGFX_MAKERFABS_TOUCHCAMERA // Makerfabs Touch with Camera
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <examples/lv_examples.h>

// put function declarations here:

#define COLOR_BACKGROUND 0xEF9E

LGFX lcd;

const unsigned int screenWidth = 320;
const unsigned int screenHeight = 480;
const unsigned int lvBufferSize = screenWidth * screenHeight / 10 * (LV_COLOR_DEPTH / 8);
uint8_t lvBuffer[lvBufferSize];

static void drawExample();

void setup()
{
    lcd.init();
    lcd.fillScreen(COLOR_BACKGROUND);
    // setting up LVGL
    lv_init();
    
    static auto *lvDisplay = lv_display_create(screenWidth, screenHeight);
    //lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, [](lv_display_t* display, const lv_area_t* area, unsigned char* data) {
            uint32_t w = lv_area_get_width(area);
            uint32_t h = lv_area_get_height(area);
            lv_draw_sw_rgb565_swap(data, w*h);
            lcd.pushImage(area->x1, area->y1, w, h, (uint16_t*)data);
            lv_display_flush_ready(display);
        });
    lv_display_set_buffers(lvDisplay, lvBuffer, nullptr, lvBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    static auto *lvInput = lv_indev_create();
    lv_indev_set_type(lvInput, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvInput, [](lv_indev_t* dev, lv_indev_data_t* data) {
      uint16_t touchX, touchY;

      data->state = LV_INDEV_STATE_REL;
      if (lcd.getTouch(&touchX, &touchY))
      {
          data->state = LV_INDEV_STATE_PR;
          data->point.x = touchX;
          data->point.y = touchY;
      }
    });

    drawExample();
}

void loop()
{
    lv_task_handler(); 
    lv_tick_inc(5);
    delay(5);
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

// void drawExample() //keyboard
// {
//     /*Create a keyboard to use it with an of the text areas*/
//     lv_obj_t * kb = lv_keyboard_create(lv_screen_active());

//     /*Create a text area. The keyboard will write here*/
//     lv_obj_t * ta1;
//     ta1 = lv_textarea_create(lv_screen_active());
//     lv_obj_align(ta1, LV_ALIGN_TOP_LEFT, 10, 10);
//     lv_obj_add_event_cb(ta1, ta_event_cb, LV_EVENT_ALL, kb);
//     lv_textarea_set_placeholder_text(ta1, "username");
//     lv_obj_set_size(ta1, 140, 80);

//     lv_obj_t * ta2;
//     ta2 = lv_textarea_create(lv_screen_active());
//     lv_obj_align(ta2, LV_ALIGN_TOP_RIGHT, -10, 10);
//     lv_obj_add_event_cb(ta2, ta_event_cb, LV_EVENT_ALL, kb);
//     lv_obj_set_size(ta2, 140, 80);

//     lv_keyboard_set_textarea(kb, ta1);
// }

static lv_obj_t * list1;
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        LV_UNUSED(obj);
        LV_LOG_WARN("Clicked: %s", lv_list_get_button_text(list1, obj));
    }
}

void lv_example_list_1(void)
{
    /*Create a list*/
    list1 = lv_list_create(lv_screen_active());
    lv_obj_set_size(list1, 180, 220);
    lv_obj_center(list1);

    /*Add buttons to the list*/
    lv_obj_t * btn;
    lv_list_add_text(list1, "File");
    btn = lv_list_add_button(list1, LV_SYMBOL_FILE, "New");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_SAVE, "Save");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_CLOSE, "Delete");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_EDIT, "Edit");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Connectivity");
    btn = lv_list_add_button(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_GPS, "Navigation");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Exit");
    btn = lv_list_add_button(list1, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_button(list1, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
}

void drawExample()
{
lv_example_list_1();
}