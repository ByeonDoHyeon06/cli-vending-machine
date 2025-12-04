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
    int value;
    int stock;
} Coin;

typedef struct {
    int drink_count;
    int coin_count;
    Drink drinks[MAX_DRINKS];
    Coin  coins[MAX_COINS];
} VendingMachine;

int load_config(VendingMachine *vm);
int save_log(const char *message, const VendingMachine *vm);
int print_config_contents(void);
int print_log_history(void);

void init_vm(VendingMachine *vm);
void run_user_mode(VendingMachine *vm);
void print_stock(const VendingMachine *vm);

void clear_screen(void);
void print_title(void);
void print_subtitle(const char *msg);
void print_line(void);
void loading_spinner(const char *msg, int ms);
void progress_bar(const char *msg, int total_steps, int delay_ms_ms);
void wait_for_enter(void);

#endif