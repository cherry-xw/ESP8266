
//********************************* run ************************************************

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(); // 启用SPIFFS文档系统
  Serial.println();
//  pinMode(LED_BUILTIN, OUTPUT);
  readE2pConf();
  // 判断是否乱码初始值或为空，以此判断是否重置内容，并直接进入配置模式
  if (conf.use != CONF_MAX_LENGTH || conf.save > WIFI_MAX || conf.next > WIFI_MAX || conf.next > conf.save) {
    resetE2pConf();
    // 同时设备直接进入配置wifi状态，而不是进入正常运行状态
    APConfigWiFi();
  } else {
    // if判断EEPROM中是否有wifi信息，并尝试逐个联网，当全部尝试连接失败后，让用户配置联网
    // 根据conf.save值，尝试连接周围wifi
    // 扫描周围ssid，如果发现相同ssid，直接连接，如果失败或者未找到相同，则继续尝试下一个，如果全部失败，则进入配置模式。
    
    WiFi.mode(WIFI_STA);
    byte n = WiFi.scanNetworks(); // 扫描所有的WiFi信息
    byte i,j;
    Serial.print("Start auto connect, scan=");
    Serial.println(n);
    boolean breakAll = true;
    for (i=0;i<conf.save && breakAll;i++){
      Serial.print("i times=");
      Serial.println(i);
      Serial.println(conf.yfi[i].ssid);
      Serial.println(conf.yfi[i].pwd);
      for (j=0;j<n;j++){
        Serial.println("=============");
        Serial.print("j times=");
        Serial.println(j);
        Serial.println(WiFi.SSID(j));
        Serial.println(String(WiFi.SSID(j)).equals(String(conf.yfi[i].ssid)));
        if (String(WiFi.SSID(j)).equals(String(conf.yfi[i].ssid))) { // 如果周围wifi与保存wifi存在相同名称，尝试一次联网
          Serial.println("find equal wifi ssid");
          if(connectWiFi(conf.yfi[i].ssid, conf.yfi[i].pwd)) {
            breakAll = false;
            break; // 如果连接成功直接结束for循环
          }
        } else {
          Serial.println("+");
        }
      }
    }
    Serial.print(i);
    Serial.print("=i, conf.save=");
    Serial.println(conf.save);
    if (i == conf.save){ // 如果全部尝试后还是连接失败，启动配置模式
      Serial.println("no yfi connect success");
      APConfigWiFi();
    }
  }
  server.begin();
  controlListenner();
}


void loop() {
  delay(3000);
  led = !led;
  server.handleClient();
  digitalWrite(LED_BUILTIN, led);
}






