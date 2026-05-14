#ifndef setup64 //具体的头文件名
#define setup64
#include"setup64.h"
#endif

#include "ctime"

class Coordinator
{
private:
    MACkey bate; //mac密钥
    paillier_prvkey_t * sk=NULL;
    double T; //T为客户相应时间的阈值，当前设置为10s
    Multriplet MT; //
    Multriplet_ASS ssMT[2]; //乘法三元组的可验证秘密份额
    uint64_t rn= 10; //随机数（正数）
    ASS ssrn_s; //coordinator与客户之间秘密分享的随机数的份额，用于验证时掩盖上16位
    SS * sstag_sat_u; //保存与上一轮所选客户之间关于客户的统计效用的ASS
    double * sys_u; //客户的系统效用
    ASS ** sssys_u; //客户系统效用的可验证秘密份额
public:
    paillier_pubkey_t * pk=NULL;
    int K = 6; //每轮选择的客户数
    float epsilon = 0.8; //从上轮被选择的客户中选择的客户数
    float alpha = 1; //对响应时间超过阈值T的客户进行惩罚
    paillier_ciphertext_t ct_ss; //接受来自客户发送的秘密份额的密文
    __uint128_t midres_ep[2] = {0,0}; //接受客户返回的开发阶段的中间结算结果的线性组合

    Coordinator(uint time_threshold, uint nselect, 
                float ratio, uint num_clients, float punish){
        T = time_threshold;
        K = nselect;
        epsilon = ratio;
        alpha = punish;
        sstag_sat_u = new SS[num_clients];
        sys_u = new double[num_clients];
        sssys_u = new ASS * [num_clients];
        for(int i=0; i<num_clients; i++)  sssys_u[i] = new ASS[2];
    }

    void FreeMemory(){
        delete[] sstag_sat_u;
        delete[] sys_u;
        delete[] sssys_u;
    }

    void KeyGen(uint64_t mackey_value){
       //初始化MAC key
       bate.key1 = mackey_value;
       mpz_init_set_ui(bate.key2.m, (unsigned long)mackey_value);
       //初始化同态加密密钥
       // paillier_pubkey_t * pk_p = &pk; //取pk的地址，无法取地址的地址
       // paillier_prvkey_t * sk_p = &sk;
       paillier_keygen(sec_param, &pk, &sk, paillier_get_rand_devurandom);
   }

    uint64_t GenRandom(){
        __uint128_t rn_s = (__uint128_t)rn*fixed;
        // rn = GenRand16();
        // int64_t rn_s = rn;
        ssrn_s.value_share.share= rn_s/2; //随机数的可验证秘密分享，用于验证时掩盖上16位
        ssrn_s.tag_share.share=(__uint128_t)(rn_s*bate.key1/2); //随机数的可验证秘密分享，用于验证时掩盖上16位

        // MT.a = GenRand16();
        // MT.b = GenRand16();
        // MT.c = MT.a*MT.b;
        //三元组的可验证秘密份额
        MT.a=4; MT.b=5; MT.c=20;
        ssMT[0].ashare.value_share.share = MT.a+rn_s;
        ssMT[0].bshare.value_share.share = MT.b+rn_s;
        ssMT[0].cshare.value_share.share = MT.c+rn_s;
        ssMT[0].ashare.tag_share.share = MT.a*bate.key1+rn_s;
        ssMT[0].bshare.tag_share.share = MT.b*bate.key1+rn_s;
        ssMT[0].cshare.tag_share.share = MT.c*bate.key1+rn_s;
        ssMT[1].ashare.value_share.share = -rn_s;
        ssMT[1].bshare.value_share.share = -rn_s;
        ssMT[1].cshare.value_share.share = -rn_s;
        ssMT[1].ashare.tag_share.share = -rn_s;
        ssMT[1].bshare.tag_share.share = -rn_s;
        ssMT[1].cshare.tag_share.share = -rn_s;
        //cout<<"a0 of MT: "<<ssMT[0].ashare.value_share.share<<", a1 of MT: "<<ssMT[1].ashare.value_share.share<<", t0 of a of MT: "<<ssMT[0].ashare.tag_share.share<<", t1 of a of MT: "<<ssMT[1].ashare.tag_share.share<<", constance: "<<ssMT[0].ashare.constant<<endl;

        return rn_s;
    }

    uint64_t SendRandom(){
        return rn*fixed;
    }

