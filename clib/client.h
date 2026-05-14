#ifndef setup //具体的头文件名
#define setup
#include"setup.h"
#endif

#include "ctime"

class Client
{
private:
    uint32_t sta_u = 0; //统计效用值
    uint32_t rn = 8; //随机数
    ASS sssta_u0; //客户自己的统计效用的份额
    ASS sssta_u1; //对方客户的统计效用的份额
    ASS sssys_u0; //客户自己的系统效用的份额
    ASS sssys_u1; //对方客户的系统效用的份额
    ASS ss_u0; //客户自己的效用值的份额
    ASS ss_u1; //对方客户的效用值的份额
    Multriplet_ASS ssMT; //调度员分配的乘法三元组的秘密分享
    ASS ssrn_c; //coordinator与客户之间秘密分享的随机数的份额，用于验证时掩盖上32位
    ASS ssUP; //更新阶段的计算结果
    ASS ssEP[6]; //开发阶段的中间计算结果
    ASS sslc; //比较结果
    ASS ssEPcheck; //用于开发阶段计算结果的验证
    uint64_t EPcheck=0; //用于开发阶段计算结果的验证
public:
    paillier_ciphertext_t ct_bate;
    double duration; //响应所需时间
    bool isexploited=false; //在上一轮是否被选择参与训练来开发数据价值

    // Client(double loss){
    //     this->sta_u=(int32_t)(loss * (uint32_t)(1<<(fixed_point-1)));
    // }

    void GetRandom(){
        rn = GenRand32();
    }

    void GetRandom_ASS(int64_t random, uint32_t mackey){
        ssrn_c.value_share.share=(int64_t)random/2; //随机数的可验证秘密分享，用于验证时掩盖上32位
        ssrn_c.tag_share.share=(int64_t)(random*mackey/2); //随机数的可验证秘密分享，用于验证时掩盖上32位
    }

    void GetstaU(double loss){
        sta_u = (uint32_t)(loss * fixed);
    }

    void GetstaU_TU(int r, int l){
        double tu=sqrt(0.1*log(r)/l);
        //cout<<"rounds: "<<r<<", last involved round: "<<l<<",temporal uncertainty: "<<tu<<endl;
        sta_u = sta_u + (uint32_t)(tu * fixed);
    }

    //客户生成关于自己的统计效用的份额与标签的份额
    void Secret_sharing_C(paillier_ciphertext_t * ct, paillier_pubkey_t * pk){
        clock_t start,end;
        start=clock();

        uint64_t rn_c = (uint64_t)rn*fixed;
        //cout<<"sssta_u0.value_share.share"<<sssta_u0.value_share.share<<endl;
        paillier_ciphertext_t ct_rn;
        paillier_ciphertext_t ct_bate_stau;
        paillier_plaintext_t pai_rn;
        paillier_plaintext_t pai_sta_u;
        mpz_init_set_d(ct_rn.c, 2); //初始化密文变量，不然会报错
        mpz_init_set_d(ct_bate_stau.c, 2); //初始化密文变量，不然会报错
        //SS sssta_u2;
        //sssta_u2.share = sta_u+rn;

        mpz_init_set_si(pai_rn.m, rn_c);
        //cout<<sta_u<<endl;
        mpz_init_set_si(pai_sta_u.m, sta_u);
        paillier_enc(&ct_rn, pk, &pai_rn, paillier_get_rand_devurandom);
        paillier_exp(pk, &ct_bate_stau, &ct_bate, &pai_sta_u);
        paillier_mul(pk, ct, &ct_bate_stau, &ct_rn);

        end=clock();
        // cout<<"time of client's Secret_sharing_C = "<<(double(end-start)/CLOCKS_PER_SEC)<<endl;
        //return sssta_u2;
    }

    SS Reshare_value(void){ //将自己的统计效用的秘密份额分享出去
        clock_t start,end;
        start=clock();

        int64_t rn_c = (int64_t)rn*fixed;
        SS sssta_u1;
        sssta_u0.value_share.share = -rn_c;
        sssta_u0.tag_share.share = -rn_c;
        sssta_u1.share = (int64_t)(sta_u+rn_c);
        // cout<<"C.reshare_v";
        // cout<<sssta_u2.share<<endl;

        end=clock();
        // cout<<"time of client's Reshare_value = "<<double(end-start)<<endl;

        return sssta_u1;
    }

