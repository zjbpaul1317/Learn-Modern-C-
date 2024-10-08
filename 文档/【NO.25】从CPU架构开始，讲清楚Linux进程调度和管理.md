# 【NO.25】从CPU架构开始，讲清楚Linux进程调度和管理

## 1. X86架构

**CPU包括3个部分**：运算单元，数据单元，控制单元

总线上主要有两类数据，一类是地址数据（要拿内存中哪个位置的数据），这类总线是**地址总线**；另一类是真正的数据，这类总线是**数据总线**

**32位CPU包含的寄存器**

- 通用寄存器：`EAX/EBX/ECX/EDX/ESI/EDI/ESP/EBP`
- 段寄存器：`CS/DS/ES/FS/GS/SS`
- 指令寄存器：`EIP`
- 标志寄存器：`EFLAGS`
- 控制寄存器：`CR0/CR2/CR3/CR4`
- 系统表指针寄存器：`IDTR/GDTR/LDTR`
- 任务寄存器：`TR`
- 调试寄存器：`DR0-DR7`

> **文章相关视频讲解：**
> **[免费订阅学习：c/c++Linux后台服务器开发高级架构师学习视频](https://link.zhihu.com/?target=https%3A//ke.qq.com/course/417774%3FflowToken%3D1033508)**
> **[Linux内核内存管理—内存映射](https://link.zhihu.com/?target=https%3A//www.bilibili.com/video/BV1EQ4y1d76X/)**
> **[Linux内核内存管理(二)](https://link.zhihu.com/?target=https%3A//www.bilibili.com/video/BV1Qy4y1g7mY/)**
> **[虚拟内存空间之VMA实战操作](https://link.zhihu.com/?target=https%3A//www.bilibili.com/video/BV1i44y1r7yL/)**
> **[剖析Linux内核MMU机制](https://link.zhihu.com/?target=https%3A//www.bilibili.com/video/BV1xy4y1W7a6/)**

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/202212122028371156310.png)

- `CS`：存储代码段的基址
- `DS`：存储数据段的基址
- `SS`：存储运行栈的基址
- `EBP`：栈基指针寄存器
- `ESP`：栈顶指针寄存器
- `IP`：指令指针寄存器，**将`CS:IP`获取到的指令存储`EIP`（指令寄存器）中**

## 2. 操作系统启动流程

主板上的ROM固化了一段初始化程序**BIOS（Basic Input and Output System，基本输入输出系统）**

Grub2是一个linux启动管理器 ，Grub2把boot.img共512字节安装到启动盘的第一个扇区，这个扇区称为**MBR（Master Boot Record，主引导记录扇区）**

- 主机上电后，CS重置为0xFFFF，IP重置为0x0000，所以第一条指令指向0xFFFF0，正好在ROM范围内，从这里开始进行**硬件检查**
- **将磁盘引导扇区读入**0x7c00处，并从0x7c000开始执行
- boot模块：bootsect.s，继续读取其它扇区
- setup模块：setup.s
- **从实模式切换到保护模式**（CR0寄存器最后一位置1）
- **初始化GDT，IDT**
- system模块：许多文件（head.s，main.c …）
- 进入main函数，**进入各种初始化操作**
  `INIT_TASK(init_task)`：系统创建第一个进程，**0号进程**。这是唯一一个没有通过fork或者kernel_thread产生的进程，是进程列表的第一个
  `trap_init()`
  `mm_init()`
  `sched_init()`
  `rest_init()`
- - `kernel_thread(kernel_init, NULL, CLONE_FS)` 创建第二个进程，这个是**1号进程**。 1号进程即将运行一个用户进程
  - 从内核态到用户态
  - ramdisk
  - 创建2号进程

0号进程 -> 1号内核进程 -> 1号用户进程(init进程) -> getty进程 -> shell进程 -> 命令行执行进程。

1. 加载内核：打开电源后，加载BIOS（只读，在主板的ROM里），硬件自检，读取MBR（主引导记录），运行Boot Loader，内核就启动了，这里还有从实模式到保护模式的切换
2. 进程：内核启动后，先启动第0号进程init_task，然后在内核运行init()函数，1号进程进入用户态变为init进程，init进程是所有用户态进程的老祖宗，其配置文件是/etc/inittab，通过该文件设置运行等级：根据/etc/inittab文件设置运行等级，比如有无网络
3. 系统初始化：启动第一个用户层文件/etc/rc.d/rc.sysinit，然后激活交换分区、检查磁盘、加载硬件模块等
4. 建立终端：系统初始化后返回init，这时守护进程也启动了，init接下来打开终端以供用户登录

- **init_task是第0号进程，唯一一个没有通过fork或kernel_thread产生的进程，是进程列表的第一个**
- **init是第1号进程，会成为用户态，是所有用户态进程的祖先**
- **kthreadd是第2号进程，会成为内核态，是所有内核态进程的祖先**

## 3. 系统调用

**int指令将CS中的CPL改为0，从而进入内核，这是用户程序发起调用内核代码的唯一方式**

**系统调用核心**

1. 用户程序中包含一段包含int指令的代码
2. 操作系统写中断处理，获取调用程序的编号
3. 操作系统根据编号执行相应代码

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202938_75065.jpg)

1. 在`glibc`里，任何一个系统调用都会调用`DO_CALL`，在`DO_CALL`里会根据系统调用名称，得到**系统调用号**，放在**寄存器`EAX`**里，把请求参数放在其它寄存器里，然后**执行`int $0x80`**
2. `int $0x80`触发一个软中断，通过它陷入内核
   内核启动时`trap_init()`指定软中断陷入门的入口，即`entry_INT80_32`
3. 保存当前用户态的寄存器，保存在**`pt_regs`结构**里
4. 将系统调用号从寄存器`EAX`里取出来，然后根据系统调用号，在**系统调用表**中找到相应的函数进行调用
5. 系统调用结束后， 原来用户态保存的现场恢复回来

## 4. 进程状态切换

- 就绪状态（ready）：等待被调度
- 运行状态（running）
- 阻塞状态（waiting）：等待资源

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202939_59603.jpg)

- 只有就绪态和运行态可以相互转换，其它的都是单向转换。就绪状态的进程通过调度算法从而获得 CPU 时间，转为运行状态；而运行状态的进程，在分配给它的 CPU 时间片用完之后就会转为就绪状态，等待下一次调度。
- 阻塞状态是缺少需要的资源从而由运行状态转换而来，但是该资源不包括 CPU 时间，缺少 CPU 时间会从运行态转换为就绪态。

## 5.进程数据结构

**Linux的PCB是task_struct结构**

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202939_30279.jpg)

### 5.1 进程状态

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202940_27637.jpg)

