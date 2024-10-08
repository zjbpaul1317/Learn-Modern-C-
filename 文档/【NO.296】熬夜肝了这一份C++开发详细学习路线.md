# 【NO.296】熬夜肝了这一份C++开发详细学习路线

一般开发岗主流的就是 Java 后台开发，前端开发以及 C++ 后台开发，现在 Go 开发也是越来越多了，今天把 C++ 后台开发学习路线补上。

写之前先来回答几个问题

### **1.C++ 后台开发有哪些岗位？**

C++ 后台开发的岗位还是很多的，例如游戏引擎开发，游戏服务端开发，音视频服务端/客户端开发，数据库内核开发等等，而且 C++ 也能用来写深度学习，做硬件底层这些。

总之，C++ 后台开发的岗位，还是很丰富的，大家不用担心找不到合适的岗位。

### **2.C++ 后台开发岗位需求量大吗？**

一般大公司大需求量会多一些，小公司需求量较少。

说到岗位需求量，那肯定是 Java 的岗位需求量是最大的，当然，学 Java 的人也是最多的，假如你要学习 C++，那我觉得你要定位大公司可能会好一点，进大公司反而会比 Java 容易。
假如你觉得自己实力很一般，够不着大公司，那我觉得你可以考虑学习 Java，因为大部分小公司，Java 岗位多一些。

但是呢，假如你是应届生，那么语言其实也不是特别重要，只要你 把计算机基础和算法学好，就算你是学 Java 的，也可以去面 C++；学 C++ 的也可以去面 Java。

我当时是学 Java 的，不过秋招那会还面了几个 C++ 岗位，直接和面试官说我不会 C++ 就可以了，他会问你其他的知识。

下面跟大家说一说 C++ 后台开发学习路线，为了方便大家做规划，每一个模块的学习，我都会说下大致的学习时间

C/C++后台开发学习路线总结图

