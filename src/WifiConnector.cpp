#include "WifiConnector.hpp"
#include <Arduino.h>
#include <WiFi.h>
#include "LVGLHandler.hpp"
#include "OnScreenKeyboard.hpp"
#include "Utility.hpp"
#include <ArduinoJson.h>
#include "FileSystemHandler.hpp"

WIFIConnector::WIFIConnector(lv_obj_t *parent)
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_t * label;
    /**************************************************************************************************
     * Main WiFi Connection Form
    **************************************************************************************************/
    
    _mutex = xSemaphoreCreateMutex();
    assert( _mutex != NULL );
    
    window = lv_win_create(parent == NULL ? lv_screen_active() : parent);
    mainWindowTitle = lv_win_add_title(window, "Not Connected!");
    lv_obj_add_flag(window, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t * cont = lv_win_get_content(window); 
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

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
                    userdata->scanNetworks();
                    userdata->updateList();
                }
            }, LV_EVENT_ALL, this);


    lv_obj_set_size(btnRefresh, 300, 40);
    lv_obj_set_align(btnRefresh, LV_ALIGN_BOTTOM_MID);
    /**************************************************************************************************
     * Connect to AP form
    **************************************************************************************************/
    connectWindow = lv_win_create(parent == NULL ? lv_screen_active() : parent);
    connectWindowTitle = lv_win_add_title(connectWindow, "Connecting to : xxxx" );
    lv_obj_add_flag(connectWindow, LV_OBJ_FLAG_HIDDEN);

    cont = lv_win_get_content(connectWindow);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lblPassword = lv_label_create(cont);
    lv_label_set_text(lblPassword, "Password: ");
    lv_obj_set_align(lblPassword, LV_ALIGN_TOP_LEFT);

    txtPassword = lv_textarea_create(cont);
    lv_textarea_set_password_mode(txtPassword, true);
    lv_textarea_set_one_line(txtPassword, true);
    OnScreenKeyBoard::getInstance().registerObject(txtPassword);
    lv_obj_set_pos(txtPassword, 0, 30);
    lv_obj_set_size(txtPassword,290, 40);


    chkbxAutoReconnect = lv_checkbox_create(cont);
    lv_checkbox_set_text(chkbxAutoReconnect, "Auto reconnect");
    lv_obj_set_pos(chkbxAutoReconnect, 0, 90);
    
    btnConnect_Done = lv_button_create(cont);
    label = lv_label_create(btnConnect_Done);
    lv_label_set_text(label, "Connect");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnConnect_Done, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    Utility::safe_lock_mutex lock(userdata->_mutex);
                    if (!userdata->connect())
                        lv_label_set_text(userdata->connectWindowTitle, "Failed to connect!");
                    else
                        lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);

                }
            }, LV_EVENT_ALL, this);
    


    btnCancel = lv_button_create(cont);
    label = lv_label_create(btnCancel);
    lv_label_set_text(label, "Cancel");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnCancel, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    LV_UNUSED(obj);
                    lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                    userdata->selectedSSID = "";
                }
            }, LV_EVENT_ALL, this);
    selectedSSID = "";
    lv_obj_set_pos(btnCancel, 10, 130);
    lv_obj_set_pos(btnConnect_Done,200 , 130);

    foundNetworks = 0;
    if (!FileSystemHandler::getInstance().exists("/WiFi"))
    {
        assert(FileSystemHandler::getInstance().mkdir("/WiFi") != true);
        assert(FileSystemHandler::getInstance().exists("/WiFi") != true);
    }

    ssids.clear();
    loadAllSavedNetworks();
    xTaskCreate(
    WiFiTask
    ,  "wifiTsk"
    ,  3000  // Stack size
    ,  this // Pass reference to a variable describing the task number
    ,  1  // Low priority
    ,  &wifiTaskHandler // Task handle is not used here - simply pass NULL
    );

}

WIFIConnector::~WIFIConnector()
{
    LockLVGLSafe obj = LockLVGLSafe();
    vTaskDelete(wifiTaskHandler);
    OnScreenKeyBoard::getInstance().removeObject(txtPassword);
    lv_obj_delete(window);
    lv_obj_delete(connectWindow);
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
}

void WIFIConnector::show()
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_remove_flag(window, LV_OBJ_FLAG_HIDDEN);
    updateList();
}

bool WIFIConnector::connect()
{
    WiFi.begin(selectedSSID.c_str(), lv_textarea_get_text(txtPassword));
    int count = 0;
    while (WiFi.status() != WL_CONNECTED && count<=2) {
        delay(1000);
        count++;
    }
    if(WiFi.status() == WL_CONNECTED && (lv_obj_get_state(chkbxAutoReconnect) & LV_STATE_CHECKED) )
        saveCredentials();
    
    return WiFi.isConnected();
}

