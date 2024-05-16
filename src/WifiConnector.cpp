#include "WifiConnector.hpp"
#include <Arduino.h>
#include <WiFi.h>
#include "LVGLHandler.hpp"
#include "OnScreenKeyboard.hpp"
#include "Utility.hpp"
#include <ArduinoJson.h>
#include <algorithm>

#include "FileSystemHandler.hpp"

WIFIConnector::WIFIConnector(lv_obj_t *parent)
{
    LockLVGLSafe obj = LockLVGLSafe();
    lv_obj_t * label;
    /**************************************************************************************************
     * Main WiFi Connection Form
    **************************************************************************************************/
    
    _mutex = xSemaphoreCreateRecursiveMutex();
    assert( _mutex != NULL );
    
    window = lv_win_create(parent == NULL ? lv_screen_active() : parent);
    mainWindowTitle = lv_win_add_title(window, "Not Connected!");
    lv_obj_add_flag(window, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t * cont = lv_win_get_content(window); 
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    wifiList = lv_list_create(cont);
    lv_obj_set_size(wifiList, 300, 280);
    lv_obj_set_align(wifiList, LV_ALIGN_TOP_MID);


    
    
    btnRefresh = lv_button_create(cont);
    label = lv_label_create(btnRefresh);
    lv_label_set_text(label, "Refresh");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnRefresh, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    userdata->scanNetworks();
                    userdata->updateList();
                }
            }, LV_EVENT_ALL, this);

    lv_obj_set_pos(btnRefresh, 200, 290);

    btnShowSaved = lv_button_create(cont);
    label = lv_label_create(btnShowSaved);
    lv_label_set_text(label, "Show saved");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnShowSaved, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    userdata->updateList(true);
                    lv_label_set_text(userdata->mainWindowTitle, "Saved WiFi");
                }
            }, LV_EVENT_ALL, this);

    lv_obj_set_pos(btnShowSaved, 10, 290);

    btnClose = lv_button_create(cont);
    label = lv_label_create(btnClose);
    lv_label_set_text(label, "Close");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnClose, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
                    lv_obj_add_flag(userdata->window, LV_OBJ_FLAG_HIDDEN);
                }
            }, LV_EVENT_ALL, this);
    lv_obj_set_size(btnClose, 300, 40);
    lv_obj_set_pos(btnClose, 0, 340);



    
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
    
    btnConnect_Update = lv_button_create(cont);
    lblCntUpdate = lv_label_create(btnConnect_Update);
    lv_label_set_text(lblCntUpdate, "Connect");
    lv_obj_center(lblCntUpdate);
    lv_obj_add_event_cb(btnConnect_Update, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    printf("want to take from here %d\r\n", __LINE__);
                    Utility::safe_lock_mutex_recursive lock(userdata->_mutex, __FILENAME__, __LINE__);
                    LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);;
                    if(userdata->showSaved)
                    {
                        userdata->saveCredentials();
                        lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                    }
                    else 
                    {
                        if (!userdata->connect())
                            lv_label_set_text(userdata->connectWindowTitle, "Failed to connect!");
                        else
                            lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                    }

                }
            }, LV_EVENT_ALL, this);
    
    btnCancel = lv_button_create(cont);
    label = lv_label_create(btnCancel);
    lv_label_set_text(label, "Cancel");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnCancel, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
                    lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                    userdata->selectedSSID = "";
                }
            }, LV_EVENT_ALL, this);

    btnForget = lv_button_create(cont);
    label = lv_label_create(btnForget);
    lv_label_set_text(label, "Forget");
    lv_obj_center(label);
    lv_obj_add_event_cb(btnForget, [](lv_event_t * e) {
                lv_event_code_t code = lv_event_get_code(e);
                WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                if(code == LV_EVENT_CLICKED) {
                    printf("want to take from here %d\r\n", __LINE__);
                    Utility::safe_lock_mutex_recursive lock(userdata->_mutex,__FILENAME__, __LINE__);
                    LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
                    userdata->ForgetNetwork(userdata->selectedSSID);
                    lv_obj_add_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                    userdata->updateList(true);
                    //TODO: disconnect if connected to this network
                }
            }, LV_EVENT_ALL, this);
    
    lv_obj_set_pos(btnCancel, 10, 130);
    lv_obj_set_pos(btnConnect_Update,200 , 130);
    lv_obj_set_size(btnForget, 280, 40);
    lv_obj_set_pos(btnForget, 10, 180);

    selectedSSID = "";
    foundNetworks = 0;

    if (!FileSystemHandler::getInstance().exists("/WiFi"))
    {
        assert(FileSystemHandler::getInstance().mkdir("/WiFi") != true);
        assert(FileSystemHandler::getInstance().exists("/WiFi") != true);
    }

    ssids.clear();
    loadAllSavedNetworks();

    showSaved = false;
    xTaskCreate(
    WiFiTask
    ,  "wifiTsk"
    ,  3000  // Stack size
    ,  this // Pass reference to a variable describing the task number
    ,  1  // Low priority
    ,  &wifiTaskHandler 
    );

}

WIFIConnector::~WIFIConnector()
{
    LockLVGLSafe obj = LockLVGLSafe(__FILENAME__, __LINE__);
    vTaskDelete(wifiTaskHandler);
    OnScreenKeyBoard::getInstance().removeObject(txtPassword);
    lv_obj_delete(window);
    lv_obj_delete(connectWindow);
    vSemaphoreDelete(_mutex);
    _mutex = NULL;
}

