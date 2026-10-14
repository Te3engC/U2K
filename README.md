# U2K

个人 Linux 系统编程练习：通过 `open/read/write` 系统调用，学习用户态程序与内核交互的基本流程。

`u2k.c` 读取 `/proc/version`，并将读取到的内容输出到标准输出。

## 使用

在 Linux 环境下编译：`gcc -Wall -Wextra u2k.c -o u2k`，运行：`./u2k`。

## 当前状态

已通过 GCC 语法检查，尚未进行运行验证；当前为基础示例，未完善中断重试及部分写入处理。
