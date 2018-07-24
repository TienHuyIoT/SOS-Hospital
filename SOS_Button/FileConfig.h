#ifndef	_FILE_CONFIG_H
#define _FILE_CONFIG_H

#define DBG_FS_PORT Serial
#define DEBUGLOG_FS(fmt, ...) DBG_FS_PORT.printf("\r\n>File Config< " fmt, ##__VA_ARGS__)

typedef enum {
  P_NULL  = (uint8_t)0, /*!< No Page*/
  P_HOME  = (uint8_t)1, /*!< Page home.htm display*/
  P_INDEX = (uint8_t)2, /*!< Page homt.htm Fn*/
} PAGE_TAG_TypeDef;

//holds the current upload
File fsUploadFile;

#define Df_NumFileOriginal  11
char * FilsOriginal[] = {
  (char*)"/config.txt",    //1
  (char*)"/edit.htm.gz",   //2
  (char*)"/graphs.js.gz",  //3
  (char*)"/favicon.ico",   //4
  (char*)"/home.htm",      //5
  (char*)"/index.htm",     //6
  (char*)"/update.htm",    //7
  (char*)"/doimatkhau.htm",      //9
  (char*)"/Reset.htm",      //10
  (char*)"/confighidden.txt"    //11
};

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

void FsFormat(void) {
  uint8_t i;
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    DEBUGLOG_FS("FS File: %s, size: %s", fileName.c_str(), formatBytes(fileSize).c_str());
    for (i = 0; i < Df_NumFileOriginal; i++) {
      if (!strcmp(fileName.c_str(), FilsOriginal[i])){
        DEBUGLOG_FS("Exsit");
        break;
      }
    }
    if (i == Df_NumFileOriginal) {
      //Xoa file
      SPIFFS.remove(fileName);
      DEBUGLOG_FS("Remove");
    }
  }
}

#define Df_LengHostName       32
#define Df_LengSsid           32
#define Df_LengPass           64
#define Df_LengAuth           10
#define Df_LengAddr           100
#define Df_LengSsidPeer       32
#define Df_LengMacPeer        17
typedef struct{
	uint32_t 		Adr;          /*Địa chỉ Node thiết bị*/
  uint16_t    DisTo;        /*Thời gian gửi dữ liệu qua web socket*/
	uint16_t 		UDPPort;      /*UDP Port*/
	uint16_t 		TCPPort;      /*TCP Port*/
  uint16_t    WSPort;       /*Websocket Port*/
  struct{
    char    ssid[Df_LengSsidPeer + 1];  /* Wifi Peer */
    char    Mac[Df_LengMacPeer + 1];  /* Địa chỉ mac peer*/
  }PEER;
  struct{
    char    Name[Df_LengAddr + 1];  /* Tên thiết bị */
    char    Addr[Df_LengAddr + 1];  /* Địa chỉ lắp đặt thiết bị*/
  }ADDR;
  struct{
    char    user[Df_LengAuth + 1];  /*User đăng nhập*/
    char    pass[Df_LengAuth + 1];  /*Pass đăng nhập*/
  }AUTH;
	struct{		
		char 		ssid[Df_LengSsid + 1];
		char 		pass[Df_LengPass + 1];
    char    HostName[Df_LengHostName+1];
		uint8_t 	SaticIp;
		IPAddress 	Ip;
		IPAddress 	Gw;
		IPAddress 	Sn;
		IPAddress 	Dns;
	}STA;
	struct{		
		char 		ssid[Df_LengSsid + 1];
		char 		pass[Df_LengPass + 1];
    char    DnsName[Df_LengHostName+1];
    IPAddress   Ip;
    IPAddress   Sn;
		uint8_t 	Chanel : 4;
		uint8_t 	Hidden : 1;   /*Hiển thị SSID. (1)-hiện, (0)-ẩn*/
	}AP;	
}CONFIGFILE;
CONFIGFILE ConfigFile;

#define Df_ReadConfig   	0
#define Df_UpdateConfig  	1
#define ParthConfig		"/config.txt"
const char* ConfigFileInit =
"{\"I2CDeveice\":2,"
"\n\"SsidPeer\":\"TienHuy\","
"\n\"MacPeer\":\"00:00:00:00:00:00\","
"\n\"NameDev\":\"NÚT NHẤN GỌI Y TÁ\","
"\n\"AddrDev\":\"200 Cộng Hòa, Phường 15, Quận Tân Bình, Tp HCM\","
"\n\"DisTo\":150,"  
"\n\"UDPPort\":25123,"	
"\n\"TCPPort\":25123,"
"\n\"WSPort\":25124,"
"\n\"AuthUser\":\"admin\","
"\n\"AuthPass\":\"admin\","
"\n\"SaticIp\":0,"
"\n\"Ip\":\"192.168.1.100\","
"\n\"Gw\":\"192.168.1.1\","
"\n\"Sn\":\"255.255.255.0\","
"\n\"Dns\":\"192.168.1.1\","
"\n\"STASsid\":\"\","
"\n\"STAPass\":\"\","
"\n\"STAName\":\"thes\","
"\n\"APSsid\":\"THES CO.,LTD BT\","
"\n\"APPass\":\"12345678\","
"\n\"APName\":\"dientuungdung.com\","
"\n\"APIp\":\"192.168.4.1\","
"\n\"APSn\":\"255.255.255.0\","
"\n\"Chanel\":2,"
"\n\"Hidden\":0}";

