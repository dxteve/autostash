#ifndef CONFIG_H
#define CONFIG_H

#include <pthread.h>

/* ---------- CONFIG ---------- */
#define MAX_FOLDERS 10
#define MAX_PATH 256
#define PIPE_PATH "/tmp/autostash_log"

#define BLUE    "\033[34m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define WHITE   "\033[37m"
#define RESET   "\033[0m"

/* ---------- GLOBALS ---------- */
extern pthread_mutex_t lock;
extern int backup_running;
extern int backup_interval;
extern int folder_count;
extern char folders[MAX_FOLDERS][MAX_PATH];
extern int log_fd;

/* ---------- STRUCT ---------- */
typedef struct {
    char source[MAX_PATH];
    char dest[MAX_PATH];
} BackupTask;

#endif