很多操作系统教科书将正在CPU上执行的进程定义为RUNNING状态，而将可执行但是尚未被调度执行的进程定义为READY状态，**这两种状态在linux下统一为TASK_RUNNING状态**

### 5.2 任务ID

在Linux系统中，一个线程组中的所有线程使用和该线程组的领头线程相同的pid，并被存放在tgid成员中。只有线程组的领头线程的pid成员才会被设置为与tgid相同的值

**注意，`getpid()`系统调用返回的是当前进程的tgid值而不是pid值**

### 5.3 用户栈和内核栈

内核在创建进程的时候，在创建`task_struct`的同时，会为进程创建相应的堆栈。**每一个进程都有两个栈，一个用户栈，存在于用户空间；一个内核栈，存在于内核空间**

当进程在用户空间运行时，CPU堆栈指针寄存器里面的内容都是用户栈地址，使用用户栈；当进程在内核空间运行时，CPU堆栈指针寄存器里面的内容是内核栈地址，使用内核栈

当进程因为中断或者系统调用陷入到内核态时，进程所使用的堆栈也要从用户栈转到内核栈。

进程陷入到内核态后，**先把用户态堆栈的地址保存在内核栈之中，然后设置堆栈指针寄存器的内容为内核栈的地址，**这样就完成了用户栈向内核栈的转换；当进程从内核态恢复到用户态时**，在内核态之后的最后将保存在内核栈里面的用户栈的地址恢复到堆栈指针寄存器即可**。这样就实现了内核栈向用户栈的转换

