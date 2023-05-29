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

char *myread(char str[BLOCK], int *count_bytes_read, FILE *stream) {
    int check = fread(str, sizeof(char), BLOCK-1, stream);
    system_call_failure(check);
    *count_bytes_read = check;

    return str;
}

int main(int argc, char *argv[]) {
    char *pipe_read = (char *) malloc(sizeof(char) * BLOCK);
    char *expected_read = (char *) malloc(sizeof(char) * BLOCK);
    FILE *expected_out_fd = fopen(argv[1], "r");
    if (expected_out_fd == NULL) return -1;

    int *num_out = (int *) malloc(sizeof(int)), *num_pipe = (int *) malloc(sizeof(int));
    int total_pipe_bytes = 0, total_output_bytes = 0, max, check, matching_bytes = 0;
    *num_out = 0, *num_pipe = 0;

    // Counts how many similar bytes <progname>.out and the pipe have
    while (1) {
        myread(pipe_read, num_pipe, stdin);
        myread(expected_read, num_out, expected_out_fd);
        total_pipe_bytes += *num_pipe;
        total_output_bytes += *num_out;
        if (*num_pipe == 0 && *num_out == 0) break;

        for (int i = 0; i < *num_pipe && i < *num_out; i++) {
            if (pipe_read[i] == expected_read[i]) matching_bytes++;
        }
    }
    check = fclose(expected_out_fd);
    system_call_failure(check);

    max = find_max(total_pipe_bytes, total_output_bytes);
    // printf("total_pipe: %d, total_expected: %d, matching: %d", total_pipe_bytes, total_output_bytes, matching_bytes);
    free(num_out);
    free(num_pipe);

    free(pipe_read);
    free(expected_read);

    if (max == 0) return 100;
    return matching_bytes*100 / max;
}