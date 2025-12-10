#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>

#include "config.h"
#include "scheduler.h"
#include "ui.h"

/* ---------- GLOBALS DEFINITION (remains here) ---------- */

pthread_mutex_t lock;
int backup_running = 0;
int backup_interval = 60;
int folder_count = 0;

char folders[MAX_FOLDERS][MAX_PATH];

/* ---------- MAIN ---------- */

int main()
{
    pthread_mutex_init(&lock, NULL);
    mkdir("backups", 0755);

    pthread_t sched_thread;
    int choice;

    while (1)
    {
        show_menu();
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            if (!backup_running)
            {
                backup_running = 1;
                pthread_create(&sched_thread, NULL, scheduler, NULL);
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
            break;

        case 6:
            backup_running = 0;
            pthread_mutex_destroy(&lock);
            exit(0);

        default:
            printf(RED "Invalid choice\n" RESET);
        }
    }
}