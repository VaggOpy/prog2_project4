#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>

#define NAMESIZE 255

int count_occurrences(char *string, char substring[]) {
    int count = 0;
    char *position = string;

    while (1) {
        position = strstr(position, substring);
        if (position == NULL) return count;
        count++;
        position += sizeof(char) * strlen(substring); 
    }
}

void system_call_failure(int variable) {
    if (variable == -1) {
        perror("System call failed.\n");
        exit(1);
    }
}

pid_t p2;

void time_handler(int signum) {
    kill(p2, SIGKILL);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        perror("Incorrect number of arguments.\n");
        return 1; 
    }   

    char testfile_c[NAMESIZE], testfile_args[NAMESIZE], testfile_name[NAMESIZE];
    char testfile_input[NAMESIZE], testfile_output[NAMESIZE];
    strcpy(testfile_c, argv[1]);
    strcpy(testfile_args, argv[2]);
    strcpy(testfile_input, argv[3]);
    strcpy(testfile_output, argv[4]);
    unsigned int timeout = atoi(argv[5]);

    int status, check, compilation_successful, compilation = 0, score;
    int termination = 0, in_out_difference = 0, memory_access = 0;

    strncpy(testfile_name, testfile_c, strlen(testfile_c)-2);
    testfile_name[strlen(testfile_c)-2] = '\0';

    char *err_out_file = (char *) malloc(sizeof(char) * (strlen(testfile_c) + 3));
    strcpy(err_out_file, testfile_name);
    err_out_file[strlen(testfile_name)] = '.';
    err_out_file[strlen(testfile_name)+1] = 'e';
    err_out_file[strlen(testfile_name)+2] = 'r';
    err_out_file[strlen(testfile_name)+3] = 'r';
    err_out_file[strlen(testfile_name)+4] = '\0';

    int p1 = fork();
    if (p1 == 0) {
        int error_output_fd = open(err_out_file, O_RDWR|O_CREAT|O_TRUNC, 0644);
        system_call_failure(error_output_fd);
        check = dup2(error_output_fd, STDERR_FILENO);
        system_call_failure(check);
        check = close(error_output_fd);
        system_call_failure(check);

        // if execlp is succesful the child process dies
        execlp("gcc", "gcc", "-Wall", testfile_c, "-o", testfile_name, NULL);
        exit(0);
    }

    waitpid(p1, &status, 0);
    int error_output_fd = open(err_out_file, O_RDWR, 0644);
    system_call_failure(error_output_fd);

    struct stat fileinfo;
    stat(err_out_file, &fileinfo);
    char *error_string = (char *) malloc(sizeof(char) * (fileinfo.st_size + 1));
    memset(error_string, '\0', fileinfo.st_size+1);

    check = read(error_output_fd, error_string, fileinfo.st_size);
    system_call_failure(check);
    check = close(error_output_fd);
    system_call_failure(check);
    free(err_out_file);

    if (strstr(error_string, " error: ") != NULL) {
        compilation_successful = 0;
        compilation = -100;
    }
    else { // compilation successful
        compilation_successful = 1;
        compilation += count_occurrences(error_string, " warning: ") * -5;
    }
    free(error_string);

    if (compilation_successful) {
        int fd[2];
        pipe(fd);

        p2 = fork();
        if (p2 == 0) {
            int testfile_input_fd = open(testfile_input, O_RDWR, 0644);
            system_call_failure(testfile_input_fd);
            check = dup2(testfile_input_fd, STDIN_FILENO);
            system_call_failure(check);
            check = close(testfile_input_fd);
            system_call_failure(check);
            
            check = dup2(fd[1], STDOUT_FILENO);
            system_call_failure(check);
            check = close(fd[0]);
            system_call_failure(check);
            check = close(fd[1]);
            system_call_failure(check);

            int testfile_args_fd = open(testfile_args, O_RDWR, 0644);
            system_call_failure(testfile_args_fd);
            int total_args = 0, letter = 0;
            char **args = (char **) malloc(sizeof(char *)), character;
            args[0] = (char *) malloc(sizeof(char));

            // moves every argument from progname.args to char **args
            while (1) {
                check = read(testfile_args_fd, &character, sizeof(char));
                system_call_failure(check);
                if (check == 0) break; 

                if (character == ' ') {
                    args[total_args][letter] = '\0';
                    total_args++;
                    letter = 0;
                    char **args1 = (char **) realloc(args, sizeof(char *) * (total_args+1));
                    if (args1 == NULL) exit(2);
                    args = args1;

                    args[total_args] = (char *) malloc(sizeof(char));
                }
                else if (character == '\n') break;

                args[total_args][letter] = character;
                letter++;
                args[total_args] = (char *) realloc(args[total_args], sizeof(char) * (letter+1));
            }
            args[total_args][letter] = '\0';

            // if execv is succesful i don't need to free(args) because p2 dies and memory gets freed
            execv(testfile_name, args);

            perror("Execv failed.\n");
            exit(0);
        }

        int p3 = fork();
        if (p3 == 0) {
            check = dup2(fd[0], STDIN_FILENO);
            system_call_failure(check);
            check = close(fd[0]);
            system_call_failure(check);
            check = close(fd[1]);
            system_call_failure(check);

            execlp("./p4diff", "./p4diff", testfile_output, NULL);

            perror("Execlp failed.\n");
            exit(0);
        }

        struct sigaction action = {{0}};
        struct itimerval timer = {{0}};

        action.sa_handler = time_handler;
        sigaction(SIGALRM, &action, NULL);

        timer.it_value.tv_sec = timeout;       
        timer.it_value.tv_usec = 0;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;

        if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
            perror("setitimer");
            exit(EXIT_FAILURE);
        }

        signal(SIGALRM, time_handler);
        
        check = close(fd[0]);
        system_call_failure(check);
        check = close(fd[1]);
        system_call_failure(check);
        waitpid(p2, &status, 0);

        // Shut down timer
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);

        if (WTERMSIG(status) == SIGKILL) termination = -100;
        else if (WTERMSIG(status) == SIGSEGV || WTERMSIG(status) == SIGABRT || 
            WTERMSIG(status) == SIGBUS) memory_access = -15;
        waitpid(p3, &status, 0);
    }
    if (WIFEXITED(status) && compilation_successful) in_out_difference = WEXITSTATUS(status);

    score = compilation + termination + in_out_difference + memory_access;
    if (score < 0) score = 0;

    printf("\nCompilation: %d\n\nTermination: %d\n\nOutput: %d\n\nMemory access: %d\n\nScore: %d\n", 
        compilation, termination, in_out_difference, memory_access, score);

    return 0;
}