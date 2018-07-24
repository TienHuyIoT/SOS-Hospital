#define ESPNOW_PORT Serial
#define ESPNOW_DBG(fmt, ...) ESPNOW_PORT.printf("\r\n>ESPNOW< " fmt, ##__VA_ARGS__)
#define CONFIG_DBG(fmt, ...) ESPNOW_PORT.printf("\r\n>CONFIG< " fmt, ##__VA_ARGS__)

#define SOS_BUTTON_IO   12 //GPIO
IOInput _IoSos(SOS_BUTTON_IO,HIGH,10,10,10);

/* Định nghĩa trạng thái input của nút nhấn */
#define Df_Button_On      LOW
#define Df_Button_Off     HIGH
/* Định nghĩa struct data cần đồng bộ giữ nút nhấn và bộ trung gian */
typedef struct {
  union {
    uint32_t Val0;
    struct{
      uint16_t Time;  /* Thời gian trạng thái input hiện tại của nút nhấn */
      union {
        uint8_t Val01;
        struct {
          uint8_t Stt : 1;  /* Bit trạng thái input của nút nhấn sau khi lấy mẫu*/
        };
      }_BIT;
    };    
  }_INFO;
} _NOW_DEVICE;

_NOW_DEVICE BUTTON;

/* Địa chỉ Mac của board trung gian */
//uint8_t RemoteMac[]={0x68,0xC6,0x3A,0xAC,0x50,0x8D};  /*Board Trung gian jack nguon 90 do*/
//uint8_t RemoteMac[]={0x68,0xC6,0x3A,0xAC,0x5A,0x8D};  /*Board Trung gian jack nguon binh thuong*/
//uint8_t RemoteMac[]={0x24,0x0a,0xc4,0x0f,0x89,0x28};  /* Board test */
uint8_t RemoteMac[]={0x5C,0xCF,0x7F,0x11,0xA2,0x76};    /* Địa chỉ Mac STA của đích cần truyền */


/* Thời gian (s) trạng thái input on của nút nhấn khi khởi động để cho phép khởi động wifi 
  Mục đích khởi động wifi để board trung gian chạy chế độ quét wifi các board nút nhấn 
  và add thông số (địa chỉ Mac ) bằng trình duyệt web trên board trung gian.

  Cách sử dụng:
  B1. Rút nguồn thiết bị
  B2. Nhẫn giữ nút nhất
  B3. Cắm nguồn thiết bị và vẫn giữ nút nhấn
  B5. Thiết bị sẽ phát ra wifi có tên "THES CO.,LTD BTxxxxxx", xxxxxx là 3 byte cuối địa chỉ mac của thiết bị
  B6. Sau khi cấu hình xong thì nhả nút nhấn, thiết bị sẽ tự khởi động lại để chạy bình thường
*/
TimeOutEvent Config_Button_Toe(2000);
#define TaskButtonSosPowerUpInit      0
#define TaskButtonSosPowerUpOk        1
#define TaskButtonSosPowerUpWaitEnd   2
#define TaskButtonSosPowerUpWaitReset 3
uint8_t TaskButtonSosPowerUp=TaskButtonSosPowerUpInit;
uint8_t Task_Config_Button_Proc(void)
{
  switch (TaskButtonSosPowerUp){
    case TaskButtonSosPowerUpInit:
      /* Nếu thời gian timeout hết thì khởi động espnow */
      if(Config_Button_Toe.ToEExpired())
      {
        CONFIG_DBG("Timeout End");    
        LED_BUTTON.attach_ms(111, 100, 4900);  // Nháy Led 0.2Hz
        EspNow_Init();  /* Khởi động espnow */
        TaskButtonSosPowerUp = TaskButtonSosPowerUpOk;
        break;
      }
      /* Nếu nút nhấn mức thấp */
      if (_IoSos.GetStatus() == LOW)
      {
        /*=================================================================
        Khởi tạo wifi
        =================================================================*/
        WifiInit();
        LED_BUTTON.attach_ms(111, 100, 100);  // Nháy Led 5Hz
        TaskButtonSosPowerUp = TaskButtonSosPowerUpWaitEnd;
        break;
      }
    break;
    case TaskButtonSosPowerUpWaitEnd:
      /* Nếu nút nhấn mức thấp */
      if (_IoSos.GetStatus() == HIGH)
      {
        /*=================================================================
        Reset Esp
        =================================================================*/
        ESPRebootTo.ToEUpdate(10);     
        TaskButtonSosPowerUp = TaskButtonSosPowerUpWaitReset;   
      }
    break;
    case TaskButtonSosPowerUpWaitReset:
    break;
    case TaskButtonSosPowerUpOk:
    break;
  }
  
  return TaskButtonSosPowerUp;
}

