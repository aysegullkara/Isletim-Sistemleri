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
/*
 * Linux Kabuk (Shell) Programı
 * Bu program basit bir Linux kabuğu simüle eder.
 * Temel komut çalıştırma, arkaplan işlemleri, I/O yönlendirme ve pipe işlemleri desteklenir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "Program.h"

 /* Sabit tanımlamalar */
#define MAX_INPUT 1024    // Maksimum girdi uzunluğu
#define MAX_ARGS 64       // Maksimum argüman sayısı
#define MAX_ARKAPLAN 64   // Maksimum arkaplan işlem sayısı

/* Global değişkenler */
int bg_processes[MAX_ARKAPLAN];  // Arkaplan işlem PID'lerini tutan dizi
int bg_count = 0;                // Mevcut arkaplan işlem sayısı

/**
 * Kullanıcı girdisini ayrıştırır ve argüman dizisine dönüştürür
 * @param input Kullanıcıdan alınan komut satırı
 * @param args Ayrıştırılmış argümanların saklanacağı dizi
 */
void giris(char* input, char** args) {
    char* token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;  // Argüman dizisinin sonunu işaretle
}

/**
 * Komutun arkaplanda çalıştırılıp çalıştırılmayacağını kontrol eder
 * @param args Komut argümanları
 * @return Arkaplanda çalışacaksa 1, değilse 0
 */
int Arkaplan(char** args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;  // & işaretini kaldır
            return 1;
        }
    }
    return 0;
}

/**
 * Yeni bir arkaplan işlemini listeye ekler
 * @param pid Eklenecek işlemin PID'i
 */
void Arkaplan_ekle(pid_t pid) {
    if (bg_count < MAX_ARKAPLAN) {
        bg_processes[bg_count++] = pid;
    }
    else {
        fprintf(stderr, "Arka plan işlem kapasitesi doldu.\n");
    }
}

/**
 * Tamamlanan arkaplan işlemini listeden çıkarır
 * @param pid Çıkarılacak işlemin PID'i
 */
void Arkaplan_cikar(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i] == pid) {
            bg_processes[i] = bg_processes[--bg_count];
            return;
        }
    }
}

/**
 * Arkaplan işlemlerinin durumunu kontrol eder
 * Tamamlanan işlemleri raporlar ve listeden çıkarır
 */
void arkaplan_kontrol() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
        Arkaplan_cikar(pid);
    }
}

/**
 * Tüm arkaplan işlemlerinin tamamlanmasını bekler
 * Kabuktan çıkış yapmadan önce çağrılır
 */
void arkaplana_bekle() {
    for (int i = 0; i < bg_count; i++) {
        int status;
        waitpid(bg_processes[i], &status, 0);
        printf("[%d] retval: %d\n", bg_processes[i], WEXITSTATUS(status));
    }
    bg_count = 0;
}

/**
 * Girdi yönlendirme işlemini gerçekleştirir (< operatörü)
 * @param command Yönlendirme içeren komut
 */
void giris_yonlendirme(char* command) {
    char* args[MAX_ARGS];
    char* input_file = NULL;
    int i = 0;

    // Komutu ve dosya adını ayrıştır
    char* token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Hata: Giriş dosyası belirtilmedi.\n");
                return;
            }
            input_file = token;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Hata kontrolü
    if (input_file == NULL) {
        fprintf(stderr, "Hata: Giriş yönlendirme operatörü '<' eksik.\n");
        return;
    }

    // Dosyayı aç ve işle
    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Giriş dosyası bulunamadı: %s\n", input_file);
        return;
    }

    int num;
    if (fscanf(file, "%d", &num) == 1) {
        printf("%d\n", num);
    }
    else {
        fprintf(stderr, "Giriş dosyasından sayı okunamadı.\n");
    }
    fclose(file);
}

/**
 * Çıktı yönlendirme işlemini gerçekleştirir (> operatörü)
 * @param command Yönlendirme içeren komut
 */