    void UpdateSys_u(const double D[], int E[], int num_clients){
        clock_t start,end;
        start=clock();

        uint64_t rn_s = rn*fixed;
        uint64_t sys_u_ui = 0;
        int i=0, j=0;
        for(j=0; j<num_clients; j++){
            //cout<<"calculate sysu for client"<<j<<"/"<<num_clients<<": "<<endl;
            sys_u[j]=pow((((double)T)/D[j]), (D[j] > T)*alpha);//SC判断客户有没有超过时间阈值，也就是更新客户的系统效用
            //cout<<"sysu for client "<<j<<":"<<sys_u[j]<<endl;
            for(i=0; i<K; i++)
                if(j==E[i]){
                    //sys_u[j]=pow((T/D[E[j]]), (D[E[j]] > T)*alpha);
                    //cout<<"sysu for the exploited client "<<E[i]<<":";
                    sys_u_ui = sys_u[E[i]] * fixed;
                    sssys_u[E[i]][0].value_share.share = (__uint128_t)(sys_u_ui+rn_s);
                    sssys_u[E[i]][0].tag_share.share = (__uint128_t)((bate.key1 * sys_u_ui)+rn_s);
                    sssys_u[E[i]][1].value_share.share = (__uint128_t)(-rn_s);
                    sssys_u[E[i]][1].tag_share.share = (__uint128_t)(-rn_s);
                    //cout<<sys_u[E[i]]<<endl;
                }
        }

        end=clock();
        cout<<"time of Coordinator's UpdateSys_u = "<<double(end-start)<<endl;
    }

    void SendCT_bate(paillier_ciphertext_t * ct){
        mpz_init_set_d(ct->c, 2);
        paillier_enc(ct, pk, &bate.key2, paillier_get_rand_devurandom);
    }

    //SC获得客户的统计效用的标签的份额
    void Getsstag_sat_u(paillier_ciphertext_t * ct, int index){ //在更新阶段获取客户的统计效用的标签的份额
        clock_t start,end;
        start=clock();

        paillier_plaintext_t pt;

        mpz_init_set_si(pt.m, 0);
        paillier_dec(&pt, pk, sk, ct);
        //cout<<(*(pt.m->_mp_d))<<endl;
        sstag_sat_u[index].share = (__uint128_t)(*(pt.m->_mp_d));

        end=clock();
        cout<<"time of Coordinator's Getsstag_sat_u = "<<double(end-start)<<endl;
    }

    void Updatesstag_sta_u_tu(int r, int l, int index){
        clock_t start,end;
        start=clock();

        double tu=sqrt(0.1*log(r)/l);
        sstag_sat_u[index].share = sstag_sat_u[index].share + (__int128_t)(bate.key1 * ((__int128_t)(tu * (uint64_t)(1<<(fixed_point)))));

        end=clock();
        cout<<"time of Coordinator's Updatesstag_sta_u_tu = "<<double(end-start)<<endl;
    }

    SS Secret_Share_value(string signal, int index1, int index2){
        SS res;
        if(signal=="share_sys_u")   res=sssys_u[index1][index2].value_share;
        else if(signal=="share_a_of_MT")   res=ssMT[index2].ashare.value_share;
        else if(signal=="share_b_of_MT")   res=ssMT[index2].bshare.value_share;
        else if(signal=="share_c_of_MT")   res=ssMT[index2].cshare.value_share;
        else if(signal=="share_rn")   res=ssrn_s.value_share;//这里设置随机数的两份份额相等
        else    cout<<"Secret_Share_value ERROR!"<<endl;

        return res;
    }

    SS Secret_Share_tag(string signal, int index1, int index2){
        clock_t start,end;
        start=clock();

        SS res;
        if(signal=="share_sys_u")   res=sssys_u[index1][index2].tag_share;
        else if(signal=="share_a_of_MT")   res=ssMT[index2].ashare.tag_share;
        else if(signal=="share_b_of_MT")   res=ssMT[index2].bshare.tag_share;
        else if(signal=="share_c_of_MT")   res=ssMT[index2].cshare.tag_share;
        else if(signal=="share_rn")   res=ssrn_s.tag_share;//这里设置随机数的两份份额相等
        else    cout<<"Secret_Share_tag ERROR!"<<endl;

        end=clock();
        cout<<"time of Coordinator's Secret_Share_tag = "<<double(end-start)<<endl;

        return res;
    }

    SS Reshare_tag(int index){ //SC在发送tag的份额时，会把用于重分享的随机数rn发送给另一方
        clock_t start,end;
        start=clock();

        __int128_t rn_s = (__int128_t)rn*fixed;
        SS res;
        res.share=sstag_sat_u[index].share+rn_s;

        end=clock();
        cout<<"time of Coordinator's Reshare_tag = "<<double(end-start)<<endl;

        return res;
    }

