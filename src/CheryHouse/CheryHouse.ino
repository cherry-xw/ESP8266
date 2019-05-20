
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <FS.h>

ESP8266WebServer server(80);

boolean led = false; // 状态信号灯状态

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  APConfigWiFi();
  WiFi.printDiag(Serial);
}

void loop() {
  delay(500);
  led = !led;
  digitalWrite(LED_BUILTIN, led);
}

boolean status = false; // 当前wifi连接状态
void APConfigWiFi () {
  IPAddress local_IP(192,168,8,1);
  IPAddress gateway(192,168,8,1);
  IPAddress subnet(255,255,255,0);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  if(WiFi.softAP("config_wifi")){
    Serial.println("Ready");
  }else{
    Serial.println("Failed!");
  }
  server.on("/", HTTP_ANY, []() {
    server.send(200, "text/html", "<!DOCTYPE html><head><meta charset=utf8 /><title>登录</title></head><body><div><label for=ssid>SSID:</label><input id=ssid /></div><div><label for=pwd>pwd:</label><input id=pwd /></div><div><button onclick=displayDate()>确定</button></div><script>var oAjax = null;try{oAjax = new XMLHttpRequest();}catch(e){oAjax = new ActiveXObject('Microsoft.XMLHTTP');}function displayDate(){var postData = '{\"pwd\":\"'+document.getElementById('pwd').value+'\",\"ssid\":\"'+document.getElementById('ssid').value+'\"}';oAjax.open('post','http://192.168.8.1/config?='+Math.random(),true);oAjax.setRequestHeader('Content-type','application/json');oAjax.send(postData);}oAjax.onreadystatechange = function(){if(oAjax.readyState == 4){try{alert(oAjax.responseText);}catch(e){alert('你访问的页面出错了');};};}</script></body></html>");
  });
  server.on("/config", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      char* reqJSON = (char*)server.arg("plain").c_str(); // 接收json数据
      Serial.println(reqJSON);
      StaticJsonDocument<200> doc; // 创建格式化后的json对象
      DeserializationError err = deserializeJson(doc, reqJSON); // 格式化json操作，返回错误信息
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
      if (connectWiFi(ssid, pwd)) { // 连接成功
        delay(1000);
        server.stop();
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

boolean connectWiFi (String ssid, String pwd) {
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