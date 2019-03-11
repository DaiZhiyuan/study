***Linux进程核心调度器之主调度器schedule***

[TOC]

# 1. 前景回顾

## 1.1 进程调度

## 1.2 进程的分类

## 1.3 Linux调度器的演变

## 1.4 Linux调度器的组成

### 1.4.1 2个调度器

### 1.4.2 6种调度策略

### 1.4.3 5个调度类

### 1.4.4 3个调度实体

# 2. 主调度器

## 2.1 调度函数的__sched前缀

## 2.2 schedule函数

### 2.2.1 schedule主框架

### 2.2.2 sched_submit_work()避免死锁

### 2.2.3 preempt_disbale和sched_preempt_enable_no_resched开关内核抢占

## 2.3 __schedule开始进程调度

### 2.3.1 __schedule函数主框架

### 2.3.2 pick_next_task选择抢占的进程

## 2.4 context_switch进程上下文切换

### 2.4.1 进程上下文切换

### 2.4.2 context_switch流程

### 2.4.3 switch_mm切换进程虚拟地址空间

### 2.4.4 switch_to切换进程堆栈和寄存器

## 2.5 need_resched，TIF_NEED_RESCHED表示与用户抢占

### 2.5.1 need_resched标识TIF_NEED_RESCHED

### 2.5.2 用户抢占和内核抢占

# 3. 总结

## 3.1 schedule调度流程

## 3.2 __schedule如何完成内核抢占

## 3.3 调度的内核抢占和用户抢占