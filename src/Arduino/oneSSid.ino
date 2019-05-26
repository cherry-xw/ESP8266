// 该文件主要用于初始化配置
#include <ESP8266WiFi.h>
// 网页服务器，用于接收发送请求
#include <ESP8266WebServer.h>
// 用于格式化JSON字符串
#include <ArduinoJson.h>
// 用于存放成功登陆的WiFi信息，在重启时直接使用该信息，而不再需要重新配置
#include <EEPROM.h>
// 用于将网页信息等存放到SPIFFS文件系统中，让网页信息不再是拼接字符串获取
// https://www.arduino.cn/thread-81675-1-1.html
// https://swf.com.tw/?p=905
#include <FS.h>

// 基础Arduino函数说明
// https://arduino-wiki.clz.me/?file=001-%E6%95%B0%E5%AD%97%E8%BE%93%E5%85%A5%E8%BE%93%E5%87%BA/002-digitalWrite
// dalao的帖子合集
// https://www.arduino.cn/home.php?mod=space&uid=93655&do=thread&view=me&type=reply&order=dateline&from=space
// Arduino引脚说明
// http://www.cnblogs.com/kekeoutlook/p/10165195.html

ESP8266WebServer server(80);



// ********************************** define **********************************************

struct {
  byte key; // 用于是否经过初始化判断
  char ssid[32];
  char pwd[16];
} conf;

boolean status = false; // 当前wifi连接状态
boolean led = true; // 状态信号灯状态







//********************************* run ************************************************

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(); // 启用SPIFFS文档系统
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  readE2pConf();
  // 判断是否乱码初始值或为空，以此判断是否重置内容，并直接进入配置模式
  if (conf.key != 179) {
//    说明是第一次使用
    resetE2pConf();
    // 同时设备直接进入配置wifi状态，而不是进入正常运行状态
    APConfigWiFi();
  } else if (conf.key == 179 && String(conf.ssid).equals("TP-LINK_205A")) {
//    说明使用过，但是未连接
      APConfigWiFi();
  } else {
    // if判断EEPROM中是否有wifi信息，并尝试逐个联网，当全部尝试连接失败后，让用户配置联网
    // 根据conf.save值遍历conf. a~g，尝试连接周围wifi
    // 扫描周围ssid，如果发现相同ssid，直接连接，如果失败或者未找到相同，则继续尝试下一个，如果全部失败，则进入配置模式。
    
    WiFi.mode(WIFI_STA);
    Serial.print("Start auto connect");
    if (!connectWiFi(conf.ssid, conf.pwd)){
      Serial.println("auto connect error");
      APConfigWiFi();
    }
    Serial.println("auto connect done");
  }
  controlListon();
}


void loop() {
  delay(3000);
  led = !led;
  server.handleClient();
  digitalWrite(LED_BUILTIN, led);
}






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

void controlListon (){
//  连接成功后添加操作控制监听
  server.on("/", [](){server.send(200, "application/json", "{\"msg\":\"eeee\",\"status\":\"successss\"}");});
  server.on("/d", [](){WebHTML("/index.html");});
}








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
      String ssid = doc["ssid"];
      String pwd = doc["pwd"];
      Serial.println("ssid: " + ssid + " pwd: " + pwd);
      if (pwd.equals("null") || ssid.equals("null") || pwd.equals("") || ssid.equals("")) {
        server.send(400, "application/json", "{\"msg\":\"ssid or password is empty\",\"status\":\"error\"}");
        return;
      }
//      提示开始配置，通过观察指示灯的闪烁情况来判断是否正在配置
//      当前正在配置中，指示灯常灭
//      如果配置失败，那么就会重新回到快速闪烁状态
//      配置成功，指示灯慢闪
      server.send(200, "application/json", "{\"msg\":\"start config\",\"status\":\"success\"}");
      if (connectWiFi(ssid.c_str(), pwd.c_str())) { // 连接成功
//        将wifi信息保存起来
        strcpy(conf.ssid, ssid.c_str());
        strcpy(conf.pwd, pwd.c_str());
        saveE2pConf();
//        当前wifi已经连接成功后，重启ESP
        delay(4000);
        Serial.println("to restart");
        WiFi.forceSleepBegin();
        wdt_reset();
        ESP.restart();
        while(1)wdt_reset();
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







// *************************************** EEPROM ***************************************************


// 最大缓存为4k！！！ 共计4096个byte
// 存储范围为4~4096之间

void resetE2pConf(){
  Serial.println("reset conf");
  conf.key = 179;
  strcpy(conf.ssid, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  strcpy(conf.pwd, "1111111111111111");
  saveE2pConf();
}

// 获取配置信息
void readE2pConf () {
  EEPROM.begin(49);
  
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < 49; i++){
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.end();
  Serial.println("getConfig");
  Serial.println(conf.ssid);
  Serial.println(conf.pwd);
}

// 保存配置区域数据
void saveE2pConf () {
  EEPROM.begin(49);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < 49; i++) {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
  EEPROM.end();
}
