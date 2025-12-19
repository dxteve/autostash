#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "config.h"
#include "ui.h"
#include "copy_engine.h"
#include "utilities.h"

void *backup_thread(void *arg) {
    BackupTask *task = (BackupTask *)arg;
    ui_log(YELLOW, "[THREAD] Starting backup: %s\n", task->source);

    copy_folder(task->source, task->dest);

    ui_log(GREEN, "[SUCCESS] Finished backing up: %s\n", task->source);
    return NULL;
}

void *scheduler(void *arg) {
    while (backup_running) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        
        pthread_t threads[MAX_FOLDERS];
        BackupTask tasks[MAX_FOLDERS];

        pthread_mutex_lock(&lock);
        int current_count = folder_count;
        if (current_count > 0) {
            ui_log(BLUE, "=== Cycle Initiated: %s ===\n", timestamp);
            for (int i = 0; i < current_count; i++) {
                snprintf(tasks[i].source, MAX_PATH, "%s", folders[i]);
                char base[MAX_PATH];
                snprintf(base, MAX_PATH, "backups/%s", timestamp);
                mkdir_recursive(base);
                snprintf(tasks[i].dest, MAX_PATH, "%s/%s", base, folders[i]);
                mkdir_recursive(tasks[i].dest);
                pthread_create(&threads[i], NULL, backup_thread, &tasks[i]);
            }
            pthread_mutex_unlock(&lock);

            for (int i = 0; i < current_count; i++)
                pthread_join(threads[i], NULL);

            ui_log(GREEN, "=== Cycle Completed Successfully ===\n");
        } else {
            pthread_mutex_unlock(&lock);
            ui_log(RED, "[IDLE] No folders found for backup.\n");
        }

        ui_log(YELLOW, "[WAIT] Next backup cycle in %d seconds...\n", backup_interval);
        for (int s = 0; s < backup_interval && backup_running; s++) sleep(1);
    }
    return NULL;
}