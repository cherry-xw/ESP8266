
// ********************************************** config **********************************************


void APConfigWiFi () {
  Serial.println("start config wifi");
  IPAddress local_IP(192,168,8,1);
  IPAddress gateway(192,168,8,1);
  IPAddress subnet(255,255,255,0);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  String wifiName = String("config_wifi_") + random(10000,99999);
  if(WiFi.softAP(wifiName.c_str())){
    Serial.println("Ready");
  }else{
    Serial.println("Failed!");
  }
  server.on("/", [](){WebHTML("/index.html");});
//  通用接口
  commonService();
  server.on("/config", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String reqJSON = server.arg("plain"); // 接收json数据
      Serial.println(reqJSON);
      StaticJsonDocument<100> doc; // 创建格式化后的json对象
      DeserializationError err = deserializeJson(doc, reqJSON.c_str()); // 格式化json操作，返回错误信息
      if (err) { // 如果出错，直接退出
        String strErr = err.c_str();
        Serial.println("ERROR: "+strErr);
        server.send(400, "application/json", "{\"msg\":\""+strErr+"\",\"status\":\"error\"}");
        return;
      }
      String pwd = doc["pwd"];
      String ssid = doc["ssid"];
      Serial.println("ssid: " + ssid + " pwd: " + pwd);
      if (pwd.equals("null") || ssid.equals("null") || pwd.equals("") || ssid.equals("")) {
        server.send(400, "application/json", FPSTR(j_no_msg));
        return;
      }
//      提示开始配置，通过观察指示灯的闪烁情况来判断是否正在配置
//      当前正在配置中，指示灯常灭
//      如果配置失败，那么就会重新回到快速闪烁状态
//      配置成功，指示灯慢闪
      server.send(200, "application/json", FPSTR(j_conf_start));
      if (connectWiFi(ssid.c_str(), pwd.c_str())) { // 连接成功
//        将wifi信息保存起来
        strcpy(conf.yfi[conf.next].ssid, ssid.c_str());
        strcpy(conf.yfi[conf.next].pwd, pwd.c_str());
        Serial.println("ssid pwd");
        if (conf.save <= WIFI_MAX) conf.save++;
        conf.next++;
        if (conf.next == WIFI_MAX) conf.next = 0;
        saveE2pConf();
//        当前wifi已经连接成功后，重启ESP
//        delay(2000);
//        ESP.restart();
//        WiFi.mode(WIFI_STA); // 在成功之后切换模式，防止通讯中断
//        status = true;
      } else { // 连接超时
        Serial.println("connect failed");
      }
    }
  });
  server.begin();
  Serial.println(status);
  Serial.println("-------------------------");
  while(!status) {
    led = !led;
    digitalWrite(LED_BUILTIN, led); // 指示灯快速闪烁表示正在等待配置WiFi
    server.handleClient(); // 这个每执行一次，就会执行所有server.on监听的路径
    delay(100); // 增加这个时间，会一定程度增加响应等待时长
  }
}

//尝试连接wifi
boolean connectWiFi (const char* ssid, const char* pwd) {
  WiFi.begin(ssid, pwd);
  int timeout  = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 40) {
    timeout++;
    delay(500);
    Serial.print(F("."));
  }
  if (timeout < 40) {
    Serial.println(F("WiFi connected"));
    Serial.print("current IP is ");
    Serial.println(WiFi.localIP().toString());
    return true;
  } else {
    WiFi.disconnect();
    Serial.println(F("connect timeout"));
    return false;
  }
}
