#include "vmcd.h"

static void run_admin_mode(VendingMachine *vm) {
    int admin_running = 1;

    while (admin_running) {
        clear_screen();
        print_title();

        print_subtitle("관리자 모드");
        puts("[1] 초기 설정 내역 출력");
        puts("[2] 초기 설정 파일 로드");
        puts("[3] 동전 및 캔음료 재고 출력");
        puts("[4] 입출고 로그 출력");
        puts("[0] 뒤로가기");
        printf("\n메뉴를 선택해 주세요: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("\n입력 형식이 올바르지 않습니다.\n");
            SLEEP_MS(800);
            continue;
        }

        switch (choice) {
            case 1:
                clear_screen();
                print_title();
                if (!print_config_contents()) {
                    printf("구성을 출력하지 못했습니다.\n");
                }
                wait_for_enter();
                break;
            case 2:
                progress_bar("구성을 준비하는 중", 20, 30);
                if (load_config(vm)) {
                    printf("\n구성이 완료되었습니다.\n");
                } else {
                    printf("\n구성을 완료하지 못했습니다.\n");
                }
                wait_for_enter();
                break;
            case 3:
                clear_screen();
                print_title();
                print_stock(vm);
                wait_for_enter();
                break;
            case 4:
                clear_screen();
                print_title();
                if (!print_log_history()) {
                    printf("로그를 출력하지 못했습니다.\n");
                }
                wait_for_enter();
                break;
            case 0:
                admin_running = 0;
                break;
            default:
                printf("\n유효하지 않은 메뉴입니다.\n");
                SLEEP_MS(800);
                break;
        }
    }
}

int main(void) {
    VendingMachine vm;
    init_vm(&vm);
    load_config(&vm);  // ★ 이 줄1
    int running = 1;
    while (running) {
        clear_screen();
        print_title();

        print_subtitle("메인 메뉴");
        puts("1. 관리자 모드");
        puts("2. 사용자 주문 모드 실행");
        puts("3. 종료");
        printf("\n메뉴를 선택해 주세요: ");

        int choice;
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("\n입력 형식이 올바르지 않습니다.\n");
            SLEEP_MS(800);
            continue;
        }

        switch (choice) {
            case 1:
                run_admin_mode(&vm);
                break;
            case 2:
                run_user_mode(&vm);
                break;
            case 3:
                running = 0;
                break;
            default:
                printf("\n유효하지 않은 메뉴입니다.\n");
                SLEEP_MS(800);
                break;
        }
    }

    clear_screen();
    printf("프로그램을 종료합니다.\n");
    return 0;
}