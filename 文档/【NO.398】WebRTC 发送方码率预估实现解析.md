# 【NO.398】WebRTC 发送方码率预估实现解析

## 1.WebRTC使用的是Google Congestion Control (简称GCC)拥塞控制，目前有两种实现：

\* 旧的实现是接收方根据收到的音视频RTP报文, 预估码率，并使用REMB RTCP报文反馈回发送方。 * 新的实现是在发送方根据接收方反馈的TransportFeedback RTCP报文，预估码率。

## 2.基于延迟的拥塞控制原理

先来看下Google Congestion Control（GCC）的标准草案：
[https://tools.ietf.org/html/draft-ietf-rmcat-gcc-02](https://link.zhihu.com/?target=https%3A//tools.ietf.org/html/draft-ietf-rmcat-gcc-02) 结合草案，可以得知GCC是基于网络延迟梯度的拥塞控制算法，判断的依据如下图：



![img](https://pic3.zhimg.com/80/v2-dc6cd2e07d1017bd738fcd60c44f721e_720w.webp)



发送方发送两个包组的间隔为 ： Ds = Ts1 - Ts0

接收方接收两个包组的间隔为： Dr = Tr1 - Tr0

如果网络没有任何抖动，那么 [ delta = Dr - Ds ] 应该是一个恒定不变的值，但是现实中网络有抖动、拥塞、丢包等情况，所以delta也是一个抖动的值。 GCC通过测量delta，来判断当前网络的使用情况，分为 OverUse (过载)，Normal(正常)，UnderUse(轻载) 这三种情况。 有同学可能会问，发送方和接收方时钟不统一，怎么计算差值呢，需要做时间对齐或者NTP同步吗?

不需要，因为我们对比的是delta，只需要单位一致即可，举个例子: seq1的包 发送时间为 16000ms(发送方时钟)，接收时间为 900ms(接收方时钟) seq2的包 发送时间为 16001ms(发送方时钟)，接收时间为 905ms(接收方时钟) 那么延迟梯度delta=(905-900) - (16001-16000) = 4

## 3.Pacing和包组

值得注意的是，延迟梯度的判断是以包组为单位的，而且必须在发送方开启pacing发送, 有以下几点原因： 单个包测量误差会过大，基于包组的测量更能反应网络的情况。

burst发送容易冲击网络，影响测量的精度。 那么怎么判断哪几个包属于一个包组呢，非常简单，按发送方的pacing速率分包组。 WebRTC pacing默认是5ms一个包组，如下图所示



![img](https://pic3.zhimg.com/80/v2-3d7459412e7dd4d0a9db8d3141809946_720w.webp)



## 4.TransportFeedback RTCP报文

再来看看transport-feedback的包结构：
[https://tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions-01](https://link.zhihu.com/?target=https%3A//tools.ietf.org/html/draft-holmer-rmcat-transport-wide-cc-extensions-01)



![img](https://pic3.zhimg.com/80/v2-dc372427de5196fc5db1a876165c432a_720w.webp)



解析这个报文，我们可以得到下面的信息：

- 接收到的包seq和包的接收时间
- 丢失的包seq
- 可以看到本质上transport-feedback是接收方对数据的ACK，并且捎带了接收的延迟梯度。

## 5.发送方码率预估

收到transport-feedback报文后需要怎么处理，结合GCC的算法来看，分为以下几步： 1.计算接收方ack了多少个字节, 统计在采样的时间窗口内接收方的接收速率 看看GCC怎么说：



![img](https://pic3.zhimg.com/80/v2-078e84d9027540881304d33298f45d96_720w.webp)



按照这个算法实现acked_bitrate_estimate，可以计算出接收方在当前时间窗口内的接收速率。

2.将包按包组归类, 计算包组的发送时间 接收时间的差值 在前面的【Pacing和包组】中已经讲过，这里不再赘述

3.按包组的delta, 进行网络状态评估 GCC的标准草案里面使用的是卡尔曼滤波器(接收方评估)，发送方评估默认的实现是Trendline Filter。

基本的原理是最小二乘法, 将多个时间点的网络抖动(delta)拟合成一条直线，如下图所示：



![img](https://pic4.zhimg.com/80/v2-0f1ab31588ffbd5094a4791eb50ca6bf_720w.webp)



根据直线斜率的变化趋势判断网络的负载情况。 上面已经得到了包组的delta，对delta做平滑计算后，按照(时间点, 平滑后的delta)， 可以在坐标系上绘制出散点图，使用最小二乘法拟合出delta随时间变化的直线，根据直线斜率计算出变化趋势。 来看看GCC里面的说法：



![img](https://pic2.zhimg.com/80/v2-f848914ce7bc259be7ec342cd4d23e91_720w.webp)



- m(i)为i时刻的包组delta，del_var_th(i)为当前判断是否过载的门槛
- m(i) < -del_var_th(i)，判断为under-use(低载)
- m(i) > del_var_th(i) 且持续至少overuse_time_th时长，判断为over-use(过载)





![img](https://pic1.zhimg.com/80/v2-29a99dc1a7f8c0dba1cfabe9785dce14_720w.webp)



del_var_th必须动态调整，不然可能会在跟TCP的竞争中被饿死（出于公平性考虑）。过大的del_var_th会对延迟变化不敏感，过小的del_var_th则会过于敏感，抖动容易被频繁误判为过载，必须动态调整，才能和基于丢包的连接（比如TCP）竞争。

## 6.根据探测的网络情况, 预估码率

总体的思想是根据当前接收方的接收码率，结合当前的网络负载情况，进行AIMD码率调整：

- 在接近收敛前，使用乘性增，接近收敛时使，用加性增。
- 当网络过载时，使用乘性减。





![img](https://pic1.zhimg.com/80/v2-9972a991b8ffc24011b86ed5f9993f6c_720w.webp)



在Decrease状态下，会不停地计算平均最大码率（average max bitrate），当前预估码率和平均最大码率差值在3个标准差以内时，进行乘性增，否则进行加性增。如果包到达速率超过了平均最大码率的3个标准差，那么需要重新计算平均最大码率。





![img](https://pic1.zhimg.com/80/v2-5e5ac8001eb28c0a8f6ab860954e7894_720w.webp)



乘性增期间，每秒最多增加8%的码率





![img](https://pic2.zhimg.com/80/v2-d7104231726f50718721e54255d1c385_720w.webp)



加性增期间，每个rtt增加“半个”包大小



![img](https://pic1.zhimg.com/80/v2-6840db97de907d629bebfecdb0e765f4_720w.webp)



评估出的码率不能超过接收速率的1.5倍



![img](https://pic1.zhimg.com/80/v2-9f0de0363116fe86672b4b3eb6ca6514_720w.webp)



当探测到网络过载时，按照0.85的速率降低码率

## 7.发送方码率预估的算法流程

将上面的几步结合起来，可以得到一个大致的算法框架

```text
struct FeedbackResultVector {
    int64_t send_time_ms; // 包发送时间(发送时记录)
    int64_t recv_time_ms; // 包接收时间(从TransportFeedback RTCP报文解析得到)
    int packet_size; // 包大小
};

// 解析TransportFeedback RTCP报文
FeedbackResultVector feedback_result_vec = 
    TransportFeedbackRtcp.Parse(rtcp_feedback);
// 遍历每个包, 进行处理
for (feedback_result : feedback_result_vec) {
    double delta = 0.0;
    // 计算ack速率(接收方接收速率)
    AckBitrateEstimate.Update(now_ms, feedback_result.packet_size);
    // 把接收反馈包按照包组分类,计算包组delta
    bool compute_delta = PacketGroup.AddPacket(feedback_result, delta);
    if (comupute_delta) {
        // 探测网络状态
        TrendLineFilter.Detect(delta);
        // 根据GCC状态机,进行AIMD码率调整
        AimdRateControl.Update(
            TrendLineFilter.NetState(), 
            AckBitrateEstimate.Bitrate()
        );
    }
}
```

原文作者：零声音视频开发

原文链接：https://zhuanlan.zhihu.com/p/428681596