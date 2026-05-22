#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "LineParser.h"

#define MAX_INPUT_SIZE 2048

int debug_mode = 0;

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process {
    cmdLine* cmd;
    pid_t pid;
    int status;
    struct process *next;
} process;

process* process_list = NULL; 

// Add a new process
void addProcess(process** process_list, cmdLine* cmd, pid_t pid) {
    process* new_process = (process*)malloc(sizeof(process));
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}

// Free process list memory
void freeProcessList(process* process_list) {
    process* current = process_list;
    while (current != NULL) {
        process* next = current->next;
        freeCmdLines(current->cmd); 
        free(current);
        current = next;
    }
}

// Update specific process status
void updateProcessStatus(process* process_list, int pid, int status) {
    process* current = process_list;
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status;
            return;
        }
        current = current->next;
    }
}

// Update all statuses using WNOHANG
void updateProcessList(process **process_list) {
    process* current = *process_list;
    while (current != NULL) {
        int status;
        pid_t res = waitpid(current->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        
        if (res > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                updateProcessStatus(*process_list, current->pid, TERMINATED);
            } else if (WIFSTOPPED(status)) {
                updateProcessStatus(*process_list, current->pid, SUSPENDED);
            } else if (WIFCONTINUED(status)) {
                updateProcessStatus(*process_list, current->pid, RUNNING);
            }
        } else if (res == -1) {
            updateProcessStatus(*process_list, current->pid, TERMINATED);
        }
        
        current = current->next;
    }
}

// Print list and remove terminated processes
void printProcessList(process** process_list) {
    updateProcessList(process_list); 
    
    printf("PID\t\tCommand\t\tSTATUS\n");
    
    process* current = *process_list;
    process* prev = NULL;
    
    while (current != NULL) {
        char* status_str;
        if (current->status == TERMINATED) status_str = "Terminated";
        else if (current->status == RUNNING) status_str = "Running";
        else if (current->status == SUSPENDED) status_str = "Suspended";
        else status_str = "Unknown";
        
        printf("%d\t\t%s\t\t%s\n", current->pid, current->cmd->arguments[0], status_str);
        
        if (current->status == TERMINATED) {
            process* to_delete = current;
            if (prev == NULL) {
                *process_list = current->next;
            } else {
                prev->next = current->next;
            }
            current = current->next;
            freeCmdLines(to_delete->cmd);
            free(to_delete);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void handle_input_redirect(const char* input_file) {
    if (input_file != NULL) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in == -1) {
            perror("open input failed");
            _exit(1);
        }
        if (dup2(fd_in, STDIN_FILENO) == -1) {
            perror("dup2 input failed");
            _exit(1);
        }
        close(fd_in);
    }
}

void handle_output_redirect(const char* output_file) {
    if (output_file != NULL) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out == -1) {
            perror("open output failed");
            _exit(1);
        }
        if (dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("dup2 output failed");
            _exit(1);
        }
        close(fd_out);
    }
}

