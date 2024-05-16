#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include "LVGLHandler.hpp"
#include "OnScreenKeyboard.hpp"
#include "WifiConnector.hpp"
#include "FileSystemHandler.hpp"



WIFIConnector *obj;
void setup()
{
    Serial.begin(115200);
    LVGLHandler::getInstance().Init();
    OnScreenKeyBoard::getInstance().Init();
    FileSystemHandler::getInstance().InitSDCard();
    WiFi.mode(WIFI_STA);
    obj = new WIFIConnector();
    obj->show();
}

void loop()
{

}
