// g++ -o test_fix test_fix.cpp
#include "iostream"
#include "iomanip"
#include "math.h"
#include "bitset"
#include "fixed_random.h"

using namespace std;

int main(){
    /*秘密值在16bit的环上，计算在32bit的环上，小数点位数为8bit，测试安全乘法*/
    /*测试结论：以无符号的32位整型进行计算，打开数值时便转换为有符号的16位整型并进行浮点数除法，得出带小数的正确计算结果*/
    // cout<<(uint32_t)(-1)<<endl;

    float x = 1.3, y = -3.5; //秘密值x，y
    uint16_t fixed = (uint16_t)1<<(12); //小数位数太多会影响正确性，太少会影响精度
    uint32_t carry = (uint32_t)1<<(16);
    cout<<"fixed: "<<fixed<<", carry: "<<carry<<endl;
    int16_t x_t = x*fixed;
    int16_t y_t = y*fixed;
    uint32_t rn = GenRand32(); //32bit长的随机数
    uint32_t a = (uint16_t)GenRand32(); //生成乘法三元组
    uint32_t b = (uint16_t)GenRand32();
    uint32_t c = a*b;
    //接下来用乘法三元组计算乘法
    uint32_t e = (uint32_t)(x_t-a);
    uint32_t f = (uint32_t)(y_t-b);
    uint32_t e_masked = e+(uint32_t)(rn*carry);
    uint32_t f_masked = f+(uint32_t)(rn*carry);
    cout<<"x_t: "<<x_t<<", y_t: "<<y_t<<endl;
    cout<<"a: "<<a<<", b: "<<b<<", c: "<<c<<endl;
    cout<<"e: "<<e<<", f: "<<f<<endl;
    cout<<"true e: "<<(float)((int16_t)e)/fixed<<", true f: "<<(float)((int16_t)f)/fixed<<endl; //转换为16位整型，这样符号位才会出来
    cout<<"e+rn: "<<e_masked<<", f+rn: "<<f_masked<<endl;
    cout<<"e: "<<(int16_t)e_masked<<", f: "<<(int16_t)f_masked<<endl;
    cout<<"rn: "<<rn<<", rn*carry: "<<(int32_t)(rn*carry)<<endl;
    int32_t z = (e*f+e*b+a*f+c)/fixed;
    //检查验证乘法是否正确
    float true_z0 = x*y;
    cout<<true_z0<<endl;
    float true_z1 = (float)x_t*(float)y_t/(fixed*fixed);
    cout<<true_z1<<endl;
    float true_z2 = (float)((int16_t)z)/fixed;
    cout<<true_z2<<endl;

    /*数据相加转换*/
    uint64_t u0=6740449245;
    uint64_t u1=6740580317;
    uint64_t u=u0+u1;
    cout<<(int64_t)u0<<";;"<<(int64_t)u1<<endl;
    cout<<(int32_t)u<<endl;

    /*数据相乘相加转换*/
    //以计算stau*sysu为例
    uint64_t e_u=26227429;
    uint64_t f_u=4311744507;
    uint64_t eu=(uint32_t)e_u;
    uint64_t fu=(uint32_t)f_u;
    uint64_t a0=18446744073709551608-18446744073709551594, a1=18446744073709551608-2; //a0=14，a1=-10
    uint64_t b0=16777226-16777211, b1=16777226-16777236; //b0=15，b1=-10
    uint64_t c0=30, c1=4294967286-4294967296; //c0=30，c1=-10
    cout<<a0<<";;"<<a1<<";;"<<b0<<";;"<<b1<<";;"<<c0<<";;"<<c1<<endl;
    cout<<"a: "<<(int32_t)(a0+a1)<<" b: "<<(int32_t)(b0+b1)<<" c: "<<(int32_t)(c0+c1)<<endl;
    cout<<"e: "<<(uint32_t)eu<<" f: "<<(uint32_t)fu<<endl;
    uint64_t sysu_stau0=(eu*fu+eu*b0+fu*a0+c0)/(uint32_t)(1<<(fixed_point));
    uint64_t sysu_stau1=      (eu*b1+fu*a1+c1)/(uint32_t)(1<<(fixed_point));
    double true_e=(double)eu/(uint32_t)(1<<(fixed_point));
    double true_f=(double)fu/(uint32_t)(1<<(fixed_point));
    uint64_t true_u=(eu*fu+eu*(b0+b1)+fu*(a0+a1)+c0+c1)/(uint32_t)(1<<(fixed_point));
    u=sysu_stau0+sysu_stau1;
    uint64_t ef=eu*fu/(uint32_t)(1<<(fixed_point));

    cout<<"u0: "<<sysu_stau0<<"; u1: "<<sysu_stau1<<endl;;
    cout<<"u: "<<(int32_t)u<<";;"<<(double)((int32_t)u)/(uint32_t)(1<<(fixed_point))<<endl;
    cout<<"True e: "<<true_e<<" True f: "<<true_f<<";;True u: "<<(double)((int32_t)true_u)/(uint32_t)(1<<(fixed_point))<<endl;
    cout<<"e*f: "<<(int64_t)ef<<" True e*f: "<<(double)(uint32_t)ef/(uint32_t)(1<<(fixed_point))<<endl;

    uint64_t e_u1=63137305;
    uint64_t f_u1=4311744507;
    uint64_t eu1=(uint32_t)e_u1;
    uint64_t fu1=(uint32_t)f_u1;
    uint64_t a10=18446744073709551608-18446744073709551594, a11=26227441-26227451; //a0=14，a1=-10
    uint64_t b10=16777226-16777211, b11=4294967286-4294967296; //b0=15，b1=-10
    uint64_t c10=30, c11=4294967286-4294967296; //c0=30，c1=-10
    cout<<a10<<";;"<<a11<<";;"<<b10<<";;"<<b11<<";;"<<c10<<";;"<<c11<<endl;
    cout<<"a: "<<(int32_t)(a10+a11)<<" b: "<<(int32_t)(b10+b11)<<" c: "<<(int32_t)(c10+c11)<<endl;
    cout<<"e: "<<(uint32_t)eu1<<" f: "<<(uint32_t)fu1<<endl;
    uint64_t sysu_stau10=(eu1*fu1+eu1*b10+fu1*a10+c10)/(uint32_t)(1<<(fixed_point));
    uint64_t sysu_stau11=      (eu1*b11+fu1*a11+c11)/(uint32_t)(1<<(fixed_point));
    double true_e1=(double)eu1/(uint32_t)(1<<(fixed_point));
    double true_f1=(double)fu1/(uint32_t)(1<<(fixed_point));
    uint64_t true_u1=(eu1*fu1+eu1*(b10+b11)+fu1*(a10+a11)+c10+c11)/(uint32_t)(1<<(fixed_point));
    u=sysu_stau10+sysu_stau11;
    uint64_t ef1=eu1*fu1/(uint32_t)(1<<(fixed_point));

    cout<<"u0: "<<sysu_stau10<<"; u1: "<<sysu_stau11<<endl;;
    cout<<"u: "<<(int32_t)u<<";;"<<(double)((int32_t)u)/(uint32_t)(1<<(fixed_point))<<endl;
    cout<<"True e: "<<true_e1<<" True f: "<<true_f1<<";;True u: "<<(double)((int32_t)true_u1)/(uint32_t)(1<<(fixed_point))<<endl;
    cout<<"e*f: "<<(int64_t)ef1<<" True e*f: "<<(double)(uint32_t)ef1/(uint32_t)(1<<(fixed_point))<<endl;

    //以计算rn*(2*(u0-u1)-1)为例
    uint64_t u00=26227458, u01=1099511627750, u10=70, u11=63137238; //u0=1.563277, u1=3.763277
    uint64_t lc0=2*(u00-u10)-(uint32_t)(1<<(fixed_point));
    uint64_t lc1=2*(u01-u11);
    uint64_t lc=lc0+lc1;
    double true_lc=(double)(int32_t)(uint32_t)(lc)/(uint32_t)(1<<(fixed_point));
    
    uint32_t plain_lc = -5.4 * ((uint32_t)(1<<(fixed_point)));
    cout<<"(uint64_t)(-1): "<<(uint64_t)(-1)<<"; lc0: "<<lc0<<"; lc1: "<<lc1<<"; lc: "<<lc<<endl;
    cout<<"True lc: "<<true_lc<<";"<<(double)(int32_t)(uint32_t)(u00-u10+u01-u11)/(uint32_t)(1<<(fixed_point))<<"; Plain lc: "<<plain_lc<<endl;
    // uint64_t a10=18446744073709551608-18446744073709551594, a11=26227441-26227451; //a0=14，a1=-10
    // uint64_t b10=16777226-16777211, b11=4294967286-4294967296; //b0=15，b1=-10
    // uint64_t c10=30, c11=4294967286-4294967296; //c0=30，c1=-10



    return 0;
}