# 1.简介
## 1.1使用复制的目的
在分布式系统中，数据通常需要被分散在多台机器上，主要为了达到以下目的：
- 扩展性，数据量因读写负载巨大，一台机器无法承载，数据分散在多台机器上可以有效地进行负载均衡，达到灵活的横向扩展。
- 容错、高可用，在分布式系统中，单机故障是常态，在单机故障下仍然希望系统能够正常工作，这时候就需要数据在多台机器上做冗余，在遇到单机故障时其他机器就可以及时接管。
- 统一的用户体验，如果系统客户端分布在多个地域，通常考虑在多个地域部署服务，以方便用户能够就近访问到他们所需要的数据，获得统一的用户体验。

数据的多机分布的方式主要有两种，一种是将数据分片保存，每个机器保存数据的部分分片（Kafka中称为Partition，其他部分系统称为Shard），另一种则是完全的冗余，其中每一份数据叫做一个副本（Kafka中称为Replica），通过数据复制技术实现。