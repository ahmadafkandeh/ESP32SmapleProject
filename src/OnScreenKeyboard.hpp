#ifndef ON_SCRREN_KEYBOARD_H
#define ON_SCRREN_KEYBOARD_H

#include <lvgl.h>
class OnScreenKeyBoard
{
public:
    static OnScreenKeyBoard * getInstance();
    void Init(lv_obj_t * scr = NULL);
    void terminate();
    void registerObject(lv_obj_t * object);
    void removeObject(lv_obj_t * object);
    void setFocus(lv_obj_t * object);

private:
    lv_obj_t * kb = NULL;

};
#endif // !ON_SCRREN_KEYBOARD_H