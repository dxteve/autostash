#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>

/* ---------- CONFIG ---------- */

#define MAX_FOLDERS 10
#define MAX_PATH 256

#define BLUE  "\033[34m"
#define RED   "\033[31m"
#define RESET "\033[0m"

/* ---------- GLOBALS ---------- */

pthread_mutex_t lock;
int backup_running = 0;
int backup_interval = 60;
int folder_count = 0;

char folders[MAX_FOLDERS][MAX_PATH];

/* ---------- STRUCT ---------- */

typedef struct
{
    char source[MAX_PATH];
    char dest[MAX_PATH];
} BackupTask;

/* ---------- UTILITIES ---------- */

void get_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d_%H-%M-%S", t);
}

void mkdir_recursive(const char *path)
{
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", path);

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

/* ---------- COPY ENGINE ---------- */

void copy_file(const char *source_file, const char *dest_file)
{
    FILE *src = fopen(source_file, "rb");
    FILE *dst = fopen(dest_file, "wb");

    if (!src || !dst)
    {
        printf(RED "[ERROR] File copy failed: %s\n" RESET, source_file);
        if (src) fclose(src);
        if (dst) fclose(dst);
        return;
    }

    char buffer[8192];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0)
        fwrite(buffer, 1, bytes, dst);

    fclose(src);
    fclose(dst);
}

void copy_folder(const char *source, const char *dest)
{
    DIR *dir = opendir(source);
    if (!dir)
    {
        printf(RED "[ERROR] Cannot open directory: %s\n" RESET, source);
        return;
    }

    mkdir(dest, 0755);

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char src[MAX_PATH], dst[MAX_PATH];
        snprintf(src, sizeof(src), "%s/%s", source, entry->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", dest, entry->d_name);

        struct stat st;
        stat(src, &st);

        if (S_ISDIR(st.st_mode))
            copy_folder(src, dst);
        else
            copy_file(src, dst);
    }

    closedir(dir);
}

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

/* ---------- UI ---------- */

void show_menu()
{
    printf(BLUE "\n========= AUTOSTASH =========\n" RESET);
    printf("1. Start Backup\n");
    printf("2. Add Folder\n");
    printf("3. Remove Folder\n");
    printf("4. Show Settings\n");
    printf("5. Change Interval\n");
    printf("6. Exit\n");
    printf("-----------------------------\n");
    printf("Choice: ");
}

void add_folder()
{
    if (folder_count >= MAX_FOLDERS)
    {
        printf(RED "Folder limit reached.\n" RESET);
        return;
    }

    printf("Enter folder path: ");
    scanf("%s", folders[folder_count]);
    folder_count++;
}

void remove_folder()
{
    int idx;
    printf("Enter folder index: ");
    scanf("%d", &idx);

    if (idx < 0 || idx >= folder_count)
    {
        printf(RED "Invalid index\n" RESET);
        return;
    }

    for (int i = idx; i < folder_count - 1; i++)
        strcpy(folders[i], folders[i + 1]);

    folder_count--;
}

void show_settings()
{
    printf(BLUE "\nFolders:\n" RESET);
    for (int i = 0; i < folder_count; i++)
        printf(" [%d] %s\n", i, folders[i]);

    printf("Interval: %d seconds\n", backup_interval);
}

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