    void Reshare_tag(int64_t rn_s){
        sssta_u0.tag_share.share = sssta_u0.tag_share.share-rn_s;
    }

    void GetASS(string signal, SS ssvalue, SS sstag){
        if(signal=="get_share_of_other_sta_u"){//获取其他客户的统计效用值的份额
            sssta_u1.value_share = ssvalue;
            sssta_u1.tag_share = sstag;
            // cout<<"C.getass";
            // cout<<sssta_u1.value_share.share<<endl;
            // cout<<sssta_u1.tag_share.share<<endl;
        }
        else if(signal=="get_share_of_self_sys_u"){//获取客户自己的系统效用秘密份额
            sssys_u0.value_share = ssvalue;
            sssys_u0.tag_share = sstag;
        }
        else if(signal=="get_share_of_other_sys_u"){//获取其他客户的系统效用秘密份额
            sssys_u1.value_share = ssvalue;
            sssys_u1.tag_share = sstag;
        }
        else if(signal=="get_share_of_a_of_MT"){//获取调度员分配的乘法三元组中a的份额
            ssMT.ashare.value_share = ssvalue;
            ssMT.ashare.tag_share = sstag;
        }
        else if(signal=="get_share_of_b_of_MT"){//获取调度员分配的乘法三元组中b的份额
            ssMT.bshare.value_share = ssvalue;
            ssMT.bshare.tag_share = sstag;
        }
        else if(signal=="get_share_of_c_of_MT"){//获取调度员分配的乘法三元组中c的份额
            ssMT.cshare.value_share = ssvalue;
            ssMT.cshare.tag_share = sstag;
        }
        else if(signal=="get_share_of_rn_of_SC"){//获取调度员分配的随机数的份额
            //不用做任何事，用之前已经分配过的随机数即可
        }
        else{
            cout<<"GetASS ERROR!"<<endl;
        }
        
    }

    void GetMTASS(Multriplet_ASS MT){//获取调度员分配的乘法三元组的份额
        ssMT = MT;
    }

    void initCheck(){
        ssEPcheck.value_share.share=0;
        ssEPcheck.tag_share.share=0;
        ssEPcheck.constant=0;
        EPcheck=0;
    }

    void recordMidresult(uint64_t midres){
        EPcheck+=midres;
    }

    uint64_t revealMidresult(){
        return EPcheck;
    }

