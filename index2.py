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
    self.l = l
    self.w = w
    self.r = r

class MemTheWholeModel:
  def __init__(self, setDevice):
    self.masterArrayDevice = {}
    for j in sorted(list(setDevice)):
      objMemDevice = MemDevice(j)
      self.masterArrayDevice[j] = objMemDevice

  def addMemPartitionToDevice(self, deviceID, objMemPartition):
    self.masterArrayDevice[deviceID].addMemPartition(objMemPartition)

class MemDevice:
  def __init__(self, deviceID):
    self.deviceID = deviceID
    self.arrayMemPartition = []
  def getAttrArrayMemPartition(self):
    return self.arrayMemPartition
  def addMemPartition(self, objMemPartition):
    self.arrayMemPartition.append(objMemPartition)
  def sum_up_all_w

class MemPartirion:
  def __init__(self, sum_of_weight_of_all_layers):                                                                                                                                                                         
    self.arrayMemTheLayer = []
    self.lOfMemPartirion = None
    self.rOfMemPartirion = None
    # this is wrong, cuz per device
    # self.sum_of_weight_of_all_layers = sum_of_weight_of_all_layers
    self.sum_of_weight_of_all_layers = -1
  def addMemTheLayer(self, objMemTheLayer):
    self.arrayMemTheLayer.append(objMemTheLayer)
  def onStopAppend(self):
    self.lOfMemPartirion = self.arrayMemTheLayer[0]
    self.rOfMemPartirion = self.arrayMemTheLayer[-1]

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
  MemDevice.addMemPartitionToDevice(devices[i], tmpMemPartirion)
MemDevice.treePrintMemDevice()


ic('END')