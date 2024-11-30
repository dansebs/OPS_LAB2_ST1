#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

volatile sig_atomic_t signal_count = 0; // Shared counter, modified in signal handler

// Signal handler for SIGUSR1
void handle_sigusr1(int sig) {
    signal_count++;
    printf("Signal received: %d, Total count: %d\n", sig, signal_count);
    fflush(stdout); // Ensure output is immediately visible
}

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

    // Set up SIGUSR1 handler
    struct sigaction sa;
    sa.sa_handler = handle_sigusr1; // Set the handler function
    sa.sa_flags = 0;               // No special flags
    sigemptyset(&sa.sa_mask);      // No signals blocked during handler execution

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction");
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
            int s = 100 + rand() % 101; // Random milliseconds in [100-200]
            for (int j = 0; j < 30; j++) {
                usleep(s * 1000); // Sleep for 's' milliseconds
                kill(parent_pid, SIGUSR1); // Send SIGUSR1 to parent
                printf("*");
                fflush(stdout);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process waits for signals
    printf("Parent process waiting for signals (Press Ctrl+C to terminate).\n");
    while (1) {
        pause(); // Wait for a signal
    }

    return EXIT_SUCCESS;
}
