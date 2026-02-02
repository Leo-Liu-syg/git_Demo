#include	"SYSCFG.h"
#include 	"FT64F0AX.h" 



#define KEY_PIN     PA0  // 按键引脚（PA0）
#define KEY_PRESSED 0    // 按键按下电平（上拉输入，按下为低）
#define KEY_RELEASED 1   // 按键释放电平
#define DEBOUNCE_MS 20   // 按键消抖时间（20ms）
/**
 * 修复后的非阻塞按键扫描（PA0）
 * 返回值：1=按键按下（消抖后，仅触发一次），0=无按键/未稳定
 */
 
