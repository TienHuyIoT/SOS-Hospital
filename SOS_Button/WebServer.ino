#define DBG_WEBSERVER_PORT Serial
#define DEBUGLOG_WEBSERVER(...) DBG_WEBSERVER_PORT.printf(__VA_ARGS__)

const char HTTP_HEAD[] PROGMEM            = "<html lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:100%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#4CAF50;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
//Style button
const char HTTP_STYLEB[] PROGMEM          = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:100%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#4CAF50;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}"
    ".switch {position: relative;display: inline-block;width: 60px;height: 34px;}.switch input {display:none;}.slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;}.slider:before {position: absolute;content: \"\";height: 26px;width: 26px;left: 4px;bottom: 4px;background-color: white;-webkit-transition: .4s;transition: .4s;}input:checked + .slider {background-color: #2196F3;}input:focus + .slider {box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before {-webkit-transform: translateX(26px);-ms-transform: translateX(26px);transform: translateX(26px);}/* Rounded sliders */.slider.round {border-radius: 34px;}.slider.round:before {border-radius: 50%;}</style>";

const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";

const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='post' action='wlansave'>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br><input id='s' name='n' length=32 placeholder='SSID'><br><input id='p' name='p' length=64 type='password' placeholder='password'><br>";
const char HTTP_FORM_END[] PROGMEM        = "<br><br><br><button type='submit'>save</button></form>";
const char HTTP_WLAN_REFRESH[] PROGMEM       = "<br><div class=\"c\"><a href=\"/wlanconf\">Scan</a></div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

uint8_t PrintPumpIndex;
uint8_t Pu3PumpIndex;

bool WebAuthCheck(char *User, char *Pass){
  if(!server.authenticate(User, Pass) && !server.authenticate("admin", "25123")){
    server.requestAuthentication();
    return 0;
  } 
  return 1; 
}

