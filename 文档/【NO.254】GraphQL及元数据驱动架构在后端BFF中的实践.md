# 【NO.254】GraphQL及元数据驱动架构在后端BFF中的实践

## 1 BFF的由来

BFF一词来自Sam Newman的一篇博文《[Pattern:Backends For Frontends](https://samnewman.io/patterns/architectural/bff/)》，指的是服务于前端的后端。BFF是解决什么问题的呢？据原文描述，随着移动互联网的兴起，原适应于桌面Web的服务端功能希望同时提供给移动App使用，而在这个过程中存在这样的问题：

- 移动App和桌面Web在UI部分存在差异。
- 移动App涉及不同的端，不仅有iOS、还有Android，这些不同端的UI之间存在差异。
- 原有后端功能和桌面Web UI之间已经存在了较大的耦合。

因为端的差异性存在，服务端的功能要针对端的差异进行适配和裁剪，而服务端的业务功能本身是相对单一的，这就产生了一个矛盾——服务端的单一业务功能和端的差异性诉求之间的矛盾。那么这个问题怎么解决呢？这也是文章的副标题所描述的”Single-purpose Edge Services for UIs and external parties”，引入BFF，由BFF来针对多端差异做适配，这也是目前业界广泛使用的一种模式。

![图1 BFF示意图](https://p0.meituan.net/travelcube/4908c43bad40c3ad4cb8d497a611b672200549.png)

图1 BFF示意图



在实际业务的实践中，导致这种端差异性的原因有很多，有技术的原因，也有业务的原因。比如，用户的客户端是Android还是iOS，是大屏还是小屏，是什么版本。再比如，业务属于哪个行业，产品形态是什么，功能投放在什么场景，面向的用户群体是谁等等。这些因素都会带来面向端的功能逻辑的差异性。

在这个问题上，笔者所在团队负责的商品展示业务有一定的发言权，同样的商品业务，在C端的展示功能逻辑，深刻受到商品类型、所在行业、交易形态、投放场所、面向群体等因素的影响。同时，面向消费者端的功能频繁迭代的属性，更是加剧并深化了这种矛盾，使其演化成了一种服务端单一稳定与端的差异灵活之间的矛盾，这也是商品展示（商品展示BFF）业务系统存在的必然性原因。本文主要在美团到店商品展示场景的背景下，介绍面临的一些问题及解决思路。

## 2 BFF背景下的核心矛盾

BFF这层的引入是解决服务端单一稳定与端的差异灵活诉求之间的矛盾，这个矛盾并不是不存在，而是转移了。由原来后端和前端之间的矛盾转移成了BFF和前端之间的矛盾。笔者所在团队的主要工作，就是和这种矛盾作斗争。下面以具体的业务场景为例，结合当前的业务特点，说明在BFF的生产模式下，我们所面临的具体问题。下图是两个不同行业的团购货架展示模块，这两个模块我们认为是两个商品的展示场景，它们是两套独立定义的产品逻辑，并且会各自迭代。

![图2 展示场景](https://p0.meituan.net/travelcube/c88cff6db3d2156f4659781c4ff88841203070.png)

图2 展示场景



在业务发展初期，这样的场景不多。BFF层系统“烟囱式”建设，功能快速开发上线满足业务的诉求，在这样的情况下，这种矛盾表现的不明显。而随着业务发展，行业的开拓，形成了许许多多这样的商品展示功能，矛盾逐渐加剧，主要表现在以下两个方面：

- 业务支撑效率：随着商品展示场景变得越来越多，API呈爆炸趋势，业务支撑效率和人力成线性关系，系统能力难以支撑业务场景的规模化拓展。
- 系统复杂度高：核心功能持续迭代，内部逻辑充斥着`if…else…`，代码过程式编写，系统复杂度较高，难以修改和维护。

那么这些问题是怎么产生的呢？这要结合“烟囱式”系统建设的背景和商品展示场景所面临的业务，以及系统特点来进行理解。

**特点一：外部依赖多、场景间取数存在差异、用户体验要求高**

图例展示了两个不同行业的团购货架模块，这样一个看似不大的模块，后端在BFF层要调用20个以上的下游服务才能把数据拿全，这是其一。在上面两个不同的场景中，需要的数据源集合存在差异，而且这种差异普遍存在，这是其二，比如足疗团购货架需要的某个数据源，在丽人团购货架上不需要，丽人团购货架需要的某个数据源，足疗团购货架不需要。尽管依赖下游服务多，同时还要保证C端的用户体验，这是其三。

这几个特点给技术带来了不小的难题：1）聚合大小难控制，聚合功能是分场景建设？还是统一建设？如果分场景建设，必然存在不同场景重复编写类似聚合逻辑的问题。如果统一建设，那么一个大而全的数据聚合中必然会存在无效的调用。2）聚合逻辑的复杂性控制问题，在这么多的数据源的情况下，不仅要考虑业务逻辑怎么写，还要考虑异步调用的编排，在代码复杂度未能良好控制的情况下，后续聚合的变更修改将会是一个难题。

**特点二：展示逻辑多、场景之间存在差异，共性个性逻辑耦合**

我们可以明显地识别某一类场景的逻辑是存在共性的，比如团单相关的展示场景。直观可以看出基本上都是展示团单维度的信息，但这只是表象。实际上在模块的生成过程中存在诸多的差异，比如以下两种差异：

- 字段拼接逻辑差异：比如以上图中两个团购货架的团购标题为例，同样是标题，在丽人团购货架中的展示规则是：**[类型] + 团购标题**，而在足疗团购货架的展示规则是：**团购标题**。
- 排序过滤逻辑差异：比如同样是团单列表，A场景按照销量倒排序，B场景按照价格排序，不同场景的排序逻辑不同。

诸如此类的**展示逻辑**的差异性还有很多。类似的场景实际上在内部存在很多差异的逻辑，后端如何应对这种差异性是一个难题，下面是最常见的一种写法，通过读取具体的条件字段来做判断实现逻辑路由，如下所示：

```java
if(category == "丽人") {
  title = "[" + category + "]" + productTitle;
} else if (category == "足疗") {
  title = productTitle；
}
```

这种方案在功能实现方面没有问题，也能够复用共同的逻辑。但是实际上在场景非常多的情况下，将会有非常多的差异性判断逻辑叠加在一起，功能一直会被持续迭代的情况下，可以想象，系统将会变得越来越复杂，越来越难以修改和维护。

**总结**：在BFF这层，不同商品展示场景存在差异。在业务发展初期，系统通过独立建设的方式支持业务快速试错，在这种情况下，业务差异性带来的问题不明显。而随着业务的不断发展，需要搭建及运营的场景越来越多，呈规模化趋势。此时，业务对技术效率提出了更高的要求。**在这种场景多、场景间存在差异的背景下，如何满足场景拓展效率同时能够控制系统的复杂性，就是我们业务场景中面临的核心问题**。

## 3 BFF应用模式分析

目前业界针对此类的解决方案主要有两种模式，一种是后端BFF模式，另一种是前端BFF模式。

### 3.1 后端BFF模式

后端BFF模式指的是BFF由后端同学负责，这种模式目前最广泛的实践是基于GraphQL搭建的后端BFF方案，具体是：后端将展示字段封装成展示服务，通过GraphQL编排之后暴露给前端使用。如下图所示：

![图3 后端BFF模式](https://p0.meituan.net/travelcube/8af9b3d43145aeb0c7733d9fc5c13270193362.png)

图3 后端BFF模式



这种模式最大的特性和优势是，当展示字段已经存在的情况下，后端不需要关心前端差异性需求，按需查询的能力由GraphQL支持。这个特性可以很好地应对不同场景存在展示字段差异性这个问题，前端直接基于GraphQL按需查询数据即可，后端不需要变更。同时，借助GraphQL的编排和聚合查询能力，后端可以将逻辑分解在不同的展示服务中，因此在一定程度上能够化解BFF这层的复杂性。

但是基于这种模式，仍然存在几个问题：展示服务颗粒度问题、数据图划分问题以及字段扩散问题，下图是基于当前模式的具体案例：

![图4 后端BFF模式（案例）](https://p0.meituan.net/travelcube/c5f78c0063a14d902c45ef7c5742c47b78108.png)

图4 后端BFF模式（案例）



**1）展示服务颗粒度设计问题**

这种方案要求展示逻辑和取数逻辑封装在一个模块中，形成一个展示服务（Presentation Service），如上图所示。而实际上展示逻辑和取数逻辑是多对多的关系，还是以前文提到的例子说明：

> **背景**：有两个展示服务，分别封装了商品标题和商品标签的查询能力。 **情景**：此时PM提了一个需求，希望商品在某个场景的标题以“[类型]+商品标题”的形式展示，此时商品标题的拼接依赖类型数据，而此时类型数据商品标签展示服务中已经调用了。 **问题**：商品标题展示服务自己调用类型数据还是将两个展示服务合并到一起？

以上描述的问题的是展示服务颗粒度把控的问题，我们可以怀疑上述的示例是不是因为展示服务的颗粒度过小？那么反过来看一看，如果将两个服务合并到一起，那么势必又会存在冗余。这是展示服务设计的难点，**核心原因在于，展示逻辑和取数逻辑本身是多对多的关系，结果却被设计放在了一起**。

**2）数据图划分问题**

通过GraphQL将多个展示服务的数据聚合到一张图（GraphQL Schema）中，形成一个数据视图，需要数据的时候只要数据在图中，就可以基于Query按需查询。那么问题来了，这个图应该怎么组织？是一张图还是多张图？图过大的话，势必带来复杂的数据关系维护问题，图过小则将会降低方案本身的价值。

**3）展示服务内部复杂性 + 模型扩散问题**

上文提到过一个商品标题的展示存在不同拼接逻辑的情况，在商品展示场景，这种逻辑特别普遍。比如同样是价格，A行业展示优惠后价格，B行业展示优惠前价格；同样是标签位置，C行业展示服务时长，而D行业展示商品特性等。那么问题来了，展示模型如何设计？以标题字段为例，是在展示模型上放个`title`字段就可以，还是分别放个`title`和`titleWithCategory`？如果是前者那么服务内部必然会存在`if…else…`这种逻辑，用于区分`title`的拼接方式，这同样会导致展示服务内部的复杂性。如果是多个字段，那么可以想象，展示服务的模型字段也将会不断扩散。

**总结**：后端BFF模式能够在一定程度上化解后端逻辑的复杂性，同时提供一个展示字段的复用机制。但是仍然存在未决问题，如展示服务的颗粒度设计问题，数据图的划分问题，以及展示服务内部的复杂性和字段扩散问题。目前这种模式实践的代表有Facebook、爱彼迎、eBay、爱奇艺、携程、去哪儿等等。

### 3.2 前端BFF模式

前端BFF模式在Sam Newman的文章中的”And Autonomy”部分有特别的介绍，指的是BFF本身由前端团队自己负责，如下示意图所示：

![图5 前端BFF模式](https://p0.meituan.net/travelcube/0e21d87858dbe63f025e043e80e4157a355488.png)

图5 前端BFF模式



这种模式的理念是，本来能一个团队交付的需求，没必要拆成两个团队，两个团队本身带来较大的沟通协作成本。本质上，也是一种将“敌我矛盾”转化为“人民内部矛盾”的思路。前端完全接手BFF的开发工作，实现数据查询的自给自足，大大减少了前后端的协作成本。但是这种模式没有提到我们关心的一些核心问题，如：复杂性如何应对、差异性如何应对、展示模型如何设计等等问题。除此之外，这种模式也存在一些前提条件及弊端，比如较为完备的前端基础设施；前端不仅仅需要关心渲染、还需要了解业务逻辑等。

**总结**：前端BFF模式通过前端自主查询和使用数据，从而达到降低跨团队协作的成本，提升BFF研发效率的效果。目前这种模式的实践代表是阿里巴巴。

## 4 基于GraphQL及元数据的信息聚合架构设计

### 4.1 整体思路

通过对后端BFF和前端BFF两种模式的分析，我们最终选择后端BFF模式，前端BFF这个方案对目前的研发模式影响较大，不仅需要大量的前端资源，而且需要建设完善的前端基础设施，方案实施成本比较高昂。

前文提到的后端GraphQL BFF模式代入我们的具体场景虽然存在一些问题，但是总体有非常大的参考价值，比如展示字段的复用思路、数据的按需查询思路等等。在商品展示场景中，**有80%的工作集中在数据的聚合和集成部分**，并且这部分具有很强的复用价值，因此信息的查询和聚合是我们面临的主要矛盾。因此，我们的思路是：**基于GraphQL+后端BFF方案改进，实现取数逻辑和展示逻辑的可沉淀、可组合、可复用**，整体架构如下示意图所示：

![图6 基于GraphQL BFF的改进思路](https://p1.meituan.net/travelcube/89256d10f2541152542a61708bd4d22492513.png)

图6 基于GraphQL BFF的改进思路



从上图可看出，与传统GraphQL BFF方案最大的差别在于我们将GraphQL下放至数据聚合部分，由于数据来源于商品领域，领域是相对稳定的，因此数据图规模可控且相对稳定。除此之外，整体架构的核心设计还包括以下三个方面：1）取数展示分离；2）查询模型归一；3）元数据驱动架构。

我们通过取数展示分离解决展示服务颗粒度问题，同时使得展示逻辑和取数逻辑可沉淀、可复用；通过查询模型归一化设计解决展示字段扩散的问题；通过元数据驱动架构实现能力的可视化，业务组件编排执行的自动化，这能够让业务开发同学聚焦于业务逻辑的本身。下面将针对这三个部分逐一展开介绍。

### 4.2 核心设计

#### 4.2.1 取数展示分离

上文提到，在商品展示场景中，展示逻辑和取数逻辑是多对多的关系，而传统的基于GraphQL的后端BFF实践方案把它们封装在一起，这是导致展示服务颗粒度难以设计的根本原因。思考一下取数逻辑和展示逻辑的关注点是什么？取数逻辑关注怎么查询和聚合数据，而展示逻辑关注怎么加工生成需要的展示字段，它们的关注点不一样，放在一起也会增加展示服务的复杂性。因此，我们的思路是将取数逻辑和展示逻辑分离开来，单独封装成逻辑单元，分别叫取数单元和展示单元。在取数展示分离之后，GraphQL也随之下沉，用于实现数据的按需聚合，如下图所示：

![图7 取数展示分离+元数据描述](https://p1.meituan.net/travelcube/7a69bcd33c7aac1aa401f9724c78a8f184459.png)

图7 取数展示分离+元数据描述



那么取数和展示逻辑的封装颗粒度是怎么样的呢？不能太小也不能太大，在颗粒度的设计上，我们有两个核心考量：1）**复用**，展示逻辑和取数逻辑在商品展示场景中，都是可以被复用的资产，我们希望它们能沉淀下来，被单独按需使用；2）**简单**，保持简单，这样容易修改和维护。基于这两点考虑，颗粒度的定义如下：

- **取数单元**：尽量只封装1个外部数据源，同时负责对外部数据源返回的模型进行简化，这部分生成的模型我们称之为取数模型。
- **展示单元**：尽量只封装1个展示字段的加工逻辑。

分开的好处是简单且可被组合使用，那么具体如何实现组合使用呢？我们的思路是通过元数据来描述它们之间的关系，基于元数据由统一的执行框架来关联运行，具体设计下文会展开介绍。通过取数和展示的分离，元数据的关联和运行时的组合调用，可以保持逻辑单元的简单，同时又满足复用诉求，这也很好地解决了传统方案中存在的**展示服务的颗粒度问题**。

#### 4.2.2 查询模型归一

展示单元的加工结果通过什么样的接口透出呢？接下来，我们介绍一下查询接口设计的问题。

**1）查询接口设计的难点**

常见查询接口的设计模式有以下两种：

- **强类型模式**：强类型模式指的是查询接口返回的是POJO对象，每一个查询结果对应POJO中的一个明确的具有特定业务含义的字段。
- **弱类型模式**：弱类型模式指的是查询结果以K-V或JSON模式返回，没有明确的静态字段。

以上两种模式在业界都有广泛应用，且它们都有明确的优缺点。强类型模式对开发者友好，但是业务是不断迭代的，与此同时，系统沉淀的展示单元会不断丰富，在这样的情况下，接口返回的DTO中的字段将会愈来愈多，每次新功能的支持，都要伴随着接口查询模型的修改，JAR版本的升级。而JAR的升级涉及数据提供方和数据消费两方，存在明显效率问题。另外，可以想象，查询模型的不断迭代，最终将会包括成百上千个字段，难以维护。

而弱类型模式恰好可以弥补这一缺点，但是弱类型模式对于开发者来说非常不友好，接口查询模型中有哪些查询结果对于开发者来说在开发的过程中完全没有感觉，但是程序员的天性就是喜欢通过代码去理解逻辑，而非配置和文档。其实，这两种接口设计模式都存在着一个共性问题——缺少抽象，下面两节，我们将介绍在接口返回的查询模型设计方面的抽象思路及框架能力支持。

**2）查询模型归一化设计**

回到商品展示场景中，一个展示字段有多种不同的实现，如商品标题的两种不同实现方式：1）商品标题；2）[类目]+商品标题。商品标题和这两种展示逻辑的关系本质上是一种抽象-具体的关系。识别这个关键点，思路就明了了，我们的思路是对查询模型做抽象。查询模型上都是抽象的展示字段，一个展示字段对应多个展示单元，如下图所示：

![图8 查询模型归一化 + 元数据描述](https://p0.meituan.net/travelcube/e4aae6b37be45f2de690e1129cecf02154846.png)

图8 查询模型归一化 + 元数据描述



在实现层面同样基于元数据描述展示字段和展示单元之间的关系，基于以上的设计思路，可以在一定程度上减缓模型的扩散，但是还不能避免扩展。比如除了价格、库存、销量等每个商品都有的标准属性之外，不同的商品类型一般还会有这个商品特有的属性。比如密室主题拼场商品才有“几人拼”这样的描述属性，这种字段本身抽象的意义不大，且放在商品查询模型中作为一个单独的字段会导致模型扩张，针对这类问题，我们的解决思路是引入扩展属性，扩展属性专门承载这类非标准的字段。通过标准字段 + 扩展属性的方式建立查询模型，能够较好地解决**字段扩散**的问题。

#### 4.2.3 元数据驱动架构

到目前为止，我们定义了如何分解**业务逻辑单元**以及如何设计**查询模型**，并提到用元数据描述它们之间的关系。基于以上定义实现的业务逻辑及模型，都具备很强的复用价值，可以作为业务资产沉淀下来。那么，为什么用元数据描述业务功能及模型之间的关系呢？

我们引入元数据描述主要有两个目的：1）代码逻辑的自动编排，通过元数据描述业务逻辑之间的关联关系，运行时可以自动基于元数据实现逻辑之间的关联执行，从而可以消除大量的人工逻辑编排代码；2）业务功能的可视化，元数据本身描述了业务逻辑所提供的功能，如下面两个示例：

> 团单基础售价字符串展示，例：30元。 团单市场价展示字段，例：100元。

这些元数据上报到系统中，可以用于展示当前系统所提供的功能。通过元数据描述组件及组件之间关联关系，通过框架解析元数据自动进行业务组件的调用执行，形成了如下的元数据架构：

![图9 元数据驱动架构](https://p0.meituan.net/travelcube/3b60a65d11a60aa6533d8e28c5589f5b111490.png)

图9 元数据驱动架构



整体架构由三个核心部分组成：

- 业务能力：标准的业务逻辑单元，包括取数单元、展示单元和查询模型，这些都是关键的可复用资产。
- 元数据：描述业务功能（如：展示单元、取数单元）以及业务功能之间的关联关系，比如展示单元依赖的数据，展示单元映射的展示字段等。
- 执行引擎：负责消费元数据，并基于元数据对业务逻辑进行调度和执行。

通过以上三个部分有机的组合在一起，形成了一个元数据驱动风格的架构。

## 5 针对GraphQL的优化实践

### 5.1 使用简化

**1）GraphQL直接使用问题**

引入GraphQL，会引入一些额外的复杂性，比如会涉及到GraphQL带来的一些概念如：Schema、RuntimeWiring，下面是基于GraphQL原生Java框架的开发过程：

![图10 原生GraphQL使用流程](https://p0.meituan.net/travelcube/cc567229c6fc727f1ec23abf2e39981f28129.png)

图10 原生GraphQL使用流程



这些概念对于未接触过GraphQL的同学来说，增加了学习和理解的成本，而这些概念和业务领域通常没有什么关系。而我们仅仅希望使用GraphQL的按需查询特性，却被GraphQL本身拖累了，业务开发同学的关注点应该聚焦在业务逻辑本身才对，这个问题如何解决呢？

著名计算机科学家David Wheeler说了一句名言，”All problems in computer science can be solved by another level of indirection”。没有加一层解决不了的问题，本质上是需要有人来对这事负责，因此我们在原生GraphQL之上增加了一层执行引擎层来解决这些问题，目标是屏蔽GraphQL的复杂性，让开发人员只需要关注业务逻辑。

**2）取数接口标准化**

首先要简化数据的接入，原生的`DataFetcher`和`DataLoader`都是处在一个比较高的抽象层次，缺少业务语义，而在查询场景，我们能够归纳出，所有的查询都属于以下三种模式：

- **1查1**：根据一个条件查询一个结果。
- **1查N**：根据一个条件查询多个结果。
- **N查N**：一查一或一查多的批量版本。

由此，我们对查询接口进行了标准化，业务开发同学基于场景判断是那种，按需选择使用即可，取数接口标准化设计如下：

![图11 查询接口标准化](https://p0.meituan.net/travelcube/d88993d39150ee563853e9167b940b4967781.png)

图11 查询接口标准化



业务开发同学按需选择所需要使用的取数器，通过泛型指定结果类型，1查1和1查N比较简单，N查N我们对其定义为批量查询接口，用于满足”N+1”的场景，其中`batchSize`字段用于指定分片大小，`batchKey`用于指定查询Key，业务开发只需要指定参数，其他的框架会自动处理。除此之外，我们还约束了返回结果必须是`CompleteFuture`，用于满足聚合查询的全链路异步化。

**3）聚合编排自动化**

取数接口标准化使得数据源的语义更清晰，开发过程按需选择即可，简化了业务的开发。但是此时业务开发同学写好`Fetcher`之后，还需要去另一个地方去写`Schema`，而且写完`Schema`还要再写`Schema`和`Fetcher`的映射关系，业务开发更享受写代码的过程，不太愿意写完代码还要去另外一个地方取配置，并且同时维护代码和对应配置也提高了出错的可能性，能否将这些冗杂的步骤移除掉？

`Schema`和`RuntimeWiring`本质上是想描述某些信息，如果这些信息换一种方式描述是不是也可以，我们的优化思路是：在业务开发过程中标记注解，通过注解标注的元数据描述这些信息，其他的事情交给框架来做。解决思路示意图如下：

![图12 注解元数据描述Schema和RuntimeWiring](https://p0.meituan.net/travelcube/9774e011766c681380eb469fa2a379ef385680.png)

图12 注解元数据描述Schema和RuntimeWiring



### 5.2 性能优化

#### 5.2.1 GraphQL性能问题

虽然GraphQL已经开源了，但是Facebook只开源了相关标准，并没有给出解决方案。GraphQL-Java框架是由社区贡献的，基于开源的GraphQL-Java作为按需查询引擎的方案，我们发现了GraphQL应用方面的一些问题，这些问题有部分是由于使用姿势不当所导致的，也有部分是GraphQL本身实现的问题，比如我们遇到的几个典型的问题：

- 耗CPU的查询解析，包括`Schema`的解析和`Query`的解析。
- 当查询模型比较复杂特别是存在大列表时候的延时问题。
- 基于反射的模型转换CPU消耗问题。
- `DataLoader`的层级调度问题。

于是，我们对使用方式和框架做了一些优化与改造，以解决上面列举的问题。本章着重介绍我们在GraphQL-Java方面的优化和改造思路。

#### 5.2.2 GraphQL编译优化

**1）GraphQL语言原理概述**

GraphQL是一种查询语言，目的是基于直观和灵活的语法构建客户端应用程序，用于描述其数据需求和交互。GraphQL属于一种领域特定语言（DSL），而我们所使用的GraphQL-Java客户端在语言编译层面是基于ANTLR 4实现的，ANTLR 4是一种基于Java编写的语言定义和识别工具，ANTLR是一种元语言（Meta-Language），它们的关系如下：

![图13 GraphQL语言基本原理示意图](https://p0.meituan.net/travelcube/c73d451b792545e1dffbef006095b9b174686.png)

图13 GraphQL语言基本原理示意图



GraphQL执行引擎所接受的`Schema`及`Query`都是基于GraphQL定义的语言所表达的内容，GraphQL执行引擎不能直接理解GraphQL，在执行之前必须由GraphQL编译器翻译成GraphQL执行引擎可理解的文档对象。而GraphQL编译器是基于Java的，经验表明在大流量场景实时解释的情况下，这部分代码将会成为CPU热点，而且还占用响应延迟，`Schema`或`Query`越复杂，性能损耗越明显。

**2）Schema及Query编译缓存**

`Schema`表达的是数据视图和取数模型同构，相对稳定，个数也不多，在我们的业务场景一个服务也就一个。因此，我们的做法是在启动的时候就将基于`Schema`构造的GraphQL执行引擎构造好，作为单例缓存下来，对于`Query`来说，每个场景的`Query`有些差异，因此`Query`的解析结果不能作为单例，我们的做法是实现`PreparsedDocumentProvider`接口，基于`Query`作为Key将`Query`编译结果缓存下来。如下图所示：

![图14 Query缓存实现示意图](https://p1.meituan.net/travelcube/81b1c7b29e8d7eb5cd2bca3b5fdbed9c77342.png)

图14 Query缓存实现示意图



#### 5.2.3 GraphQL执行引擎优化

**1）GraphQL执行机制及问题**

我们先一起了解一下GraphQL-Java执行引擎的运行机制是怎么样的。假设在执行策略上我们选取的是`AsyncExecutionStrategy`，来看看GraphQL执行引擎的执行过程：

![图15 GraphQL执行引擎执行过程](https://p1.meituan.net/travelcube/fb8708d3fe7e68cc7d8a6fa4f4e2da8f173718.png)

图15 GraphQL执行引擎执行过程



以上时序图做了些简化，去除了一些与重点无关的信息，`AsyncExecutionStrategy`的`execute`方法是对象执行策略的异步化模式实现，是查询执行的起点，也是根节点查询的入口，`AsyncExecutionStrategy`对对象的多个字段的查询逻辑，采取的是循环+异步化的实现方式，我们从`AsyncExecutionStrategy`的`execute`方法触发，理解GraphQL查询过程如下：

1. 调用当前字段所绑定的`DataFetcher`的`get`方法，如果字段没有绑定`DataFetcher`，则通过默认的`PropertyDataFetcher`查询字段，`PropertyDataFetcher`的实现是基于反射从源对象中读取查询字段。

2. 将从`DataFetcher`查询得到结果包装成`CompletableFuture`，如果结果本身是`CompletableFuture`，那么不会包装。

3. 结果

   ```
   CompletableFuture
   ```

   完成之后，调用

   ```
   completeValue
   ```

   ，基于结果类型分别处理。

   - 如果查询结果是列表类型，那么会对列表类型进行遍历，针对每个元素在递归执行`completeValue`。
   - 如果结果类型是对象类型，那么会对对象执行`execute`，又回到了起点，也就是`AsyncExecutionStrategy的execute`。

以上是GraphQL的执行过程，这个过程有什么问题呢？下面基于图上的标记顺序一起看看GraphQL在我们的业务场景中应用和实践所遇到的问题，这些问题不代表在其他场景也是问题，仅供参考：

**问题1**：`PropertyDataFetcher`CPU热点问题，`PropertyDataFetcher`在整个查询过程中属于热点代码，而其本身的实现也有一些优化空间，在运行时`PropertyDataFetcher`的执行会成为CPU热点。（具体问题可参考GitHub上的commit和Conversion：https://github.com/graphql-java/graphql-java/pull/1815）

![图16 PropertyDataFetcher成为CPU热点](https://p1.meituan.net/travelcube/a144670328dd9116740c09f3f250c8561865320.png)

图16 PropertyDataFetcher成为CPU热点



**问题2**：列表的计算耗时问题，列表计算是循环的，对于查询结果中存在大列表的场景，此时循环会造成整体查询明显的延迟。我们举个具体的例子，假设查询结果中存在一个列表大小是1000，每个元素的处理是0.01ms，那么总体耗时就是10ms，基于GraphQL的查机制，这个10ms会阻塞整个链路。

**2）类型转换优化**

通过GraphQL查询引擎拿到的GraphQL模型，和业务实现的`DataFetcher`返回的取数模型是同构，但是所有字段的类型都会被转换成GraphQL内部类型。`PropertyDataFetcher`之所以会成为CPU热点，问题就在于这个模型转换过程，业务定义的模型到GraphQL类型模型转换过程示意图如下图所示：

![图17 业务模型到GraphQL模型转换示意图](https://p0.meituan.net/travelcube/d9d8f4a804dc8db7786846719a964e8c93390.png)

图17 业务模型到GraphQL模型转换示意图



当查询结果模型中的字段非常多的时候，比如上万个，意味着每次查询有上万次的`PropertyDataFetcher`操作，实际就反映到了CPU热点问题上，这个问题我们的解决思路是保持原有业务模型不变，将非`PropertyDataFetcher`查询的结果反过来填充到业务模型上。如下示意图所示：

![图18 查询结果模型反向填充示意图](https://p0.meituan.net/travelcube/cc20f97f76e0ddc19ac77ef724f7468498880.png)

图18 查询结果模型反向填充示意图



基于这个思路，我们通过GraphQL执行引擎拿到的结果就是业务`Fetcher`返回的对象模型，这样不仅仅解决了因字段反射转换带来的CPU热点问题，同时对于业务开发来说增加了友好性。因为GraphQL模型类似JSON模型，这种模型是缺少业务类型的，业务开发直接使用起来非常麻烦。以上优化在一个场景上试点测试，结果显示该场景的平均响应时间缩短1.457ms，平均99线缩短5.82ms，平均CPU利用率降低约12%。

**3）列表计算优化**

当列表元素比较多的时候，默认的单线程遍历列表元素计算的方式所带来的延迟消耗非常明显，对于响应时间比较敏感的场景这个延迟优化很有必要。针对这个问题我们的解决思路是充分利用CPU多核心计算的能力，将列表拆分成任务，通过多线程并行执行，实现机制如下：

![图19 列表遍历多核计算思路](https://p0.meituan.net/travelcube/c48f4bfd93da1a8dfd88dd8819f579ab47249.png)

图19 列表遍历多核计算思路



#### 5.2.4 GraphQL-DataLoader调度优化

**1）DataLoader基本原理**

先简单介绍一下DataLoader的基本原理，DataLoader有两个方法，一个是`load`，一个是`dispatch`，在解决N+1问题的场景中，DataLoader是这么用的：

![图20 DataLoader基本原理](https://p0.meituan.net/travelcube/4e58a006c1ef930dd417062d93fab4cc31912.png)

图20 DataLoader基本原理



整体分为2个阶段，第一个阶段调用`load`，调用N次，第二个阶段调用`dispatch`，调用`dispatch`的时候会真正的执行数据查询，从而达到批量查询+分片的效果。

**2）DataLoader调度问题**

GraphQL-Java对DataLoader的集成支持的实现在`FieldLevelTrackingApproach`中，`FieldLevelTrackingApproach`的实现会存在怎样的问题呢？下面基于一张图表达原生DataLoader调度机制所产生的问题：

![图21 GraphQL-Java对DataLoader调度存在的问题](https://p0.meituan.net/travelcube/92a4db95c599e605d25d8f493a5862aa62026.png)

图21 GraphQL-Java对DataLoader调度存在的问题



问题很明显，基于`FieldLevelTrackingApproach`的实现，下一层级的`DataLoader`的`dispatch`是需要等到本层级的结果都回来之后才发出。基于这样的实现，查询总耗时的计算公式等于：TOTAL = MAX（Level 1 Latency）+ MAX（Level 2 Latency）+ MAX（Level 3 Latency）+ … ，**总查询耗时等于每层耗时最大的值加起来，而实际上如果链路编排由业务开发同学自己来写的话，理论上的效果是总耗时等于所有链路最长的那个链路所耗的时间**，这个才是合理的。而`FieldLevelTrackingApproach`的实现所表现出来的结果是反常识的，至于为什么这么实现，目前我们理解可能是设计者基于简单和通用方面的考虑。

问题在于以上的实现在有些业务场景下是不能接受的，比如我们的列表场景的响应时间约束一共也就不到100ms，其中几十ms是因为这个原因搭进去的。针对这个问题的解决思路，一种方式是对于响应时间要求特别高的场景独立编排，不采用GraphQL；另一种方式是在GraphQL层面解决这个问题，保持架构的统一性。接下来，介绍一下我们是如何扩展GraphQL-Java执行引擎来解决这个问题的。

**3）DataLoader调度优化**

针对DataLoader调度的性能问题，**我们的解决思路是在最后一次调用某个`DataLoader`的`load`之后，立即调用`dispatch`方法发出查询请求**，问题是我们怎么知道哪一次的load是最后一次load呢？这个问题也是解决DataLoader调度问题的难点，以下举个例子来解释我们的解决思路：

![图22 查询对象结果示意图](https://p1.meituan.net/travelcube/2bd14ce27c7084135d0e5aed505d6ed047007.png)

图22 查询对象结果示意图



假设我们查询到的模型结构如下：根节点是`Query`下的字段，字段名叫`subjects`，`subject`引用的是个列表，`subject`下有两个元素，都是`ModelA`的对象实例，`ModelA`有两个字段，`fieldA`和`fieldB`，`subjects[0]`的`fieldA`关联是`ModelB`的一个实例，`subjects[0]`的`fieldB`关联多个`ModelC`实例。

为了方便理解，我们定义一些概念，字段、字段实例、字段实例执行完、字段实例值大小、字段实例值对象执行大小、字段实例值对象执行完等等：

- **字段**：具有唯一路径，是静态的，和运行时对象大小没有关系，如：`subjects`和`subjects/fieldA`。
- **字段实例**：字段的实例，具有唯一路径，是动态的，跟运行时对象大小有关系，如：`subjects[0]/fieldA`和`subjects[1]/fieldA`是字段`subjects/fieldA`的实例。
- **字段实例执行完**：字段实例关联的对象实例都被GraphQL执行完了。
- **字段实例值大小**：字段实例引用对象实例的个数，如以上示例，`subjects[0]/fieldA`字段实例值大小是1，`subjects[0]/fieldB`字段实例值大小是3。

除了以上定义之外，我们的业务场景还满足以下条件：

- 只有1个根节点，且根节点是列表。
- `DataLoader`一定属于某个字段，某个字段下的`DataLoader`应该被执行次数等于其下的对象实例个数。

基于以上信息，我们可以得出以下问题分析：

- 在执行字段实例的时候，我们可以知道当前字段实例的大小，字段实例的大小等于字段关联`DataLoader`在当前实例下需要执行`load`的次数，因此在执行`load`之后，我们可以知道当前对象实例是否是其所在字段实例的最后一个对象。
- 一个对象的实例可能会挂在不同的字段实例下，所以仅当当前对象实例是其所在字段实例的最后一个对象实例的时候，不代表当前对象实例是所有对象实例中的最后一个，当且仅当对象实例所在节点实例是节点的最后一个实例的时候才成立。
- 我们可从字段实例大小推算字段实例的个数，比如我们知道`subjects`的大小是2，那么就知道`subjects`字段有两个字段实例`subjects[0]`和`subjects[1]`，也就知道字段`subjects/fieldA`有两个实例，`subjects[0]/fieldA`和`subjects[1]/fieldA`，因此我们从根节点可以往下推断出某个字段实例是否执行完。

通过以上分析，我们可以得出，一个对象执行完的条件是其所在的字段实例以及其所在的字段所有的父亲字段实例都执行完，且当前执行的对象实例是其所在字段实例的最后一个对象实例的时候。基于这个判断逻辑，我们的实现方案是在每次调用完`DataFetcher`的时候，判断是否需要发起`dispatch`，如果是则发起。另外，以上时机和条件存在漏发`dispatch`的问题，有个特殊情况，当当前对象实例不是最后一个，但是剩下的对象大小都为0的时候，那么就永远不会触发当前对象关联的`DataLoader`的`load`了，所以在对象大小为0的时候，需要额外再判断一次。

根据以上逻辑分析，我们实现了`DataLoader`调用链路的最优化，达到理论最优的效果。

## 6 新架构对研发模式的影响

生产力决定生产关系，元数据驱动信息聚合架构是展示场景搭建的核心生产力，而业务开发模式和过程是生产关系，因此也会随之改变。下面我们将会从开发模式和流程两个角度来介绍新架构对研发带来的影响。

### 6.1 聚焦业务的开发模式

新架构提供了一套基于业务抽象出的标准化代码分解约束。以前开发同学对系统的理解很可能就是“查一查服务，把数据粘在一起”，而现在，研发同学对于业务的理解及代码分解思路将会是一致的。比如展示单元代表的是展示逻辑，取数单元代表的是取数逻辑。同时，很多冗杂且容易出错的逻辑已经被框架屏蔽掉了，研发同学能够有更多的精力聚焦于业务逻辑本身，比如：业务数据的理解和封装，展示逻辑的理解和编写，以及查询模型的抽象和建设。如下示意图所示：

![图23 业务开发聚焦业务本身](https://p0.meituan.net/travelcube/2f52f274a01d8a1f653710609e4587c153662.png)

图23 业务开发聚焦业务本身



### 6.2 研发流程升级

新架构不仅仅影响了研发的代码编写，同时也影响着研发流程的改进，基于元数据架构实现的可视化及配置化能力，现有研发流程和之前研发流程相比有了明显的区别，如下图所示：

![图24 基于开发框架搭建展示场景前后研发流程对比](https://p0.meituan.net/travelcube/d0f2f87b8ba492aa36d753700d7aceda157241.jpg)

图24 基于开发框架搭建展示场景前后研发流程对比



以前是“一杆子捅到底”的开发模式，每个展示场景的搭建需要经历过从接口的沟通到API的开发整个过程，基于新架构之后，系统自动具备多层复用及可视化、配置化能力。

**情况一**：这是最好的情况，此时取数功能和展示功能都已经被沉淀下来，研发同学需要做的只是创建查询方案，基于运营平台按需选择需要的展示单元，拿着查询方案ID基于查询接口就可以查到需要的展示信息了，可视化、配置化界面如下示意图所示：

![图25 可视化及文案按需选用](https://p0.meituan.net/travelcube/e25868ee8b71dea4fecd7845ba689011173259.png)

图25 可视化及文案按需选用



**情况二**：此时可能没有展示功能，但是通过运营平台查看到，数据源已经接入过，那么也不难，只需要基于现有的数据源编写一段加工逻辑即可，这段加工逻辑是非常爽的一段纯逻辑的编写，数据源列表如下示意图所示：

![图26 数据源列表可视化](https://p0.meituan.net/travelcube/1cb7b12d5828ac8b6d3e0c3e9d3a7fa8122676.png)

图26 数据源列表可视化



**情况三**：最坏的情况是此时系统不能满足当前的查询能力，这种情况比较少见，因为后端服务是比较稳定的，那么也无需惊慌，只需要按照标准规范将数据源接入进来，然后编写加工逻辑片段即可，之后这些能力是可以被持续复用的。

## 7 总结

商品展示场景的复杂性体现在：场景多、依赖多、逻辑多，以及不同场景之间存在差异。在这样的背景下，如果是业务初期，怎么快怎么来，采用“烟囱式”个性化建设的方式不必有过多的质疑。但是随着业务的不断发展，功能的不断迭代，以及场景的规模化趋势，“烟囱式”个性化建设的弊端会慢慢凸显出来，包括代码复杂度高、缺少能力沉淀等问题。

本文以基于对美团到店商品展示场景所面临的核心矛盾分析，介绍了：

- 业界不同的BFF应用模式，以及不同模式的优势和缺点。
- 基于GraphQL BFF模式改进的元数据驱动的架构方案设计。
- 我们在GraphQL实践过程中遇到的问题及解决思路。
- 新架构对研发模式产生的影响呈现。

目前，笔者所在团队负责的核心商品展示场景都已迁入新架构，基于新的研发模式，我们实现了50%以上的展示逻辑复用以及1倍以上的效率提升，希望本文对大家能够有所帮助。

## 8 参考文献

- [1]https://samnewman.io/patterns/architectural/bff/
- [2] https://www.thoughtworks.com/cn/radar/techniques/graphql-for-server-side-resource-aggregation
- [3] [了解电商后台系统，看这篇就够了](http://www.woshipm.com/pd/3712746.html)
- [4][框架定义-百度百科](https://baike.baidu.com/item/框架/1212667?fr=aladdin)
- [5] [高效研发-闲鱼在数据聚合上的探索与实践](https://mp.weixin.qq.com/s?__biz=MzU4MDUxOTI5NA==&mid=2247483947&idx=1&sn=d1b600e57cca8c7304cca77a693c0b32&chksm=fd54d63aca235f2c5abf8f65b3a0f235b55723d014aa4c7c75c48fcb5ce5c32b14fb43e5b40b&mpshare=1&scene=1&srcid=0404dHE42TNCAGO7clq9K2ep#rd)
- [6] 《系统架构-复杂系统的产品设计与开发》

## 9 招聘信息

美团到店综合研发中心长期招聘前端、后端、数据仓库、机器学习/数据挖掘算法工程师，坐标上海，欢迎感兴趣的同学发送简历至：tech@meituan.com（邮件标题注明：美团到店综合研发中心—上海）。

原文作者：美团技术团队

原文链接：https://tech.meituan.com/2021/05/06/bff-graphql.html