    //打开一个秘密值并检查，在这之前要执行initCheck()
    ASS SingleCheck_C(string signal){
        clock_t start,end;
        start=clock();

        ASS res;
        ASS a = SecureConstMul(carry, ssrn_c); //对随机数进位用以掩盖上s位
        if(signal=="reveal_sum_of_sta_u"){
            //cout<<"singlecheck_c"<<d<<endl;
            res.value_share.share = ssUP.value_share.share+a.value_share.share;
            res.tag_share.share = ssUP.tag_share.share+a.tag_share.share;
            res.constant = ssUP.constant+a.constant;
        }
        else if(signal=="reveal_self_stau-a"){//用于开发阶段打开自己的秘密值的份额
            //cout<<"singlecheck_c"<<d<<endl;
            res.value_share.share = ssEP[0].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[0].tag_share.share+a.tag_share.share;
            res.constant = ssEP[0].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_self_sysu-b"){//用于开发阶段打开自己的秘密值的份额
            //cout<<"singlecheck_c"<<d<<endl;
            res.value_share.share = ssEP[1].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[1].tag_share.share+a.tag_share.share;
            res.constant = ssEP[1].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_other_stau-a"){//用于开发阶段打开对方的秘密值的份额
            //cout<<"singlecheck_c"<<d<<endl;
            res.value_share.share = ssEP[2].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[2].tag_share.share+a.tag_share.share;
            res.constant = ssEP[2].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_other_sysu-b"){//用于开发阶段打开对方的秘密值的份额
            //cout<<"singlecheck_c"<<d<<endl;
            res.value_share.share = ssEP[3].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[3].tag_share.share+a.tag_share.share;
            res.constant = ssEP[3].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_lc-a"){//用于进行效用值比较时打开对方的秘密值的份额
            res.value_share.share = ssEP[4].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[4].tag_share.share+a.tag_share.share;
            res.constant = ssEP[4].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_rn-b"){//用于进行效用值比较时打开对方的秘密值的份额
            res.value_share.share = ssEP[5].value_share.share+a.value_share.share;
            res.tag_share.share = ssEP[5].tag_share.share+a.tag_share.share;
            res.constant = ssEP[5].constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_lc"){//用于打开两个效用值相减结果掩盖后的值lc
            res.value_share.share = sslc.value_share.share+a.value_share.share;
            res.tag_share.share = sslc.tag_share.share+a.tag_share.share;
            res.constant = sslc.constant+a.constant;
            ssEPcheck.value_share.share+=res.value_share.share;
            ssEPcheck.tag_share.share+=res.tag_share.share;
            ssEPcheck.constant+=res.constant;
        }
        else if(signal=="reveal_for_check_EP"){//用于打开两个效用值相减结果掩盖后的值lc
            res.value_share.share = ssEPcheck.value_share.share;
            res.tag_share.share = ssEPcheck.tag_share.share;
            res.constant = ssEPcheck.constant;
        }
        else
            cout<<"SingleCheck_C ERROR!"<<endl;
        
        end=clock();
        // cout<<"time of client's SingleCheck_C = "<<double(end-start)<<endl;

        return res;
    }

    ASS SecureAdd(string signal){
        clock_t start,end;
        start=clock();

        if(signal=="add_of_UP"){//更新阶段求和
            //cout<<"secureadd1"<<sssta_u0.tag_share.share<<","<<sssta_u1.tag_share.share<<endl;
            ssUP.value_share.share = sssta_u0.value_share.share+sssta_u1.value_share.share;
            ssUP.tag_share.share = sssta_u0.tag_share.share+sssta_u1.tag_share.share;
            ssUP.constant = sssta_u0.constant+sssta_u1.constant;
            //cout<<"secureadd2"<<res.value_share.share<<","<<res.tag_share.share<<endl;
        }
        else if(signal=="add_of_MT_of_self_u"){//用于开发阶段计算客户自己的效用值的份额
            ssEP[0].value_share.share = sssta_u0.value_share.share-ssMT.ashare.value_share.share; //stau0-a
            ssEP[0].tag_share.share = sssta_u0.tag_share.share-ssMT.ashare.tag_share.share;
            ssEP[0].constant = sssta_u0.constant-ssMT.ashare.constant;

            ssEP[1].value_share.share = sssys_u0.value_share.share-ssMT.bshare.value_share.share; //sysu0-b
            ssEP[1].tag_share.share = sssys_u0.tag_share.share-ssMT.bshare.tag_share.share;
            ssEP[1].constant = sssta_u0.constant-ssMT.bshare.constant;
        }
        else if(signal=="add_of_MT_of_other_u"){//用于开发阶段计算对方客户的效用值的份额
            ssEP[2].value_share.share = sssta_u1.value_share.share-ssMT.ashare.value_share.share; //stau1-a
            ssEP[2].tag_share.share = sssta_u1.tag_share.share-ssMT.ashare.tag_share.share;
            ssEP[2].constant = sssta_u1.constant-ssMT.ashare.constant;

            ssEP[3].value_share.share = sssys_u1.value_share.share-ssMT.bshare.value_share.share; //sysu1-b
            ssEP[3].tag_share.share = sssys_u1.tag_share.share-ssMT.bshare.tag_share.share;
            ssEP[3].constant = sssys_u1.constant-ssMT.bshare.constant;
        }
        else if(signal=="add_of_two_u_in_c1"){//用于第一方计算两方的效用值之差的结果份额
            //计算2*(u0-u1)-1，这样计算结果不会泄露隐私
            sslc.value_share.share=2*(ss_u0.value_share.share-ss_u1.value_share.share)-1; //这里被减数与精度相关，被减数太大会导致相减的正数结果变成负数，在这里所有小数都只用24bits表示，因此我取最大的精度，也就是减去1
            sslc.tag_share.share=2*(ss_u0.tag_share.share-ss_u1.tag_share.share);
            sslc.constant=2*(ss_u0.constant-ss_u1.constant)-1;
        }
        else if(signal=="add_of_two_u_in_c2"){//用于第二方计算两方的效用值之差的结果份额
            //计算2*(u0-u1)-1，这样计算结果不会泄露隐私
            sslc.value_share.share=2*(ss_u1.value_share.share-ss_u0.value_share.share);
            sslc.tag_share.share=2*(ss_u1.tag_share.share-ss_u0.tag_share.share);
            sslc.constant=2*(ss_u1.constant-ss_u0.constant);
        }
        else if(signal=="add_of_MT_of_lc"){//用于计算比较结果lc
            ssEP[4].value_share.share = sslc.value_share.share-ssMT.ashare.value_share.share;
            ssEP[4].tag_share.share = sslc.tag_share.share-ssMT.ashare.tag_share.share;
            ssEP[4].constant = sslc.constant-ssMT.ashare.constant;
            ssEP[5].value_share.share = ssrn_c.value_share.share-ssMT.bshare.value_share.share;
            ssEP[5].tag_share.share = ssrn_c.tag_share.share-ssMT.bshare.tag_share.share;
            ssEP[5].constant = ssrn_c.constant-ssMT.bshare.constant;
        }
        else
            cout<<"SecureAdd ERROR!"<<endl;
        
        end=clock();
        // cout<<"time of client's SecureAdd = "<<double(end-start)<<endl;
    }

