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
    
    // Change to Home directory
    const char *home = getenv("HOME");
    if (home) chdir(home);

    // Setup environment
    mkdir("backups", 0755);
    mkfifo(PIPE_PATH, 0666);
    ensure_log_terminal();

    pthread_t sched_thread;
    int choice;

    while (1) {
        show_menu(); // This will now correctly call the one in ui.c
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                if (!backup_running) {
                    backup_running = 1;
                    pthread_create(&sched_thread, NULL, scheduler, NULL);
                    printf("\033[37mBackup started. \033[33m(Check log terminal)\033[0m\n");
                }
                break;
            case 2: add_folder(); break;
            case 3: remove_folder(); break;
            case 4: show_settings(); break;
            case 5:
                printf("New interval: ");
                scanf("%d", &backup_interval);
                break;
            case 6:
                backup_running = 0;
                close(log_fd);
                unlink(PIPE_PATH);
                exit(0);
            case 7:
                if (backup_running) {
                    backup_running = 0;
                    printf("\033[37mStopping backup process...\033[0m\n");
                }
                break;
            case 8:
                change_directory();
                break;
            default:
                printf("\033[31mInvalid choice.\033[0m\n");
                break;
        }
    }
    return 0;
}