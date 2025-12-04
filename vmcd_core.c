#include "vmcd.h"

// 재고 출력
void print_stock(const VendingMachine *vm) {
    print_subtitle("현재 재고");

    print_line();
    printf("번호  이름          가격    재고\n");
    print_line();
    for (int i = 0; i < vm->drink_count; ++i) {
        const Drink *d = &vm->drinks[i];
        printf("%2d)   %-10s  %5d원   %2d개\n",
               i + 1,
               d->name[0] ? d->name : "(없음)",
               d->price,
               d->stock);
    }

    print_line();
    puts("\n동전 재고");
    for (int i = 0; i < vm->coin_count; ++i) {
        printf(" %4d원: %2d개\n", vm->coins[i].value, vm->coins[i].stock);
    }
}

// 동전 재고 + 방금 넣은 동전까지 고려해서 거스름돈 계산
static int calc_change_with_stock(const VendingMachine *vm,
                                  const int inserted_count[MAX_COINS],
                                  int change,
                                  int out_change_count[MAX_COINS]) {
    int temp_stock[MAX_COINS];

    for (int i = 0; i < vm->coin_count; ++i) {
        temp_stock[i] = vm->coins[i].stock + inserted_count[i];
        out_change_count[i] = 0;
    }

    for (int i = vm->coin_count - 1; i >= 0; --i) {
        int coin_value = vm->coins[i].value;
        int max_can_use = change / coin_value;
        if (max_can_use > temp_stock[i]) {
            max_can_use = temp_stock[i];
        }
        out_change_count[i] = max_can_use;
        change -= coin_value * max_can_use;
    }

    return (change == 0); // 성공이면 1
}

void init_vm(VendingMachine *vm) {
    vm->drink_count = 0;
    vm->coin_count  = 0;
    for (int i = 0; i < MAX_DRINKS; ++i) {
        vm->drinks[i].name[0] = '\0';
        vm->drinks[i].price = 0;
        vm->drinks[i].stock = 0;
    }
    for (int i = 0; i < MAX_COINS; ++i) {
        vm->coins[i].value = 0;
        vm->coins[i].stock = 0;
    }
}

void run_user_mode(VendingMachine *vm) {
    clear_screen();
    print_title();
    print_subtitle("사용자 주문 모드");

    if (vm->drink_count == 0) {
        printf("설정된 음료가 없습니다. 구성 파일을 먼저 설정해 주세요.\n");
        wait_for_enter();
        return;
    }

    print_stock(vm);

    printf("\n구매하실 음료 번호를 선택해 주세요. (1~%d, 0: 취소)\n",
           vm->drink_count);
    printf("선택: ");

    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("입력 형식이 올바르지 않습니다.\n");
        SLEEP_MS(800);
        wait_for_enter();
        return;
    }

    if (choice == 0) {
        printf("주문을 취소하고 관리자 메뉴로 돌아갑니다.\n");
        SLEEP_MS(800);
        wait_for_enter();
        return;
    }

    if (choice < 1 || choice > vm->drink_count) {
        printf("유효하지 않은 번호입니다.\n");
        SLEEP_MS(800);
        wait_for_enter();
        return;
    }

    Drink *d = &vm->drinks[choice - 1];
    if (!d->name[0] || d->price <= 0) {
        printf("해당 음료는 설정되지 않았습니다.\n");
        SLEEP_MS(800);
        wait_for_enter();
        return;
    }

    if (d->stock <= 0) {
        printf("해당 음료 재고가 부족합니다. 주문을 취소합니다.\n");
        SLEEP_MS(1200);
        wait_for_enter();
        return;
    }

    // ===== 동전 투입 단계 =====
    clear_screen();
    print_title();
    print_subtitle("동전 투입");

    if (vm->coin_count == 0) {
        printf("동전 정보가 설정되지 않아 주문을 처리할 수 없습니다.\n");
        wait_for_enter();
        return;
    }

    printf("선택한 음료: %s (%d원)\n", d->name, d->price);
    printf("사용 가능한 동전: ");
    for (int i = 0; i < vm->coin_count; ++i) {
        printf("%d ", vm->coins[i].value);
    }
    puts("\n0을 입력하면 주문이 취소됩니다.\n");

    int inserted_total = 0;
    int inserted_count[MAX_COINS] = {0,};

    while (inserted_total < d->price) {
        printf("동전 입력 (현재 %d원 투입): ", inserted_total);
        int coin;
        if (scanf("%d", &coin) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("입력 형식이 올바르지 않습니다.\n");
            continue;
        }

        if (coin == 0) {
            printf("\n주문을 취소합니다. %d원을 반환합니다.\n", inserted_total);
            SLEEP_MS(1000);
            wait_for_enter();
            return;
        }

        int idx = -1;
        for (int i = 0; i < vm->coin_count; ++i) {
            if (vm->coins[i].value == coin) {
                idx = i;
                break;
            }
        }
        if (idx == -1) {
            printf("지원하지 않는 동전입니다. 위에 표시된 금액만 사용 가능합니다.\n");
            continue;
        }

        if (vm->coins[idx].stock + inserted_count[idx] >= MAX_STOCK) {
            printf("해당 동전 슬롯이 가득 찼습니다. 다른 동전을 사용해 주세요.\n");
            continue;
        }

        inserted_total += coin;
        inserted_count[idx]++;
        printf("동전 %d원을 넣었습니다. (합계: %d원)\n", coin, inserted_total);
    }

    int change = inserted_total - d->price;
    int change_count[MAX_COINS] = {0,};  // 반드시 0으로 초기화

    if (change > 0) {
        loading_spinner("거스름돈 계산 중", 700);

        if (!calc_change_with_stock(vm, inserted_count, change, change_count)) {
            printf("\n동전 재고 부족으로 정확한 거스름돈을 드릴 수 없습니다.\n");
            printf("주문이 취소되고 %d원을 모두 반환합니다.\n", inserted_total);
            SLEEP_MS(2000);
            wait_for_enter();
            return;
        }
    }

    // 재고 반영
    for (int i = 0; i < vm->coin_count; ++i) {
        vm->coins[i].stock += inserted_count[i];  // 투입된 동전
        vm->coins[i].stock -= change_count[i];    // 거슬러준 동전
    }
    d->stock--;

    progress_bar("음료를 내보내는 중", 30, 30);

    printf("\n%s 1개가 나왔습니다.\n", d->name);

    if (change > 0) {
        printf("거스름돈 %d원: ", change);
        for (int i = 0; i < vm->coin_count; ++i) {
            if (change_count[i] > 0) {
                printf("%d원 x %d  ", vm->coins[i].value, change_count[i]);
            }
        }
        putchar('\n');
    }

    char logbuf[256];
    snprintf(logbuf, sizeof(logbuf),
             "SALE name=%s price=%d inserted=%d change=%d stock_after=%d",
             d->name, d->price, inserted_total, change, d->stock);
    save_log(logbuf, vm);

    wait_for_enter();
}