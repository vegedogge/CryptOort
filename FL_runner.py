from tensorflow.keras import layers, models
import tensorflow as tf
import client 
import server 
# from Crypto.PublicKey import RSA
import numpy as np
import hashlib
import binascii
# import gmpy2
import os

rand_bits = 128

def CNN_model_factory():
  model = models.Sequential()
  model.add(layers.Conv2D(30, (3, 3), activation='relu', input_shape=(28, 28, 1))) #若要训练单通道的mnist数据集，则input_shape=(28, 28, 1)
                                                                                   #若训练3通道的CIFAR-10数据集，则input_shape=(32, 32, 3)
  model.add(layers.MaxPooling2D((2, 2)))
  model.add(layers.Conv2D(50, (3, 3), activation='relu'))
  model.add(layers.MaxPooling2D((2, 2)))
  model.add(layers.Flatten())
  model.add(layers.Dense(100, activation='relu'))
  model.add(layers.Dense(10,activation='softmax'))
  return model
  
def run_FL(Tr_Ds, Tr_Ls, Te_Ds, Te_Ls, global_weight, nclient, nselect_client, signal, total_testD,total_testL):
  beta  = 0.5
  alpha = 0.5/beta
  model_factory = CNN_model_factory
  clients = []

  # 循环创建多个客户端对象，每个客户端使用各自的训练数据
  for i in range(0,nclient):
    clients.append(client.Client(0, model_factory, Tr_Ds[i], Tr_Ls[i], Te_Ds[i], Te_Ls[i], learning_rate=beta, R=1, batch_size=32))
    print("Len of",i,"'s traindata and testdata: ",len(Tr_Ds[i]),len(Te_Ds[i]))
  # #设置恶意客户
  # for i in range(nclient-nmalicious,nclient):
  #   clients.append(client.Client(1, model_factory, Tr_Ds[i], Tr_Ls[i], Te_Ds[i], Te_Ls[i], learning_rate=beta, R=1, batch_size=64))
  #   malious.append(clients[i-1])
  #   print("Len of",i,"'s traindata and testdata: ",len(Tr_Ds[i]),len(Te_Ds[i]))
  #初始化SP
  server1 = server.Server(model_factory, global_weight, nselect_client, iteration=500, alpha=alpha, beta=beta)

  time_cost, Los=server1.FLtrain(clients,'MNIST',signal, total_testD,total_testL)
  
  return server1, time_cost, Los