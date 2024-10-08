# 【NO.416】盘点腾讯linux C++后台开发面试题

鹅厂是cpp的主战场，而以cpp为背景的工程师大都对os，network这块要求特别高，不像是Java这种偏重业务层的语言，之前面试Java的公司侧重还是在数据结构、网络、框架、数据库和分布式。所以OS这块吃的亏比较大。

面试分为以下几大块

- C/C++
- 网络
- 操作系统
- Linux系统
- MongoDB
- Redis
- mysql
- 算法
- 设计模式
- 分布式架构
- 系统设计
- 等等，未完待续

## 1.C/C++

1. const
2. 多态
3. 什么类不能被继承（这个题目非常经典，我当时答出了private但是他说不好，我就没想到final我以为那个是java的）

## 2.网络

1. 网络的字节序
2. 网络知识 tcp三次握手 各种细节 timewait状态
3. tcp 与 udp 区别 概念 适用范围
4. TCP四次挥手讲一下过程，最后一次ack如果客户端没收到怎么办，为什么挥手不能只有三次，为什么time_wait。
5. 对于socket编程，accept方法是干什么的，在三次握手中属于第几次，可以猜一下，为什么这么觉得。
6. tcp怎么保证有序传输的，讲下tcp的快速重传和拥塞机制，知不知道time_wait状态，这个状态出现在什么地方，有什么用？
7. 知道udp是不可靠的传输，如果你来设计一个基于udp差不多可靠的算法，怎么设计？
8. http与https有啥区别？说下https解决了什么问题，怎么解决的？说下https的握手过程。
9. tcp 粘包半包问题怎么处理？
10. keepalive 是什么东东？如何使用？
11. 列举你所知道的tcp选项，并说明其作用。
12. socket什么情况下可读？
13. nginx的epoll模型的介绍以及io多路复用模型
14. SYN Flood攻击
15. 流量控制，拥塞控制
16. TCP和UDP区别，TCP如何保证可靠性，对方是否存活(心跳检测)
17. tcpdump抓包，如何分析数据包
18. tcp如何设定超时时间
19. 基于socket网络编程和tcp/ip协议栈，讲讲从客户端send()开始，到服务端recv()结束的过程，越细越好
20. http报文格式
21. http1.1与http1.0区别，http2.0特性
22. http3了解吗
23. http1.1长连接时，发送一个请求阻塞了，返回什么状态码？
24. udp调用connect有什么作用？

## 3.操作系统

1. 进程和线程-分别的概念 区别 适用范围 它们分别的通讯方式 不同通讯方式的区别优缺点
2. 僵尸进程
3. 死锁是怎么产生的
4. CPU的执行方式
5. 代码中遇到进程阻塞，进程僵死，内存泄漏等情况怎么排查。
6. 有没有了解过协程？说下协程和线程的区别？
7. 堆是线程共有还是私有，堆是进程共有还是私有，栈呢
8. 了解过协程吗（我：携程？？？不了解呜呜呜）
9. 共享内存的使用实现原理（必考必问，然后共享内存段被映射进进程空间之后，存在于进程空间的什么位置？共享内存段最大限制是多少？）
10. c++进程内存空间分布（注意各部分的内存地址谁高谁低，注意栈从高道低分配，堆从低到高分配）
11. ELF是什么？其大小与程序中全局变量的是否初始化有什么关系（注意.bss段）
12. 使用过哪些进程间通讯机制，并详细说明（重点）
13. 多线程和多进程的区别（重点 面试官最最关心的一个问题，必须从cpu调度，上下文切换，数据共享，多核cup利用率，资源占用，等等各方面回答，然后有一个问题必须会被问到：哪些东西是一个线程私有的？答案中必须包含寄存器，否则悲催）
14. 信号：列出常见的信号，信号怎么处理？
15. i++是否原子操作？并解释为什么？？？？？？？
16. 说出你所知道的各类linux系统的各类同步机制（重点），什么是死锁？如何避免死锁（每个技术面试官必问）
17. 列举说明linux系统的各类异步机制
18. exit() _exit()的区别？
19. 如何实现守护进程？
20. linux的内存管理机制是什么？
21. linux的任务调度机制是什么？
22. 标准库函数和系统调用的区别？
23. 补充一个坑爹坑爹坑爹坑爹的问题：系统如何将一个信号通知到进程？（这一题哥没有答出来）

## 4.Linux系统

