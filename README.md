# U2K

个人 Linux 系统编程练习：通过 `open/read/write` 系统调用，学习用户态程序与内核交互的基本流程。

## 示例

- `u2k.c`：基础版，固定读取 `/proc/version` 并输出到标准输出。
- `u2k_par.c`：参数版，通过命令行指定文件路径，循环读取到文件末尾，并处理部分写入与写入无进展的情况。

## 使用

在 Linux 环境下编译和运行：

- 基础版：`gcc -Wall -Wextra u2k.c -o u2k`，运行 `./u2k`。
- 参数版：`gcc -Wall -Wextra u2k_par.c -o u2k_par`，运行 `./u2k_par /proc/version`。

参数版需要一个文件路径参数；输出可通过 shell 重定向保存到文件。

## 当前状态

两个示例均已通过 GCC 语法检查，尚未进行运行验证。参数版已加入循环读写处理，但尚未实现被信号中断（`EINTR`）后的重试。
