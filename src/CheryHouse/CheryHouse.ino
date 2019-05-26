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

#define E2P_LENGTH 9 // e2pconf长度
#define WIFI_MAX 8 // 存储账号数量
#define EACH_YFI_LENGTH 55 // 每个账号+密码长度
#define CONF_MAX_LENGTH 491 // 配置+账号最大长度
struct {
  unsigned int use; // 当前已经使用的EEPROM
  byte save; // 已经保存的wifi数量，最多10个
  byte next; // 当前将被修改的wifi账号，修改完后必须自增一次,如果save小于8，则next==save，当save==8时，则在0-7之间循环
  // 8266中的整型int 包含4个字节byte，每个字节8位bit，共计32位
  // ssid最长为11个中文（32英文数字）
  // pwd最长16位字母数字符号， 16字节
  // 加上eeprom结束符共计49字节
  // 起始配置描述占用7字节，8个wifi信息占用392，总计399字节
} conf;

struct {
  char ssid[32];
  char pwd[16];
} yfi;

boolean status = false; // 当前wifi连接状态
boolean led = false; // 状态信号灯状态






//********************************* run ************************************************

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(); // 启用SPIFFS文档系统
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
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
    for (i=0;i<conf.save;i++){
      Serial.print("i times=");
      Serial.println(i);
//      每次获取一个账号密码
      readE2pYfi(i);
      Serial.println(yfi.ssid);
      Serial.println(yfi.pwd);
      for (j=0;j<n;j++){
        Serial.print("j times=");
        Serial.println(j);
        if (String(WiFi.SSID(i)).equals(String(yfi.ssid))){ // 如果周围wifi与保存wifi存在相同名称，尝试一次联网
          if(connectWiFi(yfi.ssid, yfi.pwd)) break; // 如果连接成功直接结束for循环
        }
      }
    }
    Serial.println("auto connect done");
    if (i == conf.save){ // 如果全部尝试后还是连接失败，启动配置模式
      Serial.println("no yfi connect success");
      APConfigWiFi();
    }
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
      if (!connectWiFi(ssid.c_str(), pwd.c_str())) { // 连接成功
//        将wifi信息保存起来
        strcpy(yfi.ssid, ssid.c_str());
        strcpy(yfi.pwd, pwd.c_str());
        Serial.println("ssid pwd");
        saveE2pYfi(conf.next);
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
    return false;
  } else {
    WiFi.disconnect();
    Serial.println(F("connect timeout"));
    return false;
  }
}







// *************************************** EEPROM ***************************************************


// 最大缓存为4k！！！ 共计4096个byte
// 存储范围为4~4096之间

// 获取配置信息
void readE2pConf () {
  EEPROM.begin(E2P_LENGTH);
  int len = sizeof(conf);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < len; i++){
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.end();
  Serial.println("getConfig");
  Serial.println(conf.use);
  Serial.println(conf.save);
  Serial.println(conf.next);
}

//重置配置区域数据
void resetE2pConf () {
  Serial.println("reset conf");
  conf.use = CONF_MAX_LENGTH;
  conf.save = 0;
  conf.next = 0;
  saveE2pConf();
}

// 保存配置区域数据
void saveE2pConf () {
  int len = sizeof(conf);
  Serial.println("save conf");
  EEPROM.begin(E2P_LENGTH);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < len; i++) {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
  EEPROM.end();
}


// 存取wifi账号密码
void readE2pYfi (byte index) {
  int start = index * EACH_YFI_LENGTH + E2P_LENGTH;
  int len = sizeof(yfi);
  Serial.println(start);
  EEPROM.begin(CONF_MAX_LENGTH);
  uint8_t *p = (uint8_t*)(&yfi);
  for (int i = 0,j=0; j < len+start; i++,j++){
//    *(p + j) = EEPROM.read(i);
    Serial.print(EEPROM.read(i));
    Serial.print("_");
  }
  EEPROM.end();
}
void saveE2pYfi (byte index) {
  Serial.println(yfi.ssid);
  Serial.println(yfi.pwd);
  Serial.println(index);
  int start = index * EACH_YFI_LENGTH + E2P_LENGTH;
  int len = sizeof(yfi);
  Serial.print("yfi len = ");
  Serial.println(len);
  EEPROM.begin(CONF_MAX_LENGTH);
  uint8_t *p = (uint8_t*)(&yfi);
  for (int i = start,j=0; j < len; i++,j++) {
    Serial.print(yfi.ssid[j]);
    Serial.print(".");
    EEPROM.write(i, *(p + j));
  }
  EEPROM.commit();
  EEPROM.end();
}
