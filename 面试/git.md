引言
网上有太多讲rebase和merge的文章，但大多都是复制粘贴没有自己的理解，而且很多博客的例子写的过于复杂，让人没兴趣看下去。根据奥卡姆剃刀原则，本文举最简单例子，大白话几句就让你快速掌握rebase的核心原理和用法。

本博客将持续修订更新，看完如果还是有疑问，可以评论区留言，我解释到你彻底搞懂为止！
最新更新：2023.7.29

一、提交节点图解
首先通过简单的提交节点图解感受一下rebase在干什么

构造两个分支master和feature，其中feature是在提交点B处从master上拉出的分支

master上有一个新提交M，feature上有两个新提交C和D

![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/11b5dfb6fcb1b5e2e3d68d726b7a07bb.png)

此时我们切换到feature分支上，执行rebase命令，相当于是想要把master分支合并到feature分支（这一步的场景就可以类比为我们在自己的分支feature上开发了一段时间了，准备从主干master上拉一下最新改动。模拟了git pull --rebase的情形）

# 这两条命令等价于git rebase master feature
git checkout feature
git rebase master
1
2
3
下图为变基后的提交节点图，解释一下其工作原理：

![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/0e5ced3de53e575c3af477e2dd8a0ce6.png)


feature：待变基分支、当前分支
master：基分支、目标分支
官方原文解释（如果觉得看不懂可以直接看下一段）：当执行rebase操作时，git会从两个分支的共同祖先开始提取待变基分支上的修改，然后将待变基分支指向基分支的最新提交，最后将刚才提取的修改应用到基分支的最新提交的后面。

结合例子解释：当在feature分支上执行git rebase master时，git会从master和featuer的共同祖先B开始提取feature分支上的修改，也就是C和D两个提交，先提取到。然后将feature分支指向master分支的最新提交上，也就是M。最后把提取的C和D接到M后面，注意这里的接法，官方没说清楚，实际是会依次拿M和C、D内容分别比较，处理冲突后生成新的C’和D’。一定注意，这里新C’、D’和之前的C、D已经不一样了，是我们处理冲突后的新内容，feature指针自然最后也是指向D’

通俗解释（重要！！）：rebase，变基，可以直接理解为改变基底。feature分支是基于master分支的B拉出来的分支，feature的基底是B。而master在B之后有新的提交，就相当于此时要用master上新的提交来作为feature分支的新基底。实际操作为把B之后feature的提交先暂存下来，然后删掉原来这些提交，再找到master的最新提交位置，把存下来的提交再接上去（接上去是逐个和新基底处理冲突的过程），如此feature分支的基底就相当于变成了M而不是原来的B了。（注意，如果master上在B以后没有新提交，那么就还是用原来的B作为基，rebase操作相当于无效，此时和git merge就基本没区别了，差异只在于git merge会多一条记录Merge操作的提交记录）

上面的例子可抽象为如下实际工作场景：远程库上有一个master分支目前开发到B了，张三从B拉了代码到本地的feature分支进行开发，目前提交了两次，开发到D了；李四也从B拉到本地的master分支，他提交到了M，然后合到远程库的master上了。此时张三想从远程库master拉下最新代码，于是他在feature分支上执行了git pull origin master:feature --rebase（注意要加–rebase参数），即把远程库master分支给rebase下来，由于李四更早开发完，此时远程master上是李四的最新内容，rebase后再看张三的历史提交记录，就相当于是张三是基于李四的最新提交M进行的开发了。（但实际上张三更早拉代码下来，李四拉的晚但提交早）

二、实际git提交示例
我这里严格按照上面的图解，构造了实际的git提交示例（注意，这里重新贴了新图，之前的图逻辑上还是不够清晰）

如下图所示，ABM是master分支线，ABCD是feature分支线。

![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/397963a2cb7abffe0092791f49552f97.png)

此时，在feature分支上执行git rebase master后，会提示有冲突，这里是关键，之前没有把这个细节说清楚。冲突其实也简单，因为我们要生成新的C’和D’嘛，那C’的内容如何得到呢？照搬C的？当然不是，C’的内容就是C和M两个节点的内容合并的结果，D’的内容就是D和M两个节点的内容合并的结果。我们手动处理冲突后，执行如下命令即可：

# 先处理完C，会继续报D的冲突，所以下面命令一共会执行两次
git add file
git rebase --continue
1
2
3
变基完成以后如下图所示，ABM还是没变化，ABMC’D’是rebase完成后的feature节点图，私以为讲到这里就还是比较清楚了

![在这里插入图片描述](https://i-blog.csdnimg.cn/blog_migrate/c3fd2ca8b71df11f7031de4c07dc8be6.png)


三、推荐使用场景
搞来搞去那么多，这其实是最重要的。不同公司，不同情况有不同使用场景，不过大部分情况推荐如下：

拉公共分支最新代码——rebase，也就是git pull -r或git pull --rebase。这样的好处很明显，提交记录会比较简洁。但有个缺点就是rebase以后我就不知道我的当前分支最早是从哪个分支拉出来的了，因为基底变了嘛，所以看个人需求了。总体来说，即使是单机也不建议使用。
往公共分支上合代码——merge。如果使用rebase，那么其他开发人员想看主分支的历史，就不是原来的历史了，历史已经被你篡改了。举个例子解释下，比如张三和李四从共同的节点拉出来开发，张三先开发完提交了两次然后merge上去了，李四后来开发完如果rebase上去（注意，李四需要切换到自己本地的主分支，假设先pull了张三的最新改动下来，然后执行<git rebase 李四的开发分支>，然后再git push到远端），则李四的新提交变成了张三的新提交的新基底，本来李四的提交是最新的，结果最新的提交显示反而是张三的，就乱套了，以后有问题就不好追溯了。
正因如此，大部分公司其实会禁用rebase，不管是拉代码还是push代码统一都使用merge，虽然会多出无意义的一条提交记录“Merge … to …”，但至少能清楚地知道主线上谁合了的代码以及他们合代码的时间先后顺序