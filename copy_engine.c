#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void compress_folder(const char *source, const char *dest_zip) {
    char command[2048];
    // -r: recursive
    // -q: quiet
    // -x: EXCLUDE the backups folder to prevent nested loops
    // We use quotes around paths to handle spaces in folder names
    snprintf(command, sizeof(command), "zip -rq '%s' '%s' -x \"backups/*\"", dest_zip, source);
    system(command);
}