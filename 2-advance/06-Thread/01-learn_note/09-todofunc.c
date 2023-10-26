
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
/*
================================================================
! void (*signal(int signum, void (*handler)(int)))(int);
设置信号处理函数，以处理特定的信号。在C和C++中，它是用于处理信号的标准库函数。
信号是操作系统用于通知进程发生了某些事件或异常情况的一种通信机制。
参数：
signum：
    表示要设置信号处理函数的信号编号，它是一个整数。
    例如，SIGINT 表示中断信号，通常由用户在终端上键入 Ctrl+C 时发送给程序。
handler：
    是一个函数指针，指向用户定义的信号处理函数。这个函数接受一个整数参数，
    表示信号的编号。它可以是您自己编写的函数，用于处理接收到的信号。
返回值：
    signal 函数的返回值是一个函数指针，它指向之前设置的信号处理函数。
    通常情况下，您不需要关心 signal 的返回值，而是使用它来设置信号处理函数。
================================================================
! int sem_init(sem_t *sem, int pshared, unsigned int value);
用于初始化 POSIX 信号量的函数，它用于创建一个新的信号量或将现有信号量重新初始化。
这是在多线程和进程编程中用于同步的重要工具。
参数：
sem：
    是一个指向 sem_t 类型的信号量对象的指针。这个对象用于表示和控制信号量。
pshared：
    是一个整数，用于指定信号量的共享方式。通常可以设置为0（表示在进程内共享）
    或非0（表示在进程之间共享）。在许多系统上，只支持0，表示信号量仅在进程内共享。
    value：是一个无符号整数，表示信号量的初始值。它指定了信号量的资源数量，
    可以理解为资源的个数。它通常是一个正整数，表示可用的资源数量。
返回值：
    sem_init 函数的返回值是一个整数，表示函数是否成功执行。
    如果成功，返回值为0；
    如果出现错误，返回值将是一个非零错误码，可以使用 errno 来获取具体的错误信息。







*/

int main()
{
    signalTest();
}

void sigint(int signum)
{
    printf("接收到 SIGINT 信号，即Ctrl+C被按下。\n");
    // 可以在这里执行一些处理操作
    exit(1); // 退出程序
}

void signalTest()
{
    // 设置信号处理函数，当接收到 SIGINT 信号时调用 sigint 函数
    signal(SIGINT, sigint);

    printf("请按Ctrl+C终止程序...\n");

    // 无限循环，等待信号处理函数执行
    while (1)
    {
        // 一些程序逻辑
    }
    return 0;
}