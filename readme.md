2017-2018学年秋冬学期操作系统实验

crossroads.c为第一小题源码
mymodule.c和user.c为第二小题源码，其中mymodule.c为内核模块，user.c为用户态程序

实验一 同步互斥和Linux内核模块
实验目的
	学习使用Linux的系统调用和pthread线程库编写程序。
	充分理解对共享变量的访问需要原子操作。
	进一步理解、掌握操作系统进程和线程概念，进程或线程的同步与互斥。
	学习编写多线程程序，掌握解决多线程的同步与互斥问题。
	学习Linux模块的实现机理，掌握如何编写Linux模块。
	通过对Linux系统中进程的遍历，进一步理解操作系统进程概念和进程结构。
实验内容

1.	有两条道路双向两个车道，即每条路每个方向只有一个车道，两条道路十字交叉。假设车辆只能向前直行，而不允许转弯和后退。如果有4辆车几乎同时到达这个十字路口，如图（a）所示；相互交叉地停下来，如图（b），此时4辆车都将不能继续向前，这是一个典型的死锁问题。从操作系统原理的资源分配观点，如果4辆车都想驶过十字路口，那么对资源的要求如下：
	向北行驶的车1需要象限a和b；
	向西行驶的车2需要象限b和c；
	向南行驶的车3需要象限c和d；
	向东行驶的车4需要象限d和a。
 

我们要实现十字路口交通的车辆同步问题，防止汽车在经过十字路口时产生死锁和饥饿。在我们的系统中，东西南北各个方向不断地有车辆经过十字路口（注意：不只有4辆），同一个方向的车辆依次排队通过十字路口。按照交通规则是右边车辆优先通行，如图(a)中，若只有car1、car2、car3，那么车辆通过十字路口的顺序是car3->car2->car1。车辆通行总的规则：
1)	来自同一个方向多个车辆到达十字路口时，车辆靠右行驶，依次顺序通过；
2)	有多个方向的车辆同时到达十字路口时，按照右边车辆优先通行规则，除非该车在十字路口等待时收到一个立即通行的信号；
3)	避免产生死锁；
4)	避免产生饥饿；
5)	任何一个线程（车辆）不得采用单点调度策略；
6)	由于使用AND型信号量机制会使线程（车辆）并发度降低且引起不公平（部分线程饥饿），本题不得使用AND型信号量机制，即在上图中车辆不能要求同时满足两个象限才能顺利通过，如南方车辆不能同时判断a和b是否有空。

编写程序实现避免产生死锁和饥饿的车辆通过十字路口方案，并给出详细的设计方案，程序中要有详细的注释（每三行代码必须要有注释）。

实验提示：
1)	每一辆车的行为设计为一个单独的线程。由于有4个不同方向的车辆，需要4种不同类型的线程。
2)	使用pthread的互斥锁和条件变量解决车辆的同步与互斥。
3)	对4个不同方向的车辆，要设置车辆队列条件变量如： queueNorth、queueEast、queueSouth、queueWest。比如说，当一辆车从北方来的时候已经在过十字路口，另一辆从北方驶来的车就要等在queueNorth队列中。每一个方向都需要一个计数器来跟踪等待排队的车辆数量。
4)	按照右边车辆优先通行规则，当一辆车在等待通过路口而它右边不断有车辆到达时，这辆车及这个方向车辆队列会导致饥饿。为了防止饥饿，我们要让刚刚通过路口的A车辆发一个信号给它左边等待的B车辆，接下去让B车辆通行。需要设置下次通行车辆的条件变量firstNorth， firstEast， firstSouth， firstWest
5)	每一车辆到达十字路口时，要检测是否有死锁发生，当发生死锁时，死锁检测线程必须发出一种信号，例如：从北方来的车辆先行。
6)	假设我们设计的可执行程序文件名为p1-1，可以用'e'、'w'、's'、'n'来标识东西南北4个方向驶来的车辆，程序p1-1运行时有如下显示（你的程序不一定是这样相同的输出）：
$ ./ p1-1 nsewwewn
car 4 from West arrives at crossing
car 2 from South arrives at crossing
car 1 from North arrives at crossing
car 3 from East arrives at crossing
DEADLOCK: car jam detected, signalling North to go
car 1 from North leaving crossing
car 3 from East leaving crossing
car 2 from South leaving crossing
car 4 from West leaving crossing
car 6 from East arrives at crossing
car 5 from West arrives at crossing
car 8 from North arrives at crossing
car 5 from West leaving crossing
car 6 from East leaving crossing
car 8 from North leaving crossing
car 7 from West arrives at crossing
car 7 from West leaving crossing

2.	编写一个Linux的内核模块，其功能是遍历操作系统所有进程。该内核模块输出系统中每个进程的：名字、进程pid、进程的状态、父进程的名字等；以及统计系统中进程个数，包括统计系统中TASK_RUNNING、TASK_INTERRUPTIBLE、TASK_UNINTERRUPTIBLE、TASK_ZOMBIE、TASK_STOPPED等（还有其他状态）状态进程的个数。同时还需要编写一个用户态下执行的程序，格式化输出（显示）内核模块输出的内容。
