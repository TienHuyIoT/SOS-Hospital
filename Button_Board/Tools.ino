/*Copy 1 chuoi ky tu den ky tu cuoi cung 'Chr'*/
boolean StrncpyUntilChar(char *Dest,char *Scr, char Chr, uint16_t Leng){
  uint16_t i;
  for(i=0;i<=Leng;i++){
    if(Scr[i]==Chr){
      strncpy(Dest,Scr,i);  
      Dest[i] = 0;
      return 1;
    }  
  } 
  return 0;   
}

/** IP to long*/
uint32_t Ip2long(IPAddress ip) {
  uint8_t IPbuf[4];
  for (int i = 0; i < 4; i++) {
    IPbuf[i] = ((ip >> (8 * i)) & 0xFF);
  }
  return (*((uint32_t*)IPbuf));
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
