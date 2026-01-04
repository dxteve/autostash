#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "ui.h"
#include "copy_engine.h"
#include "utilities.h"

void *backup_thread(void *arg) {
    BackupTask *task = (BackupTask *)arg;

    // Directly compressing from source to the destination zip path
    // We append .zip to the destination name
    char zip_path[MAX_PATH + 10];
    snprintf(zip_path, sizeof(zip_path), "%s.zip", task->dest);

    ui_log(BLUE, "[ZIPPING] Compressing: %s\n", task->source);
    
    // compress_folder now takes the live source and creates the zip directly in the backup folder
    compress_folder(task->source, zip_path);

    ui_log(GREEN, "[SUCCESS] Created: %s.zip\n", task->source);
    return NULL;
}

void run_backup_cycle() {
    char absolute_base[MAX_PATH];
    // Use an absolute path for the backups directory to prevent issues when CWD changes
    if (getcwd(absolute_base, sizeof(absolute_base)) == NULL) {
        ui_log(RED, "[ERROR] Could not get current working directory.\n");
        return;
    }
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    
    pthread_t threads[MAX_FOLDERS];
    BackupTask tasks[MAX_FOLDERS];

    pthread_mutex_lock(&lock);
    int current_count = folder_count;
    
    if (current_count > 0) {
        ui_log(BLUE, "=== Backup Cycle Started: %s ===\n", timestamp);
        
        char cycle_dir[MAX_PATH];
        snprintf(cycle_dir, MAX_PATH, "%s/backups/%s", absolute_base, timestamp);
        
        // Create the directory for this specific timestamp
        mkdir_recursive(cycle_dir);

        for (int i = 0; i < current_count; i++) {
            strncpy(tasks[i].source, folders[i], MAX_PATH);
            // The destination task is the base name of the zip file inside the cycle_dir
            snprintf(tasks[i].dest, MAX_PATH, "%s/%s", cycle_dir, folders[i]);
            pthread_create(&threads[i], NULL, backup_thread, &tasks[i]);
        }
        pthread_mutex_unlock(&lock);

        // Wait for all backup threads in this cycle to finish
        for (int i = 0; i < current_count; i++)
            pthread_join(threads[i], NULL);

        ui_log(GREEN, "=== Backup Cycle Completed ===\n");
    } else {
        pthread_mutex_unlock(&lock);
        ui_log(RED, "[!] No folders in list. Add folders before backing up.\n");
    }
}

void *scheduler(void *arg) {
    while (backup_running) {
        run_backup_cycle();
        
        // Break sleep into 1-second increments to remain responsive to the 'Stop' command (Choice 7)
        ui_log(YELLOW, "[IDLE] Waiting %d seconds for next cycle...\n", backup_interval);
        for (int s = 0; s < backup_interval && backup_running; s++) {
            sleep(1);
        }
    }
    return NULL;
}