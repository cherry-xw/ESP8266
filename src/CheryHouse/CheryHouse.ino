// 该文件主要用于初始化配置
#include <ESP8266WiFi.h>
// 网页服务器，用于接收发送请求
#include <ESP8266WebServer.h>
// 用于格式化JSON字符串
#include "ArduinoJson.h"
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

#define E2P_LENGTH 8 // e2pconf长度
#define WIFI_MAX 8 // 存储账号数量
#define CONF_MAX_LENGTH 392 // 配置+账号最大长度

struct Yfi{
  char ssid[32];
  char pwd[16];
} yfi;

// 8266中的整型int 包含4个字节byte，每个字节8位bit，共计32位
// ssid最长为11个中文（32英文数字）
// pwd最长16位字母数字符号， 16字节
// 加上eeprom结束符共计49字节
// 起始配置描述占用7字节，8个wifi信息占用392，总计399字节
struct {
  unsigned int use; // 当前已经使用的EEPROM
  byte save; // 已经保存的wifi数量，最多10个
  byte next; // 当前将被修改的wifi账号，修改完后必须自增一次,如果save小于8，则next==save，当save==8时，则在0-7之间循环
  struct Yfi yfi[8]; // 保存8个账号
} conf;

boolean status = false; // 当前wifi连接状态
boolean led = false; // 状态信号灯状态


//********************************* flash ************************************************
// https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html
//使用flash存储静态变量降低ram消耗
//定义一个静态字符串
//static const char xyz[] PROGMEM = "This is a string stored in flash. Len = %u";
//调用
//FPSTR(xyz);

static const char j_conf_start[] PROGMEM = "{\"msg\":\"start config\",\"status\":\"success\"}";
static const char j_no_msg[] PROGMEM = "{\"msg\":\"ssid or password is empty\",\"status\":\"error\"}";
static const char j_index[] PROGMEM = "{\"msg\":\"eeee\",\"status\":\"successss\"}";