void WebServerInit(void) {
  /*=================================================================
    List directory
    =================================================================*/
  server.on("/list", HTTP_GET, handleFileList);
  /*=================================================================
    Load editor
    =================================================================*/
  server.on("/edit", HTTP_GET, []() {
    if (!WebAuthCheck((char*)"admin", (char*)"25123")) return;
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  /*=================================================================
    Create file
    =================================================================*/
  server.on("/edit", HTTP_PUT, handleFileCreate);
  /*=================================================================
    Delete file
    =================================================================*/
  server.on("/edit", HTTP_DELETE, handleFileDelete);

  /*=================================================================
    First callback is called after the request has ended with all parsed arguments
    Second callback handles file uploads at that location
    =================================================================*/
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  /*=================================================================
    Wifi config
    =================================================================*/
  server.on("/wifi", HTTP_GET, WMhandleInfo); // Show wifi info STA and AP
  server.on("/wlanscan", HTTP_GET, WMhandleSSIDScan); // Scan ssid
  server.on("/wlanconf", HTTP_GET, WMhandleWlan); // Select Wifi, enter pass and confirm
  server.on("/wlansave", HTTP_POST, WMhandleWlanSave); // Save ssid and pass
  server.on("/wlanconfAd", WMhandleWlanAd); // Advance config STA Wifi 
  server.on("/softapconf", HTTP_GET, WMhandleSAP); // AP wifi config
  server.on("/sapsave", HTTP_POST, WMhandleSAPSave); // AP wifi save

  /*=================================================================
    Reset ESP
    =================================================================*/
  server.on("/reset", HTTP_GET, []() {
    // Load Web timeout reset ESP
    if (!handleFileRead("/Reset.htm")) server.send(404, "text/plain", "FileNotFound");
    ESPRebootTo.ToEUpdate(100); //Reset ESP after 100ms
  });

  /*=================================================================
    Load favicon.ico từ FS
    =================================================================*/
  server.on("/favicon.ico", []() {
    if (!handleFileRead(server.uri())) server.send(404, "text/plain", "FileNotFound");
  });

  /*=================================================================
    Activated
    /activated?cmd=1/0  set, 1:Enable,  0:Disable
    /activated?cmd=<khac 0/1> read
    =================================================================*/
  server.on("/activated", []() {
    if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
    if (server.argName(0) == "cmd"){
      uint8_t Cmd = atoi(server.arg(0).c_str());
      if(Cmd==0 || Cmd==1){ 
        if(Cmd==1) ConfigActivated(Df_ConfigActiveUpdate); 
        if(Cmd==0) ConfigActivated(Df_ConfigUnActiveUpdate); 
      } else ConfigActivated(Df_ConfigActiveRead); 
      server.send(200, "text/html", "Vaule: " + String(ActivateByMac));
    }
  });

  /*=================================================================
    
    =================================================================*/
  server.on("/SetMac",HTTP_POST, []() {
    if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
    if (server.argName(0) != "Pass" || server.arg(0)!=ConfigFileHidden.MAC.Auth){
      server.send(200, "text/json", "{\"St\":\"Lỗi Pass\"}");
      return;           
    }
    if (server.argName(1) == "Ssid" && server.argName(2)=="Mac"){
      strncpy(ConfigFile.PEER.ssid,server.arg(1).c_str(),Df_LengSsidPeer); 
      strncpy(ConfigFile.PEER.Mac,server.arg(2).c_str(),Df_LengMacPeer);
      FS_FileConfig(Df_UpdateConfig); //Update file config
      server.send(200, "text/html", "Lưu Mac " + server.arg(2) + " thành công"); 
    }else{
      server.send(200, "text/json", "{\"St\":\"Lỗi Json\"}");
    }  
  });
  
  /*=================================================================
    Called when the url is not defined here
    Use it to load content from SPIFFS
    =================================================================*/
  server.onNotFound([]() {
    if(server.uri()!="/favicon.icon"){
      if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return; 
    }    
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  /*=================================================================
    get heap status, analog input value and all GPIO statuses in one json call
    =================================================================*/
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
}


/*=================================================================
  Lựa chọn cài đặt thông số STA và AP Wifi
  =================================================================*/
void WMhandleInfo() {
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  if ((server.client().remoteIP() & 0xFFFFFF) == (Ip2long(ConfigFile.AP.Ip) & 0xFFFFFF)){
    page += String("<p>Đang kết nối Wifi thiết bị: <b>") + ConfigFile.AP.ssid + "</b></p>";   
  } else {
    page += String("<p>Đang kết nối Wifi Nhà: <b>") + ConfigFile.STA.ssid + "</b></p>";
  }      
  page += F("<table><tr><th align='left'>Wifi thiết bị</th></tr>");
  page += ("<tr><td>Tên Wifi: " + String(ConfigFile.AP.ssid) + "</td></tr>");
  page += ("<tr><td>Địa chỉ IP: " + WiFi.softAPIP().toString() + "</td></tr>");
  page += F(
            "</table>"
            "<br>"
            "<table><tr><th align='left'>Wifi Nhà</th></tr>"
          );
  page += ("<tr><td>Tên Wifi: " + String(ConfigFile.STA.ssid) + "</td></tr>");
  if(ConfigFile.STA.SaticIp) page += ("<tr><td>IP tĩnh: " + WiFi.localIP().toString() + "</td></tr>");  
  else page += ("<tr><td>IP động: " + WiFi.localIP().toString() + "</td></tr>");  
  wl_status_t s = WiFi.status();
  if (s == WL_CONNECTED) page += ("<tr><td>Trạng thái kết nối: thành công</td></tr>");
  else page += ("<tr><td>Trạng thái kết nối: thất bại</td></tr>");

  page += F("</table>");

  page += F("<br><form action=\"/wlanscan\" method=\"get\"><button>Tìm và kết nối Wifi Nhà</button></form>");
  page += F("<form action=\"/wlanconfAd\" method=\"get\"><button>Cài đặt Wifi Nhà</button></form>");
  page += F("<form action=\"/softapconf\" method=\"get\"><button>Cài đặt Wifi thiết bị</button></form>");  
  page += F("<form action=\"/reset\" method=\"get\"><button>Khởi động lại thiết bị</button></form>");
  page += FPSTR(HTTP_END);

  server.send(200, "text/html", page);
}

/*=================================================================
  Lựa chọn wifi, nhập pass và xác nhận
  B1: ./wlanscan để chạy hàm SSIDScan() tìm wifi sau đó 3s sẽ tự chuyển sang B2
  B2: ./wlanconf đây là giao diện để chọn wifi và nhập mật khẩu. Bấm save để chuyển sang B3
  B3: ./wlansave  lưu cấu hình
  =================================================================*/
/*Chuyen muc tin hieu qua %*/
int getRSSIasQuality(int RSSI) {
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}
//Tìm wifi
String SsidWlan = ""; //Chuỗi toàn cục
void SSIDScan() {
  boolean       _removeDuplicateAPs     = true;
  int           _minimumQuality         = -1;
  SsidWlan = "";
  int n = WiFi.scanNetworks();
  if (n == 0) {
    SsidWlan += F("No networks found. Refresh to scan again.");
  } else {
    //sort networks
    int indices[n];
    for (int i = 0; i < n; i++) {
      indices[i] = i;
    }

    // RSSI SORT
    // old sort
    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          std::swap(indices[i], indices[j]);
        }
      }
    }

    // remove duplicates ( must be RSSI sorted )
    if (_removeDuplicateAPs) {
      String cssid;
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        cssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          if (cssid == WiFi.SSID(indices[j])) {
            indices[j] = -1; // set dup aps to index -1
          }
        }
      }
    }

    //display networks in page
    for (int i = 0; i < n; i++) {
      if (indices[i] == -1) continue; // skip dups
      int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

      if (_minimumQuality == -1 || _minimumQuality < quality) {
        String item = FPSTR(HTTP_ITEM);
        String rssiQ;
        rssiQ += quality;
        item.replace("{v}", WiFi.SSID(indices[i]));
        item.replace("{r}", rssiQ);
        if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
          item.replace("{i}", "l");
        } else {
          item.replace("{i}", "");
        }
        SsidWlan += item;
        delay(0);
      } else {
        //DEBUG_WM(F("Skipping due to quality"));
      }
    }
  }
}
//Link: ./wlanscan
void WMhandleSSIDScan() {
  if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
  DEBUGLOG_WEBSERVER("Scan Wifi SSID");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Scan Wifi SSID");
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_SCRIPT);
  page += F("<br>Đang quét WIFI<br>"
            "<p>Tự động kết nối lại sau <span id=\"counter\">3</span> giây.</p>"
            "<script type=\"text/javascript\">"
            "function countdown() {"
            "var i = document.getElementById('counter');"
            "if (parseInt(i.innerHTML)==0) {"
            "location.href = './wlanconf';"
            "}"
            "i.innerHTML = parseInt(i.innerHTML)-1;"
            "}"
            "setInterval(function(){ countdown(); },1000);"
            "</script>"
           );
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_END);
  server.send(200, "text/html", page);
  SSIDScan();
  DEBUGLOG_WEBSERVER("\r\nScan Wifi SSID Finish");
}
//Link: ./wlanconf
void WMhandleWlan() {
  if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Wlan Config");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  page += SsidWlan;
  SsidWlan = "";  //Reset chuỗi toàn cục
  page += F("<form method='post' action='wlansave'>");
  page += F("<br><input id='s' name='n' type='text' length=32 placeholder='Tên Wifi'>");
  page += F("<br><input id='p' name='p' length=64 type='text' placeholder='Mật khẩu Wifi'><br>");
  page += F("<br><button type='submit'>Save</button></form>");
  page += F("<br><div class=\"c\"><a href=\"/wlanscan\">Tìm lại Wifi</a></div>");
  page += FPSTR(HTTP_END);
  server.send(200, "text/html", page);
}
//Lưu thông số STA
void WMhandleWlanSave() {
  server.arg("n").toCharArray(ConfigFile.STA.ssid, Df_LengSsid);
  server.arg("p").toCharArray(ConfigFile.STA.pass, Df_LengPass);
  DEBUGLOG_WEBSERVER("\r\nSSID: %s", ConfigFile.STA.ssid);
  DEBUGLOG_WEBSERVER("\r\nPASS: %s\r\n", ConfigFile.STA.pass);

  WebHandleStatus("AP Config", "Cài đặt wifi OK");
  //Truong hop ssid = 0 nghia la khong ket noi den wlan
  //Cập nhật thông số vào FS
  FS_FileConfig(Df_UpdateConfig);
  //Tạo lại kết nối wifi
  WiFi.disconnect();
  WifiConnect();
}