void FS_FileConfig(uint8_t Cmd) {
  File Fs_Config;
  if (!SPIFFS.exists(ParthConfig)) {    
    Fs_Config = SPIFFS.open(ParthConfig, "w");
    Fs_Config.printf(ConfigFileInit);
    Fs_Config.close();
  }

  if (Cmd == Df_UpdateConfig) {
    DynamicJsonBuffer djbco;
    JsonObject& root = djbco.createObject();
    root["NameDev"] = ConfigFile.ADDR.Name;
    root["AddrDev"] = ConfigFile.ADDR.Addr;
    root["SsidPeer"] = ConfigFile.PEER.ssid;
    root["MacPeer"] = ConfigFile.PEER.Mac;
    root["I2CDeveice"] = ConfigFile.Adr;
    root["DisTo"] = ConfigFile.DisTo;
    root["UDPPort"] = ConfigFile.UDPPort;
    root["TCPPort"] = ConfigFile.TCPPort;
    root["WSPort"] = ConfigFile.WSPort;
    root["AuthUser"] = ConfigFile.AUTH.user;
    root["AuthPass"] = ConfigFile.AUTH.pass;
    root["SaticIp"] = ConfigFile.STA.SaticIp;
    root["Ip"] = ConfigFile.STA.Ip.toString();
    root["Gw"] = ConfigFile.STA.Gw.toString();
    root["Sn"] = ConfigFile.STA.Sn.toString();
    root["Dns"] = ConfigFile.STA.Dns.toString();
    root["STASsid"] = ConfigFile.STA.ssid;
    root["STAPass"] = ConfigFile.STA.pass;
    root["STAName"] = ConfigFile.STA.HostName;
    root["APSsid"] = ConfigFile.AP.ssid;
    root["APPass"] = ConfigFile.AP.pass;
    root["APName"] = ConfigFile.AP.DnsName;
    root["APIp"] = ConfigFile.AP.Ip.toString();
    root["APSn"] = ConfigFile.AP.Sn.toString();
    root["Chanel"] = ConfigFile.AP.Chanel;
    root["Hidden"] = ConfigFile.AP.Hidden;    
    
    Fs_Config = SPIFFS.open(ParthConfig, "w");
    root.prettyPrintTo(Fs_Config);
    Fs_Config.close();    
    return;
  }

  Fs_Config = SPIFFS.open(ParthConfig, "r");
  size_t size = Fs_Config.size();
  std::unique_ptr<char[]> ConfigBuf(new char[size+1]);
  Fs_Config.readBytes(ConfigBuf.get(),size);
  Fs_Config.close();

  DynamicJsonBuffer djbpo(size + 1);
  JsonObject& root = djbpo.parseObject(ConfigBuf.get());
  if (!root.success()) {
    DEBUGLOG_FS("JSON parsing failed!");
    return;
  }
  DEBUGLOG_FS("JSON parsing OK!");
  strncpy(ConfigFile.ADDR.Name,root["NameDev"],Df_LengAddr); 
  strncpy(ConfigFile.ADDR.Addr,root["AddrDev"],Df_LengAddr);
  strncpy(ConfigFile.PEER.ssid,root["SsidPeer"],Df_LengSsidPeer); 
  strncpy(ConfigFile.PEER.Mac,root["MacPeer"],Df_LengMacPeer);
  ConfigFile.DisTo = root["DisTo"];
  ConfigFile.Adr = root["I2CDeveice"];
	ConfigFile.UDPPort = root["UDPPort"];
	ConfigFile.TCPPort = root["TCPPort"];
  ConfigFile.WSPort = root["WSPort"];
  strncpy(ConfigFile.AUTH.user,root["AuthUser"],Df_LengAuth); 
  strncpy(ConfigFile.AUTH.pass,root["AuthPass"],Df_LengAuth);  
	ConfigFile.STA.SaticIp = root["SaticIp"];
	
	ConfigFile.STA.Ip.fromString(root["Ip"].as<String>());
  ConfigFile.STA.Gw.fromString(root["Gw"].as<String>());
  ConfigFile.STA.Sn.fromString(root["Sn"].as<String>());
  ConfigFile.STA.Dns.fromString(root["Dns"].as<String>());
	
	strncpy(ConfigFile.STA.ssid,root["STASsid"],Df_LengSsid); 
	strncpy(ConfigFile.STA.pass,root["STAPass"],Df_LengPass); 
  strncpy(ConfigFile.STA.HostName,root["STAName"],Df_LengHostName); 
	strncpy(ConfigFile.AP.ssid,root["APSsid"],Df_LengSsid); 
  /* Nếu AP name không có địa chỉ mac */
  if(strstr(ConfigFile.AP.ssid, String(ESP.getChipId(), HEX).c_str())==0){
    strncat(ConfigFile.AP.ssid, String(ESP.getChipId(), HEX).c_str(),Df_LengSsid-strlen(ConfigFile.AP.ssid));
  }
	strncpy(ConfigFile.AP.pass,root["APPass"],Df_LengPass);
  strncpy(ConfigFile.AP.DnsName,root["APName"],Df_LengHostName); 
  ConfigFile.AP.Ip.fromString(root["APIp"].as<String>());
  ConfigFile.AP.Sn.fromString(root["APSn"].as<String>());
	ConfigFile.AP.Chanel = root["Chanel"];
	ConfigFile.AP.Hidden = root["Hidden"];  	 
}

