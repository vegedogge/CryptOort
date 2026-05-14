#开发人员端
import mnist
import FL_runner
import tensorflow as tf
import time
import numpy as np


import os
# os.environ["CUDA_VISIBLE_DEVICES"] = "-1"  # 这一行注释掉就是使用gpu，不注释就是使用cpu

# gpus = tf.config.experimental.list_physical_devices('GPU')
# if gpus:
#     try:
#         # 设置GPU内存增长
#         for gpu in gpus:
#             tf.config.experimental.set_memory_growth(gpu, True)
#         # 设置训练时使用的GPU设备
#         tf.config.experimental.set_visible_devices(gpus[0], 'GPU')
#     except RuntimeError as e:
#         print(e)

trDs,trLs,teDs,teLs,total_testD,total_testL=mnist.Load_MNIST()
# print("Shape of trds and trls: ", trDs[0].shape, trLs[0].shape)

# 接下来确定初始模型的参数

global_model=FL_runner.CNN_model_factory()
# np.random.seed(11)# 设置随机种子
# new_weights = []
# for layer in global_model.layers:
#     weights = []
#     for w in layer.get_weights():
#         weights.append(np.random.rand(*w.shape))  # 生成随机参数
#     new_weights.extend(weights)
# global_model.set_weights(new_weights)
global_weight=global_model.get_weights()

# m=6
# while m<=14:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log1.txt",'a')
#     start = time.time()
#     server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 0)
#     end = time.time()
#     acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print("E for ",m,"selected participants in secure selection: ","timecost-",end-start," acc-",'%.4f' % (acc*100), file=print_log)
#     #if k > 10:
#     m+=2
#     print_log.close()
# m=6
# while m<=14:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log1.txt",'a')
#     start = time.time()
#     server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1)
#     end = time.time()
#     acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print("E for ",m,"selected participants in Oort selection: ","timecost-",end-start," acc-",'%.4f' % (acc*100), file=print_log)
#     #if k > 10:
#     m+=2
#     print_log.close()
# #server3.evaluate(x_test,y_test, 'FLTrust in test data with FL attack')
m=6
L=np.zeros(500)
while m<=9:
    print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_IJACSA2024v2.txt",'a')
    #start = time.time()
    server1, FLtime_cost, L = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 2, total_testD,total_testL)
    #end = time.time()
    acc,l = server1.evaluate(total_testD,total_testL, 'FL with our in test data')
    k = acc*100
    # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
    print("E for ",m,"selected participants in secure selection: "," acc-",'%.4f' % (acc*100), file=print_log)
    print("耗时:", FLtime_cost)
    print("loss: ",L, file=print_log)
    #if k > 10:
    m+=2
    print_log.close()

# m=10
# while m<=11:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_IJACSA2024v2.txt",'a')
#     start = time.time()
#     server1, FLtime_cost, L = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 3, total_testD,total_testL)
#     end = time.time()
#     acc,l = server1.evaluate(total_testD,total_testL, 'FL with random in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print("E for ",m,"selected participants in random selection: "," acc-",'%.4f' % (acc*100), file=print_log)
#     print("loss: ",L, file=print_log)
#     #if k > 10:
#     m+=2
#     print_log.close()











# import sysp
# import os
# CURRENT_DIR = os.path.split(os.path.abspath(__file__))[0]  # 当前目录
# config_path = CURRENT_DIR.rsplit('/', 2)[0]  # 上三级目录
# sys.path.append(config_path)


# import experiments.mnist.experiment_runner as experiment_runner

# experiment_runner.run_no_attacks('expr_no_attacks', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45, alpha=0.1,
#                                  t_mean_beta=0.1)

# experiment_runner.run_no_attacks('expr_no_attacks', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45, alpha=0.1,
#                                  t_mean_beta=0.1)

# experiment_runner.run_all('expr_to_zero_10_precent', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45, real_alpha=0.1,
#                           num_samples_per_attacker=
#                           1_000_000, attack_type='delta_to_zero', alpha=0.1, t_mean_beta=0.1)

# experiment_runner.run_all('expr_to_y_flip_10_precent', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45,
#                           real_alpha=0.1,
#                           num_samples_per_attacker=1_000_000, attack_type='y_flip', alpha=0.1, t_mean_beta=0.1)

# experiment_runner.run_all('expr_to_zero_single', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45, real_alpha=0.1,
#                           num_samples_per_attacker=10_000_000, attack_type='delta_to_zero', alpha=1, t_mean_beta=0.1,
#                           real_alpha_as_f=True)

# experiment_runner.run_all('expr_y_flip_single', seed=1, cpr='all', rounds=100, mu=1.5, sigma=3.45,
#                           real_alpha=0.1,
#                           num_samples_per_attacker=10_000_000, attack_type='y_flip', alpha=1, t_mean_beta=0.1,
#                           real_alpha_as_f=True)
