#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <FS.h>
#include <espnow.h>
#include "TimeOutEvent.h"
#include "IOBlink.h"
#include "IOInput.h"
#include "FileConfig.h"

#define COM_PORT Serial
#define COM_DEBUG(fmt, ...) COM_PORT.printf("\r\n" fmt, ##__VA_ARGS__)

/*=================================================================
  Khởi động Timeout Reboot ESP
  =================================================================*/
TimeOutEvent ESPRebootTo(0);

/*=================================================================
  Khởi tạo Object Blink IO cho Led trạng thái
  =================================================================*/
#define _LED_PIN 2
IOBlink LED_STT(_LED_PIN, LOW);

/*=================================================================
  Khởi tạo Object Blink IO cho Led Button
  =================================================================*/
#define _BUTTON_LED_PIN 14
IOBlink LED_BUTTON(_BUTTON_LED_PIN, HIGH);

/*=================================================================
  Khởi tạo Webserver, Websocket Object
  =================================================================*/
ESP8266WebServer server(25123);
ESP8266WebServer server80(80);
WebSocketsServer webSocket = WebSocketsServer(25124);
/* Biến cho phép khởi động wifi.
Bình thường sẽ off wifi, khi cài đặt mới cho phép on wifi
 Cách sử dụng để on wifi:
  B1. Rút nguồn thiết bị
  B2. Nhẫn giữ nút nhất
  B3. Cắm nguồn thiết bị và vẫn giữ nút nhấn
  B4. Đếm Led nút nhấn nháy 5 lần (tương ứng 5s) thì nhả nút nhấn ra
  B5. Thiết bị sẽ phát ra wifi có tên "THES CO.,LTD BTxxxxxx", xxxxxx là 3 byte cuối địa chỉ mac của thiết bị
*/
bool Wifi_Enable = false; 

void setup()
{
  /*=================================================================
     - Khởi tạo Serial
     - Load cau hinh tu FS
    =================================================================*/
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  
  WiFi.disconnect();

  COM_DEBUG("Hello ESP8266 Fans!");
  EepromConfig(Df_ReadEep);
  SPIFFS.begin();
  FS_FileConfig(Df_ReadConfig); //Load thong so tu FS
  FS_FileConfigHidden(Df_ReadConfigHidden);
  /*=================================================================
     Khởi tạo wifi
     - Hàm này được khởi tạo tùy thuộc trạng thái input trong Task_Config_Button_Proc()
    =================================================================*/
  //WifiInit();
  /*=================================================================
     Khởi tạo Hostname local
     Mo trinh duyet va go HostName.local/ để truy cập vào server
     New Lib 2.4.0 không phân biêt chữ hoa và chữ thường
    =================================================================*/
  if (!MDNS.begin(ConfigFile.STA.HostName))
  {
    COM_DEBUG("Error setting up MDNS responder!");
  }
  else
  {
    COM_DEBUG("mDNS responder started");
    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("http", "tcp", ConfigFile.TCPPort);
    MDNS.addService("ws", "tcp", ConfigFile.WSPort);
  }
  /*=================================================================
     Khởi tạo webserver và websocket
    =================================================================*/
  webSocket.begin();
  /*
    Load New Port. Gọi hàm này sau khi begin()
    Add in WebSocketsServer.cpp
    void WebSocketsServer::NewPort(int port) {
        _server->begin(port);
    }
    Add in WebSocketsServer.h
    void NewPort(int port = 80);
    */
  if (ConfigFile.TCPPort <= 80)
    ConfigFile.TCPPort = 25123;
  if (ConfigFile.WSPort <= 80)
    ConfigFile.WSPort = 25124;
  if (ConfigFile.WSPort == ConfigFile.TCPPort)
    ConfigFile.WSPort++;
  if (ConfigFile.WSPort != 25124)
    webSocket.NewPort(ConfigFile.WSPort);
  COM_DEBUG("Init Web Socket Port: %u", ConfigFile.WSPort);
  webSocket.onEvent(webSocketEvent);
  WebServerInit();
  WebUpdateInit();
  server.begin(ConfigFile.TCPPort);
  /*
    Load New Port. Gọi hàm này sau khi begin()
    Add in ESP8266WebServer.cpp
    void ESP8266WebServer::NewPort(int port) {
        _server.begin(port);
    }
    Add in ESP8266WebServer.h
    virtual void NewPort(int port = 80);
    server.NewPort(ConfigFile.TCPPort);
    */
  COM_DEBUG("Init Web Server Port: %u", ConfigFile.TCPPort);
  /*=================================================================
    Khi truy cap bang port 80 thi tu forward den port TCP da cai dat
    =================================================================*/
  server80.onNotFound([]() {
    server80.sendHeader("Location", String("http://") + toStringIp(server80.client().localIP()) + ":" + String(ConfigFile.TCPPort) + server80.uri(), true);
    server80.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server80.client().stop();             // Stop is needed because we sent no content length
  });
  server80.begin(); //mac dinh port 80
  /*=================================================================
     Khởi tạo timer led blink
    =================================================================*/
  LED_STT.attach_ms(111, 100, 900);    //Khoi tao timer blink led
  LED_BUTTON.attach_ms(0, 500, 500);    //Khoi tao timer blink led
  /*=================================================================
     Check Activated
    =================================================================*/
  ConfigActivated(Df_ConfigActiveRead);  
}

void loop()
{
  /*=================================================================
     Web Socket Loop
    =================================================================*/
  webSocket.loop();
  /*Timelive Ws toàn bộ các Page*/
  WsTimeLive();
  /*=================================================================
     Web Server
    =================================================================*/
  server.handleClient();
  server80.handleClient();
  /*=================================================================
     Blink Led
    =================================================================*/
  LED_STT.Blink();
  LED_BUTTON.Blink();
  /*=================================================================
     Manager Wifi
    =================================================================*/
  if(Wifi_Enable==true) WifiConnectStatusProc();
  /*=================================================================
    Button Factory
  =================================================================*/
  TaskFactoryProc();
  /*=================================================================
    Button SOS
  =================================================================*/
  Task_Customer_Button_Proc();
  /*=================================================================
     Timeout Reset Esp
    =================================================================*/
  if (ESPRebootTo.ToEExpired())
  {
    COM_DEBUG("Esp Rebooting...");
    delay(100);
    ESP.restart();
  }
}
