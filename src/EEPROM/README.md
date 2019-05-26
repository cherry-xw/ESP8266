### EEPROM
1. ESP8266中的EEPROM是用flash模拟出来的
2. 每次存取操作前，使用EEPROM.begin(len);声明，否则可能无法存或取出数据都是0，在操作结束后使用EEPROM.end()将EEPROM开辟的缓存释放掉

[基础操作与结构体操作](https://blog.csdn.net/wubo_fly/article/details/86581233)
结构体操作是使用取址与取值方式完成，使用unit8_t存放每个byte数据

[union共用体](https://blog.csdn.net/sdlgq/article/details/51931900)
可借助union类型完成该功能，但是这种操作方式不支持String，[]等数据类型，可用于结构体、基础数据类型


