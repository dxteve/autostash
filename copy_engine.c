#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void compress_folder(const char *source, const char *dest_zip) {
    char command[2048];
    snprintf(command, sizeof(command), "zip -rq '%s' '%s' -x \"backups/*\" 2>/dev/null", dest_zip, source);
    system(command);
}

void copy_file(const char *source, const char *dest) {
    char command[2048];
    snprintf(command, sizeof(command), "cp '%s' '%s' 2>/dev/null", source, dest);
    system(command);
}