    void Mergesort(int* P, int low, int hight){

        if (low < hight)
	    {
		    int mid = (low + hight) / 2;
		    Mergesort(P, low, mid);          //对 a[low,mid]进行排序
		    Mergesort(P, mid + 1, hight);    //对 a[mid+1,hight]进行排序
		    Merge(P, low, mid, hight);       //进行合并操作
	    }

    }

    void Merge(int* P, int low, int mid, int hight){  //合并函数
	    int* b = new int[hight - low + 1];  //用 new 申请一个辅助函数
	    int i = low, j = mid + 1, k = 0;    // k为 b 数组的小标
	    while (i <= mid && j <= hight)  
	    {
		    if ( sys_u[P[i]]<= sys_u[P[j]])
		    {
			    b[k++] = P[i++];  //按从小到大存放在 b 数组里面
		    }
		    else
		    {
			    b[k++] = P[j++];
		    }
	    }
	    while (i <= mid)  // j 序列结束，将剩余的 i 序列补充在 b 数组中 
	    {
		    b[k++] = P[i++];
	    }
	    while (j <= hight)// i 序列结束，将剩余的 j 序列补充在 b 数组中 
	    {
		    b[k++] = P[j++];
	    }
	    k = 0;  //从小标为 0 开始传送
	    for (int i = low; i <= hight; i++)  //将 b 数组的值传递给数组 a
	    {
		    P[i] = b[k++];
	    }
	    delete[]b;     // 辅助数组用完后，将其的空间进行释放（销毁）
    }

    void Select_Client(int num_clients, int* non_E, int* E){
        clock_t start,end;
        start=clock();

        int cut = ((float)K*0.05<1)? 1:K*0.05; //截断被开发的客户中分别最大最小的5%个客户
        int cut1 = ((float)(num_clients-K)*0.05<1)? 1:(num_clients-K)*0.05; //截断被开发的客户中分别最大最小的5%个客户
        int select_EP = K*epsilon;
        //cout<<"select_EP: "<<select_EP<<endl;
        int selected_client[K] = {0};
        int selected[num_clients] = {0};
        double prob1[K]={0.0}; //每个被开发过的客户被选中的概率
        double prob2[num_clients-K]={0.0}; //每个未被开发的客户被选中的概率
        float m;
        random_device rd; //随机数生成器
        mt19937 gen(rd()); //随机数生成器
        int index=-1; //每次选择的序号
        /*首先依据排序序号大小，在上一轮被开发过的客户截掉排名最高与最低的SC.K*5%个客户后，中选择SC.K*SC.epsilon个客户*/

        for(int i = 0; i<cut; i++){ //将被截断的客户的标志变为-1，表示不被选择
            selected[E[i]] = -1;
            selected[E[K-1-i]] = -1;
        }
        for(int i = 0; i<cut1; i++){ //将被截断的客户的标志变为-1，表示不被选择
            selected[non_E[i]] = -1;
            selected[non_E[num_clients-K-1-i]] = -1;
        }
        //以下都是依概率选择客户
        int j = 0;
        if(K-2*cut == select_EP){ //若剩余被开发的客户个数等于要选择出的客户个数，则不进行筛选，全部录用
            for(int i=cut; i<K-cut; i++){
                selected_client[j++] = E[i];
                selected[E[i]]=1;
                //cout<<"selected client: "<<P[i]<<endl;
            }    
        }
        else{ //否则，计算被开发过的客户被选择的概率并据此进行选择
            //double sum_prob1=0, sum_prob2=0;
            m = 0;
            for (int i = cut; i < K-cut; ++i) m+=i;
            for (int i = cut; i < K-cut; ++i) {
                prob1[i] = (double)i/m;  // 概率与排序序号成正比例
                //sum_prob1+=prob[P[i]];
            }
            discrete_distribution<int> dist1(prob1, prob1+K-1); //离散分布
            for(int i=0; i<select_EP; i){//依据离散分布选择客户
                index = dist1(gen);
                if(selected[E[index]]==0){ //若E[index]未被选择过并且不是被cut的，则保留这个序号，否则再进行一次选择
                   selected[E[index]]=1;
                   selected_client[i++] = E[index];
                   //cout<<"selected client: "<<P[index]<<endl;
               }
           }
        }
        /*然后依据排序序号大小，在上一轮未被开发过的客户截掉排名最高与最低的(num_clients-K)*5%个客户后，中选择num_clients-SC.K*SC.epsilon个客户*/
        if(num_clients-K-2*cut == K-select_EP){ //若剩余未被开发的客户个数等于要选择出的客户个数，则不进行筛选，全部录用
            for(int i=cut; i<num_clients-K-cut; i++){
                selected_client[j++] = non_E[i];
                selected[non_E[i]]=1;
            }
        }
        else{ //否则，计算未被开发过的客户被选择的概率并据此进行选择
            m=0;
            for (int i = cut; i < num_clients-K-cut; ++i) m+=sys_u[non_E[i]];
            for (int i = cut; i < num_clients-K-cut; ++i) {
                prob2[i] = (double)sys_u[non_E[i]]/m;  // 概率与排序序号成正比例
            //sum_prob2+=prob[P[i]];
            }
            discrete_distribution<int> dist2(prob2, prob2+num_clients-K-1);
            for(int i=select_EP; i<K; i){
                index = dist2(gen); //还是会输出概率为0的索引，需要预防！
                if(selected[non_E[index]]==0){ //若P[index]未被选择过并且不是被cut的，则保留这个序号，否则再进行一次选择
                    selected[non_E[index]]=1;
                    selected_client[i++] = non_E[index];
                    //cout<<"the index and P[index]: "<<index<<P[index]<<endl;;
                }
            }
            //cout<<endl;
        }
        // 直接选择排名最靠后的几个
        // int j = 0;
        // for(int i=K-cut-1; i>K-cut-1-select_EP;i--){ //从被开发的客户中选择排序最高的select_EP个客户
        //     selected_client[j++] = E[i];
        //     selected[E[i]]=1;
        // }
        // for(int i=num_clients-K-cut-1; i>num_clients-K-cut-1-K+select_EP;i--){ //从未被开发的客户中选择排序最高的K-select_EP个客户
        //     selected_client[j++] = non_E[i];
        //     selected[non_E[i]]=1;
        // }

        for(int i = 0; i<K; i++){
            E[i] = selected_client[i];
        }
        // for (int i = low; i < num_clients; ++i) { //打印客户被选择的概率分布
        //     cout << i << ": ";
        //     cout << prob[i] <<endl;
        // }
        // //cout<<"sum of prob: "<<sum_prob1<<";"<<sum_prob2<<endl; //验证概率分布之和是否为1
        // cout<<"选中的客户列表为："<<endl;
        // for(int i = 0; i < K; i++){
        //     cout<<E[i]<<" ";
        // }
        // cout<<endl;

        end=clock();
        cout<<"time of Coordinator's Select_Client = "<<double(end-start)<<endl;
        

    }

