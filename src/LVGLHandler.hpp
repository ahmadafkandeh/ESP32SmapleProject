#ifndef LVGL_HANDLER_H
#define LVGL_HANDLER_H

#include <Arduino.h>
#include <lvgl.h>
#include <examples/lv_examples.h>
#include <FreeRTOS.h>
#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>
const unsigned int screenWidth = 320;
const unsigned int screenHeight = 480;
const unsigned int lvBufferSize = screenWidth * screenHeight / 10 * (LV_COLOR_DEPTH / 8);
static LGFX lcd;
static uint8_t lvBuffer[lvBufferSize];
class LockLVGLSafe
{
public:
    LockLVGLSafe(char* name = "", int line = 0);
    ~LockLVGLSafe();
    int _line;
    String _name;
};

class LVGLHandler
{
protected:
    SemaphoreHandle_t lvgl_mutex;
    friend LockLVGLSafe;
    bool isInitialized = false;
    TaskHandle_t lvglTaskHandler = 0;
public:
    static LVGLHandler & getInstance();
    void Init();
    void terminate();

private:
    static void LVGLTask(void* params);
};

#endif // !LVGL_HANDLER_H