/*=================================================================
  Cấu hình STA Wifi chuyên sâu
  =================================================================*/
void WMhandleWlanAd(){
  if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
  String alert="";
  if(server.method()==HTTP_POST){
    for (uint8_t i=0; i<server.args(); i++){
      String argN = server.argName(i);
      if(argN=="Ip") ConfigFile.STA.Ip.fromString(server.arg(i));
      if(argN=="Gw") ConfigFile.STA.Gw.fromString(server.arg(i));
      if(argN=="Sn") ConfigFile.STA.Sn.fromString(server.arg(i));
      if(argN=="Dns") ConfigFile.STA.Dns.fromString(server.arg(i));
      if(argN=="En") ConfigFile.STA.SaticIp = atoi(server.arg(i).c_str());
      if(argN=="Up") ConfigFile.UDPPort = atoi(server.arg(i).c_str());
      if(argN=="Tp") ConfigFile.TCPPort = atoi(server.arg(i).c_str());
      if(argN=="Wp") ConfigFile.WSPort = atoi(server.arg(i).c_str());
      if(argN=="Ssid") server.arg(i).toCharArray(ConfigFile.STA.ssid, Df_LengSsid);
      if(argN=="Pw") server.arg(i).toCharArray(ConfigFile.STA.pass, Df_LengPass);
    }
    //Cập nhật thông số vào FS
    FS_FileConfig(Df_UpdateConfig);
    alert = F("<script type=\"text/javascript\">alert(\"Cài đặt thành công\");</script>");
  }
  
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Wifi Station Config");
  page += FPSTR(HTTP_SCRIPT);
  page += alert;
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);

  page += F("<center><h3>Cài đặt Wifi nhà</h3></center>");
  page += F("<form method='post' action='wlanconfAd'>");
  page += F("<table>");
  
  page += F("<tr><td align='left'>Tên Wifi</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Ssid' length=32 value=\"");
  page += ConfigFile.STA.ssid; 
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>Mật khẩu</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Pw' length=64 value=\"");
  page += ConfigFile.STA.pass;
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>Địa chỉ IP</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Ip' length=15 value=\"");
  page += ConfigFile.STA.Ip.toString(); 
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>Gateway</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Gw' length=15 type='text' value=\"");
  page += ConfigFile.STA.Gw.toString(); 
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>Subnet</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Sn' length=15 type='text' value=\"");
  page += ConfigFile.STA.Sn.toString(); 
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>DNS</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Dns' length=15 type='text' value=\"");
  page += ConfigFile.STA.Dns.toString(); 
  page += F("\"></td></tr>");
  
  page += F("<tr><td align='left'>IP động</td>");
  if(!ConfigFile.STA.SaticIp)page += F("<td align='right'><select name='En'><option value=\"0\" selected>Enable</option><option value=\"1\">Disable</option></select>");
  else page += F("<td align='right'><select name='En'><option value=\"0\">Enable</option><option value=\"1\" selected>Disable</option></select>");
  page += F("</td></tr>");
  page += F("</table>");
  
  page += F("<br><center><h3>Port Setting</h3></center>");
  page += F("<table><tr><td align='left'>UDP Port</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Up' length=5 value=\"");
  page += String(ConfigFile.UDPPort); 
  page += F("\"></td></tr>");
  page += F("<tr><td align='left'>WS Port</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Wp' length=5 value=\"");
  page += String(ConfigFile.WSPort); 
  page += F("\"></td></tr>");
  page += F("<tr><td align='left'>TCP Port</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='Tp' length=5 value=\"");
  page += String(ConfigFile.TCPPort); 
  page += F("\"></td></tr></table>");

  page += F("<br><button type='submit'>Save</button></form>");
  page += F("<form action=\"/reset\" method=\"get\"><button>Khởi động lại Wifi</button></form>");
  page += FPSTR(HTTP_END);
  server.send(200, "text/html", page);
}

