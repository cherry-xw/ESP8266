## Arduino基础语法
[https://www.ncnynl.com/archives/201606/19.html](https://www.ncnynl.com/archives/201606/19.html)

## 各类芯片查询
[https://www.findchips.com/](https://www.findchips.com/)

## 运存不足控制
[https://vmaker.tw/archives/13258](https://vmaker.tw/archives/13258)

```
# 查看当前内存，仅用于Arduino
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
```
