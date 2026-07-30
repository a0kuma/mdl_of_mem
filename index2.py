import os
from icecream import ic
ic.configureOutput(includeContext=True)
import sys
import datetime as dt
import wandb
import torch
import torch.nn as nn

#################TODO#################
# 1. i know that get set like what i wrote sckus ... but any who
# 2. all the print is jam
######################################

arrayW_int = [56, 16, 24, 60, 100, 40, 56, 196, 84, 24, 16, 40]
arrayD_int = [28, 8, 8, 12, 20, 20, 8, 28, 28, 12, 8, 8, 20]
balance = [2, 2, 2, 2, 2, 2]
devices = [0, 0 ,0 ,1 ,1 ,1]

class MemTheLayer:
  layers = []
  @classmethod
  def addLayer(cls, objLayer):
    cls.layers.append(objLayer)
  @classmethod
  def printLayers(cls):
    # TODO 太簡易了
    for i in range(len(cls.layers)):
      print(cls.layers[i].l, cls.layers[i].w, cls.layers[i].r)
  # this is wrong, cuz per device
  # @classmethod
  # def addWup(cls):
  #   tmp = 0
  #   for i in cls.layers:
  #     tmp += i.w
  #   return tmp
  @classmethod
  def getLayers(cls):
    return cls.layers
  def __init__(self, l, w, r):
    self.l = l #important: the output (aka dL/d di-1)
    self.w = w #important: weight it self (gradient of the weight) 
    self.r = r #TODO TBD
    self.ans = -1

class MemTheWholeModel:
  def __init__(self, setDevice):
    self.masterArrayDevice = {}
    for j in sorted(list(setDevice)):
      objMemDevice = MemDevice(j)
      self.masterArrayDevice[j] = objMemDevice
  def treePrint(self):
    """
    D: print id of it
    P: print idx of it
    L: print l w r ans of it
    """
    for i in self.masterArrayDevice:
      print('D:', self.masterArrayDevice[i].deviceID)
      for j in range(len(self.masterArrayDevice[i].arrayMemPartition)):
        print('  P:', j)
        for k in range(len(self.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer)):
          currentL = self.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer[k]
          print('    L:', currentL.l, currentL.w, currentL.r, currentL.ans)

  def addMemPartitionToDevice(self, deviceID, objMemPartition):
    self.masterArrayDevice[deviceID].addMemPartition(objMemPartition)

  def onStopAppend(self):
    for i in self.masterArrayDevice:
      self.masterArrayDevice[i].onStopAppend()

  def go_through_all_layers(self):
    for i in range(len(self.masterArrayDevice)):
      currentD = self.masterArrayDevice[i]
      for j in range(len(currentD.arrayMemPartition)):
        currentP = currentD.arrayMemPartition[j]
        for k in range(len(currentP.arrayMemTheLayer)):
          currentL = currentP.arrayMemTheLayer[k]
          tmp = 0
          tmp += currentD.sum_up_all_w()
          tmp += currentP.lOfMemPartirion.l
          tmp += currentP.SUM_OF_L_ON_allLeftPartirions
          tmp += currentP.rOfMemPartirion.r
          tmp += currentL.w
          tmp += currentL.r # TODO if not ... ? 
          tmp += currentP.sum_of_d_from_nearest_ckpt_to_current(k)
          tmp += currentD.sum_of_all_gradent_of_weight_right_of_it_self(j, k)

          currentL.ans = tmp

class MemDevice:
  def __init__(self, deviceID):
    self.deviceID = deviceID
    self.arrayMemPartition = []
    self.allLeftPartirions = {}
  def addMemPartition(self, objMemPartition):
    self.arrayMemPartition.append(objMemPartition)
  def sum_up_all_w(self):#important
    tmp = 0
    for i in self.arrayMemPartition:
      for j in i.arrayMemTheLayer:
        tmp += j.w
    return tmp
  def onStopAppend(self):
    for i in range(len(self.arrayMemPartition)):
      self.allLeftPartirions[i] = self.arrayMemPartition[:i] # better code this can be skiped
      self.arrayMemPartition[i].SUM_OF_L_ON_allLeftPartirions = MemPartirion.sumOfL(self.allLeftPartirions[i])
  def sum_of_all_gradent_of_weight_right_of_it_self(self, idx_p, ldx_l):
    # sum up p > idx_p
    tmp = 0
    for i in range(idx_p+1, len(self.arrayMemPartition)):
      for j in self.arrayMemPartition[i].arrayMemTheLayer:
        tmp += j.w
    # sum up l > ldx_l
    for j in range(ldx_l+1, len(self.arrayMemPartition[idx_p].arrayMemTheLayer)):
      tmp += self.arrayMemPartition[idx_p].arrayMemTheLayer[j].w
    return tmp 

class MemPartirion:
  @staticmethod
  def sumOfL(arrayMemTheLayer):
    tmp = 0
    for i in arrayMemTheLayer:
      tmp += i.lOfMemPartirion.l
    return tmp
  def __init__(self):                                                                                                                                                                         
    self.arrayMemTheLayer = []
    self.lOfMemPartirion = None #important
    self.rOfMemPartirion = None #important
    self.SUM_OF_L_ON_allLeftPartirions = -1  #important
    # this is wrong, cuz per device
    # self.sum_of_weight_of_all_layers = sum_of_weight_of_all_layers
  def addMemTheLayer(self, objMemTheLayer):
    self.arrayMemTheLayer.append(objMemTheLayer)
  def onStopAppend(self):
    self.lOfMemPartirion = self.arrayMemTheLayer[0]
    self.rOfMemPartirion = self.arrayMemTheLayer[-1]
  def sum_of_d_from_nearest_ckpt_to_current(self, idx):
    tmp = 0
    for i in range(0, idx):
      tmp += self.arrayMemTheLayer[i].r
    return tmp

ic('START')

for i in range(len(arrayW_int)):
  objMemTheLayer = MemTheLayer(arrayD_int[i], arrayW_int[i], arrayD_int[i+1])
  MemTheLayer.addLayer(objMemTheLayer)
  # TODO 以上兩行可以合併
MemTheLayer.printLayers()
# balance len == devices len
memWholeModel = MemTheWholeModel(set(devices))
# Partirion
tmp_sum = 0
for i in range(len(balance)):
  tmpMemPartirion = MemPartirion()
  for j in range(balance[i]): 
    tmpMemPartirion.addMemTheLayer(MemTheLayer.getLayers()[tmp_sum+j])
  tmp_sum += balance[i]
  tmpMemPartirion.onStopAppend()
  memWholeModel.addMemPartitionToDevice(devices[i], tmpMemPartirion)
memWholeModel.onStopAppend()
memWholeModel.go_through_all_layers()
memWholeModel.treePrint()



ic('END')