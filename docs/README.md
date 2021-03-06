![WonderTrader2.png](http://q92ex9g0c.bkt.clouddn.com/wt/logo_qcode_ad.jpg)

#### *What* - 什么是*WonderTrader*

* * *

* *WonderTrader*要解决的问题
    *WonderTrader*是一个针对CTA组合盘管理的开源量化交易框架，是**组合盘管理的利器**！
    *WonderTrader*旨在提供一套相对通用的技术框架，解耦量化交易技术框架中的策略和技术，从而让策略研发人员专注策略本身的逻辑，让软件开发人员专注技术框架的性能和稳定性，从而实现一个开放的、高可用的、高度可定制的量化策略技术框架。

* *WonderTrader*的设计思路
    从周边来说，*WonderTrader*本着**专业的人做专业的事**的基本原则，从一开始就考虑到很多策略研发人员的编程水平有限的实际情况，而采用C++作为底层核心框架，然后用C接口导出，从而给其他语言的调用提供了便利。
    从核心来说，要达到开放、高可用、高度可定制的设计目标，核心的设计遵循**核心逻辑+功能模块**的原则进行解耦，将各个直接功能模块全部放到外部功能模块来实现。这样策略开发者不需要关心信号的执行，而各个功能开发者也不需要考虑跟策略交互的问题。因为交互的逻辑都放到了核心里。  

* *WonderTrader*的基本架构图
    ![WonderTrader基本架构.jpg](https://image-static.segmentfault.com/406/400/4064005384-5e969b98d0161_articlex)

#### *Why* - 为什么要用*WonderTrader*

* * *

* 策略一次性编写，方便又不易出错
    无论是用*C++*开发策略在底层框架上直接运行，还是在*Python*子框架下开发策略，都能做到回测和实盘无修改直接迁移。既方便开发和迁移，又不会因为要改写代码而给已经测试好的策略引入新的问题。

* 策略可以在C++底层上直接开发，也可以在其他语言的子框架下编写，满足不同的场景需要
    用户可以根据自己的需要，选择在其他语言的子框架下开发策略（目前是*python3*），也可以直接用*C++*进行开发，*WonderTrader*都提供完备的接口（为了方便用户，*python*子框架*wtpy*和*C++*的策略调用的接口命名都保持一致）

* 策略的逻辑和执行彻底分离，让**专业的人做专业的事**。
    策略研发人员只需要关注策略本身的逻辑是否合理，而不用考虑执行的细节，例如买入到底是买开还是买平？卖出到底是卖开还是卖平？如何处理平今和平昨的佣金差别？到底是双开还是锁仓？
    执行器开发人员只需要关注算法交易的具体逻辑，也不需要考虑策略如何调用，订单要不要回报给策略处理。

* 核心模块用C++开发，相比其他解释型语言，执行效率更有保证。
    *C/C++*是偏底层的语言，在内存控制和执行效率上拥有更大的优势。虽然不像高级语言有那么多语法糖，但是成熟的社区环境，可以保障核心部分的逻辑能够有很好的解决方案。核心部分由专门的软件工程师来开发的前提下，相比起其他直接用解释型语言开发核心逻辑的方式，C++的核心模块显然更有优势。

* 一个数据落地程序+N个组合盘的方式，部署和管理都极为方便
    *WonderTrader*的数据组件，内置一个UDP广播服务，可以直接将接收到的行情数据进行广播。也就是说，数据组件可以同时给多个组合盘提供数据服务。
    内置的数据存储模块，采用内存映射文件的方式读写文件，从很大程度上降低了数据读写的开销。而且采用内存映射文件的方式，也可以轻松应对多个进程读取的应用场景。

* C接口导出，可以满足绝大多数跨语言的调用
    *WonderTrader*的项目结构，基本上都遵循静态核心库+动态C库。如果在C++环境下运行，则直接由可执行程序链接静态核心库即可。如果在其他语言下调用，则需要调用C接口导出的动态库。
    C接口导出的动态库，可以满足绝大多数跨语言调用的场景。*WonderTrader*的*python*子框架***wtpy***就是对*python3*环境下调用WonderTrader的一种实践。有需求的使用者可以根据自己的需要自行开发子框架，例如`C#`可以使用`DllImport`导出C库，而`python`则使用`ctypes`库调用C库，`java`也可以使用`JNI`调用C库。

#### *Where* - *WonderTrader*怎么获取

* * *

* *WonderTrader*的*github*地址：<https://github.com/wondertrader/wondertrader>

* *WonderTrader*的*Python3*子框架*wtpy*获取地址：<https://pypi.org/project/wtpy/>
    *wtpy*可以直接在*python3.5*以上的版本安装

    ``` json
    pip install wtpy
    ```

#### *How* - *WonderTrader*的用法

* * *

* Demo怎么运行
    github上提供了*WonderTrader*子框架***wtpy***的使用用例  
    <https://github.com/wondertrader/wondertrader/tree/master/demos/py>
    安装了***wtpy***以后，可以直接运行回测demo
    如果要运行数据demo和实盘demo，需要将合约列表更新到最新（期货demo中是contracts.json，股票demo中是stocks.json）

#### 写在最后

* * *

* WonderTrader是经过多年检验的成熟量化交易框架，从2013年开始搭建第一个版本到现在，经历过几次全部重构，最终形成如今的WonderTrader
* 关注公众号wondertrader，可以收到wondertrader的实时资讯

![WonderTrader2.png](http://q92ex9g0c.bkt.clouddn.com/wt/logo_qcode_ad.jpg)