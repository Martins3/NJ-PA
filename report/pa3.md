在 nanos-lite/ 目录下执行
make ARCH=x86-nemu update
make ARCH=x86-nemu run
即可. 另外如前文所说, 你也可以将Nanos-lite编译到 native 上并运行, 来帮助你进行调试

DPL(Descriptor Privilege Level)属性描述了一段数据所在的特权级
RPL(Requestor's Privilege Level)属性描述了请求者所在的特权级
CPL(Current Privilege Level)属性描述了当前进程的特权级,

i386中断机制不具体区分CPU异常和自陷, 甚至是将在PA4最后介绍的硬件中断, 而是对它们进
行统一的处理. 在i386中, 上述跳转目标是通过门描述符(Gate Descriptor)来指示的

们在NEMU中简化了门描述符的结构,
只保留存在位P和偏移量OFFSET:
31 23 15 7 0
+-----------------+-----------------+---+-------------------------------+
| OFFSET 31..16 | P | Don't care |4
+-----------------------------------+---+-------------------------------+
| Don't care | OFFSET 15..0 |0
+-----------------+-----------------+-----------------+-----------------+

IDT(Interrupt Descriptor Table, 中断描述符表), 

于是, 触发异常后硬
件的处理如下:
1. 依次将EFLAGS, CS(代码段寄存器), EIP寄存器的值压栈
2. 从IDTR中读出IDT的首地址
3. 根据异常号在IDT中进行索引, 找到一个门描述符
4. 将门描述符中的offset域组合成目标地址
5. 跳转到目标地址




eip and cpu.eip what is relation ?

we change the libc/stdio/fopen and fix by a stupid way !
