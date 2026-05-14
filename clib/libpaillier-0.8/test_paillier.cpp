//编译指令g++ -o output paillier.c test_paillier.cpp -lgm
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>
#include <gmpxx.h>
#include <iostream>

#include "paillier.h"

using namespace std;

int main()
{
    paillier_plaintext_t pt;
    mpz_init_set_d(pt.m, 123);
    paillier_plaintext_t pt_new;
	mpz_init_set_d(pt_new.m, 1); //一开始没有对这个值进行初始化，所以报错“段错误 (核心已转储)”
    paillier_pubkey_t * pk=NULL;
    paillier_prvkey_t * sk=NULL;

    paillier_plaintext_t a;
    mpz_init_set_d(a.m, 2);

    pk = new paillier_pubkey_t(); //为指针申请内存
    if(pk == NULL) //判断是否申请内存成功
    {
        printf("memory new error");
        return 0;
    }
    sk = new paillier_prvkey_t(); //为指针申请内存
    if(sk == NULL) //判断是否申请内存成功
    {
        printf("memory new error");
        return 0;
    }

    paillier_keygen(64, &pk, &sk, paillier_get_rand_devurandom);
    cout<<"KeyGen sccess!"<<endl;
    cout<<"public key: "; cout<<*pk->n->_mp_d<<endl;
    cout<<"private key: "; cout<<*sk->lambda->_mp_d<<endl;

    paillier_ciphertext_t ct;
    paillier_enc(&ct, pk, &pt, paillier_get_rand_devurandom);
    cout<<"Enc sccess! ciphertest: ";
    cout<<*(ct.c->_mp_d)<<endl;

    paillier_dec(&pt_new, pk, sk, &ct);
    char *pt_n=paillier_plaintext_to_str(&pt_new);
    cout<<"Dec sccess! plaintext: "; 
    cout<<*(pt_new.m->_mp_d)<<endl;

    paillier_ciphertext_t ct2;
    paillier_exp(pk, &ct2, &ct, &a);
    //paillier_plaintext_t pt2;
    cout<<"constant multiplication sccess! cyphertext: "; cout<<*ct2.c->_mp_d<<endl;
    paillier_dec(&pt_new, pk, sk, &ct2);
    cout<<"constant multiplication sccess! result: "; cout<<*pt_new.m->_mp_d<<endl;

    if(pk != NULL)
    {
        delete pk; // 防止内存泄漏(new与delete成对存在)
        pk = NULL; // 释放后置空，防止野指针
    }
    if(sk != NULL)
    {
        delete sk; // 防止内存泄漏(new与delete成对存在)
        sk = NULL; // 释放后置空，防止野指针
    }

    return 0;
}
