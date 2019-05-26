#include <EEPROM.h>
// 基本操作 https://blog.csdn.net/naisu_kun/article/details/82915959
// 存取wifi信息 https://www.arduino.cn/thread-45487-1-58.html
// String转char[] https://www.arduino.cn/thread-78392-1-1.html

// 最大缓存为4k！！！ 共计4096个byte
#define E2P_LENGTH 23 // e2pconf长度
#define WIFI_MAX 12 // 存储账号数量
struct {
  unsigned int use; // 当前已经使用的EEPROM
  byte save; // 已经保存的wifi数量，最多10个
  byte next; // 当前将被修改的wifi账号，修改完后必须自增一次,如果save小于12，则next为save+1，当save==11时，则在0-6之间循环
  // 8266中的整型int 包含4个字节byte，每个字节8位bit，共计32位
  // 从左往右数32位，每10位存一个账号密码长度信息，其中5位为ssid，5位为pwd，留最前面的两位为空
  // ssid最长为10个中文（30英文数字） 30字节 ->31
  // pwd最长16位字母数字符号， 16字节 ->17
  // 加上eeprom结束符共计48字节
  // 起始配置描述占用20字节，12个wifi信息占用576，总计596字节
  // 0b1001001100100101 & 0b0000000000001111
  // (ssid1<<25)+(pwd1<<20)+(ssid2<<15)+(pwd2<<10)+(ssid3<<5)+pwd3
  // 0b表示使用二进制表示法，这个方式表示取出后四位，可以 十进制 & 二进制 取出十进制结果
  // 从0~11个账户，ssid地址开头依次为51+48*0到11  pwd地址开头依次为68+48* 0到11
  unsigned int a;
  unsigned int b;
  unsigned int c;
  unsigned int d;
} conf;

void setup(){
  Serial.begin(115200);
   resetE2pConf ();
   String a = String("TP-LINK_205A");
  Serial.println("----------");
//   save_string_to_eeprom(10, a);
//   Serial.println(read_string_from_eeprom(10, strlen(a.c_str())));
//  getE2pConf();
//  // 判断是否乱码初始值或为空，以此判断是否重置内容
//  if (conf.use == 0 || conf.save > WIFI_MAX || conf.next > WIFI_MAX || conf.next > conf.save) {
//    resetE2pConf();
//    // 同时设备直接进入配置wifi状态，而不是进入正常运行状态
//  } else {
//    // 根据conf.save值遍历conf. a~g，尝试连接周围wifi
//    // 扫描周围ssid，如果发现相同ssid，直接连接，如果失败或者未找到相同，则继续尝试下一个，如果全部失败，则进入配置模式。
//  }
//  unsigned int ff[2];
//  calStartAddr(0, ff);
//  Serial.println(ff[0]);
//  Serial.println(ff[1]);
}

void loop(){
  delay(50000);
}

// 获取配置信息
void getE2pConf () {
  EEPROM.begin(E2P_LENGTH);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < E2P_LENGTH; i++){
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.end();
  Serial.println("getConfig");
  Serial.println(conf.use);
  Serial.println(conf.save);
  Serial.println(conf.next);
  Serial.println(conf.a);
  Serial.println(conf.a, HEX);
}

//重置配置区域数据
void resetE2pConf () {
  Serial.println("reset");
  conf.use = 596;
  conf.save = 0;
  conf.next = 0;
//  00  10111  10010  11010  01010  00011
  conf.a = 0;
  conf.b = 0;
  conf.c = 0;
  conf.d = 0;
  EEPROM.begin(E2P_LENGTH);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < E2P_LENGTH; i++) {
    EEPROM.write(i, *(p + i));
    Serial.print(*(p + i));
    Serial.print(".");
  }
  EEPROM.commit();
  EEPROM.end();
}

// 仅用来计算当前ssid和pwd长度值
// 传入需要取出信息的项index 和 用指正的方式获取数组返回值
void calStartAddr(int index, unsigned int* len) {
  unsigned int who;
  if(index < 3) {
    who = conf.a;
  } else if (index > 2 && index < 6) {
    who = conf.b;
  } else if (index > 5 && index < 9) {
    who = conf.c;
  } else if (index > 8 && index < 12) {
    who = conf.d;
  }
  len[0] = who >> (2 - index % 3) * 10 + 5 & 31;
  len[1] = who >> (2 - index % 3) * 10 & 31;
  Serial.println(len[0]);
  Serial.println(len[1]);
}

// 起始存储位置， 被存入的String
void save_string_to_eeprom(int addr, String str){
//  申请操作到地址strlen(b)（比如你只需要读写地址为100上的一个字节，该处也需输入参数101），最后一个字符为\0结束符
  int len = strlen(str.c_str()) + addr;
//  将char数组转为byte数组（好像不转换也能存）
  Serial.println(len);
  byte byteArr[len];
  str.getBytes(byteArr, strlen(str.c_str()));
//  缓存读出EEPROM总的数据量
  EEPROM.begin(len);
  for(int i = addr,j=0; i < len; i++,j++){
    EEPROM.write(i, byteArr[j]);
    Serial.print(i);
    Serial.print("_");
//    直接存入char数组
//    EEPROM.write(i, str[i]);
  }
    Serial.println();
  EEPROM.commit();
  EEPROM.end();
}


//  起始坐标，读取数据长度
String read_string_from_eeprom(int addr, int len){
  byte arr[len];
  EEPROM.begin(len+addr);
  for(int i = addr,j=0; j < len; i++,j++){
    arr[j]=EEPROM.read(i);
    Serial.print(i);
    Serial.print(".");
  }
    Serial.println();
  EEPROM.end();
  return String((char *)arr);
}
