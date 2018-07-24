#define DBG_WEBUPDATE_PORT Serial
#define DEBUGLOG_WEBUPDATE(...) DBG_WEBUPDATE_PORT.printf(__VA_ARGS__)
void WebUpdateInit(void) {
  /*=================================================================
    Post data
    =================================================================*/
  server.on("/postdata", HTTP_POST, []() {
    DEBUGLOG_WEBUPDATE("\r\nargName(0): %s", server.argName(0).c_str());
    size_t size = server.arg(0).length();
    DEBUGLOG_WEBUPDATE("\r\nserver.arg(0) %u: %s", (uint16_t)size, server.arg(0).c_str());
    
    if (server.argName(0) == "Auth" && server.argName(1) == "PassId") {
      DynamicJsonBuffer djbpo(size + 1);
      JsonObject& root = djbpo.parseObject(server.arg(0));
      if (!root.success()) {
        DEBUGLOG_WEBUPDATE("\r\nJSON parsing failed!");
        return;
      }
      if (server.arg(1) == "All") {
        if (root["OldPass"] == ConfigFile.AUTH.pass) {
          strncpy(ConfigFile.AUTH.pass, root["NewPass"], Df_LengAuth);
          FS_FileConfig(Df_UpdateConfig); //Update file config
          server.send(200, "text/html", "Đổi mật khẩu đăng nhập thành công");
        } else server.send(200, "text/html", "Mật khẩu đăng nhập cũ sai !");
      }
    }
    
    server.send(200, "text/html", "Post OK");
  });
  /*=================================================================
    Upload file vao bo nho FS
    =================================================================*/
  server.on("/upload", HTTP_POST, []() {
    WebHandleStatus("Upload ESP", "Upload File OK");
  }, handleFileUpload);
  /*=================================================================
    Update Firmware
    =================================================================*/
  server.on("/update", HTTP_POST, []() {
    String Buf = "Update ESP ";
    if (Update.hasError()) Buf += "FAIL";
    else Buf += "OK";
    WebHandleStatus("Update ESP", Buf);
    //ESP.restart();
    ESPRebootTo.ToEUpdate(1000); //Restart esp after 100ms
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      DBG_WEBUPDATE_PORT.setDebugOutput(true);
      WiFiUDP::stopAll();
      DEBUGLOG_WEBUPDATE("\r\nUpdate: %s", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(DBG_WEBUPDATE_PORT);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(DBG_WEBUPDATE_PORT);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        DEBUGLOG_WEBUPDATE("\r\nUpdate Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(DBG_WEBUPDATE_PORT);
      }
      DBG_WEBUPDATE_PORT.setDebugOutput(false);
    }
    yield();
  });
}