    ASS SecureConstMul(uint64_t c, ASS ss){
        clock_t start,end;
        start=clock();

        ASS res;
        res.value_share.share = (uint64_t)((c*ss.value_share.share));
        res.tag_share.share = (uint64_t)((c*ss.tag_share.share));
        res.constant = (uint64_t)(c*ss.constant);
        //cout<<"secureconstmul"<<res.value_share.share<<","<<res.tag_share.share<<endl;
        //cout<<ss.value_share.share<<","<<ss.tag_share.share<<endl;

        end=clock();
        // cout<<"time of client's SecureConstMul = "<<double(end-start)<<endl;

        return res;
    }

    void SecureMul(string signal, uint64_t e, uint64_t f){//这里容易有上下溢出，需要进行截断，利用SecureML中的截断协议，对每个份额进行一次截断
                                                        //而对于下溢出，则是需要防止乘数太小，一般来说，参与计算的所有数都放大了fixed倍，不会太小
        clock_t start,end;
        start=clock();

        if(signal=="mul_of_self_u"){//开发阶段计算客户自己的效用值
            ASS eb=SecureConstMul(e, ssMT.bshare);
            ASS fa=SecureConstMul(f, ssMT.ashare);
            // ss_u0.value_share.share=(uint64_t)((e*f+e*ssMT.bshare.value_share.share+f*ssMT.ashare.value_share.share+ssMT.cshare.value_share.share)/fixed);
            // ss_u0.tag_share.share=      (uint64_t)((e*ssMT.bshare.tag_share.share+f*ssMT.ashare.tag_share.share+ssMT.cshare.tag_share.share)/fixed); 

            ss_u0.value_share.share=(uint64_t)((e*f+eb.value_share.share+fa.value_share.share+ssMT.cshare.value_share.share)/fixed); 
            ss_u0.tag_share.share=      (uint64_t)((eb.tag_share.share+fa.tag_share.share+ssMT.cshare.tag_share.share)/fixed);
            ss_u0.constant = (uint64_t)((e*f+eb.constant+fa.constant+ssMT.cshare.constant)/fixed);
            //cout<<"ssu0: "<<ss_u0.value_share.share<<endl;
        }
        else if(signal=="mul_of_other_u"){//开发阶段计算对方客户的效用值
            ASS eb=SecureConstMul(e, ssMT.bshare);
            ASS fa=SecureConstMul(f, ssMT.ashare);
            ss_u1.value_share.share=(uint64_t)((eb.value_share.share+fa.value_share.share+ssMT.cshare.value_share.share)/fixed);
            ss_u1.tag_share.share=  (uint64_t)((eb.tag_share.share+fa.tag_share.share+ssMT.cshare.tag_share.share)/fixed);
            ss_u1.constant = (uint64_t)((e*f+eb.constant+fa.constant+ssMT.cshare.constant)/fixed);
            //cout<<"ssu1: "<<ss_u1.value_share.share<<endl;
        }
        else if(signal=="mul_of_lc_in_c1"){//用于比较双方效用值大小
            ASS eb=SecureConstMul(e, ssMT.bshare);
            ASS fa=SecureConstMul(f, ssMT.ashare);
            sslc.value_share.share=(uint64_t)((e*f+eb.value_share.share+fa.value_share.share+ssMT.cshare.value_share.share)/fixed);
            sslc.tag_share.share=      (uint64_t)((eb.tag_share.share+fa.tag_share.share+ssMT.cshare.tag_share.share)/fixed);
            sslc.constant = (uint64_t)((e*f+eb.constant+fa.constant+ssMT.cshare.constant)/fixed);
        }
        else if(signal=="mul_of_lc_in_c2"){//用于比较双方效用值大小
            ASS eb=SecureConstMul(e, ssMT.bshare);
            ASS fa=SecureConstMul(f, ssMT.ashare);
            sslc.value_share.share=(uint64_t)((eb.value_share.share+fa.value_share.share+ssMT.cshare.value_share.share)/fixed);
            sslc.tag_share.share=  (uint64_t)((eb.tag_share.share+fa.tag_share.share+ssMT.cshare.tag_share.share)/fixed);
            sslc.constant = (uint64_t)((e*f+eb.constant+fa.constant+ssMT.cshare.constant)/fixed);
        }
        else
            cout<<"SecureMul ERROR!"<<endl;
        
        end=clock();
        // cout<<"time of client's SecureMul = "<<double(end-start)<<endl;
    }

