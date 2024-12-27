/*
 * SAKARYA ÜNİVERSİTESİ 2024 GÜZ DÖNEMİ
 * İŞLETİM SİSTEMLERİ PROJE ÖDEVİ
 *
 * Grup üyeleri:
 * - Aysegul Kara 
 * - Dilara Cetin
 * - Hüseyin Akbal
 * - Melike Demirtas
 * - Yasin Can Kaya
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 128

// Komutun parse edilmesi
int parse_command(char* satir, char** args, char** girisDosyasi) {
    *girisDosyasi = NULL;

    char* kayitPtr;
    int argc = 0;
    char* token = strtok_r(satir, " \t", &kayitPtr);
    while (token && argc < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " \t", &kayitPtr);
            if (token) {
                *girisDosyasi = token;
            }
            else {
                fprintf(stderr, "Giriş dosyası belirtilmedi.\n");
                return -1;
            }
        }
        else {
            args[argc++] = token;
        }
        token = strtok_r(NULL, " \t", &kayitPtr);
    }
    args[argc] = NULL;

    return 0;
}

// Komutu çalıştırma fonksiyonu
int execute_command(char** args, char* girisDosyasi) {
    pid_t pid = fork();
    if (pid == 0) {
        if (girisDosyasi) {
            int dosya = open(girisDosyasi, O_RDONLY);
            if (dosya < 0) {
                fprintf(stderr, "%s giriş dosyası bulunamadı.\n", girisDosyasi);
                exit(1);
            }
            dup2(dosya, STDIN_FILENO);
            close(dosya);
        }
        execvp(args[0], args);
        perror("exec");
        fprintf(stderr, "Komut bulunamadı: %s\n", args[0]);
        exit(1);
    }
    else if (pid < 0) {
        perror("fork");
        return -1;
    }
    else {
        int durum;
        waitpid(pid, &durum, 0);
    }
    return 0;
}

// SIGCHLD sinyali handler'ı
// Arka plan süreçleri sonlandığında otomatik olarak çağrılır
// Zombie process'leri temizler ve süreç bilgilerini gösterir
void sigchld_handler(int signo) {
    int durum;
    pid_t pid;
    // WNOHANG: Non-blocking wait - beklerken programı bloklamaz
    while ((pid = waitpid(-1, &durum, WNOHANG)) > 0) {
        // Sonlanan sürecin PID'i ve dönüş değerini yazdır
        printf("[%d] retval: %d\n", pid, WEXITSTATUS(durum));
        fflush(stdout);

        // Sonlanan süreci arka plan listesinden kaldır
        // Bağlı liste üzerinde dolaşarak süreci bul ve sil
        arkaplanProcess_t** onceki = &arkaplanListesi;
        arkaplanProcess_t* suanki = arkaplanListesi;
        while (suanki) {
            if (suanki->pid == pid) {
                *onceki = suanki->sonraki;
                free(suanki); // Belleği temizle
                break;
            }
            onceki = &suanki->sonraki;
            suanki = suanki->sonraki;
        }
    }
}

// Tüm arka plan süreçlerinin tamamlanmasını bekler
// Program sonlandırılmadan önce çağrılır
void arkplanBekle() {
    arkaplanProcess_t* suanki = arkaplanListesi;
    while (suanki) {
        int durum;
        // Her süreç için blocking wait
        waitpid(suanki->pid, &durum, 0);
        printf("[%d] retval: %d\n", suanki->pid, WEXITSTATUS(durum));
        // Süreç node'unu temizle
        arkaplanProcess_t* temp = suanki;
        suanki = suanki->sonraki;
        free(temp);
    }
}

// Yeni bir arka plan sürecini listeye ekler
// @param pid: Eklenecek sürecin process ID'si

void arkaPlanProcessEkle(pid_t pid) {
    // Yeni süreç için bellek ayır
    arkaplanProcess_t* yeniProcess = malloc(sizeof(arkaplanProcess_t));
    yeniProcess->pid = pid;
    // Yeni süreci listenin başına ekle
    yeniProcess->sonraki = arkaplanListesi;
    arkaplanListesi = yeniProcess;
}

int main() {
    while (1) {
        printf("> ");
        fflush(stdout);

        char line[1024];
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) continue;

        if (strcmp(line, "quit") == 0) {
            break;
        }

        char* args[MAX_ARGS];
        char* girisDosyasi = NULL;

        if (parse_command(line, args, &girisDosyasi) == 0) {
            execute_command(args, girisDosyasi);
        }
    }

    return 0;
}
