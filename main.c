#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "scheduler.h"
#include "ui.h"

/* ---------- GLOBALS DEFINITION ---------- */
pthread_mutex_t lock;
int backup_running = 0;
int backup_interval = 60;
int folder_count = 0;
char folders[MAX_FOLDERS][MAX_PATH];
int log_fd = -1;

int main() {
    pthread_mutex_init(&lock, NULL);
    
    // Change to Home directory on startup
    const char *home = getenv("HOME");
    if (home) chdir(home);

    // Initialize environment
    mkdir("backups", 0755);
    mkfifo(PIPE_PATH, 0666);
    ensure_log_terminal();

    pthread_t sched_thread;
    int choice;

    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // Clear input buffer
            continue;
        }

        switch (choice) {
            case 1:
                if (!backup_running) {
                    backup_running = 1;
                    pthread_create(&sched_thread, NULL, scheduler, NULL);
                    printf(WHITE "Background backup cycle started. " YELLOW "(See secondary terminal)\n" RESET);
                    ui_log(BLUE, ">>> USER COMMAND: Start Background Cycle <<<\n");
                } else {
                    printf(RED "Backup cycle is already running.\n" RESET);
                }
                break;
            case 2: 
                add_folder(); 
                break;
            case 3: 
                remove_folder(); 
                break;
            case 4: 
                show_settings(); 
                break;
            case 5:
                printf("New interval (seconds): ");
                scanf("%d", &backup_interval);
                ui_log(YELLOW, "[CONFIG] Interval changed to %d seconds\n", backup_interval);
                printf(WHITE "Interval updated.\n" RESET);
                break;
            case 7:
                if (backup_running) {
                    backup_running = 0;
                    printf(WHITE "Stopping background backup... " YELLOW "(Check log terminal)\n" RESET);
                    ui_log(RED, ">>> USER COMMAND: Stop Background Cycle <<<\n");
                } else {
                    printf(RED "No background backup is currently running.\n" RESET);
                }
                break;
            case 8:
                change_directory();
                break;
            case 9:
                if (backup_running) {
                    printf(RED "Error: Cannot run manual backup while background cycle is active.\n" RESET);
                } else {
                    printf(WHITE "Starting one-time backup... " YELLOW "(See secondary terminal)\n" RESET);
                    run_backup_cycle();
                    printf(GREEN "One-time backup completed.\n" RESET);
                }
                break;
            case 0:
                backup_running = 0;
                printf(RED "Exiting application. Cleaning up...\n" RESET);
                
                // Inform the secondary terminal
                if (log_fd != -1) {
                    ui_log(RED, "\n!!! APPLICATION EXITING !!!\n");
                    sleep(1); // Give time for the message to display
                    close(log_fd);
                }
                
                unlink(PIPE_PATH);
                exit(0);
        }
    }
    return 0;
}