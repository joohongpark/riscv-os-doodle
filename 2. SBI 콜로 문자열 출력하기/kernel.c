#include "kernel.h" // 타입 정의

extern char __bss[], __bss_end[], __stack_top[];

/**
 * @brief SBI 호출을 수행하는 함수입니다.
 *
 * @param arg0 첫 번째 인자
 * @param arg1 두 번째 인자
 * @param arg2 세 번째 인자
 * @param arg3 네 번째 인자
 * @param arg4 다섯 번째 인자
 * @param arg5 여섯 번째 인자
 * @param fid 함수 ID
 * @param eid 확장 ID
 * @return struct sbiret SBI 호출의 반환 값과 오류 코드를 포함하는 구조체
 */
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid)
{
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    // SBI 호출을 위한 ecall 명령어 실행 (x10 ~ x11 (a0 ~ a1) 함수 인자 or 반환값 사용)
    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)                                                     // 출력 피연산자
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) // 입력 피연산자
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

/**
 * @brief 문자를 콘솔에 출력하는 함수입니다.
 *
 * @param ch 출력할 문자
 */
void putchar(char ch)
{
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1); // SBI 콘솔 출력 함수 호출
}

/**
 * @brief 커널의 진입점 함수입니다.
 */
void kernel_main(void)
{
    // "Hello, Kernel World!" 문자열을 콘솔에 출력합니다.
    const char *str = "Hello, Kernel World!\n";
    for (const char *p = str; *p != '\0'; p++)
    {
        putchar(*p);
    }

    // WFI(Wait For Interrupt) 명령어를 사용하여 CPU를 절전 모드로 전환합니다.
    while (1)
        __asm__ __volatile__("wfi");
}

/**
 * @brief 부트로더가 커널을 로드한 후 가장 먼저 실행하는 함수입니다.
 */
__attribute__((section(".text.boot")))
__attribute__((naked)) void
boot(void)
{
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // 스택 포인터 설정 (스택 주소는 링크 스크립트에서 정의됨)
        "j kernel_main\n"       // 커널 진입점으로 점프
        :
        : [stack_top] "r"(__stack_top));
}