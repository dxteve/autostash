#ifndef SCHEDULER_H
#define SCHEDULER_H

void *backup_thread(void *arg);
void *scheduler(void *arg);
void run_backup_cycle();

#endif // SCHEDULER_H