#include <gmp.h>
#include <gmpxx.h>
#include "/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/clib/libpaillier-0.8/paillier.h"
#include "fixed_random16b.h"
#include <iostream>
#include <math.h>
#include"string"
#include "vector"
#include <algorithm>
#include <random>

using namespace std;

#define sec_param 16 //安全参数
#define ep pow(2, -4) //精度
//#define num_clients 20 //客户总个数
#define fixed (uint8_t)(1<<(4)) //小数位
#define carry (uint32_t)(1<<(sec_param))

typedef struct
{
    int16_t key1;
    paillier_plaintext_t key2;
} MACkey; //MAC密钥

typedef struct
{
    uint32_t share;
} SS; //加性秘密分享

typedef struct
{
    SS value_share;
    SS tag_share;
    uint16_t constant = 0;
} ASS; //可验证秘密分享

typedef struct
{
    int32_t a;
    int32_t b;
    int32_t c;//c=a*b
} Multriplet; //乘法三元组

typedef struct
{
    ASS ashare;
    ASS bshare;
    ASS cshare;
} Multriplet_ASS; //乘法三元组的ASS