void WIFIConnector::show()
{
    LockLVGLSafe obj = LockLVGLSafe(__FILENAME__, __LINE__);
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
    printf("want to take from here %d\r\n", __LINE__);
Utility::safe_lock_mutex_recursive lock(_mutex,__FILENAME__, __LINE__);
    foundNetworks = WiFi.scanNetworks();
}

void WIFIConnector::updateList(bool showSaved)
{   
    lv_obj_t * btn;
    printf("want to take from here %d\r\n", __LINE__);
Utility::safe_lock_mutex_recursive lock(_mutex,__FILENAME__, __LINE__);
    LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
    this->showSaved = showSaved;
    clearList();
    if (showSaved)
    {
        
        for (const auto &obj: ssids)
        {
            btn = lv_list_add_button(wifiList, LV_SYMBOL_WIFI, obj.ssid.c_str());
                lv_obj_add_event_cb(btn, [](lv_event_t * e){
                    lv_event_code_t code = lv_event_get_code(e);
                    lv_obj_t * obj = (lv_obj_t *)lv_event_get_target(e);
                    WIFIConnector * userdata = (WIFIConnector *)lv_event_get_user_data(e);
                    
                    if(code == LV_EVENT_CLICKED) {
                        printf("want to take from here %d\r\n", __LINE__);
                    Utility::safe_lock_mutex_recursive lock(userdata->_mutex,__FILENAME__, __LINE__);
                        LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
                        userdata->selectedSSID = lv_list_get_button_text(userdata->wifiList, obj);
                        lv_label_set_text(userdata->connectWindowTitle, ("Updating : " + userdata->selectedSSID).c_str());
                        const auto ptr = std::find_if(userdata->ssids.begin(), userdata->ssids.end(), [&](const SSIDStruct& t){
                                                                                            return t.ssid == userdata->selectedSSID;});
                        lv_textarea_set_text(userdata->txtPassword, userdata->DecryptPassword(ptr->EncryptedPassword).c_str());
                        lv_obj_set_state (userdata->chkbxAutoReconnect, LV_STATE_CHECKED, ptr->autoconnect);
                        lv_obj_remove_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_remove_flag(userdata->btnForget, LV_OBJ_FLAG_HIDDEN);
                        lv_label_set_text(userdata->lblCntUpdate, "Update");
                    }
                }, LV_EVENT_CLICKED, this);
        }
    }
    else
    {
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
                        printf("want to take from here %d\r\n", __LINE__);
                    Utility::safe_lock_mutex_recursive lock(userdata->_mutex,__FILENAME__, __LINE__);
                        LockLVGLSafe lvglmtx = LockLVGLSafe(__FILENAME__, __LINE__);
                        userdata->selectedSSID = lv_list_get_button_text(userdata->wifiList, obj);
                        lv_label_set_text(userdata->connectWindowTitle, ("Connecting to : " + userdata->selectedSSID).c_str());
                        lv_obj_add_flag(userdata->btnForget, LV_OBJ_FLAG_HIDDEN);
                        lv_obj_remove_flag(userdata->connectWindow, LV_OBJ_FLAG_HIDDEN);
                        lv_label_set_text(userdata->lblCntUpdate, "Connect");
                        lv_textarea_set_text(userdata->txtPassword,"");
                    }
                }, LV_EVENT_CLICKED, this);
            }
        }
    }
}

void WIFIConnector::getCurrent()
{
    
    if (WiFi.isConnected() & !showSaved)
    {
        Utility::safe_lock_mutex_recursive lock(_mutex,__FILENAME__, __LINE__);
        LockLVGLSafe obj = LockLVGLSafe(__FILENAME__, __LINE__);
        
        //TODO: get AP name
         String tmp = "Connected to " + String(WiFi.getHostname());
         lv_label_set_text(mainWindowTitle, tmp.c_str());
    }
    else if (!showSaved)
    {
        LockLVGLSafe obj = LockLVGLSafe(__FILENAME__, __LINE__);
        lv_label_set_text(mainWindowTitle, "Not Connected!");
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
            obj.autoconnect = lv_obj_get_state(chkbxAutoReconnect) & LV_STATE_CHECKED;
            update = true;
            break;
        }
    }
    
    if(!update)
    {
        doc["items"] = ssids.size() + 1;
        SSIDStruct tmp{selectedSSID, encryptedpass, lv_obj_get_state(chkbxAutoReconnect) & LV_STATE_CHECKED};
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
        bool found = false;
        for (SSIDStruct &obj: ssids) 
        {
            if (obj.ssid.equalsIgnoreCase(ssid) && obj.autoconnect )
            {
                found = true;
                txtpass = obj.EncryptedPassword;
                Serial.println("Found network");
                if(WiFi.begin(ssid.c_str(), DecryptPassword(txtpass).c_str()) != WL_CONNECTED)
                    obj.autoconnect = false;
                return;
            }
        }
    }
}

void WIFIConnector::ForgetNetwork(String ssid)
{
    
    static JsonDocument doc;
    doc.clear();
    const auto ptr = std::find_if(ssids.begin(), ssids.end(), [&](const SSIDStruct& t){return t.ssid == ssid;});
    ssids.erase(ptr);

    doc["items"] = ssids.size();
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

void WIFIConnector::WiFiTask(void *params)
{
    WIFIConnector * obj = (WIFIConnector *)(params);
    int counter = 0;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (counter++ >=10)
        {
            obj->scanNetworks();
            counter = 0;
        }
        {
            printf("want to take from here %d\r\n", __LINE__);
            
            if(!WiFi.isConnected())
            {
                Utility::safe_lock_mutex_recursive lock(obj->_mutex,__FILENAME__, __LINE__);
                obj->ConnectToFirstAvailable();
            }
            
            obj->getCurrent();
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
