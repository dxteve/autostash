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

    if (task->type == ITEM_FOLDER) {
        // Compress folder
        char zip_path[MAX_PATH + 10];
        snprintf(zip_path, sizeof(zip_path), "%s.zip", task->dest);
        
        ui_log(BLUE, "[ZIPPING] Compressing folder: %s\n", task->source);
        compress_folder(task->source, zip_path);
        ui_log(GREEN, "[SUCCESS] Created: %s.zip\n", task->source);
    } else {
        // Copy file
        char dest_file[MAX_PATH + 10];
        snprintf(dest_file, sizeof(dest_file), "%s", task->dest);
        
        ui_log(BLUE, "[COPYING] Copying file: %s\n", task->source);
        copy_file(task->source, dest_file);
        ui_log(GREEN, "[SUCCESS] Copied: %s\n", task->source);
    }
    
    return NULL;
}

void run_backup_cycle() {
    char absolute_base[MAX_PATH];
    if (getcwd(absolute_base, sizeof(absolute_base)) == NULL) {
        ui_log(RED, "[ERROR] Could not get current working directory.\n");
        return;
    }
    
    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    
    pthread_t threads[MAX_ITEMS];
    BackupTask tasks[MAX_ITEMS];

    pthread_mutex_lock(&lock);
    int current_count = item_count;
    
    if (current_count > 0) {
        play_sound(1);
        ui_log(BLUE, "=== Backup Cycle Started: %s ===\n", timestamp);
        
        char cycle_dir[MAX_PATH];
        snprintf(cycle_dir, MAX_PATH, "%s/backups/%s", absolute_base, timestamp);
        
        mkdir_recursive(cycle_dir);

        for (int i = 0; i < current_count; i++) {
            strncpy(tasks[i].source, items[i], MAX_PATH);
            tasks[i].type = item_types[i];
            
            if (tasks[i].type == ITEM_FOLDER) {
                snprintf(tasks[i].dest, MAX_PATH, "%s/%s", cycle_dir, items[i]);
            } else {
                // For files, preserve the filename
                snprintf(tasks[i].dest, MAX_PATH, "%s/%s", cycle_dir, items[i]);
            }
            
            pthread_create(&threads[i], NULL, backup_thread, &tasks[i]);
        }
        pthread_mutex_unlock(&lock);

        for (int i = 0; i < current_count; i++)
            pthread_join(threads[i], NULL);

        ui_log(GREEN, "=== Backup Cycle Completed ===\n");
        
        // Small delay before playing completion sound to avoid overlap
        usleep(300000); // 300ms delay
        play_sound(2);
    } else {
        pthread_mutex_unlock(&lock);
        ui_log(RED, "[!] No items in list. Add files/folders before backing up.\n");
    }
}

void *scheduler(void *arg) {
    while (backup_running) {
        run_backup_cycle();
        
        if (!backup_running) break; // Exit immediately if stopped during backup
        
        ui_log(YELLOW, "[IDLE] Waiting %d seconds for next cycle...\n", backup_interval);
        
        // Wait a second before playing idle sound to separate from completion sound
        sleep(1);
        play_sound(3);
        
        for (int s = 1; s < backup_interval && backup_running; s++) {
            sleep(1);
        }
    }
    return NULL;
}