#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
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
    char cwd[MAX_PATH];
    getcwd(cwd, sizeof(cwd));
    printf(BLUE "\n========= AUTOSTASH =========\n" RESET);
    printf(WHITE "Location: " GREEN "%s\n" RESET, cwd);
    printf("1. Start Background Backup Cycle\n");
    printf("2. Add Folder to Backup\n");
    printf("3. Remove Folder\n");
    printf("4. Show Settings\n");
    printf("5. Change Interval\n");
    printf("7. Stop Background Backup\n");
    printf("8. Change Current Directory\n");
    printf("9. Backup Only Once (One-Time)\n");
    printf("0. Exit Application\n");
    printf("-----------------------------\n");
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

// Helper function to scan directories
int get_subdirectories(char **dir_list) {
    struct dirent *entry;
    DIR *dp = opendir(".");
    int count = 0;
    if (dp == NULL) return -1;

    while ((entry = readdir(dp)) != NULL && count < 500) {
        struct stat st;
        // Check if it's a directory and not '.' or '..'
        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            dir_list[count] = strdup(entry->d_name);
            count++;
        }
    }
    closedir(dp);
    return count;
}

void add_folder() {
    char *dir_list[500];
    int total_dirs = get_subdirectories(dir_list);
    if (total_dirs == -1) {
        printf(RED "Error accessing directory.\n" RESET);
        return;
    }

    int current_page = 0, selection_made = 0;
    char input[MAX_PATH];

    while (!selection_made) {
        printf(BLUE "\n--- Add Folder to Backup (Page %d) ---\n" RESET, current_page + 1);
        int start = current_page * PAGE_SIZE;
        int end = (start + PAGE_SIZE > total_dirs) ? total_dirs : start + PAGE_SIZE;

        for (int i = start; i < end; i++)
            printf(ORANGE "[%d] " GREEN "%s\n" RESET, i, dir_list[i]);

        printf("\nOptions: " WHITE "[0-%d] Index | [n] Next | [b] Back | [m] Manual | [q] Cancel\n" RESET, total_dirs - 1);
        printf("Choice: ");
        scanf("%s", input);

        if (strcmp(input, "n") == 0) {
            if (end < total_dirs) current_page++;
        } else if (strcmp(input, "b") == 0) {
            if (current_page > 0) current_page--;
        } else if (strcmp(input, "q") == 0) {
            selection_made = 1;
        } else if (strcmp(input, "m") == 0) {
            printf("Enter manual path: ");
            char manual_path[MAX_PATH];
            scanf("%s", manual_path);
            struct stat st;
            // FIX: Strict check to prevent adding junk like ".pki"
            if (stat(manual_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                pthread_mutex_lock(&lock);
                if (folder_count < MAX_FOLDERS) {
                    strcpy(folders[folder_count++], manual_path);
                    ui_log(YELLOW, "[CONFIG] Manual Add: %s\n", manual_path);
                    printf(WHITE "Folder added successfully.\n" RESET);
                    selection_made = 1;
                }
                pthread_mutex_unlock(&lock);
            } else {
                printf(RED "Invalid folder! Nothing added.\n" RESET);
            }
        } else {
            int idx = atoi(input);
            if (idx >= 0 && idx < total_dirs) {
                pthread_mutex_lock(&lock);
                if (folder_count < MAX_FOLDERS) {
                    strcpy(folders[folder_count++], dir_list[idx]);
                    ui_log(YELLOW, "[CONFIG] Added: %s\n", dir_list[idx]);
                    printf(WHITE "Folder added.\n" RESET);
                    selection_made = 1;
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }
    for (int i = 0; i < total_dirs; i++) free(dir_list[i]);
}

void change_directory() {
    char *dir_list[500];
    int total_dirs = get_subdirectories(dir_list);
    if (total_dirs == -1) return;

    int current_page = 0, finished = 0;
    char input[MAX_PATH];

    while (!finished) {
        char cwd[MAX_PATH];
        getcwd(cwd, sizeof(cwd));
        printf(BLUE "\n--- Change Directory (Current: %s) ---\n" RESET, cwd);
        
        int start = current_page * PAGE_SIZE;
        int end = (start + PAGE_SIZE > total_dirs) ? total_dirs : start + PAGE_SIZE;

        for (int i = start; i < end; i++)
            printf(ORANGE "[%d] " GREEN "%s\n" RESET, i, dir_list[i]);

        printf("\n" WHITE "[Index] Select | [..] Up | [n/b] Pages | [m] Manual | [q] Back\n" RESET);
        printf("Choice: ");
        scanf("%s", input);

        if (strcmp(input, "..") == 0) {
            chdir("..");
            finished = 1;
        } else if (strcmp(input, "n") == 0 && end < total_dirs) {
            current_page++;
        } else if (strcmp(input, "b") == 0 && current_page > 0) {
            current_page--;
        } else if (strcmp(input, "m") == 0) {
            printf("Enter target path: ");
            char target[MAX_PATH];
            scanf("%s", target);
            if (chdir(target) == 0) {
                printf(WHITE "Directory changed.\n" RESET);
                finished = 1;
            } else {
                printf(RED "Invalid path!\n" RESET);
            }
        } else if (strcmp(input, "q") == 0) {
            finished = 1;
        } else {
            int idx = atoi(input);
            if (idx >= 0 && idx < total_dirs) {
                chdir(dir_list[idx]);
                printf(WHITE "Moved to %s\n" RESET, dir_list[idx]);
                finished = 1;
            }
        }
    }
    for (int i = 0; i < total_dirs; i++) free(dir_list[i]);
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
    char cwd[MAX_PATH];
    getcwd(cwd, sizeof(cwd));

    printf(BLUE "\n--- APPLICATION SETTINGS ---\n" RESET);
    
    // Display Working Directory
    printf(WHITE "Current Working Directory:\n " GREEN "%s\n" RESET, cwd);
    
    // Display Backup Interval
    printf(WHITE "Backup Interval: " YELLOW "%d seconds\n" RESET, backup_interval);

    // Display Folder List
    printf(WHITE "Folders in Backup List (" ORANGE "%d/%d" WHITE "):\n" RESET, folder_count, MAX_FOLDERS);
    if (folder_count == 0) {
        printf(RED " [!] No folders added yet.\n" RESET);
    } else {
        for (int i = 0; i < folder_count; i++) {
            printf(ORANGE " [%d] " GREEN "%s\n" RESET, i, folders[i]);
        }
    }
    printf(BLUE "---------------------------\n" RESET);
    
    // Pause so the user can read it
    printf("Press Enter to continue...");
    getchar(); getchar(); 
}