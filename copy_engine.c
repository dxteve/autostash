#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "config.h"
#include "copy_engine.h"

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