# 【NO.468】音视频面试问题|面试技巧

从事音视频四五年，面试官都会问的面试题都有哪些？
在这里要告诉大家的是对于自己特别熟悉的，拿手的，一定要重点多说一些，其他的，不熟悉的，一带而过。而且自己做的项目，一定要特别熟悉，内部啥原理，甚至代码细节，都会被问到。 还要养成一个好习惯：多做笔记！！！俗话说，好记性不如烂笔头！所以做笔记的习惯一定要有！下面给一些示例给大家讲解一下！！

## **1.常见的音视频格式有哪些？**

**参考答案**

1. MPEG（运动图像专家组）是Motion Picture Experts Group 的缩写。这类格式包括了MPEG-1,MPEG-2和MPEG-4在内的多种视频格式。
2. AVI，音频视频交错(Audio Video Interleaved)的英文缩写。AVI这个由微软公司发布的视频格式，在视频领域可以说是最悠久的格式之一。
3. MOV，使用过Mac机的朋友应该多少接触过QuickTime。QuickTime原本是Apple公司用于Mac计算机上的一种图像视频处理软件。
4. ASF(Advanced Streaming format高级流格式)。ASF 是MICROSOFT 为了和的Real player 竞争而发展出来的一种可以直接在网上观看视频节目的文件压缩格式。
5. WMV，一种独立于编码方式的在Internet上实时传播多媒体的技术标准，Microsoft公司希望用其取代QuickTime之类的技术标准以及WAV、AVI之类的文件扩展名。
6. NAVI，如果发现原来的播放软件突然打不开此类格式的AVI文件，那你就要考虑是不是碰到了n AVI。n AVI是New AVI 的缩写，是一个名为Shadow Realm 的地下组织发展起来的一种新视频格式。
7. 3GP是一种3G流媒体的视频编码格式，主要是为了配合3G网络的高传输速度而开发的，也是目前手机中最为常见的一种视频格式。
8. REAL VIDEO（RA、RAM）格式由一开始就是定位在视频流应用方面的，也可以说是视频流技术的始创者。
9. MKV，一种后缀为MKV的视频文件频频出现在网络上，它可在一个文件中集成多条不同类型的音轨和字幕轨，而且其视频编码的自由度也非常大，可以是常见的DivX、XviD、3IVX，甚至可以是RealVideo、QuickTime、WMV 这类流式视频。
10. FLV是FLASH VIDEO的简称，FLV流媒体格式是一种新的视频格式。由于它形成的文件极小、加载速度极快，使得网络观看视频文件成为可能，它的出现有效地解决了视频文件导入Flash后，使导出的SWF文件体积庞大，不能在网络上很好的使用等缺点。
11. F4V，作为一种更小更清晰，更利于在网络传播的格式，F4V已经逐渐取代了传统FLV，也已经被大多数主流播放器兼容播放，而不需要通过转换等复杂的方式。

## **2.列举一些音频编解码常用的实现方案？**

**参考答案**

- 第一种就是采用专用的音频芯片对 语音信号进行采集和处理，音频编解码算法集成在硬件内部，如 MP3 编解码芯片、语音合成 分析芯片等。使用这种方案的优点就是处理速度块，设计周期短；缺点是局限性比较大，不灵活，难以进行系统升级。
- 第二种方案就是利用 A/D 采集卡加上计算机组成硬件平台，音频编解码算法由计算机上的软件来实现。使用这种方案的优点是价格便 宜，开发灵活并且利于系统的升级；缺点是处理速度较慢，开发难度较大。
- 第三种方案是使用高精度、高速度 的 A/D 采集芯片来完成语音信号的采集，使用可编程的数据处理能力强的芯片来实现语音信号处理的算法，然后 用 ARM 进行控制。采用这种方案的优点是系统升级能力强，可以兼容多种音频压缩格式甚至未来的音频压缩格 式，系统成本较低；缺点是开发难度较大，设计者需要移植音频的解码算法到相应的 ARM 芯片中去。

## **3.请叙述AMR基本码流结构？**

**参考答案**

AMR文件由文件头和数据帧组成，文件头标识占6个字节，后面紧跟着就是音频帧；

格式如下所示：

文件头（占 6 字节）| ：--- | 语音帧1 | 语音帧2 | … |

- 文件头： 单声道和多声道情况下文件的头部是不一致的，单声道情况下的文件头只包括一个Magic number，而多声道情况下文件头既包含Magic number，在其之后还包含一个32位的Chanel description field。多声道情况下的32位通道描述字符，前28位都是保留字符，必须设置成0，最后4位说明使用的声道个数。

语音数据： 文件头之后就是时间上连续的语音帧块了，每个帧块包含若干个8位组对齐的语音帧，相对于若干个声道，从第一个声道开始依次排列。每一个语音帧都是从一个8位的帧头开始：其中P为填充位必须设为0，每个帧都是8位组对齐的。

除此之外还很多**面试题**需要去掌握的，这里我就不一一列举啦！

![img](https://pic2.zhimg.com/80/v2-91f19bb5dca86b219cb1117c868c8541_720w.webp)

![img](https://pic3.zhimg.com/80/v2-57c8d3973ded64074499f3d5799445ce_720w.webp)

![img](https://pic2.zhimg.com/80/v2-223d78688da065a9c060746c48651a59_720w.webp)

![img](https://pic3.zhimg.com/80/v2-2777a6e0e7104730035c6124e0ba4f66_720w.webp)

![img](https://pic1.zhimg.com/80/v2-bd4c021d09a6712c067ed9ebb1ee3904_720w.webp)

原文作者：零声音视频开发

原文链接：https://zhuanlan.zhihu.com/p/428517343