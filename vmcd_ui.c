#include "vmcd.h"

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

void print_title(void) {
    puts("========================================");
    puts("         캔음료 자판기 (VMCD)");
    puts("========================================");
}

void print_subtitle(const char *msg) {
    printf("\n[%s]\n", msg);
}

void print_line(void) {
    puts("----------------------------------------");
}

void loading_spinner(const char *msg, int ms) {
    const char spinner[] = "|/-\\";
    int steps = ms / 100;
    for (int i = 0; i < steps; ++i) {
        printf("\r%s %c", msg, spinner[i % 4]);
        fflush(stdout);
        SLEEP_MS(100);
    }
    printf("\r%s 완료\n", msg);
}

void progress_bar(const char *msg, int total_steps, int delay_ms_ms) {
    const int width = 20; // # 개수
    for (int step = 0; step <= total_steps; ++step) {
        int percent = (step * 100) / total_steps;
        int filled = (step * width) / total_steps;
        printf("\r%s [", msg);
        for (int i = 0; i < width; ++i) {
            if (i < filled) putchar('#');
            else putchar(' ');
        }
        printf("] (%3d%%)", percent);
        fflush(stdout);
        SLEEP_MS(delay_ms_ms);
    }
    putchar('\n');
}


void wait_for_enter(void) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {}

    printf("\n계속하려면 Enter 키를 누르세요...");
    fflush(stdout);
    getchar();
}