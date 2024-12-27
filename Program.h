#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 128

// Arka plan süreçlerini takip için bir yapı
typedef struct arkaplanProcess {
    pid_t pid;
    struct arkaplanProcess* sonraki;
} arkaplanProcess_t;

// Global Değişkenler
arkaplanProcess_t* arkaplanListesi = NULL;
int quit_requested = 0;

// Function declarations
void sigchld_handler(int);
void arkaPlanProcessEkle(pid_t);
void arkplanBekle();
int parse_command(char* satir, char** args, char** girisDosyasi);
int execute_command(char** args, char* girisDosyasi);

#endif
