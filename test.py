# from ctypes import *
# import ctypes
# import os
# import numpy as np
# #加载动态库
# lib = ctypes.CDLL('./clib/SecSelect.so')
# # 定义 C++ 函数参数和返回值类型
# # lib.secure_participant_selection.argtypes=[ctypes.c_int, #客户响应时间阈值
# #                                            ctypes.c_int, #响应时间阈值更新步长
# #                                            ctypes.c_int, #要选择的客户数量
# #                                            ctypes.c_float, #从已开发的客户中选择的客户比例
# #                                            ctypes.c_int, #时间阈值更新的间隔轮数
# #                                            ctypes.c_float, #超过响应时间阈值的客户的惩罚
# #                                            ctypes.c_int, #客户数量
# #                                            np.ctypeslib.ndpointer(dtype=np.int32, ndim=1, flags='C_CONTIGUOUS'), #上一轮训练开发的客户
# #                                            np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #上一轮训练被开发的客户的统计效用
# #                                            np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #所有客户的响应时间
# #                                            np.ctypeslib.ndpointer(dtype=np.int32, ndim=1, flags='C_CONTIGUOUS'), #客户上一次被选中开发的轮数
# #                                            np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #每一轮的客户的统计效用总和
# #                                            ctypes.c_int #当前训练轮数
# #                                            ]
# lib.secure_participant_selection.argtypes=[ctypes.c_double, #客户响应时间阈值
#                                            ctypes.c_int, #要选择的客户数量
#                                            ctypes.c_float, #从已开发的客户中选择的客户比例
#                                            ctypes.c_float, #超过响应时间阈值的客户的惩罚
#                                            ctypes.c_int, #客户数量
#                                            ctypes.POINTER(ctypes.c_int), #上一轮训练开发的客户
#                                            ctypes.POINTER(ctypes.c_double), #上一轮训练被开发的客户的统计效用
#                                            ctypes.POINTER(ctypes.c_double), #所有客户的响应时间
#                                            ctypes.POINTER(ctypes.c_int), #客户上一次被选中开发的轮数
#                                            ctypes.POINTER(ctypes.c_double), #每一轮的客户的统计效用总和
#                                            ctypes.c_int #当前训练轮数
#                                            ]
# lib.secure_participant_selection.restype = None
# # lib.secure_participant_selection.restype = ctypes.POINTER(ctypes.c_int)

# # #定义参数
# time_threshold = ctypes.c_double(26)
# threshold_step = ctypes.c_double(5)
# nselect = ctypes.c_int(6)
# ratio = ctypes.c_float(0.8)
# T_interval = ctypes.c_int(10)
# punish = ctypes.c_float(1)
# num_clients = ctypes.c_int(20)
# E=np.array([2,4,5,6,8,9], dtype=np.int32)#[2,4,5,6,8,9][10,0,7,12,8,17] #如果有个10，就会报错段错误
# sta_u=np.array([1.3, 3.5, 2.5, 2.54, 5.7, 1.2], dtype=np.float64)#sta_u[5]={1.3, 3.5, 2.5, 2.54, 5.7};sta_u[3]={1.3, 3.5, 2.5};
# L=np.array([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1], dtype=np.int32)#一开始为L[10]={1}，可是这样只是L[0]=1，其他元素等于0
# D=np.array([11,12,1,13,1,1,1,14,1,1,15,16,17,18,19,20,21,22,23,24], dtype=np.float64)#{7, 9 , 30, 28, 5 ,7, 11, 27, 5, 8};
# #P=np.array([2,4,5,6,8,9,0,1,3,7,10,11,12,13,14,15,16,17,18,19], dtype=np.int32)#P[5]={2,4,5,6,8};P[3]={2,4,5}; //前五个为上一轮被选择的客户，后五个为未被选择的
# ssu=np.zeros(5,dtype=np.float64)

