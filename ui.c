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

void check_terminal_status() {
    int test_fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK);
    if (test_fd == -1 && errno == ENXIO) {
        terminal_running = 0;
    } else {
        terminal_running = 1;
        if (test_fd != -1) close(test_fd);
    }
}

void ensure_log_terminal() {
    int test_fd = open(PIPE_PATH, O_WRONLY | O_NONBLOCK);
    if (test_fd == -1 && errno == ENXIO) {
        system("gnome-terminal -- bash -c \"echo '--- AUTOSTASH LOG WINDOW ---'; cat " PIPE_PATH "; echo 'SHUTTING DOWN...'; sleep 2\" &");
        sleep(1);
        if (log_fd != -1) close(log_fd);
        log_fd = open(PIPE_PATH, O_WRONLY);
        terminal_running = 1;
    } else if (test_fd != -1) {
        close(test_fd);
        terminal_running = 1;
    }
}

void play_sound(int type) {
    switch(type) {
        case 1: // Start
            system("paplay /usr/share/sounds/freedesktop/stereo/message-new-instant.oga 2>/dev/null &");
            break; 
        case 2: // Finish
            system("paplay /usr/share/sounds/freedesktop/stereo/complete.oga 2>/dev/null &");
            break; 
        case 3: // Interval/Idle
            system("paplay /usr/share/sounds/freedesktop/stereo/bell.oga 2>/dev/null &");
            break;
    }
}

void show_menu() {
    char cwd[MAX_PATH];
    getcwd(cwd, sizeof(cwd));
    
    printf(BLUE "\n========= AUTOSTASH =========\n" RESET);
    printf(WHITE "Location: " GREEN "%s\n" RESET, cwd);
    
    // Show terminal status
    if (terminal_running) {
        printf(GREEN "● " WHITE "Log Terminal: " GREEN "Running\n" RESET);
    } else {
        printf(RED "● " WHITE "Log Terminal: " RED "Not Running " YELLOW "(Will restart on next action)\n" RESET);
    }
    
    printf("\n");
    printf("1. Start Background Backup Cycle\n");
    printf("2. Add File/Folder to Backup\n");
    printf("3. Remove Item from Backup\n");
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

int get_items_in_directory(char **item_list, ItemType **type_list) {
    struct dirent *entry;
    DIR *dp = opendir(".");
    int count = 0;
    if (dp == NULL) return -1;

    while ((entry = readdir(dp)) != NULL && count < 500) {
        struct stat st;
        if (stat(entry->d_name, &st) == 0) {
            // Skip . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            
            item_list[count] = strdup(entry->d_name);
            (*type_list)[count] = S_ISDIR(st.st_mode) ? ITEM_FOLDER : ITEM_FILE;
            count++;
        }
    }
    closedir(dp);
    
    // Sort: folders first, then files
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if ((*type_list)[j] > (*type_list)[j+1]) {
                // Swap
                char *temp_name = item_list[j];
                item_list[j] = item_list[j+1];
                item_list[j+1] = temp_name;
                
                ItemType temp_type = (*type_list)[j];
                (*type_list)[j] = (*type_list)[j+1];
                (*type_list)[j+1] = temp_type;
            }
        }
    }
    
    return count;
}

