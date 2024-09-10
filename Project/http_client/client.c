#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unitstd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <regex.h>


char* getIPFromDNS(char* host)
{
    struct hostnet* server = gethostbyname(host);  //使用gethostbyname解析主机名
    char* address = (char*)malloc(50);  //分配内存用于存储IP地址
    if(server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
    }
    else
    {
        printf("server 's hostname: %s\n\n", server->h_name);   //打印解析得到的主机名

        struct in_addr **addr_list = (struct in_addr **) server->h_addr_list;  //获取IP地址列表
        for(int i = 0; addr_list[i] != NULL; i++)
        {
            strcpy(address, inet_ntoa(*addr_list[i]));  //将解析到的第一个IP地址转换为字符串形式
            printf("解析到的ip地址为: IP ADDRESS->%s\n", address);  //打印IP地址
        }
    }
    return address;  //返回IP地址字符串
}

//生成HTTP请求头部
void generate_request_header(char *hostname, char *url, char *request_header)
{
    // 使用sprintf将请求头组合成一个字符串，包括GET请求行和HOST头部
    sprintf(request_header, "\
    GET / HTTP/1.1\r\n\
    HOST: %s\r\n\
    User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/66.0.3359.181 Safari/537.36\r\n\
    Cache-Control: no-cache\r\n\r\n", hostname);
    printf("-> HTTP请求报文如下\n--------HTTP Request--------\n%s--------\n", request_header); // 打印构建好的HTTP请求头部
}

//初始化服务器地址的结构体
struct sockaddr_in init_serv_addr(char* address, int port)
{
    struct  sockaddr_in serv_addr;              //定义服务器地址结构体
    memset(&serv_addr, 0, sizeof(serv_addr));   //清空结构体
    serv_addr.sin_family = AF_INET;             //设置地址族为IPV4
    serv_addr.sin_addr.s_addr = inet_addr(address);  //设置服务器IP地址
    serv_addr.sin_port = htons(port);           //设置服务器端口
    return serv_addr;                           //返回初始化好的服务器地址结构体
}

//判断URL是不是以http://或https://开头
int http_or_https(char *url)
{
    if(strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//从URL中提取主机名
char* extract_hostname(char* url)
{
    int url_len = strlen(url);

    //检查是否有HTTP或HTTPS前缀
    if(url_len >= 7 && http_or_https(url))
    {
        int i = 0;
        //查找前两个斜杠间的位置
        while(url[i] != '/' && i != url_len)
        {
            i += 1;
        }

        int www_len = strncmp(url, "www.", 4) == 0?4:0;  //检查是否已www.开头
        int end = i+2;
        //找到主机名后的第一个斜杠
        while(url[end] != '/')
        {
            ++end;
        }
        int len = end - (i+2) - www_len;  //计算主机名长度
        char* ret = malloc(len);   //分配内存存储主机名
        strncpy(ret, url + i + 2 + www_len, len);  //复制主机名
        printf("schema: http://abc.com/abc \n ret is %s\n", ret);  //打印主机名
        return ret;
    }
    else
    {
        //如果不包含HTTP或HTTPS前缀
        char* ret;
        if(strncmp(url, "www.", 4) == 0)
        {
            ret = malloc(url_len - 4);
            strncpy(ret, url+4, url_len-4);
        }
        else
        {
            ret = malloc(url_len - 1);
            strncpy(ret, url, url_len);
        }
        printf("schema: abc.com/abc \n ret is %s\n",  ret);
        return ret;
    }
}

int main()
{
    char *url = "http://example.com";  // 定义要访问的URL
    printf("\ncurl :  %s\n\n", url);    // 打印URL
    char *hostname = extract_hostname(url);  // 提取主机名
    printf("hostname is %s\n", hostname);    // 打印主机名
    char *address = getIPFromDNS(hostname);  // 获取主机的IP地址
    
    struct sockaddr_in serv_addr = init_serv_addr(address, 80); // 初始化服务器地址
    
    int sock = socket(AF_INET, SOCK_STREAM, 0); // 创建socket
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    { // 连接服务器
        printf("connect err");  // 如果连接失败，打印错误信息
        return 0;
    }

    char request_header[200]; // 定义用于存放HTTP请求头的数组
    generate_request_header(hostname, url, request_header); // 生成HTTP请求头
    int total = strlen(request_header); // 计算请求头长度
    int sent = 0;   // 初始化已发送字节
    do {
        int bytes = write(sock, request_header + sent, total - sent); // 发送请求头
        if (bytes < 0)
            printf("ERROR writing message to socket"); // 如果发送失败，打印错误信息
        if (bytes == 0)
            break;  // 如果没有数据发送，退出循环
            sent += bytes;  // 更新已发送字节
        } while (sent < total);
    
        char resp[10*1024];   // 定义响应缓冲区
        memset(resp, 0, sizeof(resp));   // 清空响应缓冲区
        total = sizeof(resp)-1;  // 设置响应缓冲区大小
        int received = 0;    // 初始化已接收字节
        do {
            int bytes = recv(sock,  resp+received, total-received, 0);  // 接收响应数据
            if (bytes < 0) 
            {
                printf("ERROR reading resp from socket");  // 如果接收失败，打印错误信息
            }
            if (bytes == 0) 
            {
                break;  // 如果没有数据接收，退出循环
            }
            received += bytes;  // 更新已接收字节
        } while(received < total);
    
        close(sock);  // 关闭socket
        free(address);  // 释放IP地址字符串所占内存
        return 0;  // 程序结束
}