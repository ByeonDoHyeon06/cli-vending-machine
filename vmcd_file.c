#include "vmcd.h"
#include <ctype.h>   // isspace

// 기본값 세팅 (config 없거나 완전 망가졌을 때)
static void init_default_data(VendingMachine *vm) {
    init_vm(vm);

    vm->drink_count = 3;
    vm->coin_count  = 3;

    strcpy(vm->drinks[0].name, "Cola");
    vm->drinks[0].price = 1200;
    vm->drinks[0].stock = 5;

    strcpy(vm->drinks[1].name, "Cider");
    vm->drinks[1].price = 1100;
    vm->drinks[1].stock = 3;

    strcpy(vm->drinks[2].name, "Coffee");
    vm->drinks[2].price = 900;
    vm->drinks[2].stock = 4;

    vm->coins[0].value = 100;  vm->coins[0].stock = 5;
    vm->coins[1].value = 500;  vm->coins[1].stock = 5;
    vm->coins[2].value = 1000; vm->coins[2].stock = 5;
}

// 현재 vm 상태를 CONFIG_FILE로 저장 (새 포맷)
static void save_config_file(const VendingMachine *vm) {
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (!fp) return;

    fprintf(fp, "# VMCD 자판기 구성 파일\n");
    fprintf(fp, "# [DRINKS] 섹션: 이름 가격 재고\n");
    fprintf(fp, "# [COINS]  섹션: 금액 재고\n\n");

    fprintf(fp, "[DRINKS]\n");
    for (int i = 0; i < vm->drink_count; ++i) {
        fprintf(fp, "%s %d %d\n",
                vm->drinks[i].name,
                vm->drinks[i].price,
                vm->drinks[i].stock);
    }

    fprintf(fp, "\n[COINS]\n");
    for (int i = 0; i < vm->coin_count; ++i) {
        fprintf(fp, "%d %d\n", vm->coins[i].value, vm->coins[i].stock);
    }

    fclose(fp);
}