void add_item() {
    char *item_list[500];
    ItemType *type_list = malloc(500 * sizeof(ItemType));
    int total_items = get_items_in_directory(item_list, &type_list);
    
    if (total_items == -1) {
        printf(RED "Error accessing directory.\n" RESET);
        free(type_list);
        return;
    }

    int current_page = 0, selection_made = 0;
    char input[MAX_PATH];

    while (!selection_made) {
        printf(BLUE "\n--- Add File/Folder to Backup (Page %d) ---\n" RESET, current_page + 1);
        int start = current_page * PAGE_SIZE;
        int end = (start + PAGE_SIZE > total_items) ? total_items : start + PAGE_SIZE;

        for (int i = start; i < end; i++) {
            if (type_list[i] == ITEM_FOLDER) {
                printf(ORANGE "[%d] " GREEN "📁 %s\n" RESET, i, item_list[i]);
            } else {
                printf(ORANGE "[%d] " WHITE "📄 %s\n" RESET, i, item_list[i]);
            }
        }

        printf("\nOptions: " WHITE "[0-%d] Index | [n] Next | [b] Back | [m] Manual | [q] Cancel\n" RESET, total_items - 1);
        printf("Choice: ");
        scanf("%s", input);

        if (strcmp(input, "n") == 0) {
            if (end < total_items) current_page++;
        } else if (strcmp(input, "b") == 0) {
            if (current_page > 0) current_page--;
        } else if (strcmp(input, "q") == 0) {
            selection_made = 1;
        } else if (strcmp(input, "m") == 0) {
            printf("Enter manual path: ");
            char manual_path[MAX_PATH];
            scanf("%s", manual_path);
            struct stat st;
            
            if (stat(manual_path, &st) == 0) {
                pthread_mutex_lock(&lock);
                if (item_count < MAX_ITEMS) {
                    strcpy(items[item_count], manual_path);
                    item_types[item_count] = S_ISDIR(st.st_mode) ? ITEM_FOLDER : ITEM_FILE;
                    
                    if (item_types[item_count] == ITEM_FOLDER) {
                        ui_log(YELLOW, "[CONFIG] Manual Add (Folder): %s\n", manual_path);
                    } else {
                        ui_log(YELLOW, "[CONFIG] Manual Add (File): %s\n", manual_path);
                    }
                    
                    item_count++;
                    printf(WHITE "Item added successfully.\n" RESET);
                    selection_made = 1;
                } else {
                    printf(RED "Maximum items limit reached!\n" RESET);
                }
                pthread_mutex_unlock(&lock);
            } else {
                printf(RED "Invalid path! Nothing added.\n" RESET);
            }
        } else {
            int idx = atoi(input);
            if (idx >= 0 && idx < total_items) {
                pthread_mutex_lock(&lock);
                if (item_count < MAX_ITEMS) {
                    strcpy(items[item_count], item_list[idx]);
                    item_types[item_count] = type_list[idx];
                    
                    if (type_list[idx] == ITEM_FOLDER) {
                        ui_log(YELLOW, "[CONFIG] Added (Folder): %s\n", item_list[idx]);
                    } else {
                        ui_log(YELLOW, "[CONFIG] Added (File): %s\n", item_list[idx]);
                    }
                    
                    item_count++;
                    printf(WHITE "Item added.\n" RESET);
                    selection_made = 1;
                } else {
                    printf(RED "Maximum items limit reached!\n" RESET);
                }
                pthread_mutex_unlock(&lock);
            }
        }
    }
    
    for (int i = 0; i < total_items; i++) free(item_list[i]);
    free(type_list);
}

void change_directory() {
    char *item_list[500];
    ItemType *type_list = malloc(500 * sizeof(ItemType));
    int total_items = get_items_in_directory(item_list, &type_list);
    
    if (total_items == -1) {
        free(type_list);
        return;
    }

    int current_page = 0, finished = 0;
    char input[MAX_PATH];

    while (!finished) {
        char cwd[MAX_PATH];
        getcwd(cwd, sizeof(cwd));
        printf(BLUE "\n--- Change Directory (Current: %s) ---\n" RESET, cwd);
        
        int start = current_page * PAGE_SIZE;
        int end = (start + PAGE_SIZE > total_items) ? total_items : start + PAGE_SIZE;

        // Only show folders for navigation
        for (int i = start; i < end; i++) {
            if (type_list[i] == ITEM_FOLDER) {
                printf(ORANGE "[%d] " GREEN "📁 %s\n" RESET, i, item_list[i]);
            }
        }

        printf("\n" WHITE "[Index] Select | [..] Up | [n/b] Pages | [m] Manual | [q] Back\n" RESET);
        printf("Choice: ");
        scanf("%s", input);

        if (strcmp(input, "..") == 0) {
            chdir("..");
            ui_log(CYAN, "[NAVIGATE] Changed to parent directory\n");
            finished = 1;
        } else if (strcmp(input, "n") == 0 && end < total_items) {
            current_page++;
        } else if (strcmp(input, "b") == 0 && current_page > 0) {
            current_page--;
        } else if (strcmp(input, "m") == 0) {
            printf("Enter target path: ");
            char target[MAX_PATH];
            scanf("%s", target);
            if (chdir(target) == 0) {
                printf(WHITE "Directory changed.\n" RESET);
                ui_log(CYAN, "[NAVIGATE] Changed directory to: %s\n", target);
                finished = 1;
            } else {
                printf(RED "Invalid path!\n" RESET);
            }
        } else if (strcmp(input, "q") == 0) {
            finished = 1;
        } else {
            int idx = atoi(input);
            if (idx >= 0 && idx < total_items && type_list[idx] == ITEM_FOLDER) {
                chdir(item_list[idx]);
                printf(WHITE "Moved to %s\n" RESET, item_list[idx]);
                ui_log(CYAN, "[NAVIGATE] Entered directory: %s\n", item_list[idx]);
                finished = 1;
            }
        }
    }
    
    for (int i = 0; i < total_items; i++) free(item_list[i]);
    free(type_list);
}

