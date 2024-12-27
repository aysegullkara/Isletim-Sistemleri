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
#include "Program.h"

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

// Kullanıcı komutunu parse eder ve bileşenlerine ayırır
// @param satir: Parse edilecek komut satırı
// @param args: Komut argümanları dizisi
// @param girisDosyasi: Input redirection (<) dosya adı
// @param cikisDosyasi: Output redirection (>) dosya adı
// @param arkaplan: Arka plan çalıştırma (&) flag'i
// @param pipeKomutları: Pipe (|) ile ayrılmış komutlar dizisi
// @param pipeSayac: Pipe sayısı
int parse_command(char* satir, char** args, char** girisDosyasi, char** cikisDosyasi, int* arkaplan, char*** pipeKomutları, int* pipeSayac) {
    // Değişkenleri başlangıç değerleriyle ayarla
    *girisDosyasi = NULL;
    *cikisDosyasi = NULL;
    *arkaplan = 0;
    *pipeSayac = 0;
    *pipeKomutları = NULL;

    // Pipe sayısını hesapla
    {
        char* harf = satir;
        int count = 0;
        while ((harf = strchr(harf, '|')) != NULL) {
            count++;
            harf++;
        }
        *pipeSayac = count;
    }

    // Pipe varsa komutları ayır
    if (*pipeSayac > 0) {
        *pipeKomutları = malloc(sizeof(char*) * (*pipeSayac + 1));
        int index = 0;
        char* kayitPtr;
        char* token = strtok_r(satir, "|", &kayitPtr);
        while (token) {
            (*pipeKomutları)[index++] = strdup(token);
            token = strtok_r(NULL, "|", &kayitPtr);
        }
        return 0;
    }

    // Komut argümanlarını parse et
    char* kayitPtr;
    int argc = 0;
    char* token = strtok_r(satir, " \t", &kayitPtr);
    while (token && argc < MAX_ARGS - 1) {
        // Input redirection (<) kontrolü
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " \t", &kayitPtr);
            if (token) {
                *girisDosyasi = token;
            }
            else {
                fprintf(stderr, "Giris dosyasi belirtilmedi.\n");
                return -1;
            }
        }
        // Output redirection (>) kontrolü
        else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " \t", &kayitPtr);
            if (token) {
                *cikisDosyasi = token;
            }
            else {
                fprintf(stderr, "Cikis dosyasi belirtilmedi.\n");
                return -1;
            }
        }
        // Arka plan çalıştırma (&) kontrolü
        else if (strcmp(token, "&") == 0) {
            *arkaplan = 1;
        }
        // Normal argüman
        else {
            args[argc++] = token;
        }
        token = strtok_r(NULL, " \t", &kayitPtr);
    }
    args[argc] = NULL; // Argüman listesini NULL ile sonlandır

    return 0;
}

// "increment" komutunu çalıştırır
// Verilen sayıyı bir artırıp yazdırır
// @param input_file: Giriş dosyası (NULL ise stdin kullanılır)
int execute_increment(const char* input_file) {
    int numara;
    FILE* girdi = stdin;

    // Giriş dosyası varsa aç
    if (input_file) {
        girdi = fopen(input_file, "r");
        if (!girdi) {
            fprintf(stderr, "%s giriş dosyası bulunamadı.\n", input_file);
            return -1;
        }
    }

    // Sayıyı oku
    if (fscanf(girdi, "%d", &numara) != 1) {
        fprintf(stderr, "Geçersiz giriş\n");
        if (input_file) fclose(girdi);
        return -1;
    }

    // Sonucu yazdır
    printf("%d\n", numara + 1);
    fflush(stdout);

    if (input_file) fclose(girdi);
    return 0;
}

// Tek bir komutu çalıştırır
// @param args: Komut ve argümanları
// @param girisDosyasi: Input redirection dosyası
// @param cikisDosyasi: Output redirection dosyası
// @param arkaplan: Arka planda çalıştırma flag'i
int execute_command(char** args, char* girisDosyasi, char* cikisDosyasi, int arkaplan) {
    // Özel "increment" komutu kontrolü
    if (strcmp(args[0], "increment") == 0) {
        return execute_increment(girisDosyasi);
    }

    // Yeni süreç oluştur
    pid_t pid = fork();
    if (pid == 0) { // Child process
        // Input redirection
        if (girisDosyasi) {
            int dosya = open(girisDosyasi, O_RDONLY);
            if (dosya < 0) {
                fprintf(stderr, "%s giris dosyasi bulunamadi.\n", girisDosyasi);
                exit(1);
            }
            dup2(dosya, STDIN_FILENO);
            close(dosya);
        }
        // Output redirection
        if (cikisDosyasi) {
            int dosya = open(cikisDosyasi, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (dosya < 0) {
                perror("output file");
                exit(1);
            }
            dup2(dosya, STDOUT_FILENO);
            close(dosya);
        }
        // Komutu çalıştır
        execvp(args[0], args);
        perror("exec");
        fprintf(stderr, "Komut bulunamadi: %s\n", args[0]);
        exit(1);
    }
    else if (pid < 0) { // Fork hatası
        perror("fork");
        return -1;
    }
    else { // Parent process
        if (arkaplan) {
            // Arka plan süreci olarak ekle
            arkaPlanProcessEkle(pid);
        }
        else {
            // Sürecin tamamlanmasını bekle
            int durum;
            waitpid(pid, &durum, 0);
        }
    }
    return 0;
}



// Ana program döngüsü
int main() {
    // SIGCHLD sinyali için handler ayarla
    struct sigaction sinyalKontrol;
    sinyalKontrol.sa_handler = sigchld_handler;
    sigemptyset(&sinyalKontrol.sa_mask);
    sinyalKontrol.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sinyalKontrol, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }

    // Ana program döngüsü
    while (!quit_requested) {
        // Prompt göster
        printf("> ");
        fflush(stdout);

        // Komut satırını oku
        char line[MAX_CMD_LEN];
        if (fgets(line, MAX_CMD_LEN, stdin) == NULL) {
            break;
        }

        // Satır sonu karakterini kaldır
        line[strcspn(line, "\n")] = '\0';

        // Boş satırları atla
        if (strlen(line) == 0) continue;

        // "quit" komutu kontrolü
        if (strcmp(line, "quit") == 0) {
            quit_requested = 1;
            break;
        }

        // Komutları çalıştır
        execute_sequential_commands(line);
    }

    // Program sonlanmadan önce tüm arka plan süreçlerinin bitmesini bekle
    arkplanBekle();
    return 0;
}