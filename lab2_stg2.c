#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
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

    srand(time(NULL));

    pid_t parent_pid = getpid(); // Get parent process ID

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        } else if (pid == 0) { // Child process
            int s = 100 + rand() % 101; // Random sleep time in [100-200] ms
            for (int j = 0; j < 30; j++) {
                usleep(s * 1000); // Sleep for 's' milliseconds
                kill(parent_pid, SIGUSR1); // Send SIGUSR1 to parent
                printf("*"); // Print '*' for each signal sent
                fflush(stdout); // Ensure '*' appears immediately
            }
            exit(EXIT_SUCCESS);
        }
    }

    printf("Parent process exiting before children.\n");
    return EXIT_SUCCESS;
}
