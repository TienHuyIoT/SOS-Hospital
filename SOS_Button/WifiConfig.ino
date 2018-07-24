#define WIFI_PORT Serial
#define WIFI_DEBUG(fmt, ...) WIFI_PORT.printf("\r\n>Wifi Config< " fmt, ##__VA_ARGS__)

const char * FwVersion = "Firmware V1.0";
const char * HwVersion = "Hardware V1.0";

TimeOutEvent WifiSTATo(600000);
TimeOutEvent WifiSTADisTo(30000);

bool WifiConnect(void) {  
  WIFI_DEBUG("STA Mac: %s", WiFi.macAddress().c_str());  
  uint8_t SsidLeng = strlen(ConfigFile.STA.ssid);
  if (SsidLeng == 0){
    WiFi.disconnect();
    WIFI_DEBUG("No SSID");
    return false;
  }  
  WIFI_DEBUG("Connecting as wifi client %s - %s",ConfigFile.STA.ssid, ConfigFile.STA.pass);
  /*
  if (ConfigFile.STA.SaticIp) {
    WiFi.config(ConfigFile.STA.Ip, ConfigFile.STA.Gw, ConfigFile.STA.Sn, ConfigFile.STA.Dns);
    WIFI_DEBUG("Static IP enable");
    WIFI_DEBUG("Ip: %s", ConfigFile.STA.Ip.toString().c_str());
    WIFI_DEBUG("Gw: %s", ConfigFile.STA.Gw.toString().c_str());
    WIFI_DEBUG("Sn: %s", ConfigFile.STA.Sn.toString().c_str());
    WIFI_DEBUG("Dns: %s", ConfigFile.STA.Dns.toString().c_str());
  }
  */
  WiFi.hostname(ConfigFile.STA.HostName); //Tên đăng ký vào router
  if (strlen(ConfigFile.STA.pass) >= 8) WiFi.begin(ConfigFile.STA.ssid, ConfigFile.STA.pass);
  else WiFi.begin(ConfigFile.STA.ssid);
  WifiSTATo.ToEUpdate(600000ul);  //Tạo lại timeout kết nối
  WifiSTADisTo.ToEUpdate(30000ul);
  return true;
}

wl_status_t wlStatus = WL_IDLE_STATUS;
void WifiConnectStatusProc(void) {
  //for reference:
  //typedef enum {
  //    WL_NO_SHIELD = 255,   // for compatibility with WiFi Shield library
  //    WL_IDLE_STATUS = 0,
  //    WL_NO_SSID_AVAIL = 1,
  //    WL_SCAN_COMPLETED = 2,
  //    WL_CONNECTED = 3,
  //    WL_CONNECT_FAILED = 4,
  //    WL_CONNECTION_LOST = 5,
  //    WL_DISCONNECTED = 6
  //} wl_status_t;
  wl_status_t St = WiFi.status();
  if (wlStatus != St) {
    wlStatus = St;
    WIFI_DEBUG("wlStatus: %u", (uint8_t)St);
    if (St == WL_CONNECTED) {
      LED_STT.attach_ms(111, 100, 900);
      ConfigFile.STA.Ip = WiFi.localIP();
      ConfigFile.STA.Gw = WiFi.gatewayIP();
      ConfigFile.STA.Sn = WiFi.subnetMask();
      ConfigFile.STA.Dns = WiFi.dnsIP();
      if (!ConfigFile.STA.SaticIp) {
        WIFI_DEBUG("Connected to %s OK", ConfigFile.STA.ssid);
        WIFI_DEBUG("Ip: %s", ConfigFile.STA.Ip.toString().c_str());
        WIFI_DEBUG("Gw: %s", ConfigFile.STA.Gw.toString().c_str());
        WIFI_DEBUG("Sn: %s", ConfigFile.STA.Sn.toString().c_str());
        WIFI_DEBUG("Dns: %s", ConfigFile.STA.Dns.toString().c_str());
      }
    } else if (St == WL_NO_SSID_AVAIL) {
      WiFi.disconnect();
      LED_STT.attach_ms(111, 100, 1900);
    }
  }

  if (WifiSTADisTo.ToEExpired()){
    if (St != WL_CONNECTED) WiFi.disconnect();
  }
  
  if (!WifiSTATo.ToEExpired()) return;
  WifiSTATo.ToEUpdate(600000ul);
  if(St == WL_IDLE_STATUS
  || strcmp(WiFi.SSID().c_str(),ConfigFile.STA.ssid) 
  || strcmp(WiFi.psk().c_str(),ConfigFile.STA.pass)) {
    WiFi.disconnect();
    WifiConnect();
  }
}

void WifiInit(void){
  if (ConfigFile.STA.SaticIp) {
    WiFi.mode(WIFI_OFF);
    WiFi.config(ConfigFile.STA.Ip, ConfigFile.STA.Gw, ConfigFile.STA.Sn, ConfigFile.STA.Dns);
    WIFI_DEBUG("Static IP enable");
    WIFI_DEBUG("Ip: %s", ConfigFile.STA.Ip.toString().c_str());
    WIFI_DEBUG("Gw: %s", ConfigFile.STA.Gw.toString().c_str());
    WIFI_DEBUG("Sn: %s", ConfigFile.STA.Sn.toString().c_str());
    WIFI_DEBUG("Dns: %s", ConfigFile.STA.Dns.toString().c_str());
  }
  if (strlen(ConfigFile.AP.ssid) > 0) {
    WiFi.mode(WIFI_AP_STA);
    //Cau hinh AP
    WIFI_DEBUG("Configuring access point %s - %s", ConfigFile.AP.ssid, ConfigFile.AP.pass);
    WiFi.softAPConfig(ConfigFile.AP.Ip, ConfigFile.AP.Ip, ConfigFile.AP.Sn);
    if (strlen(ConfigFile.AP.pass) >= 8) {
      WiFi.softAP(ConfigFile.AP.ssid, ConfigFile.AP.pass, ConfigFile.AP.Chanel, ConfigFile.AP.Hidden);
    } else WiFi.softAP(ConfigFile.AP.ssid);
    delay(500);
    IPAddress myIP = WiFi.softAPIP();
    //In ra AP IP
    WIFI_DEBUG("AP IP address: %s",myIP.toString().c_str());
    Wifi_Enable = true;
  } else {
    WiFi.mode(WIFI_STA);
  }
  if(WifiConnect()==true){
    WifiSTATo.ToEUpdate(120000ul);  //Tạo lại timeout kết nối 2p lan dau tien khoi dong
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      WIFI_DEBUG("Connect: Failed!");
    }
    Wifi_Enable = true;
  }    
}
