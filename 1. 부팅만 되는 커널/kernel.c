typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

/**
 * @brief 커널의 BSS 섹션 시작 주소, 끝 주소, 스택 최상단 주소를 외부에서 참조합니다.
 */
extern char __bss[], __bss_end[], __stack_top[];

/**
 * @brief 메모리 영역을 특정 값으로 채웁니다.
 */
void *memset(void *buf, char ch, size_t n)
{
    uint8_t *p = buf;
    for (size_t i = 0; i < n; i++)
    {
        p[i] = (uint8_t)ch;
    }
    return buf;
}

/**
 * @brief 커널의 진입점 함수입니다.
 */
void kernel_main(void)
{
    // 전역 변수들이 쓰레기 값을 가지지 않도록 BSS 섹션을 0으로 초기화합니다.
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    // 아무것도 하지 않는 커널이므로 무한 루프에 빠집니다.
    while (1)
        ;
}

/**
 * @brief 부트로더가 커널을 로드한 후 가장 먼저 실행하는 함수입니다.
 *  - 스택 포인터를 설정하고 kernel_main 함수로 점프합니다.
 *  - naked 속성을 사용하여 함수 진입과 종료 시에 불필요한 코드 (프롤로그, 에필로그 등) 생성을 방지합니다.
 *  - section(".text.boot") 속성을 사용하여 이 함수를 부트 코드 섹션에 배치합니다.
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