void execute(cmdLine *pCmdLine) {
    // Built-in commands
    if (strcmp(pCmdLine->arguments[0], "cd") == 0) {
        if (chdir(pCmdLine->arguments[1]) == -1) {
            perror("cd failed");
        }
        freeCmdLines(pCmdLine);
        return;
    }
    
    // Process Manager command
    if (strcmp(pCmdLine->arguments[0], "procs") == 0) {
        printProcessList(&process_list);
        freeCmdLines(pCmdLine);
        return;
    }

    // Signal commands with status updates
    if (strcmp(pCmdLine->arguments[0], "stop") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGSTOP) == -1) {
            perror("kill failed");
        } else {
            updateProcessStatus(process_list, pid, SUSPENDED);
        }
        freeCmdLines(pCmdLine);
        return;
    }
    if (strcmp(pCmdLine->arguments[0], "wakeup") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGCONT) == -1) {
            perror("kill failed");
        } else {
            updateProcessStatus(process_list, pid, RUNNING);
        }
        freeCmdLines(pCmdLine);
        return;
    }
    if (strcmp(pCmdLine->arguments[0], "ice") == 0) {
        int pid = atoi(pCmdLine->arguments[1]);
        if (kill(pid, SIGINT) == -1) {
            perror("kill failed");
        } else {
            updateProcessStatus(process_list, pid, TERMINATED);
        }
        freeCmdLines(pCmdLine);
        return;
    }
    if (strcmp(pCmdLine->arguments[0], "nuke") == 0) {
        pid_t pgid = atoi(pCmdLine->arguments[1]);
        if (kill(-pgid, SIGKILL) == -1) perror("kill failed");
        freeCmdLines(pCmdLine);
        return;
    }

    // Pipeline Logic
    if (pCmdLine->next != NULL) {
        // Validation: Check for I/O conflicts
        if (pCmdLine->outputRedirect != NULL) {
            fprintf(stderr, "Error: ambiguous output redirect for left-hand side process\n");
            return;
        }
        if (pCmdLine->next->inputRedirect != NULL) {
            fprintf(stderr, "Error: ambiguous input redirect for right-hand side process\n");
            return;
        }

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            return;
        }

        pid_t child1 = fork();
        if (child1 == -1) {
            perror("fork 1 failed");
            return;
        }

        if (child1 == 0) {
            // Left child process
            handle_input_redirect(pCmdLine->inputRedirect);
            close(STDOUT_FILENO);
            dup(pipefd[1]);
            close(pipefd[1]);
            close(pipefd[0]);

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                perror("Execution failed");
                _exit(1);
            }
        }

        // Parent process: add left child to process list
        addProcess(&process_list, pCmdLine, child1);
        close(pipefd[1]); // Close write-end in parent

        pid_t child2 = fork();
        if (child2 == -1) {
            perror("fork 2 failed");
            return;
        }

        if (child2 == 0) {
            // Right child process
            handle_output_redirect(pCmdLine->next->outputRedirect);
            close(STDIN_FILENO);
            dup(pipefd[0]);
            close(pipefd[0]);

            if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) == -1) {
                perror("Execution failed");
                _exit(1);
            }
        }

        // Parent process: add right child to process list
        addProcess(&process_list, pCmdLine->next, child2);
        close(pipefd[0]); 

        // Wait for both children if the pipeline is blocking
        if (pCmdLine->blocking == 1) {
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);
        }

    } else {
        // Single Command Logic
        pid_t pid = fork();

        if (pid == 0) {
            if (debug_mode) {
                fprintf(stderr, "PID: %d\n", getpid());
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
                if (pCmdLine->blocking == 1) {
                    fprintf(stderr, "Execution status: Foreground\n");
                } else {
                    fprintf(stderr, "Execution status: Background\n");
                }
            }

            handle_input_redirect(pCmdLine->inputRedirect);
            handle_output_redirect(pCmdLine->outputRedirect);

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                perror("Execution failed");
                _exit(1);
            }
        } else if (pid > 0) {
            // Parent process: add single command to process list
            addProcess(&process_list, pCmdLine, pid);
            
            // Parent waits only if the command is running in the foreground
            if (pCmdLine->blocking == 1) {
                waitpid(pid, NULL, 0);
            }
        } else {
            perror("fork failed");
        }
    }
}

int main(int argc, char **argv) {
    char cwd[PATH_MAX];
    char input[MAX_INPUT_SIZE];
    cmdLine *parsedLine;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            debug_mode = 1;
            break;
        }
    }

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            perror("getcwd failed");
            exit(1);
        }

        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0) {
            break;
        }

        if (strlen(input) == 0) {
            continue;
        }

        parsedLine = parseCmdLines(input);
        if (parsedLine != NULL) {
            execute(parsedLine);
        }
    }

    freeProcessList(process_list);
    return 0;
}