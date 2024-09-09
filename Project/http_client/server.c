#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main()
{
    /* char *address = "127.0.0.1"; */
    int sock = socket(AF_INET, SOCK_STREAM, 0);      //创建TCP套接字
    struct sockaddr_in serv_addr;                    //用于存储服务器地址
    memset(&serv_addr, 0, sizeof(serv_addr));        //初始化结构体为0
    serv_addr.sin_family = AF_INET;                  //设置地址族为IPV4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);   //绑定到本地任意地址
    serv_addr.sin_port = htons(12345);               //将端口号转换为网络字节序，并设置为12345

    if((bind(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr))) == -1)
    {
        //绑定套接字到地址和端口，出错则打印错误信息并退出
        fprintf(stderr, "Error on bind --> %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("bind succeed\n");

    /* Listening to incoming connections */
    if((listen(sock, 5)) == -1)
    {
        //进入监听状态，最多允许5个连接排队，出错则打印错误信息
        fprintf(stderr, "Error on listen --> %s", strerror(errno));
    }
    printf("listen succeed\n");

    struct sockaddr* client_address;   //用于存储客户端地址
    int client_sock;                   //客户端套接字
    char buf[100];                     //用于存储客户端发送的请求数据
    char* request_header[10];          //用于存储分解后的请求头部，每个元素为一行

    while(1)
    {
        printf("inside\n");
        socklen_t client_address_size = sizeof(client_address);  //客户端地址结构大小
        client_sock = accept(sock, (struct sockaddr*) &client_address, &client_address_size );
        //接受客户端连接，返回新的客户端套接字

        if(client_sock == -1)
        {
            //接受连接失败时打印错误信息
            printf("accept error");
        }

        printf("read\n");
        read(client_sock, buf, 80);  //从客户端读取数据，最多读取80字节
        for(int i = 0; i < 10; ++i)
        {
            //分配内存给request_header数组的每个元素，用来存储请求头行
            request_header[i] = (char*)malloc(100);
        }

        int header_count = 0;  //用于跟踪已存储的请求头行数
        int  last_position = 0;  //用于记录上一行请求头的起始位置
        printf("start loop\n");
        for(int i = 0; i < strlen(buf); ++i)
        {
            //遍历读取到的客户端请求
            if(i+1 < strlen(buf) && buf[i] == '\r' && buf[i+1] == '\n')
            {
                //如果发现行结束符'\r\n'
                i += 1;
                printf("%c", buf[i]);  //输出行结束符
                printf("copy: %s ---",buf+last_position);  // 打印需要拷贝的部分
                memcpy(request_header[header_count++], buf+last_position, i-last_position);  // 拷贝到对应的请求头
                last_position = i;  // 更新last_position以标记下一行的起始点
            } 
            else 
            {
                // 如果当前字符不是行结束符，直接输出
                printf("%c", buf[i]);
            }
        }
         printf("\n=========================\n");
        for(int i=0; i<10; ++i) 
        {
            // 打印解析到的请求头
            printf("request are %s\n", request_header[i]);
        }
    }

    /* close(sock); */  // 关闭服务器套接字（暂时注释掉）
}