import numpy as np
import tensorflow as tf
import copy
# from Crypto.PublicKey import RSA
import hashlib
import binascii
# import gmpy2
import os
import random
import math
import time
import ctypes
lib = ctypes.CDLL('./clib/SecSelect.so') #c++函数库
# 定义 C++ 函数参数和返回值类型
lib.secure_participant_selection.argtypes=[ctypes.c_double, #客户响应时间阈值
                                           ctypes.c_int, #要选择的客户数量
                                           ctypes.c_float, #从已开发的客户中选择的客户比例
                                           ctypes.c_float, #超过响应时间阈值的客户的惩罚
                                           ctypes.c_int, #客户数量
                                           np.ctypeslib.ndpointer(dtype=np.int32, ndim=1, flags='C_CONTIGUOUS'), #上一轮训练开发的客户
                                           np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #上一轮训练被开发的客户的统计效用
                                           np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #所有客户的响应时间
                                           np.ctypeslib.ndpointer(dtype=np.int32, ndim=1, flags='C_CONTIGUOUS'), #客户上一次被选中开发的轮数
                                           np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags='C_CONTIGUOUS'), #每一轮的客户的统计效用总和
                                           ctypes.c_int #当前训练轮数
                                           ]
lib.secure_participant_selection.restype = None

# network_delay = [[1,100], #省内网络延迟范围
#                   [50,1000], #国内省际内网络延迟范围
#                   [500,5000] #国外国际网络延迟范围
#                 ]
# compute_cost = [[1,1], #服务器计算速度
#                   [200,1000], #i5芯片计算时间开销相比服务器的倍数
#                   [2000,10000] #手机芯片计算时间开销相比服务器的倍数
#                 ]

