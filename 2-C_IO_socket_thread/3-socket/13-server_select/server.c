#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*

    client 实现用 selct 管理 conn_fd 和控制台输入
    

*/
/*
`select` 函数是用于多路复用 I/O 的系统调用，它的原型如下：

#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

参数说明：
- `nfds`：监视的文件描述符的最大值加1，即待监视的文件描述符集合中最大的文件描述符加1。
- `readfds`：指向可读文件描述符集合的指针，用于检查是否有数据可读。
- `writefds`：指向可写文件描述符集合的指针，用于检查是否可以写入数据。
- `exceptfds`：指向异常文件描述符集合的指针，用于检查是否有异常情况。
- `timeout`：指向等待超时时间的结构体指针，用于设置等待的最长时间。如果为 `NULL`，`select` 函数将一直等待，直到有文件描述符就绪。

返回值：
- 如果超时时间到达，没有文件描述符就绪，`select` 函数返回 0。
- 如果出错，返回 -1，并设置 `errno`。

`fd_set` 是一个文件描述符集合的数据类型，它是一个位图，用于表示一组文件描述符。在使用 `select` 函数之前，需要使用下面的宏对 `fd_set` 进行初始化和操作：

- `FD_ZERO(fd_set *set)`：将 `set` 中的所有位清零，初始化为空集。
- `FD_SET(int fd, fd_set *set)`：将 `fd` 加入到 `set` 中。
- `FD_CLR(int fd, fd_set *set)`：将 `fd` 从 `set` 中移除。
- `FD_ISSET(int fd, fd_set *set)`：检查 `fd` 是否在 `set` 中。

这些宏可以用于设置和操作 `readfds`、`writefds` 和 `exceptfds`，以指定要监视的文件描述符集合。


*/


#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    while (1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret == -1 && errno == EINTR)
            continue;
        return ret;
    }
}

// 从套接字中读取指定长度的数据，读不够不返回
ssize_t readn(int fd, void *buf, size_t count)
{
    size_t needNum = count; // 剩余需要读取的字节数
    ssize_t readed;
    char *bufp = (char *)buf;
    while (needNum > 0)
    {
        if ((readed = read(fd, bufp, needNum)) < 0)
        {
            if (errno == EINTR) // 可中断睡眠
                continue;
            return -1;
        }
        else if (readed == 0) // 若对方已关闭
        {
            return count - needNum;
        }

        // 正常情况下，read 成功读取数据，修改 bufp，让下一次读从未读的缓冲区获取
        bufp += readed;    // bufp + readed已经读取的字节数
        needNum -= readed; // 需要读的字节数 - 已经读的字节数
    }
    return count;
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = buf;
    int nleft = maxline;

    while (1)
    {
        // 看一下缓冲区有没有数据，并不移除内核缓冲区数据
        ret = recv_peek(sockfd, bufp, nleft);
        if (ret < 0)
        { // 失败
            return ret;
        }
        else if (ret == 0)
        { // 对方已关闭
            return ret;
        }

        nread = ret;
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')
            {                                     // 若缓冲区有换行符
                ret = readn(sockfd, bufp, i + 1); // 读走数据
                if (ret != i + 1)
                {
                    exit(EXIT_FAILURE);
                }
                return ret; // 有换行符就返回，并返回读走的数据
            }
        }

        if (nread > nleft)
        { // 如果读到的数据大于一行最大数，异常处理
            exit(EXIT_FAILURE);
        }

        nleft -= nread; // 若缓冲区没有换行符，把剩余的数据读走
        ret = readn(sockfd, bufp, nread);
        if (ret != nread)
        {
            exit(EXIT_FAILURE);
        }
    }
}

// 向套接字中写入指定长度的数据，写不够不返回
ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t needNum = count;
    ssize_t nwritten;
    const char *bufp = (const char *)buf;
    while (needNum > 0)
    {
        if ((nwritten = write(fd, bufp, needNum)) < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (nwritten == 0)
            continue;

        bufp += nwritten;
        needNum -= nwritten;
    }
    return count;
}

void printfIPAndPort(int sockfd)
{
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sockfd, (struct sockaddr *)&localaddr, &addrlen) < 0)
        ERR_EXIT("getsockname");
    printf("pid:%d,ip=%s port=%d\n", getpid(), inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));
}

// 处理客户端请求
void do_service(int conn)
{
    char recvbuf[1024];
    while (1)
    {
        memset(&recvbuf, 0, sizeof(recvbuf));

        int ret = readline(conn, recvbuf, 1024);
        if (ret == -1)
        {
            ERR_EXIT("readline");
        }
        else if (ret == 0)
        {
            printf("client close\n");
            break;
        }

        if (!strcmp("server quit\n", recvbuf))
        {
            break;
        }

        // 将接收到的数据输出到标准输出
        // printfIPAndPort(conn);
        fputs(recvbuf, stdout);

        // 回写数据给客户端
        writen(conn, &recvbuf, ret);
    }

    close(conn);
    exit(EXIT_SUCCESS);
}

/*
success 3
terminated 5
虽然只接受到 3 个信号，但是仍然处理了 5 个子进程，
*/
void sig_handler(int signum)
{
    int mypid = 0;
    int saved_errno = errno; // 保存 errno，以便在 waitpid 调用后恢复
    int status;
    printf("process pid:%d recv SIGCHLD\n ", getpid());
    // wait(NULL);
    // 仍然存在信号丢失，但是使用 while
    while ((mypid = waitpid(-1, NULL, WNOHANG) > 0))
    {
        // 处理已终止的子进程
        if (WIFEXITED(status))
        {
            printf("Child process %d terminated with status: %d\n", mypid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child process %d terminated by signal %d\n", mypid, WTERMSIG(status));
        }
    }
    errno = saved_errno; // 恢复 errno
    printf("process pid:%d handle success\n ", getpid());
}

int main(void)
{
    // 当客户端异常断开后,服务端子进程会变成僵尸进程,解决
    // signal(SIGCHLD, SIG_IGN); // 忽略

    signal(SIGCHLD, sig_handler);

    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");

    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int conn;

    pid_t pid;
    while (1)
    {
        if ((conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen)) < 0)
            ERR_EXIT("accept");

        printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        if (pid == 0)
        {
            close(listenfd);
            do_service(conn);
            // exit(EXIT_SUCCESS);
        }
        else
        {
            close(conn);
        }
    }

    return 0;
}
