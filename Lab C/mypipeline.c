#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t child1, child2;
    
    char *cmd1[] = {"ps", "-xl", NULL};
    char *cmd2[] = {"grep", "5", NULL};

    // 1. Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>forking...)\n");
    
    // 2. Fork a first child process (child1)
    child1 = fork();
    if (child1 == -1) {
        perror("fork 1 failed");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        // 3. In the child1 process:
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        close(STDOUT_FILENO); // 3.1 Close standard output.
        dup(pipefd[1]);       // 3.2 Copy the write-end descriptor to the empty table space.
        close(pipefd[1]);     // 3.3 Close the duplicated write-end descriptor.
        close(pipefd[0]);     // Close unnecesarry read-end descriptor.

        fprintf(stderr, "(child1>going to execute cmd: ps -xl)\n");
        execvp(cmd1[0], cmd1); // 3.4 Execute "ps -xl"
        
        perror("execvp ps failed");
        exit(EXIT_FAILURE);
    }

    // parent process:
    fprintf(stderr, "(parent_process>created process with id: %d)\n", child1);
    
    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    close(pipefd[1]); // 4. Close the write end of the pipe, otherwise the second child will wait forever.

    fprintf(stderr, "(parent_process>forking...)\n");
    
    // 5. Fork a second child process (child2)
    child2 = fork();
    if (child2 == -1) {
        perror("fork 2 failed");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        // 6. In the child2 process:
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        close(STDIN_FILENO); // 6.1 Close standard input
        dup(pipefd[0]);      // 6.2 Duplicate read-end of the pipe
        close(pipefd[0]);    // 6.3 Close the file descriptor that was duplicated

        fprintf(stderr, "(child2>going to execute cmd: grep 5)\n");
        execvp(cmd2[0], cmd2); // 6.4 Execute "grep 5"
        
        perror("execvp grep failed");
        exit(EXIT_FAILURE);
    }

    // In the parent process:
    fprintf(stderr, "(parent_process>created process with id: %d)\n", child2);
    
    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
    close(pipefd[0]); // 7. Close the read end of the pipe, prevents resource leak.

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    // 8. Wait for child processes to terminate
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}