bool WIFIConnector::isSelectedAPOpen()
{
    for(int i = 0 ; i < foundNetworks; i++)
    {
        String ssid = WiFi.SSID(i);
        if(selectedSSID.equalsIgnoreCase(ssid)) 
        { 
            return (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        }
    }
    return false;
}

void WIFIConnector::scanNetworks()
{
    Utility::safe_lock_mutex lock(_mutex);
    foundNetworks = WiFi.scanNetworks();
}

void WIFIConnector::updateList()
{   
    lv_obj_t * btn;
    Utility::safe_lock_mutex lock(_mutex);
    clearList();
    for(int i = 0 ; i < foundNetworks; i++)
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
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                
                if(code == LV_EVENT_CLICKED) {
                    userdata->selectedSSID = lv_list_get_button_text(userdata->wifiList, obj);
                    lv_label_set_text(userdata->connectWindowTitle, ("Connecting to : " + userdata->selectedSSID).c_str());
                    lv_obj_remove_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                }
            }, LV_EVENT_CLICKED, this);
        }
    }
}

void WIFIConnector::getCurrent()
{
    if (WiFi.isConnected())
    {
        //TODO: get AP name
        LockLVGLSafe obj = LockLVGLSafe();
         String tmp = "Connected to " + String(WiFi.getHostname());
         lv_label_set_text(mainWindowTitle, tmp.c_str());
    }
}

void WIFIConnector::clearList() {
    auto cnt = lv_obj_get_child_count(wifiList);
    for (uint32_t i = 0 ; i < cnt ; i++) {
        lv_obj_del(lv_obj_get_child(wifiList, 0));
    }
}

void WIFIConnector::saveCredentials()
{
    static JsonDocument doc;
    doc.clear();
    
    String encryptedpass = encryptPassword(lv_textarea_get_text(txtPassword));
    lv_textarea_set_text(txtPassword,"");

    bool update = false;
    for (SSIDStruct & obj: ssids )
    {
        if (obj.ssid.equalsIgnoreCase(selectedSSID))
        {
            obj.EncryptedPassword = encryptedpass;
            update = true;
            break;
        }
    }
    
    if(!update)
    {
        doc["items"] = ssids.size() + 1;
        SSIDStruct tmp{selectedSSID, encryptedpass};
        ssids.push_back(tmp);
    }
    else
    {
        doc["items"] = ssids.size();
    }

    JsonArray tmp = doc["ssids"].to<JsonArray>();

    for (SSIDStruct & obj: ssids )
    {
        tmp.add(obj.ssid.c_str());
        tmp.add(obj.EncryptedPassword.c_str());
    }

    std::string output;

    doc.shrinkToFit();  // optional

    serializeJson(doc, output);

    File f = FileSystemHandler::getInstance().openFile("/WiFi/ssids.txt", "w");
    if (f)
    {
        FileSystemHandler::getInstance().write(f, (uint8_t*)output.c_str(), output.length());
        FileSystemHandler::getInstance().closeFile(f);
        output.clear();
    }

}

void WIFIConnector::loadAllSavedNetworks()
{
    static JsonDocument doc;
    doc.clear();
    File f = FileSystemHandler::getInstance().openFile("/WiFi/ssids.txt", "r");
    if (f)
    {
        int size = FileSystemHandler::getInstance().size(f);
        uint8_t buf[size];
        memset(buf, 0, size);
        int ret = FileSystemHandler::getInstance().read(f, buf, size) ;
        FileSystemHandler::getInstance().closeFile(f);
        if (ret == size)
        {
            Serial.printf("got WiFi saved data:%s\r\n", buf);
            auto ret = deserializeJson(doc, buf);
            if ( ret != DeserializationError::Code::Ok)
            {   
                Serial.printf("failed to deserialize buffer with code %s\r\n", ret.c_str());
                return;
            }
            ssids.clear();
            int items = doc["items"];
            for (int i = 0; i < items*2; i++)
            {
                const char* tssid = doc["ssids"][i];
                const char* tpswd = doc["ssids"][++i];
                SSIDStruct tmp{tssid, tpswd};
                ssids.push_back(tmp);
            }
            
        }
    }
}

void WIFIConnector::ConnectToFirstAvailable()
{
    String txtpass = "";
    for(int i = 0 ; i < foundNetworks; i++)
    {
        String ssid = WiFi.SSID(i);
        auto cnt = lv_obj_get_child_count(wifiList);
        bool found = false;
        for (const SSIDStruct &obj: ssids) 
        {
            if (obj.ssid.equalsIgnoreCase(ssid))
            {
                found = true;
                txtpass = obj.EncryptedPassword;
                Serial.println("Found network");
                break;
            }
        }
        if (found)
        {
            WiFi.begin(ssid.c_str(), DecryptPassword(txtpass).c_str());
            break;
        }
    }
}

void WIFIConnector::ForgetNetwork(String ssid)
{
    //TODO: implement me
}

void WIFIConnector::WiFiTask(void *params)
{
    WIFIConnector * obj = (WIFIConnector *)(params);
    vTaskDelay(1000);
    while(1)
    {
        vTaskDelay(3000);
        obj->scanNetworks();
        {
            Utility::safe_lock_mutex lock(obj->_mutex);
            if(!WiFi.isConnected())
            {
                obj->ConnectToFirstAvailable();
            }
            else{
                obj->getCurrent();
            }
        }
        

    }
}

String WIFIConnector::encryptPassword(String rawPass)
{
    //TODO: implement
    return rawPass;
}

String WIFIConnector::DecryptPassword(String encpass)
{
    //TODO: implement
    return encpass;
}
