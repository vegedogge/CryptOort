//g++ -o SecSelect ./libpaillier-0.8/paillier.c SecSelect.cpp -lgmp
//g++ -shared -o SecSelect.so -fPIC SecSelect.cpp ./libpaillier-0.8/paillier.c ./fixed_random.cpp -lgmp //编译成动态链接库
#include "iostream"
#include "iomanip"
#include "bitset"
#include "ctime"

#include "coordinator16b.h"
#include "client16b.h"
#include "fixed_random16b.h"

bool SecComp(Coordinator * SC, Client * C, int c1, int c2); //安全比较
void SecMerge(Coordinator * SC, Client * C, int* E, int low, int mid, int hight); //安全递归归并
void SecMergesort(Coordinator * SC, Client * C, int* E, int low, int hight); //安全递归归并排序
// void SecMerge(Coordinator * SC, Client * C, int A[], int maxn, int L1,int R1,int L2,int R2); //安全非递归归并
// void SecMergesort(Coordinator * SC, Client * C, int A[],int n);  //安全非递归归并排序

extern "C"{
void secure_participant_selection(double time_threshold, uint nselect, float ratio, float punish, int num_clients, 
                                  int E[], const double sta_u[], const double D[], const int L[], double ssu[], int rounds);

//输入参数E为上一轮参与训练的客户，sta_u为所有的统计效用，T为响应时间阈值，D为所有客户的响应时间，ssu为每轮的统计效用的总和，输出为被选中的客户集合P
void secure_participant_selection(double time_threshold, uint nselect, float ratio, float punish, int num_clients, 
                                  int E[], const double sta_u[], const double D[], const int L[], double ssu[], int rounds)
{
    //int E[nselect];
    Coordinator SC(time_threshold, nselect, ratio, num_clients, punish);
    Client C[num_clients];
    uint16_t mac_key_value=2;

    //for(int i=0; i<nselect; i++)    E[i]=E_c[i];

    if(rounds==1) cout<<"This is the first round of training and secure participant selection should not be invoked!!!"<<endl;

    /*初始化调度员SC与客户*/
    SC.KeyGen(mac_key_value);
    int32_t rn_s = SC.GenRandom(); //rn_s
    // cout<<"SC.GenRandom success! rn="<<rn_s<<endl;
    // cout<<"SC.KeyGen success!"<<endl;
    int non_E[num_clients-SC.K]; //未被开发的客户的编号
    int k = 0;
    for(int i=0; i<num_clients; i++){//划分客户集合为上轮参与训练的被开发的客户与未被开发的客户
        bool isinE = 0; //判断第i个客户是否在集合E中
        C[i].duration = D[i]; //客户响应SC所需的时间
        C[i].initCheck(); //初始化验证所需的变量，方便后续进行计算正确性验证
        SC.SendCT_bate(&C[i].ct_bate); //SC发送mac key的密文给客户
        for(int j=0; j<SC.K; j++){
            if(i == E[j]){
                isinE = 1;
                C[i].isexploited=true;
                C[i].GetstaU(sta_u[j]); //将浮点类型的统计效用转换为定点数，小数位为32
                //cout<<"stau for client"<<E[j]<<" : "<<sta_u[j]<<", "<<endl;
                C[i].GetRandom_ASS(rn_s, mac_key_value); //每个客户都初始化在预处理阶段与SC秘密分享的随机数，这一步在预处理阶段完成
                break;
            }   
        }
        if(!isinE){
            non_E[k++] = i;
            C[i].isexploited=false;
            C[i].GetstaU(0);
            //cout<<"stau for client"<<i<<" : 0, "<<endl;
        }  
    }
    // cout<<"Client get stau and ct_mac_key success!"<<endl;

    /***接下来利用隐私计算技术对客户排序***/

    ASS addi, addj, addk;
    double su=0,suij=0;
    bool isMalicious=false;
    float bound = (float)SC.K/2;

    /**更新阶段**/

    /*首先SC与每个客户生成客户的统计效用值的标签的秘密份额*/
    paillier_ciphertext_t ct_sstag;
    mpz_init_set_d(ct_sstag.c, 2); //初始化密文变量，不然会报错
    for(int i=0; i<SC.K; i++){ //客户与SC协同产生统计效用的标签的加性秘密份额
        C[E[i]].Secret_sharing_C(&ct_sstag, SC.pk);//函数执行正确

        SC.Getsstag_sat_u(&ct_sstag, E[i]);//SC获得客户的统计效用的mac的份额
    }
    // cout<<"Secret sharing mac of stau success!"<<endl;

    /*然后SC分别发送调度信息UP_assign_1，UP_assign_2给C1，C2*/
    /*同时两个客户相互分享统计效用值，并在SC的辅助下对所有客户的统计效用求和*/
    int upper = ceil(bound);
    if(upper!=SC.K/2){ //客户个数是奇数，则中间三个客户执行三方安全加法
        int j = (int)bound;
        int i = j-1, k = j+1;
        upper = i;
        C[E[k]].GetASS("get_share_of_other_sta_u", C[E[j]].Reshare_value(), SC.Reshare_tag(E[j]));//函数正确
        C[E[j]].GetASS("get_share_of_other_sta_u", C[E[i]].Reshare_value(), SC.Reshare_tag(E[i]));
        C[E[i]].GetASS("get_share_of_other_sta_u", C[E[k]].Reshare_value(), SC.Reshare_tag(E[k]));//省略了重分享用于掩盖上s位的随机数
        C[E[k]].Reshare_tag(rn_s);
        C[E[j]].Reshare_tag(rn_s);
        C[E[i]].Reshare_tag(rn_s);
        C[E[k]].SecureAdd("add_of_UP");
        C[E[j]].SecureAdd("add_of_UP");
        C[E[i]].SecureAdd("add_of_UP");
        addk=C[E[k]].SingleCheck_C("reveal_sum_of_sta_u");
        addj=C[E[j]].SingleCheck_C("reveal_sum_of_sta_u");
        addi=C[E[i]].SingleCheck_C("reveal_sum_of_sta_u");
        isMalicious=SC.SingleCheck_S("check_three_shares",addi,addj,addk,&suij);
        // if(isMalicious) cout<<"client"<<E[i]<<" and "<<"client"<<E[j]<<" and "<<"client"<<E[k]<<" are malicious!"<<endl;
        // else cout<<"client"<<E[i]<<" and "<<"client"<<E[j]<<" and "<<"client"<<E[k]<<" are honest!"<<endl;
        su=su + (double)(suij);
        //cout<<suij<<endl;
    }
    for(int j=0,i=0; i<upper && upper!=0; i++){//求统计效用的和，这个值需要返回    
        j = SC.K-i-1;
        C[E[i]].GetASS("get_share_of_other_sta_u", C[E[j]].Reshare_value(), SC.Reshare_tag(E[j]));//函数正确
        C[E[j]].GetASS("get_share_of_other_sta_u", C[E[i]].Reshare_value(), SC.Reshare_tag(E[i]));//省略了重分享用于掩盖上s位的随机数
        C[E[i]].Reshare_tag(rn_s);
        C[E[j]].Reshare_tag(rn_s);
        C[E[i]].SecureAdd("add_of_UP");
        C[E[j]].SecureAdd("add_of_UP");
        addi=C[E[i]].SingleCheck_C("reveal_sum_of_sta_u");
        addj=C[E[j]].SingleCheck_C("reveal_sum_of_sta_u");
        //cout<<"success!"<<endl;
        isMalicious=SC.SingleCheck_S("check_two_shares",addi,addj,addk,&suij);
        //cout<<"success!"<<endl;
        // if(isMalicious) cout<<"client"<<E[i]<<" and "<<"client"<<E[j]<<" are malicious!"<<endl;
        // else cout<<"client"<<E[i]<<" and "<<"client"<<E[j]<<" are honest!"<<endl;
        //cout<<"the sum of static utilities: "<<(double)(suij/isodd)<<endl;
        su=su + (double)(suij);
    }
    ssu[rounds-2]=su;
    //cout<<"the sum of static utilities: "<<su<<endl;

    // /*SC更新时间阈值T*/ //这一部分可以在协议执行之前进行
    // if(rounds>=2*SC.W+1){
    //     SC.UpdateT(ssu, rounds-2);
    // }

    /*SC更新每个客户的系统效用*/
    //cout<<"System utility: "<<endl;
    SC.UpdateSys_u(D, E, num_clients);
    // cout<<"Update phase fnish!"<<endl;

    /**开发阶段**/

    /*客户先在统计效用加上temporal uncertainty，SC也更新统计效用的tag的share*/
    for(int i=0;i<SC.K;i++){
        //cout<<E[i]<<": ";
        C[E[i]].GetstaU_TU(rounds, L[E[i]]);
        SC.Updatesstag_sta_u_tu(rounds, L[E[i]], E[i]);
    }

    /*计算效用值并据此对客户排序*/
    SecMergesort(&SC, C, E, 0, SC.K-1);
    // cout<<"不用递归进行筛选"<<endl;
    // SecMergesort(&SC, C, E, SC.K);
    // cout<<"Sorted clients accordding their sta_u: ";
    // for(int i=0; i<SC.K; i++)  cout<<E[i]<<" ";     cout<<endl;

    /*SC在本地依据系统效用对其他客户排序*/
    clock_t start,end;
    start=clock();
    SC.Mergesort(non_E, 0, num_clients-SC.K-1);
    end=clock();
    cout<<"time of Coordinator's Mergesort = "<<double(end-start)<<endl;
    // cout<<"Sorted clients according sysu: ";
    // for(int i=0; i<num_clients-SC.K; i++)  cout<<non_E[i]<<" ";     cout<<endl;
    // cout<<"Exploitation phase fnish!"<<endl;

    /**筛选阶段**/
    SC.Select_Client(num_clients, non_E, E); //加入这个函数后就导致执行内存溢出，删掉依概率选取客户后不会产生溢出
    // cout<<"选中的客户列表为："<<endl;
    // for(int i = 0; i < SC.K; i++){
    //     cout<<E[i]<<" ";
    // }
    // cout<<endl;
    SC.FreeMemory(); //释放函数中计算过程中分配的内存
    // cout<<"Selection phase fnish!"<<endl;

    //return E;
}
}

