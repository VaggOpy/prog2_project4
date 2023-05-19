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

    int status, check;
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

        execlp("gcc", "gcc", "-Wall", testfile_c, "-o", testfile_name, NULL);

        // execv failed
        free(err_out_file);
        perror("P1 could not create executable.\n");
        exit(2); // terminate child
    }

    waitpid(p1, &status, 0);
    if (WIFEXITED(status) == 2) {
        free(err_out_file);
        printf("child failed to create executable\n");
        return 0;
    }
    int error_output_fd = open(err_out_file, O_RDWR, 0644);

    struct stat fileinfo;
    stat(err_out_file, &fileinfo);
    char *error_string = (char *) malloc(sizeof(char) * (fileinfo.st_size + 1));
    memset(error_string, '\0', fileinfo.st_size+1);
    read(error_output_fd, error_string, fileinfo.st_size);
    close(error_output_fd);

    if (strstr(error_string, " error: ") != NULL) {
        compilation = -100;
    }
    else { // compilation successful
        compilation += count_occurrences(error_string, " warning: ") * -5;
    }
    free(error_string);
    free(err_out_file);

    printf("%d\n", compilation);

    return 0;
}