// 앞뒤 공백 제거된 포인터 반환
static char* trim_left(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

// config 읽기
int load_config(VendingMachine *vm) {
    FILE *fp = fopen(CONFIG_FILE, "r");
    char line[256];

    if (!fp) {
        printf("\n구성 파일이 없어 기본 설정으로 초기화합니다.\n");
        init_default_data(vm);
        save_config_file(vm);
        save_log("ADMIN init_default_config_no_file", vm);
        return 1;
    }

    init_vm(vm);  // 깨끗하게 초기화

    // ===== [DRINKS] 찾기 =====
    int found_drinks = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *p = trim_left(line);
        if (*p == '#' || *p == '\0' || *p == '\n') continue;
        if (strncmp(p, "[DRINKS]", 8) == 0) {
            found_drinks = 1;
            break;
        }
    }

    if (!found_drinks) {
        printf("\n구성 파일에서 [DRINKS] 섹션을 찾지 못했습니다.\n");
        fclose(fp);
        init_default_data(vm);
        save_config_file(vm);
        save_log("ADMIN init_default_config_missing_drinks", vm);
        return 1;
    }

    // ===== [DRINKS] 섹션 읽기 =====
    int drink_idx = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *p = trim_left(line);
        if (*p == '#' || *p == '\n' || *p == '\0') continue;

        if (*p == '[') {
            // 다음 섹션([COINS]) 도달
            // 이 줄은 나중에 다시 처리해야 하므로 파일 포인터를 되돌릴 수 없어서
            // 간단하게 [COINS]로 간주하고 루프 탈출
            if (strncmp(p, "[COINS]", 7) != 0) {
                printf("\n알 수 없는 섹션: %s", p);
                fclose(fp);
                init_default_data(vm);
                save_config_file(vm);
                save_log("ADMIN init_default_config_bad_section", vm);
                return 1;
            }
            // [COINS] 섹션으로 넘어가기 전에 drink_count 확정
            break;
        }

        if (drink_idx >= MAX_DRINKS) {
            printf("\n음료 개수가 MAX_DRINKS(%d)를 초과했습니다. 나머지는 무시합니다.\n",
                   MAX_DRINKS);
            continue;
        }

        Drink *d = &vm->drinks[drink_idx];
        if (sscanf(p, "%31s %d %d", d->name, &d->price, &d->stock) != 3) {
            printf("\n[DRINKS] 항목 파싱에 실패했습니다: %s", p);
            fclose(fp);
            init_default_data(vm);
            save_config_file(vm);
            save_log("ADMIN init_default_config_bad_drinks", vm);
            return 1;
        }
        if (d->stock < 0) d->stock = 0;
        if (d->stock > MAX_STOCK) d->stock = MAX_STOCK;

        drink_idx++;
    }
    vm->drink_count = drink_idx;

    if (vm->drink_count == 0) {
        printf("\n[DRINKS]에 유효한 음료가 없습니다.\n");
        fclose(fp);
        init_default_data(vm);
        save_config_file(vm);
        save_log("ADMIN init_default_config_empty_drinks", vm);
        return 1;
    }

    // 지금 파일 포인터는 [COINS] 줄 직후 또는 그 안. 다시 한 번 [COINS] 찾기
    int found_coins = 0;
    // 방금 읽은 줄이 [COINS]가 아닐 수도 있어서, 현재 위치부터 다시 탐색
    fseek(fp, 0, SEEK_SET);  // 간단히 처음부터 다시 읽어서 [COINS] 찾자

    while (fgets(line, sizeof(line), fp)) {
        char *p = trim_left(line);
        if (*p == '#' || *p == '\0' || *p == '\n') continue;
        if (strncmp(p, "[COINS]", 7) == 0) {
            found_coins = 1;
            break;
        }
    }

    if (!found_coins) {
        printf("\n구성 파일에서 [COINS] 섹션을 찾지 못했습니다.\n");
        fclose(fp);
        init_default_data(vm);
        save_config_file(vm);
        save_log("ADMIN init_default_config_missing_coins", vm);
        return 1;
    }

    // ===== [COINS] 섹션 읽기 =====
    int coin_idx = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *p = trim_left(line);
        if (*p == '#' || *p == '\n' || *p == '\0') continue;
        if (*p == '[') break; // 다음 섹션이 생긴다면 중단

        if (coin_idx >= MAX_COINS) {
            printf("\n동전 종류가 MAX_COINS(%d)를 초과했습니다. 나머지는 무시합니다.\n",
                   MAX_COINS);
            continue;
        }

        Coin *c = &vm->coins[coin_idx];
        if (sscanf(p, "%d %d", &c->value, &c->stock) != 2) {
            printf("\n[COINS] 항목 파싱에 실패했습니다: %s", p);
            fclose(fp);
            init_default_data(vm);
            save_config_file(vm);
            save_log("ADMIN init_default_config_bad_coins", vm);
            return 1;
        }
        if (c->stock < 0) c->stock = 0;
        if (c->stock > MAX_STOCK) c->stock = MAX_STOCK;

        coin_idx++;
    }
    vm->coin_count = coin_idx;

    if (vm->coin_count == 0) {
        printf("\n[COINS]에 유효한 동전 정보가 없습니다.\n");
        fclose(fp);
        init_default_data(vm);
        save_config_file(vm);
        save_log("ADMIN init_default_config_empty_coins", vm);
        return 1;
    }

    fclose(fp);
    save_log("ADMIN load_config", vm);
    return 1;
}

int save_log(const char *message, const VendingMachine *vm) {
    (void)vm; // 필요하면 vm 상태도 상세히 남길 수 있음

    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return 0;

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            tm_now->tm_year + 1900,
            tm_now->tm_mon + 1,
            tm_now->tm_mday,
            tm_now->tm_hour,
            tm_now->tm_min,
            tm_now->tm_sec,
            message);

    fclose(fp);
    return 1;
}