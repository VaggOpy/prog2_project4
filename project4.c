#include <stdio.h>
#include <stdlib.h>

// #define NAMESIZE 255

int main(int argc, char *argv[]) {
    if (argc != 6) return 1;    

    char testfile[] = argv[1];
    char testfile_args[] = argv[2];
    char testfile_input[] = argv[3];
    char testfile_output[] = argv[4];
    unsigned int timeout = atoi(argv[5]);


    return 0;
}