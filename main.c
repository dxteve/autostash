#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "scheduler.h"
#include "ui.h"

pthread_mutex_t lock;
int backup_running = 0;
int backup_interval = 60;
int folder_count = 0;
char folders[MAX_FOLDERS][MAX_PATH];
int log_fd = -1;

int main() {
    pthread_mutex_init(&lock, NULL);
    mkdir("backups", 0755);
    mkfifo(PIPE_PATH, 0666);

    ensure_log_terminal(); // Initial terminal start

    pthread_t sched_thread;
    int choice;

    while (1) {
        show_menu();
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                if (!backup_running) {
                    backup_running = 1;
                    pthread_create(&sched_thread, NULL, scheduler, NULL);
                    printf(WHITE "Backup cycle started. " YELLOW "(See secondary terminal for detailed information)\n" RESET);
                    ui_log(BLUE, ">>> USER COMMAND: Start Backup <<<\n");
                }
                break;
            case 2: add_folder(); break;
            case 3: remove_folder(); break;
            case 4: 
                show_settings();
                printf(WHITE "Settings displayed. " YELLOW "(See secondary terminal for task logs)\n" RESET);
                break;
            case 5:
                printf("New interval: ");
                scanf("%d", &backup_interval);
                printf(WHITE "Interval updated. " YELLOW "(See secondary terminal for confirmation)\n" RESET);
                ui_log(YELLOW, "[CONFIG] Interval changed to %d seconds\n", backup_interval);
                break;
            case 6:
                backup_running = 0;
                printf(RED "Exiting application...\n" RESET);
                close(log_fd);
                unlink(PIPE_PATH);
                exit(0);
            case 7:
                if (backup_running) {
                    backup_running = 0;
                    printf(WHITE "Stopping backup process... " YELLOW "(See secondary terminal for status)\n" RESET);
                    ui_log(RED, ">>> USER COMMAND: Stop Backup <<<\n");
                }
                break;
        }
    }
}