void remove_item() {
    printf(BLUE "\n--- Remove Item from Backup ---\n" RESET);
    
    if (item_count == 0) {
        printf(RED "No items in backup list.\n" RESET);
        return;
    }
    
    // Display all items with their types
    printf(WHITE "Current items in backup list:\n" RESET);
    for (int i = 0; i < item_count; i++) {
        if (item_types[i] == ITEM_FOLDER) {
            printf(ORANGE "[%d] " GREEN "📁 %s\n" RESET, i, items[i]);
        } else {
            printf(ORANGE "[%d] " WHITE "📄 %s\n" RESET, i, items[i]);
        }
    }
    
    int idx;
    printf("\nEnter index to remove (or -1 to cancel): ");
    scanf("%d", &idx);

    if (idx == -1) {
        printf(YELLOW "Cancelled.\n" RESET);
        return;
    }

    pthread_mutex_lock(&lock);
    if (idx >= 0 && idx < item_count) {
        if (item_types[idx] == ITEM_FOLDER) {
            ui_log(RED, "[CONFIG] Removed (Folder): %s\n", items[idx]);
        } else {
            ui_log(RED, "[CONFIG] Removed (File): %s\n", items[idx]);
        }
        
        for (int i = idx; i < item_count - 1; i++) {
            strcpy(items[i], items[i+1]);
            item_types[i] = item_types[i+1];
        }
        item_count--;
        printf(WHITE "Item removed successfully. " YELLOW "(See secondary terminal for logs)\n" RESET);
    } else {
        printf(RED "Invalid index!\n" RESET);
    }
    pthread_mutex_unlock(&lock);
}

void show_settings() {
    char cwd[MAX_PATH];
    getcwd(cwd, sizeof(cwd));

    printf(BLUE "\n========= APPLICATION SETTINGS =========\n" RESET);
    
    printf(WHITE "Current Working Directory:\n " GREEN "%s\n" RESET, cwd);
    printf(WHITE "Backup Interval: " YELLOW "%d seconds\n" RESET, backup_interval);
    printf(WHITE "Log Terminal Status: ");
    if (terminal_running) {
        printf(GREEN "Running\n" RESET);
    } else {
        printf(RED "Not Running\n" RESET);
    }

    printf(WHITE "\nItems in Backup List (" ORANGE "%d/%d" WHITE "):\n" RESET, item_count, MAX_ITEMS);
    
    if (item_count == 0) {
        printf(RED " [!] No items added yet.\n" RESET);
    } else {
        // Count folders and files
        int folder_count = 0, file_count = 0;
        for (int i = 0; i < item_count; i++) {
            if (item_types[i] == ITEM_FOLDER) folder_count++;
            else file_count++;
        }
        
        // Display folders
        if (folder_count > 0) {
            printf(WHITE "\n Folders (%d):\n" RESET, folder_count);
            int folder_idx = 0;
            for (int i = 0; i < item_count; i++) {
                if (item_types[i] == ITEM_FOLDER) {
                    printf(ORANGE "  [%d] " GREEN "📁 %s\n" RESET, i, items[i]);
                    folder_idx++;
                }
            }
        }
        
        // Display files
        if (file_count > 0) {
            printf(WHITE "\n Files (%d):\n" RESET, file_count);
            for (int i = 0; i < item_count; i++) {
                if (item_types[i] == ITEM_FILE) {
                    printf(ORANGE "  [%d] " WHITE "📄 %s\n" RESET, i, items[i]);
                }
            }
        }
    }
    
    printf(BLUE "\n========================================\n" RESET);
    printf("Press Enter to continue...");
    getchar(); getchar(); 
}