# 【NO.483】Linux服务器开发,手写分布式锁

## 0.前言

分布式锁需要考虑很多事情，第一网络是否正常，第二个提供分布式锁这台机器的高可用性。



![在这里插入图片描述](https://img-blog.csdnimg.cn/066dce11917f4a9b8aaacbef6690cedf.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bGv6Zeo5bGx6bih5Y-r5oiR5bCP6bih,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)



## 1.网络模块封装

### 1.1 io检测部分的接口：

- 添加读事件是因为当有客户端连接事件来临时，我们好去处理这个事件。

- 写事件这是因为当我们服务端作为客户端去连接第三方服务时候，需要注册写事件。

- 删除事件就比如关闭连接时，我们需要将事件从检测模块中删除。

- 修改事件就比如当客户端发来事件我们需要检测读事件，但是接收失败了我们要把读事件修改为注册写事件进行反馈。

- 检测事件肯定是必须的啦！

  ### io操作：

  绑定监听，接收连接，建立连接，关闭fd，读，写，fd属性。



需要注意的是：fd类型中clientfd写事件触发的情况，服务器收到客户端的数据可以直接调用write()函数，**只有当写入失败时才要注册写事件，检测何时可写。**应该将发送失败的数据缓存起来，等事件可写了，再将缓冲区的数据进行发送。
断开连接或者错误，都是交给读写操作。

## 2.协程调度

没有协程之前，当成功接收到一个数据后，要调用多个回调函数,(解析数据、查库拿数据、返回给客户端)等等。现在我们考虑的是，将三个序列利用协程的方式进行粘合，三个回调变成一个协程回调执行序列。
为每一个fd执行一个序列，每个协程是一个执行序列。
lua虚拟机不支持协程，并且没有类似于pthread_create()函数，也就是说主协程被自动创建。**lua虚拟机同时只能有一个协程在运行。**主协程负责调度其他协程。
主协程会进行事件检测，不断地从epoll中去取事件，根据事件去唤醒其他协程。

![在这里插入图片描述](https://img-blog.csdnimg.cn/76fef6d536174cd590cc6ca23038ee1a.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bGv6Zeo5bGx6bih5Y-r5oiR5bCP6bih,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)



- lua和go语言中的协程方案是最完善的。
- epoll注册写事件触发，说明三次握手后同步包可以发送了，表示连接已经建立成功了。
- 异步的执行逻辑，同步的写法。
- 当连接建立成功后，connectfd和clientfd的流程变成一样。

## 3.异步连接池

### 3.1 为什么需要异步连接池？

现在所有的连接已经变成一个执行序列，连接由异步变为同步。此时，一个连接同时只能在一个协程中运行，是并发不是并行，也就是说只有一个执行序多个对列，依次执行。
pool_size是尺子最大连接数，backlog堆积的操作数。

![在这里插入图片描述](https://img-blog.csdnimg.cn/05edc8699a0847829c504fe0bdfe566b.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bGv6Zeo5bGx6bih5Y-r5oiR5bCP6bih,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)



## 4.缓冲池设计 用户读写缓冲区

Mark老师举例，4个协程在使用，4个协程在等待，超出的协程会报错。cache记录的是随时可用连接，free记录的是正在使用的连接。connect>8是要报错，小于pool_size要创建或是从cache里去取，cache没有说明都在free里。
给连接池起名字，默认是ip：端口格式，比如127.0.0.1:8888。

![在这里插入图片描述](https://img-blog.csdnimg.cn/d72c6b45546748718c40a650321c00e7.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bGv6Zeo5bGx6bih5Y-r5oiR5bCP6bih,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)



## 5.定时器设计

**定时器是在lua层实现了一个最小堆，每一个任务生成一个协程，但是要考虑回收协程，尤其是在删除的时候。**这块做的还不完善，Mark老师也希望大家一起帮忙搞一搞，先记下这件事吧！

## 6.总结

Mrk老师说道:做任何事情，拆分的思想很重要，联想起以前一个数学老师的话，“老太太吃柿子，要捡软的捏。”以后遇到问题和困难，也应该先按照这个思路去处理问题。作为开发还有一点是非常重的，就是**测试**的能力，这方面得想办法学习提升一下。通过本节课，我初步了解了分布式锁，感觉对于skynet也有了更好的认识。在成长的路上，有Mark老师陪伴，好幸福啊~

原文作者：[屯门山鸡叫我小鸡](https://blog.csdn.net/sinat_28294665)

原文链接：https://bbs.csdn.net/topics/604975373