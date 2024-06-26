#include "LVGLHandler.hpp"

LockLVGLSafe::LockLVGLSafe(char * name, int line ):
    _name(name), _line(line){
    LV_ASSERT(LVGLHandler::getInstance().lvgl_mutex != NULL);
    if (line)
        Serial.printf("LVGL before taking mutex for %s from line %d\r\n", _name.c_str(), line);
    xSemaphoreTakeRecursive(LVGLHandler::getInstance().lvgl_mutex, portMAX_DELAY);

    if (line)
        Serial.printf("LVGL mutex taken for %s from line %d\r\n", _name.c_str(), line);
}
LockLVGLSafe::~LockLVGLSafe() {
    xSemaphoreGiveRecursive(LVGLHandler::getInstance().lvgl_mutex);
    if (_line)
        Serial.printf("LVGL mutex released for %s from line %d\r\n", _name.c_str(), _line);
}

LVGLHandler & LVGLHandler::getInstance()
{
    static LVGLHandler mInstance;
    return mInstance;
}

void LVGLHandler::Init()
{
    lcd.init();
    lcd.fillScreen(0xEF9E);
    // setting up LVGL
    lv_init();
    isInitialized = true;
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();

    LV_ASSERT( lvgl_mutex != NULL );

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

    /* this function needs to be called once before calling from threads, 
     *   other wise since it needs a huge amount of stack for first initialization, then the OS will panic. */
    lv_task_handler(); 

    xTaskCreate(
    LVGLTask
    ,  "lvglTsk"
    ,  6000  // Stack size
    ,  this // Pass reference to a variable describing the task number
    ,  1  // Low priority
    ,  &lvglTaskHandler
    );
}

void LVGLHandler::terminate() {
    if(isInitialized) {
        xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY);
        if(lvglTaskHandler != 0)
        {
            vTaskDelete(lvglTaskHandler);
            lvglTaskHandler = 0;
        }
        lv_deinit();
        vSemaphoreDelete(lvgl_mutex);
        lvgl_mutex = NULL;
        isInitialized = false;
    }
}

void LVGLHandler::LVGLTask(void *params)
{
    LVGLHandler * obj = (LVGLHandler *) params;
    while(1)
    {
        xSemaphoreTakeRecursive(obj->lvgl_mutex, portMAX_DELAY);
        lv_task_handler(); 
        lv_tick_inc(50);
        xSemaphoreGiveRecursive(obj->lvgl_mutex);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}