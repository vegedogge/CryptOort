import tensorflow as tf
import numpy as np
import hashlib
import binascii
import os
import random
import time
import copy

rand_bits = 128


class Client:
  def __init__(self, ismalicious, model_factory, Tr_D, Tr_L, Te_D, Te_L, learning_rate, R, batch_size):
    self.attacker = False  # 是否启用攻击
    self.malious = ismalicious # 客户是否是恶意的
    self.labelflip = False  # 标签翻转攻击
    self.threat_model = None   # 客户威胁模型
    self._x = Tr_D #训练数据
    self._y = Tr_L #训练数据标签
    self._x0 = Te_D #测试数据
    self._y0 = Te_L #测试数据标签
    self.label_set = [0,1,2,3,4,5,6,7,8,9]
    self._model = model_factory()
    self.learning_rate = learning_rate
    self.epochs = R  # 这个是自定义选择的
    self.batch_size = batch_size
    self.steps_per_epoch = 1

  def FLtrain(self, server_weights):
    """ if self.malious == 1: #客户是恶意的，翻转一次图片的label
      if self.labelflip==False:
        label0=[1,0,0,0,0,0,0,0,0,0]
        for i in range(len(self._y)):
          self._y[i]=label0
        self.labelflip==True """

    self._model.compile(  # compile 是keras 自带的编译函数，用于配置训练方法
        optimizer=tf.keras.optimizers.SGD(learning_rate=self.learning_rate),  # 学习率
        loss = tf.keras.losses.CategoricalCrossentropy(), # 执行损失函数
        metrics=['accuracy']  # 获取准确率指标，模型自带
      )
    self._model.set_weights(server_weights)
    # print("Date shape of self._x and _y: ", self._x.shape, self._y.shape) #数据形状有问题
    loss = self.evaluate(self._x, self._y, " Test in local of client") #获取全局模型在客户本地的损失值

    #执行一轮本地训练获取梯度
    self._model.fit(x = self._x, y = self._y, verbose=0,
                    epochs = self.epochs, batch_size = self.batch_size, 
                    steps_per_epoch=self.steps_per_epoch,  # steps_per_epoch=1, 这意味着在全局过程中我们只先执行一次训练，正常来说每个epoch应该执行 数据集大小/batch_size次
                    # reduce_retracing=True
                    )
    new_weights = self._model.get_weights()  # get_weights() 系统自带，获取模型参数
    delta_weights = [new_w - old_w for new_w, old_w in zip(new_weights, server_weights)] #将刚得到的本地梯度和上一轮的全局梯度作差

    return delta_weights, loss
  
  def train_local(self, server_weights, iters): #本地训练模型，不参与FL
    self._model.compile(
      optimizer=tf.keras.optimizers.SGD(learning_rate=self.learning_rate),
      loss = tf.keras.losses.CategoricalCrossentropy(),
      metrics=['accuracy']
    )
    self._model.set_weights(server_weights)
    print("success!")
    self._model.fit(x = self._x, y = self._y, verbose=0,  # verbose=0表示不打印训练过程, 1表示打印训练过程, 2表示输出每个周期的日志信息
                    epochs = iters, batch_size = self.batch_size, 
                    #steps_per_epoch=self.steps_per_epoch, 这里就正常进行训练了，steps_per_epoch= 数据集大小/batch_size；前面的 FLtranin()函数中，steps_per_epoch=1，只执行一次训练
                    )
    print("success!")
    new_weights = self._model.get_weights()
    delta_weights = [new_w - old_w for new_w, old_w in zip(new_weights, server_weights)] #梯度

    return delta_weights
  
  def evaluate(self, x_test, y_test, expr_basename): # expr_basename 表示当前实验的名称
    loss, acc = self._model.evaluate(x_test, y_test, verbose=0)  # evaluate() 系统自带，用于评估模型在测试集上的表现，返回损失值和准确率
    print(f'{expr_basename} loss: {loss} - accuracy: {acc:.2%}')  # 打印损失值和准确率，例如 Test in local of client： loss: 0.0 - accuracy: 100.00%
    return loss
  
  def isMalicious(self):
     return self.malious