/*
- Khi nhấn nút SOS (GPIO5) từ 2s, thiết bị sẽ cập nhật trạng thái nút nhấn "ON" vào bít "BUTTON._INFO._BIT.Stt"
và gửi ngay lập tức trạng thái "OFF" về cho bộ trung gian.
- Khi nhả nút nhấn từ 200ms, thiết bị sẽ cập nhật trạng thái nút nhấn "OFF" vào bít "BUTTON._INFO._BIT.Stt"
và gửi ngay lập tức trạng thái "OFF" về cho bộ trung gian.
- Nếu không có thay đổi trạng thái nút nhấn. Định kì 1s-2s thiết bị sẽ gửi trạng thái "ON/OFF" của nút nhấn về 
cho bộ trung gian.
*/
TimeOutEvent Send_Data_Toe(2000);
void Task_Customer_Button_Proc(void)
{
  /* Luôn đặt hàm quét input đầu tiên để cập nhật trạng input cho chương trinh bên dưới */
  _IoSos.loop();

  /* Nếu task config chưa kết thúc, thì không chạy những chương trình bên dưới */
  if(Task_Config_Button_Proc()!=TaskButtonSosPowerUpOk) return;
  
  /* Timeout send dữ liệu đến bộ trung gian qua espnow */
  if(Send_Data_Toe.ToEExpired()){
    /* Tạo timeout 1s + random 1s để truyền data, 
    mục đích hạn chế trường hợp nhiều thiết bị cùng truyền 1 lần theo chu kì giống nhau */
    Send_Data_Toe.ToEUpdate(2000 + random(1000));
    BUTTON._INFO.Time = _IoSos.GetTime(BUTTON._INFO._BIT.Stt)/1000;
    /*=================================================================
     Board đã được kích hoạt
    =================================================================*/
    if (ActivateByMac == Df_MacActivated){
      esp_now_send(RemoteMac, (uint8_t*)&(BUTTON._INFO.Val0), sizeof(BUTTON._INFO.Val0)); 
    }
  }  
  /* Nếu nút nhấn mức thấp */
  if (_IoSos.GetStatus() == Df_Button_On)
  {
    /* Lấy thời gian nhấn nút */    
    if (BUTTON._INFO._BIT.Stt==Df_Button_Off && _IoSos.GetTime(Df_Button_On) >= 2000)
    {
      ESPNOW_DBG("Nhan Nut goi y ta");
      /* Nháy nhanh Led Button*/
      LED_BUTTON.attach_ms(111, 100, 400);  
      BUTTON._INFO._BIT.Stt = Df_Button_On;   
      Send_Data_Toe.ToEUpdate(random(100));
    }
  }else{
     /* Lấy thời gian nhấn nút */
    if (BUTTON._INFO._BIT.Stt==Df_Button_On && _IoSos.GetTime(Df_Button_Off) >= 200)
    {
      ESPNOW_DBG("Ket thuc goi y ta");
      /* Nháy chậm Led Button*/
      LED_BUTTON.attach_ms(111, 100, 4900);  
      BUTTON._INFO._BIT.Stt = Df_Button_Off;   
      Send_Data_Toe.ToEUpdate(random(100));
    } 
  }
}

// callback when data is recv from Master
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  ESPNOW_DBG("\t\tLast Packet Recv from: %s",macStr);
  ESPNOW_DBG("\t\tLast Packet Recv Data (%d): %s",data_len,(char *)data);
  _NOW_DEVICE Bt;
  memcpy((uint8_t*)&(Bt._INFO.Val0), data, sizeof(Bt._INFO.Val0)); 
  ESPNOW_DBG("Button: %s, Time: %u\r\n",Bt._INFO._BIT.Stt==Df_Button_On ? "On" : "Off", Bt._INFO.Time);
}

// callback when data is sent from Master to Slave
void OnDataSent(uint8_t *mac_addr, uint8_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  ESPNOW_DBG("Last Packet Sent to: %s", macStr);
  ESPNOW_DBG("Last Packet Send Status: %s\r\n",(status == 0) ? "Delivery Success" : "Delivery Fail");
}

void EspNow_Init(void) {
  if (esp_now_init() != 0) {
    ESPNOW_DBG("Init failed.");
    ESP.restart();
  }
  else {
    ESPNOW_DBG("Init ok");    
  }
  // This is the mac address of the Slave in AP Mode
  ESPNOW_DBG("AP Mac: %s", WiFi.softAPmacAddress().c_str());
  ESPNOW_DBG("STA Mac: %s", WiFi.macAddress().c_str());
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  if(esp_now_add_peer(RemoteMac, ESP_NOW_ROLE_CONTROLLER,4, NULL, 0)==0){
    uint8_t *M = RemoteMac;
    ESPNOW_DBG("Pair success: %02X %02X %02X %02X %02X %02X", M[0], M[1], M[2], M[3], M[4], M[5]);
  }
  // Once ESPNow is successfully Init, we will register for Send CB to  
  esp_now_register_recv_cb(OnDataRecv);
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
}