bool SecComp(Coordinator * SC, Client * C, int c1, int c2){ //在SC的协助下进行安全比较
    C[c1].initCheck();
    C[c2].initCheck();
    //SC分发正随机数以及乘法三元组
    C[c1].GetASS("get_share_of_a_of_MT", SC->Secret_Share_value("share_a_of_MT", c1, 0), SC->Secret_Share_tag("share_a_of_MT", c1, 0));//获取三元组中的a
    C[c2].GetASS("get_share_of_a_of_MT", SC->Secret_Share_value("share_a_of_MT", c2, 1), SC->Secret_Share_tag("share_a_of_MT", c2, 1));
    C[c1].GetASS("get_share_of_b_of_MT", SC->Secret_Share_value("share_b_of_MT", c1, 0), SC->Secret_Share_tag("share_b_of_MT", c1, 0));//获取三元组中的b
    C[c2].GetASS("get_share_of_b_of_MT", SC->Secret_Share_value("share_b_of_MT", c2, 1), SC->Secret_Share_tag("share_b_of_MT", c2, 1));
    C[c1].GetASS("get_share_of_c_of_MT", SC->Secret_Share_value("share_c_of_MT", c1, 0), SC->Secret_Share_tag("share_c_of_MT", c1, 0));//获取三元组中的c，c=a*B
    C[c2].GetASS("get_share_of_c_of_MT", SC->Secret_Share_value("share_c_of_MT", c2, 1), SC->Secret_Share_tag("share_c_of_MT", c2, 1));
    //重分享客户的统计效用
    int32_t rn_s=(int32_t)SC->SendRandom();
    C[c1].GetASS("get_share_of_other_sta_u", C[c2].Reshare_value(), SC->Reshare_tag(c2));//函数正确
    C[c2].GetASS("get_share_of_other_sta_u", C[c1].Reshare_value(), SC->Reshare_tag(c1));//省略了重分享用于掩盖上s位的随机数
    C[c1].Reshare_tag(rn_s);
    C[c2].Reshare_tag(rn_s);
    //C[c1].showSS(1); C[c2].showSS(1); //以上四行正确
    //SC分发客户的系统效用的份额
    C[c1].GetASS("get_share_of_self_sys_u", SC->Secret_Share_value("share_sys_u", c1, 0), SC->Secret_Share_tag("share_sys_u", c1, 0));
    C[c1].GetASS("get_share_of_other_sys_u", SC->Secret_Share_value("share_sys_u", c2, 1), SC->Secret_Share_tag("share_sys_u", c2, 1));
    C[c2].GetASS("get_share_of_self_sys_u", SC->Secret_Share_value("share_sys_u", c2, 0), SC->Secret_Share_tag("share_sys_u", c2, 0));
    C[c2].GetASS("get_share_of_other_sys_u", SC->Secret_Share_value("share_sys_u", c1, 1), SC->Secret_Share_tag("share_sys_u", c1, 1));
    //C[c1].showSS(2); C[c2].showSS(2); //以上四行正确
    //c1与c2计算双方的效用值，也即一次乘法
    /*例如求stau*sysu，令e=stau-a，f=sysu-b，则stau*sysu=e*f+e*b+f*a+a*b。*/
    C[c1].SecureAdd("add_of_MT_of_self_u"); C[c2].SecureAdd("add_of_MT_of_self_u");
    C[c1].SecureAdd("add_of_MT_of_other_u"); C[c2].SecureAdd("add_of_MT_of_other_u"); 
    //C[c1].showSS(3); C[c2].showSS(3); //以上两行正确
    ASS sse_u[4] = {C[c1].SingleCheck_C("reveal_self_stau-a"), C[c2].SingleCheck_C("reveal_other_stau-a"), C[c2].SingleCheck_C("reveal_self_stau-a"), C[c1].SingleCheck_C("reveal_other_stau-a")};//安全乘法打开的中间值e
    ASS ssf_u[4] = {C[c1].SingleCheck_C("reveal_self_sysu-b"), C[c2].SingleCheck_C("reveal_other_sysu-b"), C[c2].SingleCheck_C("reveal_self_sysu-b"), C[c1].SingleCheck_C("reveal_other_sysu-b")};//安全乘法打开的中间值f
    uint32_t e_u[2] = {sse_u[0].value_share.share+sse_u[1].value_share.share, sse_u[2].value_share.share+sse_u[3].value_share.share};
    uint32_t f_u[2] = {ssf_u[0].value_share.share+ssf_u[1].value_share.share, ssf_u[2].value_share.share+ssf_u[3].value_share.share};
    C[c1].recordMidresult(e_u[0]+e_u[1]+f_u[0]+f_u[1]); //客户计算并保存计算中间结果的线性组合
    C[c2].recordMidresult(e_u[0]+e_u[1]+f_u[0]+f_u[1]); //客户计算并保存计算中间结果的线性组合
    //cout<<"e0: "<<e_u[0]<<"; e1: "<<e_u[1]; cout<<"; f0: "<<f_u[0]<<"; f1: "<<f_u[1]<<endl;; //以上四行正确
    C[c1].SecureMul("mul_of_self_u", (uint16_t)e_u[0], (uint16_t)f_u[0]); C[c2].SecureMul("mul_of_self_u", (uint16_t)e_u[1], (uint16_t)f_u[1]);
    C[c1].SecureMul("mul_of_other_u", (uint16_t)e_u[1], (uint16_t)f_u[1]); C[c2].SecureMul("mul_of_other_u", (uint16_t)e_u[0], (uint16_t)f_u[0]);
    //C[c1].showSS(4); C[c2].showSS(4); //以上两行正确
    //c1与c2比较他们的效用值大小，也即一次减法与加法
    C[c1].SecureAdd("add_of_two_u_in_c1"); C[c2].SecureAdd("add_of_two_u_in_c2");
    C[c1].SecureAdd("add_of_MT_of_lc"); C[c2].SecureAdd("add_of_MT_of_lc");
    //C[c1].showSS(5); C[c2].showSS(5); //以上两行正确
    ASS sse_lc[2] = {C[c1].SingleCheck_C("reveal_lc-a"), C[c2].SingleCheck_C("reveal_lc-a")};//安全乘法打开的中间值e
    ASS ssf_lc[2] = {C[c1].SingleCheck_C("reveal_rn-b"), C[c2].SingleCheck_C("reveal_rn-b")};//安全乘法打开的中间值f
    uint32_t e_lc = sse_lc[0].value_share.share+sse_lc[1].value_share.share;
    uint32_t f_lc = ssf_lc[0].value_share.share+ssf_lc[1].value_share.share;
    C[c1].recordMidresult(e_lc+f_lc); //客户计算并保存计算中间结果的线性组合
    C[c2].recordMidresult(e_lc+f_lc); //客户计算并保存计算中间结果的线性组合
    //cout<<"e_lc: "<<(double)(int16_t)(uint16_t)e_lc/fixed; cout<<"; f_lc: "<<(double)(int16_t)(uint16_t)f_lc/fixed<<endl;; //以上四行正确
    C[c1].SecureMul("mul_of_lc_in_c1", (uint16_t)e_lc, (uint16_t)f_lc); C[c2].SecureMul("mul_of_lc_in_c2", (uint16_t)e_lc, (uint16_t)f_lc);
    //C[c1].showSS(6); C[c2].showSS(6); // 以上六行正确
    //打开比较结果lc，通过lc的符号来判断两个效用值的大小，lc<0 or lc>0。
    uint32_t lc_masked = C[c1].SingleCheck_C("reveal_lc").value_share.share+C[c2].SingleCheck_C("reveal_lc").value_share.share;
    uint16_t lc_int = (uint16_t)(lc_masked);
    double lc = (double)(int16_t)lc_int/fixed;
    C[c1].recordMidresult(lc_masked); //客户计算并保存计算中间结果的线性组合
    C[c2].recordMidresult(lc_masked); //客户计算并保存计算中间结果的线性组合
    //cout<<c1<<","<<c2<<","<<lc<<endl;

    return (lc<0);
}

