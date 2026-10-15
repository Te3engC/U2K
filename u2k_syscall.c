/*
 * u2k_syscall.c是系统调用的复制程序
 * open -> syscall srcfile -> write dstfile
 * */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>

int main(int argc, char *argv[])
{
    char buf[512];

    if(argc != 3){
        fprintf(stderr, "Usage: %s <file> <destination>\n", argv[0]);
        return 1;
    } 

    int src_fd = open(argv[1], O_RDONLY);
    if(src_fd < 0){
        perror("open error");
        return 1;
    }

   /* 目标地址的创建 
    * O_WRONLY:只写, O_CREAT: 不存在则创建, O_EXCL:与O_CREAT配合，已经存在则报错
    * 0644：所有者读写，组和其他人读，实际权限还受umask限制
    * 不用O_TRUNC,避免不小心清空已有文件。
    */  
    int dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (dst_fd < 0) {
         perror("open destination");
         close(src_fd);
         return 1;
    }
    
    while(1){
        
        /* SYS_read:选择读取系统调用，src_fd:从哪个文件读取
         * src_fd: 从哪个文件读取
         * buf: 用户缓冲区地址
         * sizeof(buf): 最多读取多少字节
         */
        ssize_t n = syscall(SYS_read,src_fd, buf, sizeof(buf));
        size_t offset = 0;        
        
        /* 循环读取，当文件比缓冲区大的时候，将文件分块读取
         * n < 0 读取失败
         * n = 0 结束循环
         * n > 0 持续输出
         */
        if(n < 0){
            
            //读取信号被终端，重新尝试
            if(errno == EINTR){
                continue; //处于外层读取函数，执行后回到下一轮
            }

            perror("read");
            close(dst_fd);
            close(src_fd);
            return 1;
        }

        if(n == 0){
            break;
        }

        /*
         * 循环写，一次未输出完的数据继续输出 
         * buf+offset:从未输出的位置开始
         * n-offset:还有多少个字节未输出
         */
        while(offset < (size_t)n){
            
            ssize_t written = write(dst_fd, buf + offset, (size_t)n - offset);
           
            if(written < 0){
                if(errno == EINTR){
                    continue;
                }

                perror("write");
                close(dst_fd);
                close(src_fd);
                return 1;
            }

            if(written == 0){
                fprintf(stderr, "write made no progress\n");
                close(dst_fd);
                close(src_fd);
                return 1;
            }

            offset +=(size_t)written;
        }
        
    }
   

    //检查关闭情况
     if(close(dst_fd) == -1){
        perror("close");
        close(src_fd);//目标路径失败，释放源文件
        return 1;
    }

    if(close(src_fd) == -1){
        perror("close");
        return 1;    
    }    
    
    return 0;
}