#define Df_LengMacAuth          10
typedef struct{ 
  struct{   
    char    Auth[Df_LengMacAuth + 1];
  }MAC;  
}CONFIGFILEHIDDEN;
CONFIGFILEHIDDEN ConfigFileHidden;
#define Df_ReadConfigHidden     0
#define Df_UpdateConfigHidden   1
#define ParthConfigHidden   "/confighidden.txt"
const char* ConfigHiddenFileInit =
"{\"MacAuth\":\"1234\","
"\n\"NC\":0}";

void FS_FileConfigHidden(uint8_t Cmd) {
  File Fs_Config;
  if (!SPIFFS.exists(ParthConfigHidden)) {    
    Fs_Config = SPIFFS.open(ParthConfigHidden, "w");
    Fs_Config.printf(ConfigHiddenFileInit);
    Fs_Config.close();
  }

  if (Cmd == Df_UpdateConfigHidden) {
    DynamicJsonBuffer djbco;
    JsonObject& root = djbco.createObject();
    root["MacAuth"] = ConfigFileHidden.MAC.Auth;
    root["NC"] = 0;     
    Fs_Config = SPIFFS.open(ParthConfigHidden, "w");
    root.prettyPrintTo(Fs_Config);
    Fs_Config.close();    
    return;
  }

  Fs_Config = SPIFFS.open(ParthConfigHidden, "r");
  size_t size = Fs_Config.size();
  std::unique_ptr<char[]> ConfigBuf(new char[size+1]);
  Fs_Config.readBytes(ConfigBuf.get(),size);
  Fs_Config.close();

  DynamicJsonBuffer djbpo(size + 1);
  JsonObject& root = djbpo.parseObject(ConfigBuf.get());
  if (!root.success()) {
    DEBUGLOG_FS("JSON parsing failed!");
    return;
  }
  strncpy(ConfigFileHidden.MAC.Auth,root["MacAuth"],Df_LengMacAuth);    
}

/*=================================================================
    EEPROM
    =================================================================*/
#define DBG_EEPROM_PORT Serial
#define DEBUGLOG_EEPROM(fmt, ...) DBG_EEPROM_PORT.printf("\r\n>Eeprom Config< " fmt, ##__VA_ARGS__)
typedef struct{  
  union{
    uint32_t Block0;
    struct{
      uint8_t AutoConfig;
      uint8_t Activated;
    };
  }; 
  union{
    uint32_t Block1_2[2];
    struct{
      byte mac[6];  
    };
  };  
}EEPSTR;
EEPSTR EepStr;
#define Df_MacUnActivate     0
#define Df_MacActivated      1
uint8_t ActivateByMac = Df_MacUnActivate;
#define Df_ReadEep     0
#define Df_UpdateEep   1
void EepromConfig(uint8_t Cmd) {
  EEPROM.begin(512);
  if(Cmd == Df_ReadEep) EEPROM.get(0, EepStr);
  if(Cmd == Df_UpdateEep) EEPROM.put(0, EepStr);
  DEBUGLOG_EEPROM("Eeprom.AutoConfig: %u", EepStr.AutoConfig);
  DEBUGLOG_EEPROM("Eeprom.Activated: %u", EepStr.Activated);
  if(EepStr.AutoConfig!=128){
    EepStr.AutoConfig = 128;
    EEPROM.write(0,EepStr.AutoConfig);
  }  
  EEPROM.end();
}

#define Df_ConfigActiveRead       0
#define Df_ConfigActiveUpdate     1
#define Df_ConfigUnActiveUpdate   2
void ConfigActivated(uint8_t Cmd){
  byte mac[6];
  WiFi.macAddress(mac);
  DEBUGLOG_EEPROM("Mac: %02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  if(Cmd==Df_ConfigActiveUpdate) memcpy(EepStr.mac,mac,sizeof(EepStr.mac));
  if(Cmd==Df_ConfigUnActiveUpdate) memset(EepStr.mac,0xff,sizeof(EepStr.mac));
  //Nếu địa chỉ Mac không được activated
  if(memcmp(EepStr.mac,mac,sizeof(EepStr.mac))) ActivateByMac = Df_MacUnActivate;
  else ActivateByMac = Df_MacActivated;
  DEBUGLOG_EEPROM("ActivateByMac: %u", ActivateByMac);
  if(Cmd != Df_ConfigActiveRead) EepromConfig(Df_UpdateEep);
}
#endif
