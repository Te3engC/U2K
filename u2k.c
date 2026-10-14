#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char buf[512];

    /* 先获取句柄 open("/.../...", 权限);  */
    int fd = open("/proc/version", O_RDONLY);
    if(fd < 0){
        perror("open error");
        return 1;
    }
    
    /* 应用向内核提供句柄 虚拟地址 以及字节大小 
     * 内核读取请求通过请求，通过受控的用户内存访问机制，把数据放入buf
     * 
     */
    ssize_t n =read(fd, buf, sizeof(buf));
    close(fd);

    if(n < 0){
        perror("read");
        return 1;
    }

    /* 请求内核把缓冲区前n个字节写道标准输出
     * STDOUT_FILENO通常是1.他可能对应终端，也可能因为重定向二对应文件或管道
     */
    ssize_t written = write(STDOUT_FILENO, buf, n);
    if(written < 0){
        perror("write");
        return 1;
    }

    return 0;
}