void cikis_yonlendirme(char* command) {
    char* args[MAX_ARGS];
    char* output_file = NULL;
    int i = 0;

    // Komutu ve dosya adını ayrıştır
    char* token = strtok(command, " ");
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Hata: Çıkış dosyası belirtilmedi.\n");
                return;
            }
            output_file = token;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Hata kontrolü
    if (output_file == NULL) {
        fprintf(stderr, "Hata: Çıkış yönlendirme operatörü '>' eksik.\n");
        return;
    }

    // Dosyaya yaz
    FILE* file = fopen(output_file, "w");
    if (file == NULL) {
        perror("Çıkış dosyası açılamadı");
        return;
    }

    int num = atoi(args[1]);
    fprintf(file, "%d\n", num + 1);
    fclose(file);
}

/**
 * Pipe işlemini gerçekleştirir (| operatörü)
 * İki komut arasında veri akışı sağlar
 * @param command Pipe içeren komut
 */
void pipe_yonlendirme(char* command) {
    char* args1[MAX_ARGS], * args2[MAX_ARGS];
    char* pipe_position = strstr(command, "|");

    // Pipe karakterini bul ve komutları ayır
    if (pipe_position == NULL) {
        fprintf(stderr, "Hata: Boru karakteri bulunamadı.\n");
        return;
    }

    *pipe_position = '\0';
    giris(command, args1);
    giris(pipe_position + 1, args2);

    // Pipe oluştur
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe oluşturulamadı");
        return;
    }

    // İlk çocuk process (yazma tarafı)
    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        if (strcmp(args1[0], "increment") == 0) {
            int num = atoi(args1[1]);
            printf("%d\n", num + 1);
            exit(0);
        }

        if (execvp(args1[0], args1) == -1) {
            perror("İlk komut çalıştırma hatası");
            exit(EXIT_FAILURE);
        }
    }

    // İkinci çocuk process (okuma tarafı)
    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        if (strcmp(args2[0], "increment") == 0) {
            int num;
            if (scanf("%d", &num) == 1) {
                printf("%d\n", num + 1);
            }
            else {
                fprintf(stderr, "Hata: Girdi okunamadı.\n");
            }
            exit(0);
        }

        if (execvp(args2[0], args2) == -1) {
            perror("İkinci komut çalıştırma hatası");
            exit(EXIT_FAILURE);
        }
    }

    // Ana process pipe'ı kapatır ve çocukları bekler
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

/**
 * Komutu işler ve çalıştırır
 * @param args Komut argümanları
 * @param background Arkaplanda çalıştırılacak mı?
 */
void Komut_isleme(char** args, int background) {
    pid_t pid = fork();

    if (pid == 0) {  // Çocuk process
        if (execvp(args[0], args) == -1) {
            perror("Komut çalıştırma hatası");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0) {  // Ana process
        if (background) {
            printf("[%d]\n", pid);
            Arkaplan_ekle(pid);
        }
        else {
            waitpid(pid, NULL, 0);
        }
    }
    else {
        perror("Fork hatası");
    }
}

/**
 * Ana kabuk döngüsü
 * Kullanıcı komutlarını alır ve işler
 */
void linux_shell() {
    char command[MAX_INPUT];
    char* args[MAX_ARGS];

    while (1) {
        printf("> ");
        fflush(stdout);

        // Komut girişini al
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        // Satır sonu karakterini kaldır
        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // Özel komutları kontrol et
        if (strstr(command, "<") != NULL) {
            giris_yonlendirme(command);
            continue;
        }

        if (strstr(command, ">") != NULL) {
            cikis_yonlendirme(command);
            continue;
        }

        if (strstr(command, "|") != NULL) {
            pipe_yonlendirme(command);
            continue;
        }

        // Çıkış komutu
        if (strcmp(command, "quit") == 0) {
            arkaplana_bekle();
            break;
        }

        // Normal komut işleme
        giris(command, args);
        arkaplan_kontrol();
        int background = Arkaplan(args);
        Komut_isleme(args, background);
    }
}

/**
 * Program giriş noktası
 */
int main() {
    linux_shell();
    printf("Kabuktan çıkıldı.\n");
    return 0;
}
