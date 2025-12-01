#include "vmcd.h"

int main(void) {
    VendingMachine vm;
    init_vm(&vm);
    load_config(&vm);  // ★ 이 줄1
    int running = 1;
    while (running) {
        clear_screen();
        print_title();

        print_subtitle("관리자 메뉴");
        puts("1. 구성 불러오기 (또는 기본값 초기화)");
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
                progress_bar("구성을 준비하는 중", 20, 30);
                if (load_config(&vm)) {
                    printf("\n구성이 완료되었습니다.\n");
                } else {
                    printf("\n구성을 완료하지 못했습니다.\n");
                }
                wait_for_enter();
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