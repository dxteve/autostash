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
int item_count = 0;
char items[MAX_ITEMS][MAX_PATH];
ItemType item_types[MAX_ITEMS];
int log_fd = -1;
int terminal_running = 0;

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
        check_terminal_status();  // Check terminal before showing menu
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                ensure_log_terminal();  // Ensure terminal is running
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
                ensure_log_terminal();
                add_item(); 
                break;
            case 3:
                ensure_log_terminal();
                remove_item(); 
                break;
            case 4:
                ensure_log_terminal();
                show_settings(); 
                break;
            case 5:
                ensure_log_terminal();
                printf("New interval (seconds): ");
                scanf("%d", &backup_interval);
                ui_log(YELLOW, "[CONFIG] Interval changed to %d seconds\n", backup_interval);
                printf(WHITE "Interval updated.\n" RESET);
                break;
            case 7:
                ensure_log_terminal();
                if (backup_running) {
                    backup_running = 0;
                    printf(WHITE "Stopping background backup... " YELLOW "(Check log terminal)\n" RESET);
                    ui_log(RED, ">>> USER COMMAND: Stop Background Cycle <<<\n");
                } else {
                    printf(RED "No background backup is currently running.\n" RESET);
                }
                break;
            case 8:
                ensure_log_terminal();
                change_directory();
                break;
            case 9:
                ensure_log_terminal();
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
                
                if (log_fd != -1) {
                    ui_log(RED, "\n!!! APPLICATION EXITING !!!\n");
                    sleep(1);
                    close(log_fd);
                }
                
                unlink(PIPE_PATH);
                exit(0);
        }
    }
    return 0;
}