void SecMerge(Coordinator * SC, Client * C, int* P, int low, int mid, int hight)  //合并函数
{
	int* b = new int[hight - low + 1];  //用 new 申请一个辅助数组
	int i = low, j = mid + 1, k = 0;    // k为 b 数组的小标
    ASS Midres1, Midres2;
    double temp=0;
	while (i <= mid && j <= hight)
	{
		if (SecComp(SC, C, P[i], P[j]))//替换为安全比较协议，u_E[i] <= u_E[j] == SecComp(SC, C, E[i], E[j])==1
		{
            cout<<"Compare clients: "<<P[i]<<", "<<P[j]<<" ";
            Midres1=C[P[i]].SingleCheck_C("reveal_for_check_EP");
            Midres2=C[P[j]].SingleCheck_C("reveal_for_check_EP");
            SC->GetMidres_EP(C[P[i]].revealMidresult(),C[P[j]].revealMidresult());
            SC->SingleCheck_S("check_for_EP",Midres1,Midres2,Midres1,&temp); //SC验证开发阶段的计算是否正确
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

void SecMergesort(Coordinator * SC, Client * C, int* P, int low, int hight) //归并排序
{
	if (low < hight)
	{
		int mid = (low + hight) / 2;
		SecMergesort(SC, C, P, low, mid);          //对 a[low,mid]进行排序
		SecMergesort(SC, C, P, mid + 1, hight);    //对 a[mid+1,hight]进行排序
		SecMerge(SC, C, P, low, mid, hight);       //进行合并操作
	}
}

// void SecMerge(Coordinator * SC, Client * C, int A[], int maxn, int L1,int R1,int L2,int R2){
// 	int i=L1,j=L2;
//     int * temp = new int[maxn];
// 	int index = 0;	
// 	while(i<=R1&&j<=R2){//是≤不是＜ 
// 		if(SecComp(SC, C, A[i], A[j])){
// 			temp[index++] = A[i++];
// 		}else{
// 			temp[index++] = A[j++];
// 		}
// 	}
// 	while(i<=R1)temp[index++] = A[i++];//这个while很容易写成if 
// 	while(j<=R2)temp[index++] = A[j++];
// 	for(int i=0;i<index;i++){
// 		A[L1+i] = temp[i];
// 	}
//     delete[]temp;
// }

// void SecMergesort(Coordinator * SC, Client * C, int A[],int n){//非递归实现的归并排序 
// 	//step为组内元素个数(每归并一次乘二)
// 	for(int step=2;step/2<n;step*=2){
// 		//每step个元素一组，组内左一半step/2和右一半step/2的元素合并，注意有很多个这样的组
// 		for(int i=0;i<n;i+=step){
// 			int mid = i+step/2-1;//step/2为左子区间元素个数
// 			if(mid+1<n){//右子区间存在元素则合并 
// 				SecMerge(SC,C,A,n,i,mid,mid+1,min(i+step-1,n));
// 			}
// 		} 
// 	} 
// }

int main(){
    int E[6]={2,4,5,6,8,10};//E[5]={2,4,5,6,8};E[3]={2,4,5}; //如果有元素是10，则会报错段错误
    double sta_u[6]={1.3, 3.5, 2.5, 2.54, 5.7, 1.2};//sta_u[5]={1.3, 3.5, 2.5, 2.54, 5.7};sta_u[3]={1.3, 3.5, 2.5};
    int T = 10;
    int L[20]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; //一开始为L[10]={1}，可是这样只是L[0]=1，其他元素等于0
    double D[20]={11,  12, 1, 13, 1 ,1, 1, 14, 1, 1, 15, 16,17,18,19,20,21,22,23,24};//{7, 9 , 30, 28, 5 ,7, 11, 27, 5, 8};
    //int P[20]={2,4,5,6,8,9,0,1,3,7,10,11,12,13,14,15,16,17,18,19};//P[5]={2,4,5,6,8};P[3]={2,4,5}; //前五个为上一轮被选择的客户，后五个为未被选择的
    double ssu[100];
    for(int i =2; i<6;i++){
        cout<<endl;
        cout<<"The "<<i<<" instance: "<<endl;
        cout<<endl;
        secure_participant_selection(26, 6,  0.8, 1, 20, E, sta_u, D, L, ssu, 2);
        cout<<"E: ";
        for(int j=0; j<6; j++){
           cout<<E[j]<<":"<<sta_u[j]<<", "; //输出检查发现stau发生了改变
        }
        cout<<endl;
    }
    
    return 0;
}