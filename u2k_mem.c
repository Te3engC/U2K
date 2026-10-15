/*
 * u2k_.mem.c是复制程序，在此基础上实现了内存malloc和mmap的调用查看
 * open -> read srcfile -> write dstfile
 * */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>


static void show_maps(const char *stage)
{

    fprintf(stderr, "\n--- maps: %s ---\n", stage);
    /* 读取/proc/self/maps，观察当前程序的虚拟内存区域 */
    FILE *maps = fopen("/proc/self/maps", "r");// r代表只读

    if(maps == NULL){
        perror("open maps");
    } else{
        char line[256];

        while(fgets(line, sizeof(line), maps) != NULL){
           if(fputs(line, stderr) == EOF){
               perror("output maps");
               break;
           }
        }

        if(ferror(maps)){
            perror("read maps");
        }

        if(fclose(maps) == EOF){
            perror("close maps");
        }
    }    

}

int main(int argc, char *argv[])
{
    char buf[512];
    int marker = 0;

    fprintf(stderr, "marker address : %p\n", (void *)&marker); 
    fprintf(stderr, "buf address: %p\n", (void *)buf);
    fprintf(stderr, "buf capacity: %zu bytes\n", sizeof(buf));

    if(argc != 3){
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    } 

    /* 新增内存观察 */
    
    //malloc()
    size_t capacity = 512;
    char *heap_buf = malloc(capacity);

    if(heap_buf == NULL){
        fprintf(stderr, "malloc faild\n");
        return 1;
    }
    
    fprintf(stderr, "pointer address: %p\n", (void *)&heap_buf);
    fprintf(stderr, "memory  address: %p\n", (void *)heap_buf);

    //获取页大小
    long page_size = sysconf(_SC_PAGESIZE);
    
    if(page_size <= 0){
        fprintf(stderr, "cannot get page size\n");
        free(heap_buf);
        return 1;
    }    
    
    size_t map_length = (size_t)page_size * 4;
    
    fprintf(stderr, "page size: %ld bytes\n", page_size);
    fprintf(stderr, "mapping length: %zu bytes\n", map_length); 

    /* void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset)
     * NULL: 让内核选择映射地址
     * map_length: 请求映射的长度
     * PROT_READ | PROT_WRITE: 允许读写
     * MAP_PRIVATE: 私有映射
     * MAP_ANONYMOUS: 匿名内存，不对应文件
     * -1，0：匿名映射使用的文件描述符与偏移参数
     * 申请堆内存 → 建立匿名映射 → 检查结果 → 打印映射地址 → 解除映射 → 用堆内存复文件
     * ->释放堆内存
     */
    char *mapped_buf = mmap(NULL, map_length,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1, 0);

    if(mapped_buf == MAP_FAILED){
        perror("mmap");
        free(heap_buf);
        return 1;
    }


    fprintf(stderr, "maped address: %p\n", (void *)mapped_buf);
    
    /* 设置并打印具体映射内存的内容 */
    //字符
    mapped_buf[0] = 'A';
    mapped_buf[map_length - 1] = 'Z';

    fprintf(stderr, "first=%c, last=%c\n",
            mapped_buf[0],mapped_buf[map_length - 1]);

    // 逐页访问并且赋值
    for(size_t i = 0; i < 4; i++){
        size_t offset = i * (size_t)page_size;

        mapped_buf[offset] = (char)('A' + i);

        fprintf(stderr, "page %zu: address=%p, value=%c\n",
                i, (void*)(mapped_buf + offset),
                mapped_buf[offset]);
    }       

    show_maps("before munmap");

    /* 取消映射
     * int munmap(void* start,size_t length);
     * 
     */ 
    if(munmap(mapped_buf, map_length) == -1){
        perror("munmap");
        free(heap_buf);
        return 1;
    }

    show_maps("after munmap");
    /*
     * &heap_buf：指针变量自身的地址，通常在栈上.
     * heap_buf：变量中保存的地址，指向动态分配的内存。
     */

    int src_fd = open(argv[1], O_RDONLY);
    if(src_fd < 0){
        perror("open error");
        free(heap_buf);
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
         free(heap_buf);
         close(src_fd);
         return 1;
    }
    
    while(1){
       
       ssize_t n = read(src_fd, heap_buf, capacity); 
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
            free(heap_buf);
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
            
            ssize_t written = write(dst_fd, heap_buf + offset, (size_t)n - offset);
           
            if(written < 0){
                if(errno == EINTR){
                    continue;
                }

                perror("write");
                free(heap_buf);
                close(dst_fd);
                close(src_fd);
                return 1;
            }

            if(written == 0){
                fprintf(stderr, "write made no progress\n");
                free(heap_buf);
                close(dst_fd);
                close(src_fd);
                return 1;
            }

            offset +=(size_t)written;
        }    
        
    }
   
    free(heap_buf);

    //检查关闭情况
     if(close(dst_fd) == -1){
        perror("close");
        close(src_fd);//目标文件关闭失败，释放源文件
        return 1;
    }

    if(close(src_fd) == -1){
        perror("close");
        return 1;    
    }    
    
    return 0;
}










