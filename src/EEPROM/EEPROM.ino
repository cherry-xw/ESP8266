#include <EEPROM.h>
// 基本操作 https://blog.csdn.net/naisu_kun/article/details/82915959
// 存取wifi信息 https://www.arduino.cn/thread-45487-1-58.html
// String转char[] https://www.arduino.cn/thread-78392-1-1.html

// 最大缓存为4k！！！ 共计4096个byte
#define E2P_LENGTH 31 // e2pconf长度
union e2pConf {
  // 总长为30byte
  struct {
    byte save; // 已经保存的wifi数量，最多7个
    byte next; // 当前将被修改的wifi账号，修改完后必须自增一次,如果save小于7，则next为save+1，当save==7时，则在0-6之间循环
    // 整形包含四个字节，每个字节8位，共计32位
    // 从左往右数1-10位ssid开始位置11-16ssid长度17-26pwd开始位置27-32pwd长度
    // 10位表示空间为0-1024 6位表示空间为0-64
    // ssid最长为10个中文 30字节
    // pwd最长16位字母数字符号， 16字节
    // 加上eeprom结束符共计47字节
    // 起始配置描述占用31字节，7个wifi信息占用329，总计360字节，未超过1024
    // 0b1001001100100101 & 0b0000000000001111
    // (ssid_pos<<22)+(ssid_len<<16)+(pwd_pos<<6)+pwd_len
    // 0b表示使用二进制表示法，这个方式表示取出后四位，可以 十进制 & 二进制 取出十进制结果
    unsigned int a;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int e;
    unsigned int f;
    unsigned int g;
  } loc;
  byte buffer[31];
} conf;

void setup(){
  Serial.begin(115200);
  // String a = String("我a的v速.度_$%#!玩");
  Serial.println(a);
  Serial.println("----------");
  // save_string_to_eeprom(0, a);

  // Serial.println(read_string_from_eeprom(0, strlen(a.c_str()) + 1));
  getE2pConf();
  // 判断是否乱码初始值，以此判断是否重置内容
  if (conf.loc.save > 7 || conf.loc.next > 7 || conf.loc.next > conf.loc.save) {
    resetE2pConf();
    // 同时设备直接进入配置wifi状态，而不是进入正常运行状态
  } else {
    // 根据conf.loc.save值遍历conf.loc. a~g，尝试连接周围wifi
    // 扫描周围ssid，如果发现相同ssid，直接连接，如果失败或者未找到相同，则继续尝试下一个，如果全部失败，则进入配置模式。
  }
}

void loop(){
  delay(5000);
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
