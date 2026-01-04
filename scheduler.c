#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "ui.h"
#include "copy_engine.h"
#include "utilities.h"

// Helper to get the absolute path of the 'backups' folder
void get_abs_backup_path(char *buffer) {
    char cwd[MAX_PATH];
    getcwd(cwd, sizeof(cwd));
    snprintf(buffer, MAX_PATH, "%s/backups", cwd);
}

void *backup_thread(void *arg) {
    BackupTask *task = (BackupTask *)arg;
    ui_log(YELLOW, "[COMPRESS] Archiving: %s\n", task->source);

    char zip_path[MAX_PATH + 10];
    snprintf(zip_path, sizeof(zip_path), "%s.zip", task->dest);

    compress_folder(task->source, zip_path);

    ui_log(GREEN, "[SUCCESS] Saved to: %s.zip\n", task->source);
    return NULL;
}

void *scheduler(void *arg) {
    // Store the absolute path to the backups folder once at start
    char abs_backups_base[MAX_PATH];
    getcwd(abs_backups_base, sizeof(abs_backups_base));
    strcat(abs_backups_base, "/backups");

    while (backup_running) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        
        pthread_t threads[MAX_FOLDERS];
        BackupTask tasks[MAX_FOLDERS];

        pthread_mutex_lock(&lock);
        int current_count = folder_count;
        
        if (current_count > 0) {
            ui_log(BLUE, "=== Cycle Initiated: %s ===\n", timestamp);
            
            char cycle_dir[MAX_PATH];
            snprintf(cycle_dir, MAX_PATH, "%s/%s", abs_backups_base, timestamp);
            mkdir_recursive(cycle_dir);

            for (int i = 0; i < current_count; i++) {
                strncpy(tasks[i].source, folders[i], MAX_PATH);
                // Destination is absolute path / backups / timestamp / foldername
                snprintf(tasks[i].dest, MAX_PATH, "%s/%s", cycle_dir, folders[i]);
                
                pthread_create(&threads[i], NULL, backup_thread, &tasks[i]);
            }
            pthread_mutex_unlock(&lock);

            for (int i = 0; i < current_count; i++)
                pthread_join(threads[i], NULL);

            ui_log(GREEN, "=== Cycle Finished (All Clean) ===\n");
        } else {
            pthread_mutex_unlock(&lock);
        }

        ui_log(YELLOW, "[IDLE] Sleeping for %d seconds...\n", backup_interval);
        for (int s = 0; s < backup_interval && backup_running; s++) sleep(1);
    }
    return NULL;
}