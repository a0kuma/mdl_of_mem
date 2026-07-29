import os
from icecream import ic
ic.configureOutput(includeContext=True)
import sys
import datetime as dt
import wandb
import torch
import torch.nn as nn

#################TODO#################
# i know that get set like what i wrote sckus ... but any who
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
  @classmethod
  def getLayers(cls):
    return cls.layers
  def __init__(self, l, w, r):
    self.l = l
    self.w = w
    self.r = r

class MemDevice:
  masterArrayDevice = {}
  @classmethod
  def initMemDevice(cls, setDevice):
    for j in sorted(list(setDevice)):
      objMemDevice = MemDevice(j)
      cls.masterArrayDevice[j] = objMemDevice
  @classmethod
  def treePrintMemDevice(cls):
    for i in sorted(cls.masterArrayDevice.keys()):
      print('deviceID:', cls.masterArrayDevice[i].deviceID)
      for j in range(len(cls.masterArrayDevice[i].arrayMemPartition)):
        print('  MemPartition:', j)
        for k in range(len(cls.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer)):
          print('    MemTheLayer:', k, 'l:', cls.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer[k].l, 'w:', cls.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer[k].w, 'r:', cls.masterArrayDevice[i].arrayMemPartition[j].arrayMemTheLayer[k].r)
  @classmethod
  def addMemPartitionToDevice(cls, deviceID, objMemPartition):
    cls.masterArrayDevice[deviceID].addMemPartition(objMemPartition)
  def __init__(self, deviceID):
    self.deviceID = deviceID
    self.arrayMemPartition = []
  def getAttrArrayMemPartition(self):
    return self.arrayMemPartition
  def addMemPartition(self, objMemPartition):
    self.arrayMemPartition.append(objMemPartition)

class MemPartirion:
  def __init__(self):                                                                                                                                                                         
    self.arrayMemTheLayer = []
    self.lOfMemPartirion = None
    self.rOfMemPartirion = None
  def addMemTheLayer(self, objMemTheLayer):
    self.arrayMemTheLayer.append(objMemTheLayer)
  def onStopAppend(self):
    pass

ic('START')

for i in range(len(arrayW_int)):
  objMemTheLayer = MemTheLayer(arrayD_int[i], arrayW_int[i], arrayD_int[i+1])
  MemTheLayer.addLayer(objMemTheLayer)
  # TODO 以上兩行可以合併
MemTheLayer.printLayers()
# balance len == devices len
MemDevice.initMemDevice(set(devices))
MemDevice.treePrintMemDevice()
# Partirion
tmp_sum = 0
for i in range(len(balance)):
  tmpMemPartirion = MemPartirion()
  for j in range(balance[i]): 
    tmpMemPartirion.addMemTheLayer(MemTheLayer.getLayers()[tmp_sum+j])
  tmp_sum += balance[i]
  MemDevice.addMemPartitionToDevice(devices[i], tmpMemPartirion)
MemDevice.treePrintMemDevice()


ic('END')