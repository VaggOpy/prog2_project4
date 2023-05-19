#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

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

int main(int argc, char *argv[]) {
    if (argc != 6) {
        perror("Incorrect number of arguments.\n");
        return 1; 
    }   

    char testfile_c[NAMESIZE], testfile_args[NAMESIZE];
    char testfile_input[NAMESIZE], testfile_output[NAMESIZE];
    strcpy(testfile_c, argv[1]);
    strcpy(testfile_args, argv[2]);
    strcpy(testfile_input, argv[3]);
    strcpy(testfile_output, argv[4]);
    unsigned int timeout = atoi(argv[5]);

    int status, check, compilation_successful;
    int compilation = 0, termination = 0, in_out_difference = 0, memory_access = 0;

    char testfile_name[NAMESIZE];
    strncpy(testfile_name, testfile_c, strlen(testfile_c)-2);
    testfile_name[strlen(testfile_name)] = '\0';

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
        dup2(error_output_fd, STDERR_FILENO);
        close(error_output_fd);

        // if execlp is succesful the child process dies
        execlp("gcc", "gcc", "-Wall", testfile_c, "-o", testfile_name, NULL);
    }

    waitpid(p1, &status, 0);
    int error_output_fd = open(err_out_file, O_RDWR, 0644);

    struct stat fileinfo;
    stat(err_out_file, &fileinfo);
    char *error_string = (char *) malloc(sizeof(char) * (fileinfo.st_size + 1));
    memset(error_string, '\0', fileinfo.st_size+1);

    read(error_output_fd, error_string, fileinfo.st_size);
    close(error_output_fd);
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

        int p2 = fork();
        if (p2 == 0) {
            int testfile_input_fd = open(testfile_input, O_RDWR, 0644);
            dup2(testfile_input_fd, STDIN_FILENO);
            close(testfile_input_fd);
            
            // dup2(fd[1], STDOUT_FILENO);

            int testfile_args_fd = open(testfile_args, O_RDWR, 0644);
            int total_args = 0, letter = 0;
            char **args = (char **) malloc(sizeof(char *)), character;
            args[0] = (char *) malloc(sizeof(char));

            // moves every argument from progname.args to char **args
            while (1) {
                check = read(testfile_args_fd, &character, sizeof(char));
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
        }

        int p3 = fork();
        if (p3 == 0) {
            dup2(fd[0], STDIN_FILENO);

        }

        waitpid(p2, &status, 0);
        waitpid(p3, &status, 0);
        close(fd[0]);
        close(fd[1]);


    }

    return 0;
}