#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of children>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Number of children must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        } else if (pid == 0) { // Child process
            int s = 100 + rand() % 101; // Random milliseconds in [100-200]
            printf("Child %d: Random sleep time = %d ms\n", getpid(), s);
            usleep(s * 1000); // Convert ms to us and sleep
            exit(EXIT_SUCCESS); // Child exits after printing
        }
        // Parent continues the loop to create the next child
    }

    printf("Parent process exiting after forking all children.\n");
    return EXIT_SUCCESS;
}