在进程从用户态转到内核态的时候，进程的内核栈总是空的。这是因为当进程在用户态运行时使用用户栈，当进程陷入到内核态时，内核保存进程在内核态运行的相关信息，但是一旦进程返回到用户态后，内核栈中保存的信息全部无效，因此每次进程从用户态陷入内核的时候得到的内核栈都是空的。**所以在进程陷入内核的时候，直接把内核栈的栈顶地址给堆栈指针寄存器就可以了**

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202941_30547.jpg)

`task_struct->stack`指向进程的内核栈，**大小为8K**

```
union thread_union {    struct thread_info thread_info;    unsigned long stack[THREAD_SIZE/sizeof(long)];};
```

整个内核栈用union表示，thread_info和stack共用一段存储空间，thread_info占用低地址。在pt_regs和STACK_END_MAGIC之间，就是内核代码的运行栈。当内核栈增长超过STACK_END_MAGIC就会报内核栈溢出

**thread_info**：存储内核态运行的一些信息，如指向`task_struct`的task指针，使得陷入内核态之后仍然能够找到当前进程的`task_struct`，还包括是否允许内核中断的preemt_count开关等等

**pt_regs**：**存储用户态的硬件上下文**，用户态进入内核态后，由于使用的栈、内存地址空间、代码段等都不同，所以用户态的eip、esp、ebp等需要保存现场，内核态恢复到用户态时再将栈中的信息恢复到硬件。由于进程调度一定会在内核态的schedule函数，用户态的所有硬件信息都保存在pt_regs中了。SAVE_ALL指令就是将用户态的cpu寄存器值保存如内核栈，RESTORE_ALL就是将pt_regs中的值恢复到寄存器中，这两个指令在介绍中断的时候还会提到

TSS（task state segment）：这是intel为上层做进程切换提供的硬件支持，还有一个TR（task register）寄存器专门指向这个内存区域。当TR指针值变更时，intel会将当前所有寄存器值存放到当前进程的tss中，然后再讲切换进程的目标tss值加载后寄存器中

这里很多人都会有疑问，不是有内核栈的pt_regs存储硬件上下文了吗，为什么还要有tss？前文说过，**进程切换都是在内核态，而pt_regs是保存的用户态的硬件上下文，tss用于保存内核态的硬件上下文**

但是linux并没有买账使用tss，因为linux实现进程切换时并不需要所有寄存器都切换一次，如果使用tr去切换tss就必须切换全部寄存器，性能开销会很高。这也是intel设计的败笔，没有把这个功能做的更加的开放导致linux没有用。linux使用的是软切换，主要使用thread_struct，tss仅使用esp0这个值，用于进程在用户态 -> 内核态时，硬件会自动将该值填充到esp寄存器。在初始化时仅为每1个cpu仅绑定一个tss，然后tr指针一直指向这个tss，永不切换。

thread_struct：一个和硬件体系强相关的结构体，**用来存储内核态切换时的硬件上下文**

### 5.4 context_switch执行过程

内存空间切换

将curr_task设置为新进程的task

将cpu寄存器保存到旧进程的thread_struct结构

将新进程的thread_struct的寄存器的值写入cpu

切换栈顶指针

## 6. Linux调度算法

### 6.1 调度子系统

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202941_38524.jpg)

### 6.2 调度策略和调度类

- Linux把进程分成**实时进程**和**非实时进程**，非实时进程进一步分成**交互式进程**和**批处理进程**
- 实时进程要求执行时间控制在一定范围内，**基于优先级调度，可抢占低优先级进程**

