/*
 * SAKARYA ‹N›VERS›TES› 2024 G‹Z D÷NEM›
 * ›ﬁLET›M S›STEMLER› PROJE ÷DEV›
 *
 * Grup ¸yeleri:
 * - Aysegul Kara
 * - Dilara Cetin
 * - H¸seyin Akbal
 * - Melike Demirtas
 * - Yasin Can Kaya
 */
#ifndef PROGRAM_H
#define PROGRAM_H

void Komut_isleme(char** args, int background);
void Arkaplan_ekle(pid_t pid);
void arkaplan_kontrol();
void arkaplana_bekle();
void Arkaplan_cikar(pid_t pid);
int Arkaplan(char** args);
void linux_shell();
void giris_yonlendirme(char* command);
void cikis_yonlendirme(char* command);
void pipe_yonlendirme(char* command);

#endif
