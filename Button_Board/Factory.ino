/*
Khi nhấn nút Factory (GPIO0) từ 5s-10s thiết bị sẽ factory lại các thông số cấu hình cho wifi
*/

#define FACTORY_PORT Serial
#define FACTORY_DBG(fmt, ...) FACTORY_PORT.printf("\r\n>FACTORY< " fmt, ##__VA_ARGS__)

#define FACTORY_IO 0 //GPIO0
IOInput _IoFactory(FACTORY_IO,HIGH,10,10,10);
void TaskFactoryProc(void)
{
  _IoFactory.loop();
  /* Nếu nút nhấn mức thấp */
  if (_IoFactory.GetStatus() == LOW)
  {
    /* Lấy thời gian nhấn nút */
    uint32_t T = _IoFactory.GetTime(LOW);
    /*Nếu thời gian nút nhấn từ 5s - 10s thì chạy factory*/
    if (T > 5000 && T < 10000)
    {
      /* Cài thời gian nút nhấn mức thấp 10s để ko chạy lại điều kiện này */
      _IoFactory.set_low_time_ms(10000);
      /* Nháy chậm 1s Led status*/
      LED_STT.attach_ms(111, 1000, 1000); 
      FACTORY_DBG("OK");
      strncpy(ConfigFile.AP.ssid, String("THES CO.,LTD BT" + String(ESP.getChipId(), HEX)).c_str(), Df_LengSsid);
      strncpy(ConfigFile.AP.pass, "12345678", Df_LengPass);
      strncpy(ConfigFile.AUTH.user, "admin", Df_LengAuth);
      strncpy(ConfigFile.AUTH.pass, "admin", Df_LengAuth);
      memset(ConfigFile.STA.ssid, 0, Df_LengSsid);
      memset(ConfigFile.STA.pass, 0, Df_LengPass);
      ConfigFile.AP.Chanel = 2;
      ConfigFile.AP.Hidden = 0;
      ConfigFile.STA.SaticIp = 0;
      FS_FileConfig(Df_UpdateConfig);
    }
  }
}