#开发人员端
import CIFAR
import FL_runner_CIFAR
import tensorflow as tf
import numpy as np
import time
#import psutil

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

# print("Split Data:")
start_time = time.time()
trDs,trLs,teDs,teLs,total_testD,total_testL=CIFAR.Load_CIFAR10()
# print("Split Data Finished!")

global_model=FL_runner_CIFAR.CNN_model_factory()
global_weight=global_model.get_weights()

m = 6
L=np.zeros(500)
""" while m<=14:
    print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_CIFAR.txt",'a')
    server1, FLtime_cost, L = FL_runner_CIFAR.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 0,total_testD,total_testL)
    acc,l = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
    k = acc*100
    # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
    print("E for ",m,"selected participants in secure selection: "," acc-",'%.4f' % (acc*100), file=print_log)
    print("loss: ",L, file=print_log)
    m+=2
    print_log.close() """

print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_CIFAR.txt",'a')
server1, FLtime_cost, L = FL_runner_CIFAR.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 1,total_testD,total_testL)
acc,l = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
k = acc*100
# print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
print("E for ",m,"selected participants in secure selection: "," acc-",'%.4f' % (acc*100), file=print_log)
print("loss: ",L, file=print_log)
print(time.ctime(), file=print_log)
#m+=2
print_log.close()

end_time = time.time()
elapsed_time = end_time - start_time
print(f"运行时间: {FLtime_cost}  秒")
print(f"运行时间: {elapsed_time}  秒")


# m=6
# while m<=14:
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log_CIFAR.txt",'a')
#     server1, FLtime_cost = FL_runner_CIFAR.run_FL(trDs,trLs,teDs,teLs,global_weight, 20, m, 3, total_testD,total_testL)
#     acc = server1.evaluate(total_testD,total_testL, 'FL with oort in test data')
#     k = acc*100
#     # print("E for ",m,"malicious participants: ","timecost-",FLtime_cost," acc-",'%.4f' % (acc*100))
#     print("E for ",m,"selected participants in random selection: "," acc-",'%.4f' % (acc*100), file=print_log)
#     print("loss: ",L, file=print_log)
#     m+=2
#     print_log.close()