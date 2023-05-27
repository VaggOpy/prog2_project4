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

char *myread(char str[BLOCK+1], int *count_bytes_read, FILE *stream) {
    for (int i = 0; i <= BLOCK; i++) str[i] = '\0';

    int check = fread(str, sizeof(char), BLOCK, stream);
    system_call_failure(check);

    *count_bytes_read += check;
    str[check] = '\0';

    return str;
}

int main(int argc, char *argv[]) {
    char pipe_read[BLOCK+1], expected_read[BLOCK+1], str1[BLOCK+1], str2[BLOCK+1];
    FILE *expected_out_fd = fopen(argv[1], "r");
    if (expected_out_fd == NULL) return -1;

    int *total_expected_bytes = (int *) malloc(sizeof(int)), matching_bytes = 0;
    int *total_pipe_bytes = (int *) malloc(sizeof(int)), max, check;
    *total_expected_bytes = 0, *total_pipe_bytes = 0;

    // Counts how many similar bytes <progname>.out and the pipe have
    while (1) {
        for (int i = 0; i <= BLOCK; i++) {
            pipe_read[i] = '\0';
            expected_read[i] = '\0';
        }

        strcpy(pipe_read, myread(str1, total_pipe_bytes, stdin));
        strcpy(expected_read, myread(str2, total_expected_bytes, expected_out_fd));
        if (pipe_read[0] == '\0' && expected_read[0] == '\0') break;

        for (int i = 0; i <= BLOCK && pipe_read[i] != '\0'; i++) {
            if (pipe_read[i] == expected_read[i]) {
                matching_bytes++;
            }
        }
    }
    check = fclose(expected_out_fd);
    system_call_failure(check);

    max = find_max(*total_expected_bytes, *total_pipe_bytes);
    // printf("total_pipe: %d, total_expected: %d, matching: %d", *total_pipe_bytes, *total_expected_bytes, matching_bytes);
    free(total_expected_bytes);
    free(total_pipe_bytes);

    if (max == 0) return 100;
    return matching_bytes*100 / max;
}