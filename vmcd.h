#ifndef VMCD_H
#define VMCD_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

// 최대 개수만 제한 (원하면 더 늘려도 됨)
#define MAX_DRINKS 10
#define MAX_COINS  10
#define MAX_STOCK  10

#define CONFIG_FILE "vmcd_config.txt"
#define LOG_FILE    "vmcd_log.txt"

typedef struct {
    char name[32];
    int price;
    int stock;
} Drink;

typedef struct {
    int value;   // 동전 금액 (예: 50, 100, 500, 1000 등)
    int stock;
} Coin;

typedef struct {
    int drink_count;              // 실제 음료 개수
    int coin_count;               // 실제 동전 종류 개수
    Drink drinks[MAX_DRINKS];
    Coin  coins[MAX_COINS];
} VendingMachine;

// vmcd_file.c
int load_config(VendingMachine *vm);
int save_log(const char *message, const VendingMachine *vm);

// vmcd_core.c
void init_vm(VendingMachine *vm);
void run_user_mode(VendingMachine *vm);

// vmcd_ui.c
void clear_screen(void);
void print_title(void);
void print_subtitle(const char *msg);
void print_line(void);
void loading_spinner(const char *msg, int ms);
void progress_bar(const char *msg, int total_steps, int delay_ms_ms);
void wait_for_enter(void);

#endif