    void showSS(int index){
        if(index == 1){
            cout<<"sssta_u0.value:"<<sssta_u0.value_share.share;
            cout<<" sssta_u0.tag:"<<sssta_u0.tag_share.share;
            cout<<" constance:"<<sssta_u0.constant<<endl;
            cout<<"sssta_u1.value:"<<sssta_u1.value_share.share;
            cout<<" sssta_u1.tag:"<<sssta_u1.tag_share.share;
            cout<<" constance:"<<sssta_u1.constant<<endl;
        }
        else if(index == 2){
            cout<<"sssys_u0.value:"<<sssys_u0.value_share.share;
            cout<<" sssys_u0.tag:"<<sssys_u0.tag_share.share;
            cout<<" constance:"<<sssys_u0.constant<<endl;
            cout<<"sssys_u1.value:"<<sssys_u1.value_share.share;
            cout<<" sssys_u1.tag:"<<sssys_u1.tag_share.share;
            cout<<" constance:"<<sssys_u1.constant<<endl;
        }
        else if(index == 3){
            cout<<"sssta_u0-ssMT.a:"<<ssEP[0].value_share.share;
            cout<<";sssys_u0-ssMT.b:"<<ssEP[1].value_share.share<<endl;;
            cout<<"sssta_u1-ssMT.a:"<<ssEP[2].value_share.share;
            cout<<";sssys_u1-ssMT.b:"<<ssEP[3].value_share.share<<endl;
        }
        else if(index == 4){
            cout<<"ssu0:"<<ss_u0.value_share.share;
            cout<<";ssu1:"<<ss_u1.value_share.share<<endl;;
        }
        else if(index == 5){
            cout<<"ss(u0-u1):"<<sslc.value_share.share;
            cout<<"; ss(u0-u1-a)0:"<<ssEP[4].value_share.share;
            cout<<"; ss(srn-b)0:"<<ssEP[5].value_share.share<<endl;;
        }
        else if(index == 6){
            cout<<"sslc:"<<sslc.value_share.share<<endl;;
        }
        else
            cout<<"ShowSS ERROR!"<<endl;
        
    }
    
};