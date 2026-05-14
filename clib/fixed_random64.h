#define fixed_point 4 //小数部分位数

#define fix_rn64(rn) (uint64_t)(rn * (1<<(fixed_point))) //浮点数转换为定点数（以整数表示）
#define fix_rn32(rn) (uint32_t)(rn * (1<<(fixed_point))) //浮点数转换为定点数（以整数表示）

// //generate 32-bit random fixed number
// uint32_t GenRand32(void);

// //generate 64-bit random fixed number
// uint64_t GenRand64(void);