```
task_struct {  unsigned int policy  // 调度策略  int prio, static_prio, normal_prio;   // 优先级  unsigned int rt_priority;  const struct sched_class* sched_class;   // 调度器类}
// 调度策略定义#define SCHED_NORMAL   0         // 普通进程#define SCHED_FIFO     1         // 相同优先级先来先服务#define SCHED_RR       2         // 相同优先级轮流调度#define SCHED_BATCH    3         // 后台进程#define SCHED_IDLE     5         // 空闲进程#define SCHED_DEADLINE 6         // 相同优先级电梯算法（deadline距离当前时间点最近）
```

- 实时进程的优先级范围是0-99，非实时进程优先级范围100-139，**数值越小，优先级越高**
- **实时进程的调度策略**：SCHED_FIFO、SCHED_RR、SCHED_DEADLINE
- **非实时进程的调度策略**：SCHED_NORMAL、SCHED_BATCH、SCHED_IDLE

```
// 调度器类定义stop_sched_class  // 优先级最高的任务会使用这种策略，中断所有其他线程，且不会被其他任务打断dl_sched_class    // 对应deadline调度策略rt_sched_class    // 对应RR算法或者FIFO算法的调度策略，具体调度策略由进程的task_struct->policy指定fair_sched_class  // 普通进程的调度策略idle_sched_class  // 空闲进程的调度策略
```

### 6.3 CFS调度算法（完全公平调度算法）

- CFS给每个进程安排一个**虚拟运行时间vruntime**，正在运行的进程vruntime随tick不断增大，没有运行的进程vruntime不变，**vruntime小的会被优先运行**
- 对于不同优先级的进程，换算vruntime时**优先级高的算少，优先级低的算多**，这样优先级高的进程实际运行时间就变多了
- 调度队列使用红黑树，红黑树的节点是调度实体

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202942_76856.jpg)

- CFS的队列是一棵红黑树，红黑树的节点是调度实体，每个调度实体都属于一个task_struct，task_struct里面有指针指向这个进程属于哪个调度类
- CPU需要找下一个任务执行时，会按照优先级依次调用调度类，不同的调度类操作不同的队列。当然rt_sched_class先被调用，它会在rt_rq上找下一个任务，只有找不到的时候，才轮到fair_sched_class被调用，它会在cfs_rq上找下一个任务。这样保证了实时任务的优先级永远大于普通任务

### 6.4 调度触发

- 调度触发有两种类型：**主动调度**和**被动调度**（抢占式调度）
- 进程的调度都最终会调用到内核态的**__schedule函数**

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202943_48749.jpg)

**主动调度**

- 进程发生需要等待IO的系统调用
- 进程主动sleep
- 进程等待占用信号量或mutex（自旋锁不会触发调度）

**抢占式调度**（**先标记为应该被抢占，等到调用__schedule函数时才调度**）

- 时钟中断处理函数
- 当一个进程被唤醒的时候

### 6.5 抢占的时机

用户态的抢占时机

- 从系统调用返回用户态
- 从中断返回用户态

内核态的抢占时机

- 在内核态的执行中，有的操作是不能被中断的，在进行这些操作之前，先调用preempt_disable关闭抢占，当再次打开的时候，就是一次内核态被抢占的机会
- 从中断返内核态

## 7. fork过程

fork系统调用最终调用*do*fork函数

1. 复制结构copy_process
2. 唤醒新进程wake_up_new_task

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202943_53131.jpg)

## 8. 线程创建过程

线程不是一个完全由内核实现的机制，它是由内核态和用户态合作完成的

**调用clone系统调用**

创建进程调用的系统调用是fork，在copy_process函数里面，会将五大结构`files_struct`、`fs_struct`、`sighand_struct`、`signal_struct`、`mm_struct`都复制一遍，从此父进程和子进程各用各的数据结构

创建线程调用的系统调用是clone，在copy_process函数里面，**五大结构仅仅是引用计数加一**，即线程共享进程的数据结构

