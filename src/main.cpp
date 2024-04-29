#include <Arduino.h>
#include <Wire.h>


#include "LVGLHandler.hpp"
#include "OnScreenKeyboard.hpp"
#include "WifiConnector.hpp"


// put function declarations here:

static void drawExample();
void setup()
{
    Serial.begin(115200);
    LVGLHandler::getInstance().Init();
    OnScreenKeyBoard::getInstance().Init();
    drawExample();
}

void loop()
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_task_handler(); 
    lv_tick_inc(1);
    delay(1);
}

void drawExample() //keyboard
{
    /*Create a keyboard to use it with an of the text areas*/
    // lv_obj_t * kb = lv_keyboard_create(lv_screen_active());

    /*Create a text area. The keyboard will write here*/
    lv_obj_t * ta1;
    ta1 = lv_textarea_create(lv_screen_active());
    lv_obj_align(ta1, LV_ALIGN_TOP_LEFT, 10, 10);
    OnScreenKeyBoard::getInstance().registerObject(ta1);
    lv_textarea_set_placeholder_text(ta1, "username");
    lv_obj_set_size(ta1, 140, 80);

    lv_obj_t * ta2;
    ta2 = lv_textarea_create(lv_screen_active());
    lv_obj_align(ta2, LV_ALIGN_TOP_RIGHT, -10, 10);
    // lv_obj_add_event_cb(ta2, ta_event_cb, LV_EVENT_ALL, kb);
    OnScreenKeyBoard::getInstance().registerObject(ta2);
    lv_obj_set_size(ta2, 140, 80);

    OnScreenKeyBoard::getInstance().setFocus(ta1);
}

// static lv_obj_t * list1;
// static void event_handler(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
//     if(code == LV_EVENT_CLICKED) {
//         LV_UNUSED(obj);
//         LV_LOG_WARN("Clicked: %s", lv_list_get_button_text(list1, obj));
//     }
// }

// void lv_example_list_1(void)
// {
//     /*Create a list*/
//     list1 = lv_list_create(lv_screen_active());
//     lv_obj_set_size(list1, 180, 220);
//     lv_obj_center(list1);

//     /*Add buttons to the list*/
//     lv_obj_t * btn;
//     lv_list_add_text(list1, "File");
//     btn = lv_list_add_button(list1, LV_SYMBOL_FILE, "New");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_DIRECTORY, "Open");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_SAVE, "Save");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_CLOSE, "Delete");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_EDIT, "Edit");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

//     lv_list_add_text(list1, "Connectivity");
//     btn = lv_list_add_button(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_GPS, "Navigation");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_USB, "USB");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

//     lv_list_add_text(list1, "Exit");
//     btn = lv_list_add_button(list1, LV_SYMBOL_OK, "Apply");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
//     btn = lv_list_add_button(list1, LV_SYMBOL_CLOSE, "Close");
//     lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
// }

// void drawExample()
// {
// lv_example_list_1();
// }