#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

volatile sig_atomic_t signal_count = 0; // Shared counter for SIGUSR1 signals

pid_t *child_pids = NULL; // Array to store child PIDs
int num_children = 0;     // Number of child processes

// Signal handler for SIGUSR1
void handle_sigusr1(int sig) {
    signal_count++;
    printf("Signal received: %d, Total count: %d\n", sig, signal_count);
    fflush(stdout);
}

// Signal handler for SIGCHLD
void handle_sigchld(int sig) {
    int status;
    pid_t pid;

    // Reap all terminated child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Parent: Reaped child %d\n", pid);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of children>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_children = atoi(argv[1]);
    if (num_children <= 0) {
        fprintf(stderr, "Number of children must be greater than 0.\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    child_pids = malloc(num_children * sizeof(pid_t));
    if (!child_pids) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    // Set up SIGUSR1 handler
    struct sigaction sa_usr1;
    sa_usr1.sa_handler = handle_sigusr1;
    sa_usr1.sa_flags = 0;
    sigemptyset(&sa_usr1.sa_mask);
    if (sigaction(SIGUSR1, &sa_usr1, NULL) < 0) {
        perror("sigaction");
        free(child_pids);
        return EXIT_FAILURE;
    }

    // Set up SIGCHLD handler
    struct sigaction sa_chld;
    sa_chld.sa_handler = handle_sigchld;
    sa_chld.sa_flags = SA_RESTART; // Restart interrupted system calls
    sigemptyset(&sa_chld.sa_mask);
    if (sigaction(SIGCHLD, &sa_chld, NULL) < 0) {
        perror("sigaction");
        free(child_pids);
        return EXIT_FAILURE;
    }

    // Fork children
    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(child_pids);
            return EXIT_FAILURE;
        } else if (pid == 0) { // Child process
            // Set up SIGUSR2 handler in child
            struct sigaction sa_usr2;
            sa_usr2.sa_handler = handle_sigusr1; // Reuse SIGUSR1 handler to terminate
            sa_usr2.sa_flags = 0;
            sigemptyset(&sa_usr2.sa_mask);
            if (sigaction(SIGUSR2, &sa_usr2, NULL) < 0) {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            int s = 100 + rand() % 101; // Random milliseconds in [100-200]
            while (1) {
                usleep(s * 1000); // Sleep for 's' milliseconds
                kill(getppid(), SIGUSR1); // Send SIGUSR1 to parent

            }
        } else { // Parent process
            child_pids[i] = pid; // Store child PID
        }
    }

    // Parent process: Wait for 100 SIGUSR1 signals
    printf("Parent process waiting for 100 SIGUSR1 signals...\n");
    while (signal_count < 100) {
        pause(); // Wait for signals
    }

    // Send SIGUSR2 to all children
    printf("\nParent: 100 signals received, terminating children.\n");
    for (int i = 0; i < num_children; i++) {
        kill(child_pids[i], SIGUSR2);
    }

    // Free allocated memory
    free(child_pids);

    printf("Parent: All children terminated. Exiting.\n");
    return EXIT_SUCCESS;
}
