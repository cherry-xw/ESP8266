
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

boolean led = false; // 状态信号灯状态

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(); // 启用SPIFFS文档系统
  Serial.println();
//  pinMode(LED_BUILTIN, OUTPUT);
//  if判断EEPROM中是否有wifi信息，并尝试逐个联网，当全部尝试连接失败后，让用户配置联网
  APConfigWiFi();
  WiFi.printDiag(Serial);
//  当前wifi已经连接成功后，重启ESP
//  ESP.restart();
  server.on("/", [](){server.send(200, "application/json", "{\"msg\":\"eeee\",\"status\":\"successss\"}");});
  server.on("/d", [](){WebHTML("/index.html");});
}

void loop() {
  delay(500);
  led = !led;
  server.handleClient();
  digitalWrite(LED_BUILTIN, led);
}

boolean status = false; // 当前wifi连接状态
void APConfigWiFi () {
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
        server.send(400, "application/json", "{\"msg\":\"ssid or password is empty\",\"status\":\"error\"}");
        return;
      }
//      提示开始配置，通过观察指示灯的闪烁情况来判断是否正在配置
//      当前正在配置中，指示灯常灭
//      如果配置失败，那么就会重新回到快速闪烁状态
//      配置成功，指示灯慢闪
      server.send(200, "application/json", "{\"msg\":\"start config\",\"status\":\"success\"}");
      if (connectWiFi(ssid.c_str(), pwd.c_str())) { // 连接成功
        delay(1000);
        WiFi.mode(WIFI_STA); // 在成功之后切换模式，防止通讯中断
        status = true;
      } else { // 连接超时
        Serial.println("connect failed");
      }
    }
  });
  server.begin();
  while(!status) {
    led = !led;
    digitalWrite(LED_BUILTIN, led); // 指示灯快速闪烁表示正在等待配置WiFi
    server.handleClient(); // 这个每执行一次，就会执行所有server.on监听的路径
    delay(100); // 增加这个时间，会一定程度增加响应等待时长
  }
}

// 获取文件系统中的网页文件
void WebHTML(String path) {
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, "text/html");
  file.close();
}

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

// 获取配置信息
void getE2pConf () {
  EEPROM.get(E2P_LENGTH, conf);
}

//重置配置区域数据
void resetE2pConf () {
  Serial.println(conf.loc.save);
  Serial.println(conf.loc.next);
  EEPROM.begin(E2P_LENGTH);
  for(int i = addr; i < len; i++){
    EEPROM.write(i, byteArr[i]);
  }
}

// 起始存储位置， 被存入的String
void save_string_to_eeprom(int addr, String str){
//  申请操作到地址strlen(b)（比如你只需要读写地址为100上的一个字节，该处也需输入参数101），最后一个字符为\0结束符
  int len = strlen(str.c_str()) + 1 + addr;
//  将char数组转为byte数组（好像不转换也能存）
  byte byteArr[len];
  str.getBytes(byteArr, len);
//  EEPROM写入的数据量
  EEPROM.begin(len);
  for(int i = addr; i < len; i++){
    EEPROM.write(i, byteArr[i]);
//    直接存入char数组
//    EEPROM.write(i, str[i]);
  }
  EEPROM.commit();
}


//  起始坐标，读取数据长度
String read_string_from_eeprom(int addr, int len){
  byte arr[len];
  for(int i = addr; i < len+addr; i++){
    arr[i]=EEPROM.read(i);
  }
  return String((char *)arr);
}

