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
    char *character = malloc(sizeof(char));
    int check, i;

    for (i = 0; i < BLOCK; i++) {
        check = fread(character, sizeof(char), 1, stream);
        if (check == 0) break;
        if (*character == '\r') {
            i--;
            continue;
        }

        str[i] = *character;
        (*count_bytes_read)++;
    }

    if (i <= BLOCK) str[i] = '\0';
    if (i == 0) {
        char new[BLOCK+1] = {'\0'};
        strcpy(str, new);
    }

    free(character);
    return str;
}

int main(int argc, char *argv[]) {
    char pipe_read[BLOCK+1], expected_read[BLOCK+1], str1[BLOCK+1], str2[BLOCK+1];
    FILE *expected_out_fd = fopen(argv[1], "r");
    if (expected_out_fd == NULL) return -1;

    int *total_expected_bytes = (int *) malloc(sizeof(int)), matching_bytes = 0;
    int *total_pipe_bytes = (int *) malloc(sizeof(int)), max;
    *total_expected_bytes = 0, *total_pipe_bytes = 0;

    // Counts how many similar bytes <progname>.out and the pipe have
    while (1) {
        strcpy(pipe_read, myread(str1, total_pipe_bytes, stdin));
        strcpy(expected_read, myread(str2, total_expected_bytes, expected_out_fd));
        if (pipe_read[0] == '\0') break;

        for (int i = 0; i < BLOCK && (pipe_read[i] != '\0'); i++) {
            if (pipe_read[i] == expected_read[i]) matching_bytes++; 
        }
    }
    fclose(expected_out_fd);

    max = find_max(*total_expected_bytes, *total_pipe_bytes);
    // printf("mybytes:%d expected_bytes:%d same:%d max:%d\n", *total_pipe_bytes, *total_expected_bytes, matching_bytes, max);
    free(total_expected_bytes);
    free(total_pipe_bytes);

    if (max == 0) return 0;
    return matching_bytes*100 / max;
}