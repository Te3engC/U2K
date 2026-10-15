#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    char buf[512];

    if(argc != 2){
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    /* 先获取句柄 open("/.../...", 权限);  */
    int fd = open(argv[1], O_RDONLY);
    if(fd < 0){
        perror("open error");
        return 1;
    }
    
    while(1){
        ssize_t n = read(fd, buf, sizeof(buf));
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
            close(fd);
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
            
            ssize_t written = write(STDOUT_FILENO, buf + offset, (size_t)n - offset);
           
            if(written < 0){
                if(errno == EINTR){
                    continue;
                }

                perror("write");
                close(fd);
                return 1;
            }

            if(written == 0){
                fprintf(stderr, "write made no progress\n");
                close(fd);
                return 1;
            }

            offset +=(size_t)written;
        }
        
    }
   

    //检查关闭情况
    if(close(fd) == -1){
        perror("close");
        return 1;    
    }    

    return 0;
}










