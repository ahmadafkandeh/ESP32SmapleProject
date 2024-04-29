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
    LockLVGLSafe();
    ~LockLVGLSafe();
};

class LVGLHandler
{
    SemaphoreHandle_t lvgl_mutex;
    friend LockLVGLSafe;
    bool isInitialized = false;
public:
    static LVGLHandler & getInstance();
    void Init();
    void terminate();
};

#endif // !LVGL_HANDLER_H
