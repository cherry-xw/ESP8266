#include <SoftwareSerial.h>  //引用库文件
SoftwareSerial DLSerial(6, 7);  // 定义软串口 RX(插到D6口), TX(插到D7口)

void setup() {
  //其它操作就跟Serial一样了
  //如：
//  DLSerial.begin(9600);
  Serial.begin(9600);
  Serial.println("begin");
}

void loop() {
//    while (DLSerial.available() > 0) {
//      Serial.print("data=");
//      Serial.println(DLSerial.read(), HEX);
//      delay(2);
//    }
    delay(1400);
    Serial.println("-");
}
//DLSerial.read();
//DLSerial.write();
//
//DLSerial.print();
//DLSerial.println();
