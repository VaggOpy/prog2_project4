#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NAMESIZE 255

int count_occurrences(char *string, char *substring) {
    int count = 0;
    char *position = string;

    while (1) {
        position = strstr(position, substring);
        if (position == NULL) break;
        count++;
        position += sizeof(char) * strlen(substring); 
    }
    return count;
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

    int p1 = fork(), status, check, total_warnings, contains_error;

    char err_out_file[NAMESIZE];
    if (p1 == 0) {
        char testfile_name[NAMESIZE];
        strncpy(testfile_name, testfile_c, strlen(testfile_c)-2);
        testfile_name[strlen(testfile_name)] = '\0';

        strcpy(err_out_file, testfile_name);
        err_out_file[strlen(testfile_name)] = '.';
        err_out_file[strlen(testfile_name)+1] = 'e';
        err_out_file[strlen(testfile_name)+2] = 'r';
        err_out_file[strlen(testfile_name)+3] = 'r';
        err_out_file[strlen(testfile_name)+4] = '\0';

        int error_output_fd = open(err_out_file, O_RDWR|O_CREAT|O_TRUNC, 0644);
        dup2(error_output_fd, STDERR_FILENO);
        close(error_output_fd);

        char *command_args[] = {"-Wall", testfile_c, "-o", testfile_name, NULL};
        execv("gcc", command_args);

        perror("P1 could not create executable.\n");
        return 1;
    }

    waitpid(p1, &status, 0);
    int error_output_fd = open(err_out_file, O_RDWR, 0644);

    struct stat fileStat;
    stat(err_out_file, &fileStat);
    char *error_string = (char *) malloc(sizeof(char) * fileStat.st_size);

    read(error_output_fd, error_string, fileStat.st_size);

    if (strstr("error:", error_string) == != NULL) contains_error = 1; 
    // if (strstr("error:", error_string)) 

    close(error_output_fd);


    return 0;
}