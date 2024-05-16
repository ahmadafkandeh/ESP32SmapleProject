#include "OnScreenKeyboard.hpp"
#include "LVGLHandler.hpp"

static void EventHandler(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_move_foreground(kb);
        lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

OnScreenKeyBoard &OnScreenKeyBoard::getInstance() {
    static OnScreenKeyBoard instance;
    return instance;
}

void OnScreenKeyBoard::Init(lv_obj_t *scr) {
    if (scr == NULL) {
        scr = lv_screen_active();
    }

    LV_ASSERT(scr != NULL);

    if(kb != NULL) {
        lv_obj_del(kb);
    }
    kb = lv_keyboard_create(scr);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
}

void OnScreenKeyBoard::terminate() {
     if(kb != NULL) {
        lv_obj_del(kb);
        kb = NULL;
    }
}

void OnScreenKeyBoard::registerObject(lv_obj_t *object) {
    LockLVGLSafe obj =  LockLVGLSafe(__FILENAME__, __LINE__);
    lv_obj_add_event_cb(object, EventHandler, 
                        LV_EVENT_ALL, kb);
}

void OnScreenKeyBoard::removeObject(lv_obj_t *object) {
    LockLVGLSafe obj =  LockLVGLSafe(__FILENAME__, __LINE__);
    lv_obj_remove_event_cb(object,EventHandler);
}

void OnScreenKeyBoard::setFocus(lv_obj_t *object) {
    LockLVGLSafe obj =  LockLVGLSafe(__FILENAME__, __LINE__);
    lv_keyboard_set_textarea(kb, object);
    lv_obj_remove_flag(kb, LV_OBJ_FLAG_HIDDEN);
}
