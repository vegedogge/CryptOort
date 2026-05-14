import random
import numpy as np
import tensorflow as tf

#加载mnist数据集并将样本按标签归纳
def CIFAR_DataLabel(train_data,train_label,test_data,test_label):
  labels = np.zeros([10,10])
  for i in range(0,10):
      labels[i][i] = 1
      
  labeled_traindata = [[],[],[],[],[],[],[],[],[],[]]
  labeled_testdata = [[],[],[],[],[],[],[],[],[],[]]
  for i in range(0,len(train_label)):#将数据按标签归纳
    label_int = int(train_label[i])
    labeled_traindata[label_int].append(train_data[i])
  for i in range(0,len(test_label)):#将数据按标签归纳
    label_int = int(test_label[i])
    labeled_testdata[label_int].append(test_data[i])
    
  for i in range(0,10):
    random.shuffle(labeled_traindata[i])
    random.shuffle(labeled_testdata[i])
    
  #print(len(labeled_traindata), len(labels))
  return labeled_traindata,labeled_testdata,labels

#划分mnist数据集
def Split_Dataset(labeled_traindata,labeled_testdata,labels,nclients=20): #抽样产生服务器的良好的数据集
  clients_traindata = []
  clients_trainlabel = []
  clients_testdata = []
  clients_testlabel = []
  for i in range(0,nclients):
    clients_traindata.append([])
    clients_trainlabel.append([])
    clients_testdata.append([])
    clients_testlabel.append([])

  #为客户划分train集
  amount = np.zeros(nclients)
  for i in range(0,10):
    for j in range(0,len(labeled_traindata[i])):#随机分配nmember个客户的数据
      id = random.randint(0,nclients-1)
      # id1 = random.randint(0,9)
      # id2 = random.randint(10,nclients-1)
      while amount[id] >= 2500: #若客户id的数据已经足够，则将样本分配给其他客户
        # id1 = random.randint(0,9)
        id = random.randint(0,nclients-1)
      # while amount[id2] >= 6000: #若客户id的数据已经足够，则将样本分配给其他客户
        # id2 = random.randint(10,nclients-1)
      #print(id1, id2)
      clients_traindata[id].append(labeled_traindata[i][j])
      clients_trainlabel[id].append(labels[i])
      # clients_traindata[id2].append(labeled_traindata[i][j])
      # clients_trainlabel[id2].append(labels[i])
      amount[id]+=1
      # amount[id2]+=1
    # print(amount)
  #为客户划分test集
  amount = np.zeros(nclients)
  for i in range(0,10):
    for j in range(0,len(labeled_testdata[i])):#随机分配nmember个客户的测试数据
      id = random.randint(0,nclients-1)
      # id1 = random.randint(0,9)
      # id2 = random.randint(10,nclients-1)
      while amount[id] >= 500: #若客户id的数据已经足够，则将样本分配给其他客户
        id = random.randint(0,nclients-1)
      # while amount[id2] >= 1000: #若客户id的数据已经足够，则将样本分配给其他客户
      #   id2 = random.randint(10,nclients-1)
      clients_testdata[id].append(labeled_testdata[i][j])
      clients_testlabel[id].append(labels[i])
      # clients_testdata[id1].append(labeled_testdata[i][j])
      # clients_testdata[id2].append(labeled_testdata[i][j])
      # clients_testlabel[id1].append(labels[i])
      # clients_testlabel[id2].append(labels[i])
      amount[id]+=1
      # amount[id1]+=1
      # amount[id2]+=1
    
    # print(amount)

  #for i in range(1,nmembers-1):
    #print("Len of", i, "'s traindata and testdata: ",len(members_traindata[i]),len(members_testdata[i]))

  return clients_traindata,clients_trainlabel,clients_testdata,clients_testlabel

#处理加载的数据集
def Load_CIFAR10(nclients=20):
  (x_train, y_train), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()
  x_train, x_test = x_train / 255.0, x_test / 255.0
  labeled_trainD, labeled_testD, labels = CIFAR_DataLabel(x_train, y_train, x_test, y_test)
  tr_datasets, tr_labelsets, te_datasets, te_labelsets = Split_Dataset(labeled_trainD, labeled_testD, labels)

  for m in range(nclients):
    tr_datasets[m] = np.array(tr_datasets[m])
    tr_labelsets[m] = np.array(tr_labelsets[m])
    te_datasets[m] = np.array(te_datasets[m])
    te_labelsets[m] = np.array(te_labelsets[m])

  total_testL = tf.keras.utils.to_categorical(y_test, num_classes=10)
  total_testD = np.array(x_test)

  return tr_datasets, tr_labelsets, te_datasets, te_labelsets, total_testD, total_testL