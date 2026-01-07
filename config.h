#ifndef CONFIG_H
#define CONFIG_H

#include <pthread.h>
#include <fcntl.h>

/* ---------- CONFIG ---------- */
#define MAX_ITEMS 20  // Increased to handle both files and folders
#define MAX_PATH 256
#define PAGE_SIZE 10
#define PIPE_PATH "/tmp/autostash_log"

#define BLUE    "\033[34m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define WHITE   "\033[37m"
#define ORANGE  "\033[38;5;208m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

/* ---------- ITEM TYPES ---------- */
typedef enum {
    ITEM_FOLDER = 0,
    ITEM_FILE = 1
} ItemType;

/* ---------- GLOBALS ---------- */
extern pthread_mutex_t lock;
extern int backup_running;
extern int backup_interval;
extern int item_count;
extern char items[MAX_ITEMS][MAX_PATH];
extern ItemType item_types[MAX_ITEMS];
extern int log_fd;
extern int terminal_running;

/* ---------- STRUCT ---------- */
typedef struct {
    char source[MAX_PATH];
    char dest[MAX_PATH];
    ItemType type;
} BackupTask;

#endif