class Server:
  def __init__(self, model_factory, global_weight, nselect, iteration, alpha, beta):
    self._model = model_factory()
    self.nselect_client = nselect  # 选择的客户端数量
    self.global_itera = iteration  # 全局迭代次数
    self.alpha = alpha    
    self._model.set_weights(global_weight)
    self._model.compile(
      optimizer=tf.keras.optimizers.SGD(learning_rate=beta),
      loss = tf.keras.losses.CategoricalCrossentropy(),   # 使用交叉熵损失函数
      metrics=['accuracy']
    )

    #以下参数用于筛选
    self.time_threshold = ctypes.c_double(25) #响应时间阈值，初始化为 25
    self.threshold_step = ctypes.c_double(0.1) #响应时间阈值更新步长，初始化为0.1
    self.ratio = ctypes.c_float(0.8) #从上一轮参与训练的客户中进行筛选的比例
    self.T_interval = ctypes.c_int(25) #响应时间阈值的跟新间隔
    self.punish = ctypes.c_float(1) #响应时间超过阈值的惩罚因子

  def evaluate(self, x_test, y_test, expr_basename):
    loss, acc = self._model.evaluate(x_test, y_test, verbose=0)
    print(f'{expr_basename} loss: {loss} - accuracy: {acc:.2%}')
    return acc, loss

  # selected_index 存储已被选择参与当前轮次训练的客户端索引，Lastinvolved存储参与训练的客户端上一次参与训练的轮次  
  def participants_selection(self, nclients, selected_index, round, Duration, Lastinvolved, sta_u, ssu):

    client_poor=np.zeros(nclients, dtype=np.int32)#客户池，前一部分元素为上一轮被选择的客户，剩余元素为未被选择的，元素的范围为0到19
    selected = np.zeros(nclients) # 标识0-19号客户是否被选中，0：未被选，1：被选

    if round==0: # 如果是第一轮训练，则按响应时间选择客户
      sort_D = np.argsort(Duration) # 将D的元素索引按元素值从小到大排序
      # print("Sort_D: ", sort_D)
      
      cut = math.ceil(nclients * 0.05) # 截断的客户数

      #依概率选择客户
      m=0
      upper = len(sort_D)-cut
      for i in range(cut,upper):
        m+=i
      prob = [(upper-i)/m for i in range(cut,upper)] #客户被选中的概率
      sort_D = sort_D[cut:-cut] #将客户数组截断
      for i in range(self.nselect_client):
        index = random.choices(sort_D, weights=prob, k=1) # 依概率选择客户
        while selected[index[0].astype(int)]!=0:
          index = random.choices(sort_D, weights=prob, k=1) # 依概率选择客户
        client_poor[i] = index[0].astype(int)
        selected[index[0].astype(int)]=1

    if round>0:#进行参与者筛选协议
      if round>=2*self.T_interval.value: #跟新响应时间阈值，按方案设计，这部分应该在协议执行部分，实际上，也可以在协议执行之前
        s1=0
        s2=0
        for i in range(round-2*self.T_interval.value,round-self.T_interval.value):   s1+=ssu[i]
        for i in range(round-self.T_interval.value,round): s2+=ssu[i]
        if s1>s2:
          self.time_threshold.value = self.time_threshold.value+self.threshold_step.value
          print("updating T: ", self.time_threshold.value)

      lib.secure_participant_selection(self.time_threshold, ctypes.c_int(self.nselect_client), 
                                       self.ratio, self.punish, ctypes.c_int(nclients), 
                                       selected_index, sta_u, Duration, Lastinvolved, ssu, ctypes.c_int(round+1))
      for i in range(self.nselect_client):
        client_poor[i] = selected_index[i]
        selected[selected_index[i]] = 1
    
    i=self.nselect_client
    while i<len(client_poor): # 更新客户池
      for j in range(nclients):
         if selected[j] == 0:
          client_poor[i] = j
          i=i+1
      
    # print("Client poor: ", client_poor)
    # print("ssu: ", ssu)
    return client_poor, ssu
    # selected_clients = [clients[i] for i in selected_index] #更新被选中的客户列表
  
  def Oort_select(self, nclients, selected_client, round, Duration, Lastinvolved, sta_u, ssu):
    
    client_poor=np.zeros(nclients, dtype=np.int32)#客户池，前一部分元素为上一轮被选择的客户，剩余元素为未被选择的，元素的范围为0到19
    selected = np.zeros(nclients) # 标识0-19号客户是否被选中，0：未被选，1：被选
    selected_index = np.zeros(self.nselect_client, dtype=np.int32) #被选中的客户编号

    if round==0: # 如果是第一轮训练，则按响应时间选择客户
      sort_D = np.argsort(Duration) # 将D的元素索引按元素值从小到大排序
      # print("sort_D:", sort_D)
      cut = math.ceil(nclients * 0.05) # 截断的客户数
      
      #依概率选择客户
      m=0
      upper = len(sort_D)-cut
      for i in range(cut,upper):
        m+=i
      prob = [(upper-i)/m for i in range(cut,upper)] #客户被选中的概率
      sort_D = sort_D[cut:-cut] #将客户数组截断
      for i in range(self.nselect_client):
        index = random.choices(sort_D, weights=prob, k=1) # 依概率选择客户
        while selected[index[0].astype(int)]!=0:
          index = random.choices(sort_D, weights=prob, k=1) # 依概率选择客户
        selected_index[i] = index[0].astype('int32') # 随机抽取输出的index是一个包含一个浮点元素的列表
        client_poor[i] = index[0].astype(int)
        selected[index[0].astype(int)]=1
      # print("selected_index: ", selected_index)

    if round>0:
      if round>=2*self.T_interval.value: #跟新响应时间阈值，按方案设计，这部分应该在协议执行部分，实际上，也可以在协议执行之前
        s1=0
        s2=0
        for i in range(round-2*self.T_interval.value,round-self.T_interval.value):   s1+=ssu[i]
        for i in range(round-self.T_interval.value,round): s2+=ssu[i]
        if s1>s2:
          self.time_threshold.value = self.time_threshold.value+self.threshold_step.value
          print("updating T: ", self.time_threshold.value)

      U=np.zeros(self.nselect_client)
      sysu=[pow(self.time_threshold.value/d, (self.time_threshold.value<d)) for d in Duration]
      for i in range(self.nselect_client):
        U[i]=(sta_u[i]+pow((0.1*math.log(round)/Lastinvolved[selected_client[i]]),0.5))*sysu[selected_client[i]] # 计算每个上一轮参与训练的客户的效用值
      sort_U = np.argsort(U)
      # print("U: ",U)
      # print("sort_U: ",sort_U)

      #首先从上一轮参与训练的客户中选择0.8*self.nselect_client个
      cut=0.95*U[sort_U[self.nselect_client-math.floor(0.8*self.nselect_client)]]
      # print("sort_U[self.nselect_client-math.floor(0.8*self.nselect_client)]: ", sort_U[self.nselect_client-math.floor(0.8*self.nselect_client)])
      W=[]
      sumutil=0
      for i in range(self.nselect_client):
        if U[i]>cut:
          W.append(selected_client[i])
          sumutil+=U[i]
      # print("W: ", W)
      prob=np.zeros(nclients)
      for i in range(self.nselect_client):
        for j in range(len(W)):
          if selected_client[i]==W[j]:
            prob[selected_client[i]]=U[i]/sumutil

      # print("prob:", prob)
      for i in range(math.floor(0.8*self.nselect_client)):
        index = random.choices(range(nclients), weights=prob, k=1) # 依概率选择客户
        while (selected[index[0]]!=0 or prob[index[0]]==0):
          index = random.choices(range(nclients), weights=prob, k=1) # 依概率选择客户
        selected_index[i] = index[0] # 随机抽取输出的index是一个包含一个浮点元素的列表
        # print("selected_index[i]", selected_index[i])
        client_poor[i] = index[0]
        selected[index[0]]=1
      #依据响应速度从上一轮未参与训练的客户中选择self.nselect_client-0.8*self.nselect_client个
      for i in range(self.nselect_client): #将上一轮参预训练的客户全部标记为1
        for j in range(math.floor(0.8*self.nselect_client)):
          if selected_client[i] == selected_index[j]:
            selected[selected_client[i]]=1
        if selected[selected_client[i]]==0:
          selected[selected_client[i]]=-1 #上一轮参与训练但是没有参与依概率筛选的客户标记为-1
      sumd=0
      prob2=np.zeros(nclients)
      for i in range(nclients):
        if selected[i]==0: sumd+=Duration[i]
      for i in range(nclients):
        if selected[i]==0: prob2[i]=Duration[i]/sumd
      # print("prob2:",prob2)
      for i in range(self.nselect_client-math.floor(0.8*self.nselect_client)):
        index = random.choices(range(nclients), weights=prob2, k=1) # 依概率选择客户
        # print("index: ", index, index[0])
        while (selected[index[0]]!=0 or prob2[index[0]]==0):
          index = random.choices(range(nclients), weights=prob2, k=1) # 依概率选择客户
        selected_index[i+math.floor(0.8*self.nselect_client)] = index[0] # 随机抽取输出的index是一个包含一个浮点元素的列表
        # print("selected_index[i]", selected_index[i+math.floor(0.8*self.nselect_client)])
        client_poor[i+math.floor(0.8*self.nselect_client)] = index[0]
        selected[index[0]]=1

    
    i=self.nselect_client
    while i<len(client_poor): # 更新客户池
      for j in range(nclients):
         if selected[j] == 0 or selected[j] == -1:
          client_poor[i] = j
          i=i+1
      
    return client_poor, ssu

  def select_by_loss(self, round, nclients, selected_client, loss):

    client_poor=np.zeros(nclients, dtype=np.int32)#客户池，前一部分元素为上一轮被选择的客户，剩余元素为未被选择的，元素的范围为0到19
    selected = np.zeros(nclients) # 标识0-19号客户是否被选中，0：未被选，1：被选
    selected_index = np.zeros(self.nselect_client, dtype=np.int32) #被选中的客户编号

    if round==0: # 如果是第一轮训练，则随机选择客户
      prob = [1/20 for i in range(20)] #客户被选中的概率
      client_id = range(20)
      for i in range(self.nselect_client):
        index = random.choices(client_id, weights=prob, k=1) # 依概率选择客户
        while selected[index[0]]!=0:
          index = random.choices(client_id, weights=prob, k=1) # 依概率选择客户
        selected_index[i] = index[0] # 随机抽取输出的index是一个包含一个浮点元素的列表
        client_poor[i] = index[0]
        selected[index[0]]=1
      # print("selected_index: ", selected_index)
      # print("client_poor: ", client_poor)

    if round>0:
      sort_client = [x for _, x in sorted(zip(loss, selected_client))]
      for i in range(0, math.ceil(0.8*self.nselect_client)):
        selected_index[i] = sort_client[i].astype('int32')
        client_poor[i] = sort_client[i]
        selected[sort_client[i]]=1
      for i in range(math.ceil(0.8*self.nselect_client), self.nselect_client):
        selected[sort_client[i]]=1
      prob2 = [1/20 for i in range(20)] #客户被选中的概率
      client_id = range(20)
      for i in range(math.ceil(0.8*self.nselect_client), self.nselect_client):
        index = random.choices(client_id, weights=prob2, k=1) # 依概率选择客户
        while selected[index[0]]!=0:
          index = random.choices(client_id, weights=prob2, k=1) # 依概率选择客户
        selected_index[i] = index[0] # 随机抽取输出的index是一个包含一个浮点元素的列表
        client_poor[i] = index[0]
        selected[index[0]]=1
  

    return client_poor
    
  def select_by_random(self):
    if self.nselect_client > 20:
        return "Error: n 不能大于 20"
    
    # 生成包含 0 到 19 的数字列表
    numbers = list(range(20))
    
    # 从数字列表中随机选择 n 个数字
    client_poor = random.sample(numbers, self.nselect_client)
    
    return client_poor

  # def select_by_random(self, nclients):

  #   client_poor=np.zeros(nclients, dtype=np.int32)#客户池，前一部分元素为上一轮被选择的客户，剩余元素为未被选择的，元素的范围为0到19
  #   selected = np.zeros(nclients) # 标识0-19号客户是否被选中，0：未被选，1：被选

  #   prob = [1/20 for i in range(20)] #客户被选中的概率
  #   client_id = range(20)
  #   for i in range(self.nselect_client):
  #     index = random.choices(client_id, weights=prob, k=1) # 依概率选择客户
  #     while selected[index[0]]!=0:
  #       index = random.choices(client_id, weights=prob, k=1) # 依概率选择客户
  #     client_poor[i] = index[0]
  #     selected[index[0]]=1

  #     return client_poor
      

  def FLtrain(self, clients, expr_basename, signal, total_testD,total_testL):
    timecost=0

    nclients = len(clients)
    selected_index=np.zeros(self.nselect_client) #被选中参加训练的客户的序号，号码为0到19
    sta_u=np.zeros(self.nselect_client, dtype=np.float64)#上一轮被开发的客户的统计效用
    L=np.ones(nclients, dtype=np.int32)#元素的范围为1到global_itera
    ssu=np.zeros(self.global_itera, dtype=np.float64)#每一轮中客户的统计效用之和
    D=np.array([0.35*1+(20*2)/1000,0.35*1+(2000*2)/1000, #前两个客户分别为国内国际的服务器
               0.35*10+(20*2)/1000,0.35*10+(20*2)/1000,0.35*10+(20*2)/1000, #客户2-4为省内的手提电脑
               0.35*10+(50*2)/1000,0.35*10+(50*2)/1000,0.35*10+(50*2)/1000, #客户5-7为国内省外的手提电脑
               0.35*10+(2000*2)/1000,0.35*10+(2000*2)/1000, #客户8-9为国外的手提电脑
               0.35*100+(20*2)/1000,0.35*100+(20*2)/1000,0.35*100+(20*2)/1000,0.35*100+(20*2)/1000,0.35*100+(2000*2)/1000, #客户10-13为省内的移动设备，客户14为国外的移动设备
               0.35*100+(50*2)/1000,0.35*100+(50*2)/1000,0.35*100+(50*2)/1000,0.35*100+(50*2)/1000,0.35*100+(50*2)/1000], #客户15-19为国内省外的移动设备
               dtype=np.float64)#客户的响应时间
    
    LOS=np.zeros(self.global_itera)

    for r in range(0, self.global_itera):

      server_weight=self._model.get_weights()
      new_server_weights = copy.deepcopy(server_weight) # 全局模型的参数的副本，服务器模型的参数将设为全局模型参数
      # deltas = [] # 客户梯度

      time_selection_s = time.time()
      if signal==0:
        client_poor, ssu=self.participants_selection(nclients, selected_index, r, D, L, sta_u, ssu)
      if signal==1:
        client_poor, ssu=self.Oort_select(nclients, selected_index, r, D, L, sta_u, ssu)
      if signal==2:
        client_poor = self.select_by_loss(r,nclients,selected_index,sta_u)
      if signal==3:
        client_poor = self.select_by_random()

      selected_index=client_poor[0:self.nselect_client] #被选中参加训练的客户的序号，号码为0到19
      
      # print("selected_index: ", selected_index)
      selected_clients = [clients[i] for i in selected_index] #更新被选中的客户列表
      time_selection_e = time.time()

      print("Selected clients: ", selected_index)
      print("Client poor: ",client_poor)
      # print("ssu: ", ssu)

      #开始训练
      # delta_weight = [0 for x in server_weight[0]]
      time_FL_s = time.time()
      for i, client in enumerate(selected_clients):
        print(f'{expr_basename} round={r + 1}/{self.global_itera}, client {selected_index[i]}',
              end='')
        local_delta, loss = client.FLtrain(server_weight) # 客户本地训练完成，输出梯度等
        # deltas.append(local_delta)
        sta_u[i] = loss
        L[selected_index[i]] = r+1
        if i==0:
          delta_weight = local_delta
        else:
          for j in range(0,len(delta_weight)):
            delta_weight[j] += local_delta[j]
      del local_delta
      if i != len(selected_clients) - 1:
        print('\r', end='') #\r将光标回退到该行开头
      else:
        print('')

      #聚合, FedAvg
      # delta_weight = [0 for x in deltas[0][0]]
      # for i in range(0,len(deltas)):
      #   for j in range(0,len(delta_weight)):
      #     delta_weight[j] += deltas[i][j]
      
      for j in range(0,len(delta_weight)):
        new_server_weights[j] += self.alpha*(delta_weight[j]/(self.nselect_client)) #更新模型
      self._model.set_weights(new_server_weights)
      time_FL_e = time.time()
      # del deltas

      a, LOS[r]=self.evaluate(total_testD,total_testL, 'FL with our in test data')

      timecost+=(time_FL_e-time_FL_s+time_selection_e-time_selection_s)

    return timecost/self.global_itera, LOS
  



