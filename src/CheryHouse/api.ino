
// ***************************************** API  ************************************************
// 获取文件系统中的网页文件
void WebHTML(String path) {
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, "text/html");
  file.close();
}

// 与配置网页使用的通用接口
void commonService () {
  server.on("/scan", HTTP_GET, [](){
    int n = WiFi.scanNetworks();
    String data = "[";
    if (n == 0) {
      server.send(200, "application/json", "{\"msg\":\"around no wifi\",\"status\":\"success\"}");
    } else {
      for (int i = 0; i < n; ++i) {
        data += "{\"signal\":"
          +String(WiFi.RSSI(i))
          +",\"ssid\":\""
          +String(WiFi.SSID(i))
          +"\",\"encryptionType\":"
          +String(WiFi.encryptionType(i))
          +"},";
      }
      data=data.substring(0, data.length() - 1);
      data += "]";
//      wifi加密类型TKIP (WPA) = 2，WEP = 5，CCMP (WPA) = 4，NONE = 7，AUTO = 8
      server.send(200, "application/json", "{\"msg\":\"wifi msgs\",\"data\":"+data+",\"status\":\"success\"}");
    }
  });
}

//  连接成功后添加操作控制监听
void controlListenner (){
  server.on("/", [](){server.send(200, "application/json", FPSTR(j_index));});
  server.on("/d", [](){WebHTML("/index.html");});
}