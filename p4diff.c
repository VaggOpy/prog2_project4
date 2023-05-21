#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK 64

void system_call_failure(int variable) {
    if (variable == -1) {
        perror("System call failed.\n");
        exit(1);
    }
}

int find_max(int num1, int num2) {
    if (num1 > num2) return num1;
    return num2;
}

char *myread(char str[BLOCK], int **count_bytes_read, FILE *stream) {
    int check = fread(str, BLOCK, BLOCK, stream);
    *count_bytes_read += check;

    if (check < BLOCK) str[check] = '\0';
    if (check == 0) {
        char new[BLOCK] = {'\0'};
        strcpy(str, new);
    }

    return str;
}

int main(int argc, char *argv[]) {
    char pipe_read[BLOCK], expected_read[BLOCK], str1[BLOCK], str2[BLOCK];
    int *total_expected_bytes = (int *) malloc(sizeof(int)), matching_bytes = 0;
    int *total_pipe_bytes = (int *) malloc(sizeof(int)), max;
    *total_expected_bytes = 0, *total_pipe_bytes = 0;

    FILE *expected_out_fd = fopen(argv[1], "r");
    if (expected_out_fd == NULL) return -1;

    // Counts how many similar bytes <progname>.out and the pipe have
    while (1) {
        strcpy(pipe_read, myread(str1, &total_pipe_bytes, stdin));
        strcpy(expected_read, myread(str2, &total_expected_bytes, expected_out_fd));

        printf("%s %s", pipe_read, expected_read);
        for (int i = 0; i < BLOCK && (pipe_read[i] != '\0'); i++) {
            if (pipe_read[i] == expected_read[i]) matching_bytes++; 
        }
    }
    fclose(expected_out_fd);

    max = find_max(*total_expected_bytes, *total_pipe_bytes);
    free(total_expected_bytes);
    free(total_pipe_bytes);

    // printf("mybytes:%d expected_bytes:%d same:%d max:%d\n", total_pipe_bytes, total_expected_bytes, matching_bytes, max);
    if (max == 0) return 0;
    return matching_bytes*100 / max;
}