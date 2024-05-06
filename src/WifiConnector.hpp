#ifndef __WIFI_CONNECTOR_H_
#define __WIFI_CONNECTOR_H_

#include <lvgl.h>

class WIFIConnector
{
    public:
    WIFIConnector(lv_obj_t *parent = NULL);
    ~WIFIConnector();
    void show();
    void connect();
protected:
    void updateList();
    void getCurrent();
    void clearList();

private:
    lv_obj_t * wifiList;
    lv_obj_t * window;
    lv_obj_t * connectWindow;
    lv_obj_t * lblSSID;
    lv_obj_t * lblSSIDName;
    lv_obj_t * lblPassword;
    lv_obj_t * txtPassword;
    lv_obj_t * chkbxAutoReconnect; 
    lv_obj_t * btnCancel;
    lv_obj_t * btnConnect_Done;
    lv_obj_t * btnRefresh;
};

#endif // !__WIFI_CONNECTOR_H_