![img](https://pic3.zhimg.com/80/v2-8c5a0c0d084a216d21b300724d87c306_720w.webp)

#### 2.1.C++ 基础

假如你有 C 语言基础，那么这块感觉花个三四个月就能拿下了，假如你是零基础的，估计还得学两三个月的 C 语言，也就是说，得花半年时间才行，没有 C 语言基础的看这个 C 语言教程：[一份评价超高的 C 语言入门教程]
C++ 这块，重点需要学习的就是一些**关键字**、**面向对象**以及 **STL 容器**的知识，特别是 STL，还得研究下他们的一些源码，下面我总结一下一些比较重要的知识（其实是根据面试结果来挑选）。

1. 指针与引用的区别，C 与 C++ 的区别，struct 与 class 的区别
2. struct 内存对齐问题，sizeof 与 strlen 区别
3. 面向对象的三大特性：封装、继承、多态
4. 类的访问权限：private、protected、public
5. 类的构造函数、析构函数、赋值函数、拷贝函数
6. 移动构造函数与拷贝构造函数对比
7. 内存分区：全局区、堆区、栈区、常量区、代码区
8. 虚函数实现动态多态的原理、虚函数与纯虚函数的区别
9. 深拷贝与浅拷贝的区别
10. 一些关键字：static, const, extern, volatile 等
11. 四种类型转换：static_cast、dynamic_cast、const_cast、reinterpret_cast
12. 静态与多态：重写、重载、模板
13. 四种智能指针及底层实现：auto_ptr、unique_ptr、shared_ptr、weak_ptr
14. 右值引用
15. std::move函数
16. 迭代器原理与迭代器失效问题
17. 一些重要的 STL：vector, list, map, set 等。
18. 容器对比，如 map 与 unordered_map 对比，set 与 unordered_set 对比，vector 与 list 比较等。
19. STL容器空间配置器

等等。
根据书来学就可以了，然后学到一些重点，可以重点关注一下。
书籍推荐：

1、《C++Primer》，这本书内容很多的，把前面基础的十几章先看一看，不用从头到尾全啃，后面可以**字典**来使用。

2、《STL 源码剖析》，必看书籍，得知道常见 STL 的原理，建议看个两三遍。

3、《深度探索C++对象模型》，这本主要讲解**面向对象**的相关知识，可以帮你扫清各种迷雾。

#### 2.2.计算机网络

无论你是从事啥岗位，无论是校招还是社招，计算机网络基本都会问，特特是腾讯，字节，shopee，小米等这些非 Java 系的公司，问的更多。这块认真学，**一个半月**就可以搞定了。

计算机网络就是一堆协议的构成，下面是一些比较重要的知识点，学的时候可以重点关注下。

**物理层、链路层**：

1. MTU，MAC地址，以太网协议。
2. 广播与 ARP 协议

**网络层**

1. ip 地址分类
2. IP 地址与 MAC 地址区别
3. 子网划分，子网掩码
4. ICMP 协议及其应用
5. 路由寻址
6. 局域网，广域网区别

**传输层**（主要就是 TCP）

1. TCP首部报文格式（SYN、ACK、FIN、RST必须知道）
2. TCP滑动窗口原理，TCP 超时重传时间选择
3. TCP 拥塞控制，TCP 流量控制
4. TCP 三次握手与四次挥手以及状态码的变化
5. TCP连接释放中TIME_WAIT状态的作用
6. SYN 泛洪攻击
7. TCP 粘包，心跳包
8. UDP 如何实现可靠传输
9. UDP 与 TCP 的区别
10. UDP 以及 TCP 的应用场景

**应用层**

1. DNS 原理以及应用
2. HTTP 报文格式，HTTP1.0、HTTP1.1、HTTP2.0 之间的区别
3. HTTP 请求方法的区别：GET、HEAD、POST、PUT、DELETE
4. HTTP 状态码
5. HTTP 与 HTTPS 的区别
6. 数字证书，对称加密与非对称加密
7. cookie与session区别
8. 输入一个URL到显示页面的流程（越详细越好，搞明白这个，网络这块就差不多了）

书籍推荐：零基础可以先看《图解HTTP》，当然，也可以直接看《计算机网网络：自顶向下》这本书，这本书建议看两遍以及以上，还有时间的可以看《TCP/IP详解卷1：协议》。

### 3.操作系统

操作系统和计算机网络差不多，不过计算机网络会问的多一些，操作系统会少一些，学到时候如果可以带着问题去学是最好的，例如

咋就还有进程和线程之分？为什么要有挂起、运行、阻塞等这么多种状态？怎么就还有悲观锁和乐观锁，他们的本质区别？

进程咋还会出现死锁，都有哪些处理策略？进程都有哪些调度算法？

虚拟内存解决了什么问题？为啥每个进程的内存地址就是独立的呢？

为啥 cpu 很快而内存很慢？磁盘怎么就更慢了？

总结起来大致：
1、进程与线程区别
2、线程同步的方式：互斥锁、自旋锁、读写锁、条件变量
3、互斥锁与自旋锁的底层区别
4、孤儿进程与僵尸进程
5、死锁及避免
6、多线程与多进程比较
7、进程间通信：PIPE、FIFO、消息队列、信号量、共享内存、socket
8、管道与消息队列对比
9、fork进程的底层：读时共享，写时复制
10、线程上下文切换的流程
11、进程上下文切换的流程
12、进程的调度算法
13、阻塞IO与非阻塞IO
14、同步与异步的概念
15、静态链接与动态链接的过程
16、虚拟内存概念（非常重要）
17、MMU地址翻译的具体流程
18、缺页处理过程
19、缺页置换算法：最久未使用算法、先进先出算法、最佳置换算法

书籍推荐：《现代操作系统》

这里也有一门合并的视频：[C/C++后台开发学习视频](https://link.zhihu.com/?target=https%3A//ke.qq.com/course/417774%3FflowToken%3D1013189)

### 4.MySQL(一个月左右)

数据库一般主流的有 MySQL 和 Oracle，不过建议大家学习 MySQL 了，因为大部分公司都是使用 MySQL，也是属于面试必问，而且工作中 MySQL 也是接触的最多的，毕竟工作 crud 才是常态。

下面这些是我认为比较重要的知识点：

1、一条 sql 语句是如何执行的？也就是说，从客户端执行了一条 sql 命令，服务端会进行哪些处理？（例如验证身份，是否启用缓存啥的）。

2、索引相关：索引是如何实现的？多种引擎的实现区别？聚族索引，非聚族索引，二级索引，唯一索引、最左匹配原则等等（非常重要）。

3、事务相关：例如事务的隔离是如何实现的？事务是如何保证原子性？不同的事务看到的数据怎么就不一样了？难道每个事务都拷贝一份视图？MVCC 的实现原理（重要）等等。

4、各种锁相关：例如表锁，行锁，间隙锁，共享锁，排他锁。这些锁的出现主要是用来解决哪些问题？（重要）

5、日志相关：redolog，binlog，undolog，这些日志的实现原理，为了解决怎么问题？日志也是非常重要的吧，面试也问的挺多。

6、数据库的主从备份、如何保证数据不丢失、如何保证高可用等等。

7、一些故障排查的命令，例如慢查询，sql 的执行计划，索引统计的刷新等等。

对于 2-4 这四个相关知识，面试被问到的频率是最高的，有时候面试会让你说一说索引，如果你知道的多的话就可以疯狂扯一波了，记得我当时总结了一套扯的模版：

先说从 B 树角度说为啥索引会快-》趁机说一下索引的其他实现方式-〉不同引擎在索引实现上的不同-》系统是如果判断是否要使用索引的-〉明明加了索引却不走索引？

只有你对各种数据结构和索引原理都懂，你才能扯的起来，对于事物和锁也是，当时面试官问了我事务是如何保证一致性的，刚好我研究过 ，redolog，binlog，undolog 这些日志，然后和面试官扯了好久。

书籍：《MySQL必知必会》和《MySQL技术内幕》

### 5.网络编程

网络编程这块，有些公司还是问的挺多的，特别是 IO 多路复用，同步非同步 IO，阻塞非阻塞啥的，当时面腾讯基本每次都问，，，，学习 C++ 这块还是要重视一下，下面我说一下比较重要的吧。

1、IO多路复用：select、poll、epoll的区别（非常重要，几乎必问，回答得越底层越好，要会使用）
2、手撕一个最简单的server端服务器（socket、bind、listen、accept这四个API一定要非常熟练）
3、线程池
4、基于事件驱动的reactor模式
5、边沿触发与水平触发的区别
6、非阻塞IO与阻塞IO区别

书籍：可以看一看《Unix网络编程》

### 6.数据结构与算法

数据结构与算法，我觉得是需要花最多时间的，因为算法这块，很难快速突击，从基础数据结构与各种算法思想到 leetcode 刷题，如果你零基础，那真的需要挺久的，不过你有一些基础，可能会快一点，看你想掌握到什么程度了。

我这里大致说一下学习流程吧

1、先跟着书学**基础数据结构与算法**：链表，队列，栈，哈希表，二叉树，图，十大排序，二分查找。

2、之后了解一下算法思想：递归，深度与广度搜索，枚举，动态规划这些。

入门数据结构推荐《数据结构与算法分析：c语言描述版》这本书，学的过程中，也可以配合刷题，一般刷《剑指 offer》 + LeetCode 刷个两三百就差不多了，没时间到就先刷 《剑指 offer》吧。

### 7.项目

项目是必须要做的了，Java 的项目教程满天飞，不过 C++ 的会少一些，不过大家可以跟着书，或者 github 上找或者自己花点钱买一个付费视频吧。

推荐自学项目：实现 http服务器（ github 一堆源码、音视频服务器）、实现一个聊天系统(这块有些书就有附带)

### 8.学习顺序

我建议有时间的，可以先入门下 C++ ，然后就是开始学习数据结构与算法，算法这块长期保持刷题，然后一边深入学习 C++，之后学习计算机网络，操作系统，在之后学习网络编程，项目这块放到最后面。

如果时间比较紧的，算法这块可以放松一点，C++ 和项目可以优先，计算机基础可以突击学习，通过视频或者别人总结的笔记突击。

总之，这一套学下来，感觉需要大概8个月的时间，当然，这个不好衡量，还得看你自己掌握了哪些基础。

### 9.总结

学了之后要验证自己学得如何，可以来小编的网站看看这些面试题，通过面试题查漏补缺

小破站网址：后续会越来越完善，包括各种算法也都会更新，建议大家收藏。

总之，关于校招，学习路线，面试题等等，很多我在网站都更新了，包括个人经历，大家迷茫没事做时，可以多打开看看。

最后，大家加油，努力学两年，争取日后那个好的 offer.

原文链接：[https://juejin.cn/post/69972481](https://link.zhihu.com/?target=https%3A//juejin.cn/post/6997248187413037070)

作者：[Hu先生的Linux](https://www.zhihu.com/people/huhu520-10)