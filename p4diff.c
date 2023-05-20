#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#define BLOCK 64

void system_call_failure(int variable) {
    if (variable == -1) {
        perror("System call failed.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    char *correct_output_file = argv[1], pipe_read[BLOCK], correct_output_read[BLOCK];
    int total_same_bytes = 0;

    FILE *correct_output_fd = fopen(correct_output_file, "r");
    if (correct_output_fd == NULL) return -1;

    // Counts how many same bytes <progname>.out and <progname> 's output have
    while (fgets(pipe_read, BLOCK, stdin) != NULL) {
        if (fgets(correct_output_read, BLOCK, correct_output_fd) == NULL) break;
        for (int i = 0; i < BLOCK && (correct_output_read[i] != '\0' &&
                                      pipe_read[i] != '\0'); i++) {
            if (correct_output_read[i] == pipe_read[i]) total_same_bytes++; 
        }
    }
    fclose(correct_output_fd);

    printf("%d\n", total_same_bytes);

    return 0;
}