# E_pointer =E.ctypes.data_as(POINTER(c_int))
# sta_u_pointer = sta_u.ctypes.data_as(POINTER(c_double))
# L_pointer = L.ctypes.data_as(POINTER(c_int))
# D_pointer = D.ctypes.data_as(POINTER(c_double))
# ssu_pointer = ssu.ctypes.data_as(POINTER(c_double))
# #调用参与者筛选函数
# for r in range(1,5):
#     print(r+1, "instance: ")
#     lib.secure_participant_selection(time_threshold, nselect, ratio, punish, num_clients, 
#                                      E_pointer, sta_u_pointer, D_pointer, L_pointer, ssu_pointer, ctypes.c_int(r+1))
#     # temp = lib.secure_participant_selection(time_threshold, threshold_step, nselect, ratio, T_interval, punish, num_clients, E, sta_u, D, L, ssu, ctypes.c_int(r+1))
#     # E = np.ctypeslib.as_array(temp, (1,))
#     #我认为很可能是python将内存释放了，然后lib.secure_participant_selection又会访问这块被释放的内存，导致报错
#     # print("temp: ",temp)

#     print("Selection finished!")
#     print(ssu[r])
#     print(E)

# 测试文件读写
# for i in range(5):
#     print_log = open("/home/Yang/PycharmProjects/ZengHuang/FL/privacy_participant_selection/log1.txt",'a')
#     print(i, file=print_log)
#     print_log.close()

# # 测试gpu功能
# import tensorflow as tf
# from tensorflow.keras import datasets, layers, models

# # 设置GPU内存增长
# gpus = tf.config.experimental.list_physical_devices('GPU')
# for gpu in gpus:
#     tf.config.experimental.set_memory_growth(gpu, True)

# # 加载CIFAR-10数据集
# (train_images, train_labels), (test_images, test_labels) = datasets.cifar10.load_data()

# # 将像素值缩放到0到1之间
# train_images, test_images = train_images / 255.0, test_images / 255.0

# # 定义卷积模型
# model = models.Sequential([
#     layers.Conv2D(32, (3, 3), activation='relu', input_shape=(32, 32, 3)),
#     layers.MaxPooling2D((2, 2)),
#     layers.Conv2D(64, (3, 3), activation='relu'),
#     layers.MaxPooling2D((2, 2)),
#     layers.Conv2D(64, (3, 3), activation='relu'),
#     layers.Flatten(),
#     layers.Dense(64, activation='relu'),
#     layers.Dense(10)
# ])

# # 编译模型
# model.compile(optimizer='adam',
#               loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
#               metrics=['accuracy'])

# # 训练模型
# history = model.fit(train_images, train_labels, epochs=100,
#                     validation_data=(test_images, test_labels))

# #测试排序函数效果
# import numpy as np

# a = [3, 1, 4, 2, 5, 9]  # 数组a
# weights = [30, 20, 10, 40, 50, 60]  # 对应的权重数组

# # 使用zip函数将数组a和权重数组打包在一起，然后按照权重进行排序
# sorted_result = [x for _, x in sorted(zip(weights, a))]

# # 输出排序后的结果
# print(sorted_result)

#测试tensorflow与gpu驱动的兼容
import tensorflow as tf

# 获取可见的 GPU 设备列表
gpus = tf.config.experimental.list_physical_devices('GPU')

if gpus:
    # 获取第一块 GPU 的详细信息（假设有多块 GPU，这里只获取第一块）
    gpu = gpus[0]

    # 打印 CUDA 版本
    print("CUDA version: ", tf.test.is_built_with_cuda())

    # 获取 GPU 设备名称
    gpu_name = gpu.name

    # 获取 cuDNN 版本信息
    cudnn_version = tf.strings.split(tf.strings.split(gpu_name, "compute ")[-1], "/")[0]
    print("cuDNN version: ", cudnn_version.numpy().decode())
else:
    print("No GPU found")
