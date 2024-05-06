#include "WifiConnector.hpp"
#include <Arduino.h>
#include <WiFi.h>
#include "LVGLHandler.hpp"




WIFIConnector::WIFIConnector(lv_obj_t *parent)
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_t * label;

    window = lv_win_create(parent == NULL ? lv_screen_active() : parent);
    lv_win_add_title(window, "Not Connected!");
    lv_obj_add_flag(window, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * cont = lv_win_get_content(window); 
    wifiList = lv_list_create(cont);
    lv_obj_set_size(wifiList, 300, 350);
    lv_obj_set_align(wifiList, LV_ALIGN_TOP_MID);

    btnRefresh = lv_button_create(cont);
    label = lv_label_create(btnRefresh);
    lv_label_set_text(label, "Refresh");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnRefresh, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    LV_UNUSED(obj);
                    LV_LOG_WARN("Refresh clicked!");
                    userdata->updateList();
                }
            }, LV_EVENT_ALL, this);


    lv_obj_set_size(btnRefresh, 300, 40);
    lv_obj_set_align(btnRefresh, LV_ALIGN_BOTTOM_MID);

    connectWindow = lv_win_create(parent == NULL ? lv_screen_active() : parent);
    lv_win_add_title(connectWindow, "Connecting to : xxxx");
    lv_obj_add_flag(connectWindow, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t * cont = lv_win_get_content(connectWindow); 


}

WIFIConnector::~WIFIConnector()
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_delete(window);
    lv_obj_delete(connectWindow);
}

void WIFIConnector::show()
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_remove_flag(window, LV_OBJ_FLAG_HIDDEN);
    updateList();
}

void WIFIConnector::connect()
{
}

void WIFIConnector::updateList()
{
    int n = WiFi.scanNetworks();
    lv_obj_t * btn;
    clearList();
    for(int i = 0 ; i < n; i++)
    {
        String ssid = WiFi.SSID(i);
        auto cnt = lv_obj_get_child_count(wifiList);
        bool douplicate = false;
        for (uint32_t y = 0 ; y < cnt ; y++) {
            btn = lv_obj_get_child(wifiList, y);
            String tmp = lv_list_get_button_text(wifiList, btn);
            if(tmp.equalsIgnoreCase(ssid)) { 
                douplicate = true;
                continue;
            }
        }
        if(!douplicate)
        {
            btn = lv_list_add_button(wifiList, LV_SYMBOL_WIFI, ssid.c_str());
            lv_obj_add_event_cb(btn, [](lv_event_t * e){
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                lv_obj_t * userdata = (lv_obj_t *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    LV_UNUSED(obj);
                    LV_LOG_WARN("Clicked: %s", lv_list_get_button_text(userdata, obj));
                }
            }, LV_EVENT_CLICKED, (void*)wifiList);
        }
    }
}

void WIFIConnector::getCurrent()
{
    if (WiFi.isConnected())
    {
        // String tmp = "Connected to " + WiFi.
        // lv_win_add_title(window, "Connected!");
    }
}

void WIFIConnector::clearList() {
    auto cnt = lv_obj_get_child_count(wifiList);
    for (uint32_t i = 0 ; i < cnt ; i++) {
        lv_obj_del(lv_obj_get_child(wifiList, 0));
    }
}
