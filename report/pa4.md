1. rethinking \_Context
  1. why we can change the context
  2. the details of change context

2. asm_trap()会根据栈上保存的上下文结构来恢复A的上下文. 

当想要找到其它进程的上下文结构的时候, 只要寻找这个进程相关的cp指针即可.

WARN update variable name **cp** and **tf**

Nanos-lite使用一个联合体来把其它信息放置在进程堆栈的底部.
代码为每一个进程分配了一个32KB的堆栈, 已经足够使用了, 不会出现栈溢出导致未定义行为. 
在进行上下文切换的时候, 只需要把PCB中的cp指针返回给CTE的irq_handle()函数即可,
剩余部分的代码会根据上下文结构恢复现场. 我们只要稍稍借助数学归纳法, 就可以让我们相信这个过程对于正在运行的进程来说总是正确的.

我们只需要在进程的栈上人工创建一个上下文结构


创建上下文是通过CTE提供的\_kcontext()函数 (在nexus-am/am/arch/x86-nemu/src/cte.c中定义)来实现的, 它的原型是

```
_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg);
```

i386提供了一个CR3(control register 3)寄存器, 专门用于存放页目录的基地址.

首先需要准备一些内核页表

第二项工作是调用AM的`_vme_init()`函数, 
填写内核的页目录和页表, 然后设置CR3寄存器,
最后通过设置CR0寄存器来开启分页机制

由于我们不打算实现保护机制,
在page_translate()函数的实现中,
你务必使用assertion检查页目录项和页表项的present位, 如果发现了一个无效的表项,
及时终止NEMU的运行, 否则调试将会异常困难

为了在NEMU中实现分页机制, 你需要添加**CR3**寄存器和**CR0**寄存器, 以及相应的操作它们的指令

```
int _map(_Protect *p, void *va, void *pa, int prot);
```

它用于将地址空间p中虚拟地址va所在的虚拟页, 以prot的权限映射到pa所在的物理页. 当prot中的present位为0时, 表示让va的映射无效.


## todo
2. change to use `cr0` gp bit
3. 通过\_map()把这一物理页映射到用户进程的虚拟地址空间中
3. 首先需要将navy-apps/Makefile.compile中的链接地址
-Ttext参数改为0x8048000, 
这是为了避免用户进程的虚拟地址空间与内核相互重叠
4. 同样的, nanos-lite/src/loader.c中的DEFAULT_ENTRY也需要作相应的修改. 这时, "虚拟地址作为物理地址的抽象"这一好处已经体现出来了: 原则上用户进程可以运行在任意的虚拟地址, 不受物理内存容量的限制. 
5. \_Protect结构体作为PCB的一部分
6.  你需要在AM中实现\_map()函数(在nexus-am/am/arch/x86-nemu/src/vme.c中定义), 你可以通过p-\>ptr获取页目录的基地址. 若在映射过程中发现需要申请新的页表, 可以通过回调函数pgalloc_usr()向Nanos-lite获取一页空闲的物理页.

7. **present**

8. do 0x8000400 overflow ? 
9. can we cover 2GB space even we can use different space ?
  1. `_map` will handle this, if we can make a line
  2. 






1. what maybe cause error ?
  1. decode function
  2. **再次提醒, 只有进入保护模式并开启分页机制之后才会进行页级地址转换**
  3. **eip** read the vaddr too, how to change them ?
  4. they have same walp path, even has different address directory, they will cover the use page table entry too.
  5. 





# questions we are wondering


A huge bug:
eip and cpu.eip