![img](https://linuxcpp.0voice.com/zb_users/upload/2022/12/12/20221212202944_59887.jpg)

## 9. Linux装载和运行程序

Shell进程在读取用户输入的命令之后会调用fork复制出一个新的Shell进程，然后新的Shell进程调用exec执行新的程序

## 10. 函数调用堆栈

3个重要的寄存器

- `EIP`指向CPU下次要执行指令
- `ESP`栈指针寄存器，永远指向系统栈最上面一个**栈帧的栈顶**
- `EBP`基址指针寄存器，永远指向系统栈最上面一个**栈帧的栈底**

栈帧

每个函数每次调用，都有自己独立的栈帧，`ESP`指向当前栈帧的栈顶，`EBP`指向当前栈帧的栈底

栈从高地址向低地址扩展

```
push eax; == esp=esp-4;eax->[esp];pop eax; == [esp]->eax;esp=esp+4;
```

在`main()`里调用`A(a, b)`

1. `push b;` 最右边参数先入栈
2. `push a;`
3. `push eip;` 将A函数的`EIP`压栈
4. `push ebp;` 将A函数的`EBP`压栈（把main函数的栈帧底部保存起来，不用保存栈帧顶部）
5. `mov ebp esp;` 将当前的`ESP`赋值给`EBP`，相当于`EBP`从指向main的栈基址指向了A的栈基址
   …
6. 当从A函数返回时，`ESP`移动到栈帧底部（释放局部变量），然后把main函数的栈帧底部弹出到`EBP`，再弹出返回地址到`EIP`上，`ESP`继续移动划过参数，这样，`EBP`，`ESP`就回到了调用函数前的状态，即恢复了原来的main的栈帧

## 11. CPU上下文切换

CPU上下文：**CPU相关寄存器**

CPU上下文切换：保存前一个任务的CPU上下文，加载新任务的CPU上下文，然后运行新任务

根据任务不同，CPU上下文切换分成不同场景

- 进程上下文切换
- 线程上下文切换
- 中断上下文切换

进程可以在用户空间运行，也可以在内核空间运行，用户态和内核态的转变通过系统调用完成；

系统调用过程发生了CPU上下文切换

- 一次系统调用，发生了两次CPU上下文切换

**进程的切换发生在内核态，进程的上下文不仅包括了虚拟内存，栈，全局变量等用户空间的资源，还包括内核堆栈，寄存器等内核空间的状态**

进程上下文切换比系统调用多了一步

当虚拟内存更新后，TLB也需要更新，内存访问会随之变慢，影响所有处理器的进程

线程上下文切换

- 线程是调度的基本单位
- 前后两个线程属于不同进程，切换过程同进程上下文切换
- 前后两个线程属于同一进程，切换时虚拟内存等共享资源不动，只切换线程私有数据等非共享资源

中断上下文只包括内核中断服务程序执行所必需的的状态，包括CPU寄存器，内核堆栈，硬件中断参数等

过多的上下文切换，会把CPU时间消耗在寄存器、内核栈以及虚拟内存等数据的保存和恢复上，从而缩短进程真正运行的时间，导致系统的整体性能大幅下降

```
# 每隔5s输出1组数据vmstat 5#    cs(context switch)是每秒上下文切换的次数#    in(interrupt)则是每秒中断的次数#    r(Running or Runnable)是就绪队列的长度，也就是正在运行和等待CPU的进程数#    b(Blocked)则是处于不可中断睡眠状态的进程数# 每隔5s输出1组数据pidstat -wt 5#    cswch 每秒自愿上下文切换的次数#    nvcswch 每秒非自愿上下文切换的次数
```

## 12. 内核线程、轻量级进程、用户进程

Linux下只有一种类型的进程，就是task_struct

一个进程由于其运行空间的不同, 从而有内核线程和用户进程的区分, 内核线程运行在内核空间, 之所以称之为线程是因为它没有虚拟地址空间，只能访问内核的代码和数据。而用户进程则运行在用户空间，但是可以通过中断, 系统调用等方式从用户态陷入内核态

用户进程运行在用户空间上, 而一些通过共享资源实现的一组进程我们称之为线程组。linux下内核其实本质上没有线程的概念，linux下线程其实上是与其他进程共享某些资源的进程而已。但是我们习惯上还是称他们为线程或者轻量级进程

## 13. 进程栈，线程栈，内核栈，中断栈

- 进程栈就是Linux内存结构的栈，向低地址增长
- **线程栈由mmap获得，和进程处于同一地址空间，线程栈是事先分配好的，不能动态增长**
- 内核栈在地址空间的最高处，当用户调用系统调用时就进入内核栈，进程间的内核栈是相互独立的
- 中断栈用于运行中断响应程序

## 14. 中断

**中断导致系统从用户态转为内核态**

- 用户程序调用系统调用引起的**软件中断**
- 由外部设备产生的**硬件中断**
- 代码执行出错导致的**异常**

**软件中断**

用户进程调用系统调用时，会触发软件中断（第128号中断），该中断使得系统转到内核态，并执行相应的中断处理程序，中断处理程序根据系统调用号调用对应的系统调用

**硬件中断**

CPU收到硬件中断后，就会通知操作系统，每个硬件中断有对应一个中断号，并对应一个中断处理程序（ISR）

中断处理程序一般分为两部分（top half，bottom half），执行前面部分时会禁止所有中断，做一些有严格时限的工作，在执行后面部分时则允许被打断

Linux2.6之后，每个处理器都拥有一个中断栈专门用来执行中断处理程序

**中断处理流程**

1. 硬件中断到达中断控制器，后者检查该中断是否被屏蔽
2. 中断到达CPU
3. 系统跳转到一个预定义的内存位置，执行预定义的指令
4. 屏蔽相同的中断，检查是否有对应的ISR
5. 如果有就执行对应的ISR
6. 检查是否需要调用schedule()，返回被中断处

**用户态和内核态**

系统调用、异常、外围设备中断会导致系统进入内核态

系统调用：malloc内存分配

异常：缺页异常

外围设备中断：鼠标键盘

从用户态转入内核态时，系统需要先保存进程上下文，切换到内核栈，更新寄存器，将权限修改为特权级，转而去执行相应指令

## 15. 进程、线程、协程

进程

- 进程是资源分配的基本单位，它是程序执行时的一个实例。程序运行时系统就会创建一个进程，并为它分配资源，然后把该进程放入进程就绪队列，进程调度器选中它的时候就会为它分配CPU时间，程序开始真正运行
- 一个进程收到来自客户端新的请求时，可以通过`fork()`复制出一个子进程来处理，父进程只需负责监控请求的到来，这样就能做到并发处理。根据写时拷贝（COW）机制，分为两个进程继续运行后面的代码。**fork分别在父进程和子进程中返回，在子进程永远返回0，在父进程返回的是子进程的pid**

线程

- 线程是资源调度的基本单位，一个进程可以由很多个线程组成，线程间共享进程的所有资源，每个线程有自己的堆栈和局部变量。线程由CPU独立调度执行，在多CPU环境下就允许多个线程同时运行。同样多线程也可以实现并发操作，每个请求分配一个线程来处理

线程和进程的区别

- 进程是资源分配的最小单位，线程是资源调度的最小单位
- 一个进程的所有线程共享地址空间，但每个线程具有独立的程序计数器，寄存器，栈，CPU切换的代价和创建一个线程的开销都比进程小
- 进程间通信需要通过IPC进行，线程间通信更方便，但要处理好**同步与互斥**，
- 多进程程序更健壮，因为进程有自己独立的地址空间，多线程程序只要有一个线程死掉，整个进程就死掉了

协程

协程是用户态的轻量级线程，其调度由用户控制，共享进程的地址空间，协程有自己的栈和寄存器，在切换时需要保存/恢复状态，切换时减少了切换到内核态的开销. 一个进程和线程都可以有多个协程. **相比函数调用, 协程可以在遇到IO阻塞时交出控制权**

- 子程序，或者称为函数，在所有语言中都是层级调用，比如A调用B，B在执行过程中又调用了C，C执行完毕返回，B执行完毕返回，最后是A执行完毕。
- 所以子程序调用是通过栈实现的，一个线程就是执行一个子程序。
- 子程序调用总是一个入口，一次返回，调用顺序是明确的。
- 而协程的调用和子程序不同。协程看上去也是子程序，但执行过程中，在子程序内部可中断，然后转而执行别的子程序，在适当的时候再返回来接着执行。
- 在一个子程序中中断，去执行其他子程序，不是函数调用，有点类似CPU的中断
- 协程的特点在于是一个线程执行，那和多线程比，协程有何优势？
- - **最大的优势就是协程极高的执行效率**。因为子程序切换不是线程切换，而是由程序自身控制，因此，没有线程切换的开销，和多线程比，线程数量越多，协程的性能优势就越明显。
  - **第二大优势就是不需要多线程的锁机制**，因为只有一个线程，也不存在同时写变量冲突，在协程中控制共享资源不加锁，只需要判断状态就好了，所以执行效率比多线程高很多。
- 因为协程是一个线程执行，那怎么利用多核CPU呢？
- - 最简单的方法是多进程+协程，既充分利用多核，又充分发挥协程的高效率，可获得极高的性能。
  - Python对协程的支持还非常有限，用在generator中的yield可以一定程度上实现协程。虽然支持不完全，但已经可以发挥相当大的威力了。

## 16. 多进程与多线程的使用场景

多进程适用于**CPU密集型**，或者**多机分布式**场景中, 易于多机扩展

**多线程模型的优势是线程间切换代价较小**，因此适用于**I/O密集型**的工作场景，因此I/O密集型的工作场景经常会由于I/O阻塞导致频繁的切换线程。同时，多线程模型也适用于**单机多核**分布式场景

## 17. 协程

协程（coroutine）又叫微线程、纤程，完全位于用户态，一个程序可以有多个协程

协程的执行过程类似于子例程，允许子例程在特定地方挂起和恢复

协程是一种伪多线程，在一个线程内执行，**由用户切换**，由**用户选择切换时机**，**没有进入内核态**，只涉及CPU上下文切换，所以切换效率很高

缺点：协程适用于IO密集型，不适用于CPU密集型

## 18. 进程调度算法

- RR 时间片轮转调度算法
- FCFS 先来先服务调度算法
- SJF 短作业优先调度算法
- 优先级调度算法
- 多级队列调度算法
- 多级反馈队列调度算法
- 高响应比优先调度算法

对于单处理器系统，每次只允许一个进程运行，任何其他进程必须等待，直到CPU空闲能被调度为止，**多道程序**的目的是在任何时候都有某些进程在运行，以使CPU使用率最大化。

- 先到先服务FCFS，用了FIFO队列，非抢占
- 轮转法调度Round Robin，就绪队列作为循环队列，按**时间片**切换，一般20ms~50ms
- 最短作业优先调度SJF，先处理短进程，平均等待时间最小，所以是**最佳**的，但是很难知道每个作业要多少实现，所以不太现实
- 优先级调度，会有**饥饿**现象，低优先级的进程永远得不到调度
- 多级队列调度，根据进程的属性（如内存大小、类型、优先级）分到特定队列，不同队列执行不同的调度算法
- 多级反馈队列调度，允许进程在队列之间移动
- - 如果进程使用过多的CPU时间，那么它会被移到更低的优先级队列。
  - 这种方案将I/O密集型和交互进程放在更高优先级队列上。
  - 此外，在较低优先级队列中等待过长的进程会被移到更高优先级队列。这种形式的老化可阻止饥饿的发生。

CPU密集转为IO密集

饥饿(starvation)是什么，如何解决

饥饿是指某进程因为优先级的关系一直得不到CPU的调度，可能永远处于等待/就绪状态；定期提升进程的优先级

原文链接：https://zhuanlan.zhihu.com/p/378155667

原文作者：零声Github整理库