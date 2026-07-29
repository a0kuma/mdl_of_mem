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
    for i in range(len(cls.layers)):
      print(cls.layers[i].l, cls.layers[i].w, cls.layers[i].r)
  def __init__(self, l, w, r):
    self.l = l
    self.w = w
    self.r = r

class MemFullModel:
  def __init__(self):
    self.arrayMemDevice = []
  def addMemDevice(self, objMemDevice):
    self.arrayMemDevice.append(objMemDevice)

class MemDevice:
  masterArrayDevice = []
  @classmethod
  def addMemDevice(cls, objMemDevice):
    cls.masterArrayDevice.append(objMemDevice)
  def __init__(self):
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

ic('END')