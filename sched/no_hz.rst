# NO_HZ: Reducing Scheduling-Clock Ticks
本文档描述了可以减少调度时钟中断数量的Kconfig选项和boot参数，这样可以改进能效并减少OS jitter。减少OS jitter对一些类型的HPC和real-time的应用非常重要。
这里有三个主要的方式来管理调度时钟中断（称为调度时钟tick或简单称为tick）：
- 不忽略任何调度tick（CONFIG_HZ_PERIODIC=y或者CONFIG_NO_HZ=n）。正常情况下你不希望选择这个选项。
- 对idle CPUs忽略调度tick（CONFIG_NO_HZ_IDLE=y）。这是最通用的方式，也是默认方式。
- 对idle CPUs或仅有一个runnable task的CPU忽略调度tick。如果你没有运行realtime的应用或HPC负载时，你正常情况不希望使用该选项
这三种方式将在下面三个章节里描述，后面跟着对RCU特定的思考，讨论测试以及已知的问题。
## Never Omit Scheduling-Clock Ticks
早期的内核版本不支持忽略调度时钟中断。它表明在一些情况下这种方式仍然是正确的方式，比如在高负载的环境下有很多短burst的task，这时通常
idle周期又非常短暂。对于这种负载，调度时钟中断正常情况会正常产生，因为对于每个CPU正常情况下有多个runnable task。在这种情况下，尝试
关掉调度时钟tick除了增加进入/退出idle的切换以及user/kernel态的转换之外没有其他效果。
这种方式可以通过CONFIG_HZ_PERIODIC=y被选择。
但是如果你在运行长时间idle的轻负载，不忽略调度时钟tick将会导致额外的功耗。这对battery-powered的设备非常不好，它会导致减少battery寿命。
如果你运行轻负载，你应该阅读下面章节。
另外如果你在运行real-time负载或HPC负载，调度时钟中断可能会降低你的应用性能。

## Omit Scheduling-Clock Ticks For Idle CPUs
如果一个CPU为idle，没有必要对其发送调度时钟中断。毕竟调度时钟中断的主要目的是在多个任务中将CPU转移到其他的任务，而不是只专注在其中一个
任务，而idle CPU没有任何任务可以转移。
不接受调度时钟中断的idle CPUs被称为“dyntick-idle”，“in dyntick-idle mode”，“in nohz mode”，或“running tickless”。本文档使用“dyntick-idle mode”。
CONFIG_NO_HZ_IDLE=y选项使内核避免发送调度时钟中断给idle CPU, 这对battery-powered设备和高虚拟化mainframe非常重要。battery-powered设备
如果运行在CONFIG_PERIODIC=y时将很快会用完电池，通常是CONFIG_NO_HZ_IDLE=y的2-3倍快。运行1500 OS实例的mainframe会发现它的CPU时间一般被没有
必要的调度时钟中断耗尽。在这些情况下，有强烈的动机去避免往idle CPU发送调度时钟中断。当然，dyntick-idle mode并不是免费的：
- 它会增加进入/退出idle loop路径上的指令数
- 在很多架构上，dyntick-idle mode也会增加时钟重新编程操作的数量
因此如果系统有比较激进的real-time响应限制，通常会运行CONFIG_HZ_PERIODIC=y的内核，这是为了减少从idle转化的latency。
boot参数"nohz="用于指定dyntick-idle mode, 可以通过nohz=off关闭dyntick-idle mode。默认nohz=on且CONFIG_NO_HZ_IDLE=y时使能dyntick-idle mode.

## Omit Scheduling-Clock Ticks For CPUs with Only One Runnable Task
如果CPU仅有一个runnable task, 没有必要对该CPU发送调度时钟中断，因为没有其他的task可以切换。
注意对只有一个runnable task的CPU不发送调度时钟中断意味着对idle CPUs也不会发送。
CONFIG_NO_HZ_FUL=y选项可以避免让内核发送调度时钟中断给只有一个runnable task的CPU，这些CPU被称为“adaptive-ticks CPUs”。
这对于有强烈real-time回复要求的应用非常重要，因为它可以改进糟糕的响应时间，响应时间可能由调度时钟中断持续延长。
如果某一个CPU在给定的过程被延长，其他CPU也被迫等待idle直到延长的CPU完成。因此，这个延长可能是加倍的。在这种情况下，强烈
意愿不发送调度时钟中断。
默认情况不会配置adaptive-ticks CPU。“nohz_full=”启动参数指定adaptive-ticks CPU。比如"nohz_full=1,6-8"表示CPU 1, 6, 7, 8都
为adaptive-ticks CPU。注意不能将所有CPU都作为adaptive-ticks CPU：至少要有一个non-adaptive-tick CPU保持Online来处理timekeeping task， 这是为了保证系统调用gettimeofday()时在adaptive-tick CPU上返回准确的值。
（这对于CONFIG_NO_HZ_IDLE没有问题，因为在idle CPU上没有可以运行的用户态进程，没有clock要求）注意这意味着在CONFIG_NO_HZ_FULL=y有效果时必须至少有2个CPU。
最后，adaptive-ticks CPUs必须有他们rcu callbacks offload。这在“RCU IMPLICATIONS”章节会涉及。
正常情况下，一个CPU可以保持adaptive-ticks mode尽可能长。切换到内核态模式并不能自动改变mode。在必要对时候，比如CPU进入RCU callback时，CPU会退出adaptive-tick mode。
与dyntick-idle mode类似，adaptive-tick mode也有损耗：
1. CONFIG_NO_HZ_FULL选择CONFIG_NO_HZ_COMMON，因此无法在不运行dynamic idle情况下运行adaptive tick mode。这个依赖也扩展到了实现。CONFIG_NO_HZ_FULL存在CONFIG_NO_HZ_IDLE的代价。
2. 由于需要通知内核子系统（RCU）在mode上的变化，user/kernel切换会有一些损耗
3. POSIX CPU timer会阻止CPU进入adaptive-tick mode。Real-time应用需要采取其他的手段代替posix cpu timer
4. 如果有超过硬件可容纳的perf event处于Pending, 正常他们通过Round-robin方式处理。adaptive-tick mode可能会阻止RR的发生。在进入adaptive-tick mode后可以阻止大量的perf event处于Pending，这可能可以解决这个问题
5. 对adaptive-tick CPU的调度统计的计算可能与non-adaptive-tick CPU不一样。同样这可能扰乱real-time task的负载均衡。
尽管随着时间推移，上述问题会有改善。adaptive-tick对于很多real-time和计算性的应用很有作用。但是，上面列的问题意味着adaptive-ticks mode还不能作为默认使能。