1. linux的各种命令 给你场景让你解决
2. Linux了解么，查看进程状态ps，查看cpu状态 top。查看占用端口的进程号netstat grep
3. Linux的cpu 100怎么排查，top jstack，日志，gui工具
4. Linux操作系统了解么
5. 怎么查看CPU负载，怎么查看一个客户下有多少进程
6. Linux内核是怎么实现定时器的
7. gdb怎么查看某个线程
8. core dump有没有遇到过，gdb怎么调试
9. linux如何设置core文件生成
10. linux如何设置开机自启动
11. linux用过哪些命令、工具
12. 用过哪些工具检测程序性能，如何定位性能瓶颈的地方
13. netstat tcpdump ipcs ipcrm （如果这四个命令没听说过或者不能熟练使用，基本上可以回家，通过的概率较小 ^_^ ，这四个命令的熟练掌握程度基本上能体现面试者实际开发和调试程序的经验)
14. cpu 内存 硬盘 等等与系统性能调试相关的命令必须熟练掌握，设置修改权限 tcp网络状态查看 各进程状态 抓包相关等相关命令 必须熟练掌握
15. awk sed需掌握
16. gdb调试相关的经验，会被问到

## 5.MongoDB

1. 关于大数据存储的（mongodb hadoop）各种原理 mongodb又问的深入很多

## 6.Redis

1. Redis内存数据库的内存指的是共享内存么
2. Redis的持久化方式
3. Redis和MySQL有什么区别，用于什么场景。
4. redis有没有用过，常用的数据结构以及在业务中使用的场景，redis的hash怎么实现的
5. 问了下缓存更新的模式，以及会出现的问题和应对思路？
6. redis的sentinel上投票选举的问题 raft算法
7. redis单线程结构有什么优势？有什么问题？ 主要优势单线程，避免线程切换产生静态消耗，缺点是容易阻塞，虽然redis使用io复用epoll和输入缓冲区把命令按照队列先进先出输入等等
8. 你觉得针对redis这些缺点那些命令在redis上不可使用？ 比如keys、hgetall等等这些命令 建议用scan等等 这方面阐述
9. 你觉得为什么项目中没有用mysql而用了es，redis在这里到底起到了什么作用？因为架构上这里理解不清楚，最后回答自己都觉得有漏洞了
10. 你觉得redis什么算有用？ 有用？ 是说存进去了还是说命中缓存？最后把缓存命中率是什么说了一遍
11. 你们这边redis集群是怎么样子的
12. 平常redis用的多的数据结构是什么，跳表实现，怎么维护索引，当时我说是一个简单的二分，手写二分算法，并且时间复杂度是怎么计算出来的 （2的k次方等于n k等于logn）

## 7.MySQL

1. 你们后端用什么数据库做持久化的？有没有用到分库分表，怎么做的？
2. 索引的常见实现方式有哪些，有哪些区别?MySQL的存储引擎有哪些，有哪些区别？InnoDB使用的是什么方式实现索引，怎么实现的？说下聚簇索引和非聚簇索引的区别?
3. mysql查询优化
4. MySQL的索引，B+树性质。
5. B+树和B树，联合索引等原理
6. mysql的悲观锁和乐观锁区别和应用，ABA问题的解决
7. 项目性能瓶颈在哪，数据库表怎么设计
8. 假设项目的性能瓶颈出现在写数据库上，应该怎么解决峰值时写速度慢的问题
9. 假设数据库需要保存一年的数据，每天一百万条数据，一张表最多存一千万条数据，应该怎么设计表
10. 数据库自增索引。100台服务器，每台服务器有若干个用户，用户有id，同时会有新用户加入。实现id自增，统计用户个数？不能重复，好像是这样的。
11. mysql，会考sql语言，服务器数据库大规模数据怎么设计，db各种性能指标

## 8.算法

1. 堆栈
2. 有序数组排序，二分，复杂度
3. 常见排序算法，说下快排过程，时间复杂度
4. 有N个节点的满二叉树的高度。1+logN
5. 如何实现关键字输入提示，使用字典树，复杂度多少，有没有其他方案，答哈希，如果是中文呢，分词后建立字典树？
6. hashmap的实现讲一下吧，讲的很详细了。讲一下红黑树的结构，查询性能等。
7. 快排的时间复杂度，冒泡时间复杂度，快排是否稳定，快排的过程
8. 100w个数，怎么找到前1000个最大的，堆排序，怎么构造，怎么调整，时间复杂度。
9. 一个矩阵，从左上角到右下角，每个位置有一个权值。可以上下左右走，到达右下角的路径权值最小怎么走。
10. 四辆小车，每辆车加满油可以走一公里，问怎么能让一辆小车走最远。说了好几种方案，面试官引导我优化了一下，但是还是不满意，最后他说跳过。
11. MySQL的索引，B+树性质。
12. 十亿和数找到前100个最大的，堆排序，怎么实现，怎么调整。
13. 布隆过滤器
14. hash表解决冲突的方法
15. 跳表插入删除过程
16. 让你实现一个哈希表，怎么做（当时按照Redis中哈希表的实现原理回答）

## 9.设计模式

