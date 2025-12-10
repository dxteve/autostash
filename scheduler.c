#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "config.h"
#include "utilities.h"
#include "copy_engine.h"
#include "scheduler.h"

/* ---------- THREAD ---------- */

void *backup_thread(void *arg)
{
    BackupTask *task = (BackupTask *)arg;

    pthread_mutex_lock(&lock);
    printf(BLUE "Backing up: %s\n" RESET, task->source);
    pthread_mutex_unlock(&lock);

    copy_folder(task->source, task->dest);

    pthread_mutex_lock(&lock);
    printf(BLUE "Completed: %s\n" RESET, task->source);
    pthread_mutex_unlock(&lock);
    return NULL;
}

/* ---------- SCHEDULER ---------- */

void *scheduler(void *arg)
{
    while (backup_running)
    {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));

        pthread_t threads[MAX_FOLDERS];
        BackupTask tasks[MAX_FOLDERS];

        printf(BLUE "\n=== Backup Cycle (%s) ===\n" RESET, timestamp);

        for (int i = 0; i < folder_count; i++)
        {
            snprintf(tasks[i].source, MAX_PATH, "%s", folders[i]);

            char base[MAX_PATH];
            snprintf(base, MAX_PATH, "backups/%s", timestamp);
            mkdir_recursive(base);

            snprintf(tasks[i].dest, MAX_PATH, "%s/%s", base, folders[i]);
            mkdir_recursive(tasks[i].dest);

            pthread_create(&threads[i], NULL, backup_thread, &tasks[i]);
        }

        for (int i = 0; i < folder_count; i++)
            pthread_join(threads[i], NULL);

        printf(BLUE "Next backup in %d seconds...\n" RESET, backup_interval);
        sleep(backup_interval);
    }
    return NULL;
}