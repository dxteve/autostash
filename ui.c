#include <stdio.h>
#include <string.h>

#include "config.h"
#include "ui.h"

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