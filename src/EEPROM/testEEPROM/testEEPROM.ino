#include <EEPROM.h>
// 基本操作 https://blog.csdn.net/naisu_kun/article/details/82915959
// 存取wifi信息 https://www.arduino.cn/thread-45487-1-58.html
// String转char[] https://www.arduino.cn/thread-78392-1-1.html
//union xStr {
//  char
//}
void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  String a = String("A_TTP");
  Serial.println(a);
  Serial.println(a.length());
  char b[a.length()];
  strcpy(b,a.c_str());
  Serial.println(b);
  Serial.println(si(b));
  //申请操作到地址strlen(b)（比如你只需要读写地址为100上的一个字节，该处也需输入参数101），猜测最后一个字符为\0结束符
  EEPROM.begin(strlen(b)+1);
  for(int addr = 0; addr<strlen(b)+1; addr++){
    // write方法是以字节为存储单位的
    EEPROM.write(addr, ); //写数据
  }
  EEPROM.commit(); //保存更改的数据
}

void loop(){
  delay(20000);
}
