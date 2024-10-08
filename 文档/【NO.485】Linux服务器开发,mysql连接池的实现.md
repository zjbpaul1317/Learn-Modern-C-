# 【NO.485】Linux服务器开发,mysql连接池的实现

## 0.前言

连接池和线程池的的关系，当线程数量小于连接数量时就需要等到连接释放再去争夺连接资源。线程池是主动连接执行任务，连接池和内存池相似都是被动获取，执行任务后归还。

## 1.池化技术

池化技术的作用是减少资源创建次数，提高程序的响应性能。

![在这里插入图片描述](https://img-blog.csdnimg.cn/379c7c1fce8b48fdb4a221929b248d43.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5bGv6Zeo5bGx6bih5Y-r5oiR5bCP6bih,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)


可以看到，在sql执行部分的执行效率是非常低的，只有执行sql语句才是真正干活的时间。



## 2.什么是数据库连接池

用一个连接容器进行存储，7个线程请求到7个连接，握手挥手的时间都可以节省。



![img](https://img-community.csdnimg.cn/images/32b5323c679d41808ed8e632a49689d9.png)



## 3.为什么使用数据库连接池

优点：

- 降低网络开销
- 连接复用，有效减少连接数
- 提升性能，避免频繁的新建连接，新建连接开销比较大
- 没有TIME_WAIT状态问题
  缺点:
- 设计优点复杂
  假如每个线程绑定一个连接，这会导致代码的耦合性比较高。有些任务不需要连接数据库，代码冗余。
  测试发现，数据库在同一台电脑上使用连接池性要比不使用连接池快一倍，如果在不同电脑上可能会差更多。

## 4.数据库连接池运行机制

当需要访问数据库时去连接池取，使用完毕后进行归还。系统关闭之前，要断开所有连接并释放连接占用的系统资源，个人感觉这个挺好理解，符合人类的常规逻辑。上图：



![img](https://img-community.csdnimg.cn/images/51e449a371ac4ec1b3d1e6da0e09ea7d.png)



## 5.连接池和线程池的关系

首先，连接池的连接对象和线程池的线程数量是相对应的。其次线程执行完任务时要关闭连接对象。
最后，线程池是主动调用任务，而线程池是被动的接受，感觉更像是一个小受。

## 6.线程池设计要点

1、连接数据库：涉及数据库ip、端口、用户名、密码、数据库名字

- 独立的连接通道
- 配置最小连接数和最大的连接数。
  2、需要一个管理连接的队列：管理连接，获取连接。list、queue
  3、获取连接对象
  4、归还连接对象
  5、连接池的名字

## 7.连接池的具体实现

### 7.1 编译步骤

```bash
 rm -rf build



 mkdir build



 cd build



 cmake ..



 make



 ./test_dbpool 4 4 1 2000   



 



![img](https://img-community.csdnimg.cn/images/97d1f1f8540948508a944aa2cdb1a14f.png "#left")



 



 



## (4是线程数量 4 是连接数量 1是开启连接池 2000线程数量) 
```

构造函数中初始化必要参数，先创建最小连接数。

```c
GetDBConn(int timeout_ms)；//线程数量多而连接数量不够时，进行超时判断防止等待。
```

- 当当前连接数小于最大连接数时，说明可以新建连接，将连接插入空闲连接，然后再去给予连接句柄。
- 当当前连接数大于最大连接数时，就不能再去创建连接，等待超时时间，可以设置的比较大。过了超时时间仍然拿不到，那很遗憾就返回空啦！有空闲连接就能给予句柄了。
- 决不允许两个任务共用一个连接。

如果真的要销毁连接池：

- 1、先销毁线程池，确保所有任务退出；
- 2、再去销毁连接池。

连接需要归还了我们才去销毁：

- 1、资源申请释放的顺序非常重要
- 2、异步编程是比如容易崩，资源释放异步函数还在使用

数据库重连机制：

- 每次操作之前先去测试链路是否通。
- 先去执行任务，再去处理连接不同的问题。

## 8.连接池连接设置数量

根据测试结果，线程数连接数开到32的时候性能不再提高、
开启多少线程是最快的：N为核心数

- 计算密集型 N+1

- IO密集型 根据实际阻塞的时间去测试，2N+2 这一块感觉接下来要去思考。

  

  ## 9.总结

  通过今天听了零声学院Darren老师的课程，真的是如醍醐灌顶般，瞬间明白了mysql异步连接池的运行原理和实际开发中遇到性能问题该如何解决。自己觉得收获颇丰，对异步连接池有了新的认识，自己有信心有能力对老师的连接池进行改造。从最初的懵懂少年，通过不断地学习和努力，我已经逐渐成为了翩翩少年。是零声学院的老师助我成长，伴我入眠。

  

原文作者：[屯门山鸡叫我小鸡](https://blog.csdn.net/sinat_28294665)

原文链接：https://bbs.csdn.net/topics/605005724