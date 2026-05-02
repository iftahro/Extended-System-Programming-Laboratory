#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    int pipefd[2];
    
    // pipefd[0] is the read end, pipefd[1] is the write end
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child process reads from the pipe
        close(pipefd[1]);
        char buffer[1024];
        int nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (nbytes > 0) {
            buffer[nbytes] = '\0';
            printf("Child received: %s\n", buffer);
        }
        close(pipefd[0]);
        _exit(0);
    } else {
        // Parent process writes to the pipe
        close(pipefd[0]);
        write(pipefd[1], argv[1], strlen(argv[1]));
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
    }

    return 0;
}