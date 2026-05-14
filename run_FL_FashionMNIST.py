# #开发人员端
# import mnist
# import FL_runner

# # print("Split Data:")
# trDs,trLs,teDs,teLs,total_testD,total_testL=mnist.Load_FashionMNIST()
# # print("Split Data Finished!")

# global_model=FL_runner.CNN_model_factory()
# global_weight=global_model.get_weights()

# m=6
# while m<=14:
#     server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 0)
#     acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/IJ24_fM.txt",'a')
#     print("E for ",m,"selected participants in secure selection: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100), file=print_log)
#     print_log.close()
#     #if k > 10:
#     m+=2
# m=6
# while m<=14:
#     server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1)
#     acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/IJ24_FM.txt",'a')
#     print("E for ",m,"selected participants in Oort selection: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100), file=print_log)
#     print_log.close()
#     #if k > 10:
#     m+=2

# #server3.evaluate(x_test,y_test, 'FLTrust in test data with FL attack')

#开发人员端
import mnist
import FL_runner
import tensorflow as tf
import time
import numpy as np


import os
os.environ["CUDA_VISIBLE_DEVICES"] = "-1"  # 这一行注释掉就是使用gpu，不注释就是使用cpu

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

trDs,trLs,teDs,teLs,total_testD,total_testL=mnist.Load_FashionMNIST()
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
m=8
L = np.zeros(500)
""" while m<=14:
    print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log1.txt",'a')
    start = time.time()
    #server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1)
    server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1, total_testD, total_testL)
    end = time.time()
    acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
    k = acc*100
    # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
    print("E for ",m,"selected participants in Oort selection: ","timecost-",end-start," acc-",'%.4f' % (acc*100), file=print_log)
    #if k > 10:
    m+=2
    print_log.close() """

print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log1.txt",'a')
#start = time.time()
#server1, FLtime_cost = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1)
server1, FLtime_cost, L = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 0, total_testD, total_testL)

acc, l = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
k = acc*100
# print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
print("E for ",m,"selected participants in Oort selection: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100), file=print_log)
print("loss: ",L, file=print_log)
print_log.close()
#end = time.time()
#elapsed_time = end - start
print(f"运行时间: {FLtime_cost}  秒")
#print(f"运行时间: {elapsed_time}  秒")
#server3.evaluate(x_test,y_test, 'FLTrust in test data with FL attack')
# m=14
# L=np.zeros(500)
# while m<=14:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_log_IJ24FM.txt",'a')
#     start = time.time()
#     server1, FLtime_cost, L = FL_runner.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 2, total_testD,total_testL)
#     end = time.time()
#     acc,l = server1.evaluate(total_testD,total_testL, 'FL with our in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print("E for ",m,"selected participants in secure selection: "," acc-",'%.4f' % (acc*100), file=print_log)
#     print("loss: ",L, file=print_log)
#     #if k > 10:
#     m+=2
#     print_log.close()

# m=15
# while m<=13:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_IJ24FM.txt",'a')
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