1. 对于单例模式，有什么使用场景了，讲了全局id生成器，他问我分布式id生成器怎么实现，说了zk，问我zk了解原理不，讲了zab，然后就没问啦。
2. 除了单例模式，知道适配器模式怎么实现么，有什么用

## 10.分布式架构

1. CAP BASE理论
2. 看你项目里面用了etcd，讲解下etcd干什么用的，怎么保证高可用和一致性？
3. 既然你提到了raft算法，讲下raft算法的基本流程？raft算法里面如果出现脑裂怎么处理？有没有了解过paxos和zookeeper的zab算法，他们之前有啥区别？
4. rpc有没有了解

## 11.系统设计

1. 朋友之间的点对点关系用图维护，怎么判断两人是否是朋友，并查集，时间复杂度，过程。
2. 10g文件，只有2g内存，怎么查找文件中指定的字符串出现位置。
3. Linux大文件怎么查某一行的内容。
4. 秒杀系统的架构设计
5. 十亿个数的集合和10w个数的集合，如何求它们的交集。
6. 回到网络，刚才你说到直播场景，知道直播的架构怎么设计么，要点是什么，说了几个不太对，他说要避免广播风暴，答不会。
7. 针对自己最熟悉的项目，画出项目的架构图，主要的数据表结构，项目中使用到的技术点，项目的总峰值qps，时延，以及有没有分析过时延出现的耗时分别出现在什么地方，项目有啥改进的地方没有？
8. 如果请求出现问题没有响应，如何定位问题，说下思路？
9. 除了公司项目之外，业务有没有研究过知名项目或做出过贡献？
10. go程和线程有什么区别？ 答：1 起一个go程大概只需要4kb的内存，起一个Java线程需要1.5MB的内存；go程的调度在用户态非常轻量，Java线程的切换成本比较高。接着问为啥成本比较高？因为Java线程的调度需要在用户态和内核态切换所以成本高？为啥在用户态和内核态之间切换调度成本比较高？简单说了下内核态和用户态的定义。接着问，还是没有明白为啥成本高？心里瞬间崩溃，没完没了了呀，OS这块依旧是痛呀，支支吾吾半天放弃了。
11. 服务器CPU 100%怎么定位？可能是由于平时定位业务问题的思维定势，加之处于蒙蔽状态，随口就是：先查看监控面板看有无突发流量异常，接着查看业务日志是否有异常，针对CPU100%那个时间段，取一个典型业务流程的日志查看。最后才提到使用top命令来监控看是哪个进程占用到100%。果然阵脚大乱，张口就来，捂脸。。。 本来正确的思路应该是先用top定位出问题的进程，再用top定位到出问题的线程，再打印线程堆栈查看运行情况，这个流程换平时肯定能答出来，但是，但是没有但是。还是得好好总结。
12. 最后问了一个系统设计题目（朋友圈的设计），白板上面画出系统的架构图，主要的表结构和讲解主要的业务流程，如果用户变多流量变大，架构将怎么扩展，怎样应对？ 这个答的也有点乱，直接上来自顾自的用了一个通用的架构，感觉毫无亮点。后面反思应该先定位业务的特点，这个业务明显是读多写少，然后和面试官沟通一期刚开始的方案的用户量，性能要求，单机目标qps是什么等等？在明确系统的特点和约束之后再来设计，而不是一开始就是用典型互联网的那种通用架构自顾自己搞自己的方案。
13. 设计一个限流的算法
14. 定时器除了小根堆，还可以怎么做
15. 项目性能瓶颈在哪，数据库表怎么设计
16. .在高并发的生产环境中（非调试场景下），如果出现数据包的丢失，如何定位问题
17. 补充一个最最重要，最最坑爹，最最有难度的一个题目：一个每秒百万级访问量的互联网服务器，每个访问都有数据计算和I/O操作，如果让你设计，你怎么设计？

## 12.道友总结

1. tcp/udp，http和https还有网络这块（各种网络模型，已经select，poll和epoll）一定要非常熟悉
2. 一定要有拿的出手的项目经验，而且要能够讲清楚，讲清楚项目中取舍，设计模型和数据表
3. 分布式要非常熟悉
4. 常见问题定位一定要有思路
5. 操作系统，还是操作系统，重要的事情说三遍
6. 系统设计，思路，思路，思路，一定要思路清晰，一定要总结下系统设计的流程
7. 一点很重要的心得，平时blog和专栏看的再多，如果没有自己的思考不过是过眼云烟，根本不会成为自己的东西，就像内核态和用户态，平常也看过，但是没细想，突然要自己说，还真说不出来，这就很尴尬了。勿以浮沙筑高台，基础这种东西还是需要时间去慢慢打牢，多去思考和总结。

原文地址：https://zhuanlan.zhihu.com/p/103027724

作者：linux