// *************************************** EEPROM ***************************************************


// 最大缓存为4k！！！ 共计4096个byte
// 存储范围为4~4096之间

//重置配置区域数据
void resetE2pConf () {
  Serial.println("reset conf");
  conf.use = CONF_MAX_LENGTH;
  conf.save = 0;
  conf.next = 0;
  saveE2pConf();
}

// 获取配置信息
void readE2pConf () {
  EEPROM.begin(CONF_MAX_LENGTH);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < CONF_MAX_LENGTH; i++){
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.end();
  Serial.println("getConfig");
  Serial.println(conf.use);
  Serial.println(conf.save);
  Serial.println(conf.next);
}

// 保存配置区域数据
void saveE2pConf () {
  Serial.println("save conf");
  EEPROM.begin(CONF_MAX_LENGTH);
  uint8_t *p = (uint8_t*)(&conf);
  for (int i = 0; i < CONF_MAX_LENGTH; i++) {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
  EEPROM.end();
}
