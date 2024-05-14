#ifndef __WIFI_CONNECTOR_H_
#define __WIFI_CONNECTOR_H_

#include <Arduino.h>
#include <lvgl.h>
#include <list>
#include <mutex>

struct SSIDStruct
{
    String ssid;
    String EncryptedPassword;
};
class WIFIConnector
{
    public:
    WIFIConnector(lv_obj_t *parent = NULL);
    ~WIFIConnector();
    void show();

protected:
    bool connect();
    bool isSelectedAPOpen();
    void scanNetworks();
    void updateList();
    void getCurrent();
    void clearList();
    void saveCredentials();
    void loadAllSavedNetworks();
    void ConnectToFirstAvailable();
    void ForgetNetwork(String ssid);
    static void WiFiTask(void* params);
    String encryptPassword(String rawPass);
    String DecryptPassword(String encpass);

protected:
    lv_obj_t * wifiList;
    lv_obj_t * window;
    lv_obj_t * connectWindow;
    lv_obj_t * lblPassword;
    lv_obj_t * txtPassword;
    lv_obj_t * chkbxAutoReconnect; 
    lv_obj_t * btnCancel;
    lv_obj_t * btnConnect_Done;
    lv_obj_t * btnRefresh;
    lv_obj_t * mainWindowTitle;
    lv_obj_t * connectWindowTitle;
    lv_obj_t * btnClose;
    lv_obj_t * btnSavedAPs;
    String selectedSSID; 
    int foundNetworks;
    SemaphoreHandle_t _mutex;
    TaskHandle_t wifiTaskHandler;
    std::list<SSIDStruct> ssids;

};

#endif // !__WIFI_CONNECTOR_H_
