#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "config.h"
#include "ui.h"

void ensure_log_terminal() {
    // Attempt to open the pipe in non-blocking mode to check for a reader
    int test_fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK);
    
    if (test_fd == -1 && errno == ENXIO) {
        // ENXIO means no process has the pipe open for reading (terminal closed)
        system("gnome-terminal -- bash -c \"echo '--- AUTOSTASH LOG WINDOW ---'; cat " PIPE_PATH "; read\" &");
        
        // Give the terminal a moment to start and open the pipe
        sleep(1);
        
        if (log_fd != -1) close(log_fd);
        log_fd = open(PIPE_PATH, O_WRONLY);
    } else if (test_fd != -1) {
        close(test_fd); // Reader exists, everything is fine
    }
}

void show_menu() {
    printf(BLUE "\n========= AUTOSTASH (MAIN) =========\n" RESET);
    printf("1. Start Backup Cycle\n");
    printf("2. Add Folder\n");
    printf("3. Remove Folder\n");
    printf("4. Show Settings\n");
    printf("5. Change Interval\n");
    printf("6. Exit Application\n");
    printf("7. Stop Background Backup\n");
    printf("------------------------------------\n");
    printf("Choice: ");
    fflush(stdout);
}

void ui_log(const char *color, const char *format, ...) {
    ensure_log_terminal();
    if (log_fd == -1) return;

    char buffer[1024];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    write(log_fd, color, strlen(color));
    write(log_fd, buffer, len);
    write(log_fd, RESET, strlen(RESET));
}

void add_folder() {
    printf("Enter folder path: ");
    char path[MAX_PATH];
    scanf("%s", path);
    
    pthread_mutex_lock(&lock);
    if (folder_count < MAX_FOLDERS) {
        strcpy(folders[folder_count++], path);
        printf(WHITE "Folder configuration updated. " YELLOW "(See secondary terminal for logs)\n" RESET);
        ui_log(YELLOW, "[CONFIG] Added folder to watch-list: %s\n", path);
    } else {
        printf(RED "Error: Folder limit reached.\n" RESET);
    }
    pthread_mutex_unlock(&lock);
}

void remove_folder() {
    int idx;
    printf("Enter index to remove: ");
    scanf("%d", &idx);

    pthread_mutex_lock(&lock);
    if (idx >= 0 && idx < folder_count) {
        ui_log(RED, "[CONFIG] Removed folder from watch-list: %s\n", folders[idx]);
        for (int i = idx; i < folder_count - 1; i++)
            strcpy(folders[i], folders[i+1]);
        folder_count--;
        printf(WHITE "Folder removed successfully. " YELLOW "(See secondary terminal for logs)\n" RESET);
    }
    pthread_mutex_unlock(&lock);
}

void show_settings() {
    printf("\n--- CURRENT SETTINGS ---\n");
    for (int i = 0; i < folder_count; i++)
        printf("[%d] %s\n", i, folders[i]);
    printf("Interval: %d seconds\n", backup_interval);
}