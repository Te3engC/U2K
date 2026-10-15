# U2K

个人 Linux 系统编程练习：通过 `open/read/write` 系统调用，学习用户态程序与内核交互的基本流程。

## 功能区别

| 文件 | 主要功能 | 新增或改进点 |
| --- | --- | --- |
| `u2k.c` | 读取 `/proc/version` 并输出 | 基础版：只读、写一次，最多读取 512 字节 |
| `u2k_par.c` | 按参数路径读取文件并输出 | 循环读到文件末尾，部分写入时继续补写 |
| `u2k_errno.c` | 增强读写错误处理 | 增加 `EINTR` 重试，并检查 `close()` 结果 |
| `u2k_copy.c` | 将源文件复制到新目标文件 | 输出从标准输出改为文件，目标已存在时报错，不覆盖 |
| `u2k_syscall.c` | 显式系统调用读取 | 指定 `SYS_read`，写入仍用 `write()` |
| `u2k_mem.c` | 文件复制与内存观察 | 堆缓冲区复制，观察栈/堆地址、匿名映射和 `/proc/self/maps` |

`read()` 与 `syscall(SYS_read, ...)` 最终都会请求内核读取；后者显式指定系统调用号，并非绕过内核。
`u2k_mem.c` 的复制缓冲区来自 `malloc()`；匿名 `mmap()` 用于逐页访问实验，并对比解除映射前后的内存布局。

## 使用

在 Linux 下编译，例如：`gcc -Wall -Wextra u2k_syscall.c -o u2k_syscall`。
其他示例替换对应源文件和输出程序名。

- 基础版：`./u2k`。
- 参数版：`./u2k_par /proc/version`。
- 错误处理版：`./u2k_errno /proc/version`。
- 文件复制版：`./u2k_copy source.txt destination.txt`。
- 系统调用版：`./u2k_syscall source.txt destination.txt`。
- 内存观察版：`./u2k_mem source.txt destination.txt`。

复制前需准备可读的源文件；目标文件必须不存在。

## 当前状态

示例已通过 GCC 语法检查，尚未运行验证。基础版和参数版未实现 `EINTR` 重试，其余示例已加入读写中断重试。
