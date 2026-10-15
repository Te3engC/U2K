# U2K

个人 Linux 系统编程练习：通过 `open/read/write` 系统调用，学习用户态程序与内核交互的基本流程。

## 功能区别

| 文件 | 主要功能 | 相比上一版的变化 |
| --- | --- | --- |
| `u2k.c` | 读取 `/proc/version` 并输出 | 基础版：只读、写一次，最多读取 512 字节 |
| `u2k_par.c` | 按参数路径读取文件并输出 | 循环读到文件末尾，部分写入时继续补写 |
| `u2k_errno.c` | 增强读写错误处理 | 增加 `EINTR` 重试，并检查 `close()` 结果 |
| `u2k_copy.c` | 将源文件复制到新目标文件 | 输出从标准输出改为文件，目标已存在时报错，不覆盖 |

## 使用

在 Linux 环境下编译和运行：

- 基础版：`gcc -Wall -Wextra u2k.c -o u2k`，运行 `./u2k`。
- 参数版：`gcc -Wall -Wextra u2k_par.c -o u2k_par`，运行 `./u2k_par /proc/version`。
- 错误处理版：`gcc -Wall -Wextra u2k_errno.c -o u2k_errno`，运行 `./u2k_errno /proc/version`。
- 文件复制版：`gcc -Wall -Wextra u2k_copy.c -o u2k_copy`，运行 `./u2k_copy source.txt destination.txt`。

复制前需准备可读的源文件；目标文件必须不存在。

## 当前状态

示例已通过 GCC 语法检查，尚未运行验证。`EINTR` 重试仅在错误处理版和文件复制版中实现。
