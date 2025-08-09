#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

// 信号处理函数
void signalHandler(int sig) {
    printf("Child process received signal: %d\n", sig);
    // 执行一些动作，例如退出程序
    exit(0);
}

int main() {
    // 创建子进程
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // 子进程
        // 注册信号处理器
        signal(SIGUSR1, signalHandler);

        // 等待信号
        pause();

        printf("Child process exiting normally.\n");
        return 0;
    } else {
        // 父进程
        // 获取子进程的 PID
        pid_t child_pid = pid;

        // 向子进程发送 SIGUSR1 信号
        if (kill(child_pid, SIGUSR1) == -1) {
            perror("kill");
            return 1;
        }

        // 等待子进程结束
        wait(NULL);

        printf("Parent process: Child process terminated.\n");
        return 0;
    }
}