    void GetMidres_EP(__uint128_t midres1, __uint128_t midres2){
        midres_ep[0] = midres1;
        midres_ep[1] = midres2;
    }

    bool SingleCheck_S(string signal, ASS ss0, ASS ss1, ASS ss2, double * res){
        clock_t start,end;
        start=clock();

        int64_t a;
        __int128_t tag;
        __int128_t value;
        if(signal=="check_three_shares"){
            a = (int64_t)(ss0.value_share.share+ss1.value_share.share+ss2.value_share.share);//恢复出秘密值
            *res = (double)a/fixed;
            //检查正确性
            tag = (__int128_t)(ss0.tag_share.share+ss1.tag_share.share+ss2.tag_share.share);
            value = (__int128_t)(ss0.value_share.share+ss1.value_share.share+ss2.value_share.share-(ss0.constant+ss1.constant+ss2.constant)/3);
            //cout<<tag<<","<<value<<","<<(float)(abs(tag-bate.key1*value))/(uint64_t)(1<<(fixed_point-1))<<endl;
        }
        else if(signal=="check_two_shares"){
            //cout<<ss1.value_share.share<<","<<ss2.value_share.share<<endl;
            //cout<<"value with masked upper k bits"<<(uint128_t)((ss1.value_share.share+ss2.value_share.share))<<endl;//经检查，确实带有掩盖
            a = (int64_t)(ss0.value_share.share+ss1.value_share.share);//恢复出秘密值
            *res = (double)a/fixed;
            //检查正确性
            tag = (__int128_t)(ss0.tag_share.share+ss1.tag_share.share);
            value = (__int128_t)(ss0.value_share.share+ss1.value_share.share-ss0.constant);
            //cout<<tag<<","<<value<<","<<(float)(abs(tag-bate.key1*value))/(uint64_t)(1<<(fixed_point-1))<<endl;
        }
        else if(signal=="check_for_EP"){
            //检查正确性
            tag = (__int128_t)(ss0.tag_share.share+ss1.tag_share.share);
            value = (__int128_t)(ss0.value_share.share+ss1.value_share.share-ss0.constant);
            // cout<<tag<<","<<value<<",";
        }
        else
            cout<<"SingleCheck_S ERROR!"<<endl;

        // cout<<"SingleCheck_S res: "<<(double)(abs(tag-bate.key1*value))/fixed<<endl;

        end=clock();
        cout<<"time of Coordinator's SingleCheck_S = "<<double(end-start)<<endl;

        return ((double)(abs(tag-bate.key1*value))/fixed<ep);
    }

};