/*=================================================================
  Cấu hình wifi thiết bị
  =================================================================*/
void WMhandleSAP(){
  if (!WebAuthCheck(ConfigFile.AUTH.user, ConfigFile.AUTH.pass)) return;
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "SoftAP Config");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);

  page += F("<br><center><h3>Cài đặt Wifi</h3></center>");
  page += F("<form method='post' action='sapsave'>");
  page += F("<table><tr><td align='left'>Đặt tên Wifi</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='n' length=32 value=\"");
  page += ConfigFile.AP.ssid; 
  page += F("\"></td></tr>");
  page += F("<tr><td align='left'>Mật khẩu Wifi</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='p' length=64 type='text' value=\"");
  page += ConfigFile.AP.pass; 
  page += F("\"></td></tr></table>");
  
  page += F("<br><br><center><h3>Advanced Setting</h3></center>");
  page += F("<table><tr><td align='left'>Channel(1-13)</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='c' length=2 value=\"");
  page += String(ConfigFile.AP.Chanel); 
  page += F("\"></td></tr>");
  page += F("<tr><td align='left'>Hidden(0/1)</td>");
  page += F("<td align='right'><input style=\"text-align:right;\" name='h' length=1 value=\"");
  page += String(ConfigFile.AP.Hidden); 
  page += F("\"></td></tr></table>");
  
  page += F("<br><button type='submit'>Save</button></form>");
  page += FPSTR(HTTP_END);
  server.send(200, "text/html", page);
}
void WMhandleSAPSave() {
  server.arg("n").toCharArray(ConfigFile.AP.ssid, Df_LengSsid);
  server.arg("p").toCharArray(ConfigFile.AP.pass, Df_LengPass);
  ConfigFile.AP.Chanel = atoi(server.arg("c").c_str());
  if(ConfigFile.AP.Chanel==0 || ConfigFile.AP.Chanel>13) ConfigFile.AP.Chanel = 2;
  ConfigFile.AP.Hidden = atoi(server.arg("h").c_str());
  if(ConfigFile.AP.Hidden>1) ConfigFile.AP.Hidden = 0;
  DEBUGLOG_WEBSERVER("\r\nSSID: %s", ConfigFile.AP.ssid);
  DEBUGLOG_WEBSERVER("\r\nPASS: %s", ConfigFile.AP.pass);
  DEBUGLOG_WEBSERVER("\r\nChanel: %u", ConfigFile.AP.Chanel);
  DEBUGLOG_WEBSERVER("\r\nHidden: %u", ConfigFile.AP.Hidden);
  WebHandleStatus("AP Config","Cài đặt wifi OK");
  //Lưu vào FS
  FS_FileConfig(Df_UpdateConfig);
}

/*=================================================================
  Web server trả lời bằng alert
  WebHandleStatus("Nhãn tiêu đề", "Nội dung alert");
  =================================================================*/
void WebHandleStatus(String Title, String Text) {
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", Title);
  page += "<script type=\"text/javascript\">alert(\"" + Text + "\");</script>";
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  page += "<center><h2>" + Text + "</h2>";
  page += F("</center>");
  page += FPSTR(HTTP_END);
  server.send(200, "text/html", page);
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  DEBUGLOG_WEBSERVER("\r\nhandleFileRead: %s", path.c_str());
  if (path.endsWith("/")) path += "home.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit" && server.uri() != "/upload") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    DEBUGLOG_WEBSERVER("\r\nhandleFileUpload Name: %s", filename.c_str());
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DEBUGLOG_WEBSERVER("handleFileUpload Data: %lu",upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    DEBUGLOG_WEBSERVER("\r\nhandleFileUpload Size: %lu", upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DEBUGLOG_WEBSERVER("handleFileDelete: %s", path.c_str());
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DEBUGLOG_WEBSERVER("handleFileCreate: %s", path.c_str());
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DEBUGLOG_WEBSERVER("handleFileList: %s", path.c_str());
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
