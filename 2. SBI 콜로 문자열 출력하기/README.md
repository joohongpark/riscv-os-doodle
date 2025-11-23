# 2. SBI 콜로 문자열 출력하기

SBI(Supervisor Binary Interface) 콜을 이용해 콘솔에 문자열을 출력하는 커널 만들기

## 주요 구성

### [kernel.h](kernel.h)
- 타입 정의: `uint8_t`, `uint32_t`, `size_t`
- `struct sbiret`: SBI 호출의 반환 값을 나타내는 구조체
  - `error`: 오류 코드
  - `value`: 반환 값

### [kernel.c](kernel.c)
- `boot()`: 부트로더가 가장 먼저 실행하는 함수
  - 스택 포인터 설정 후 `kernel_main`으로 점프
  - `.text.boot` 섹션에 배치되어 진입점 역할
- `sbi_call()`: SBI 호출을 수행하는 함수
  - RISC-V의 `ecall` 명령어를 사용하여 M-mode(OpenSBI)와 통신
  - 인라인 어셈블리로 레지스터 a0~a7 설정 및 결과 반환
  - 인자: arg0~arg5 (함수별 인자), fid (함수 ID), eid (확장 ID)
  - 반환: `struct sbiret` (error, value)
- `putchar()`: 단일 문자를 콘솔에 출력하는 함수
  - SBI 콘솔 출력 함수 호출 (eid=1, fid=0)
- `kernel_main()`: 커널 메인 함수
  - "Hello, Kernel World!" 문자열을 문자 단위로 출력
  - WFI(Wait For Interrupt) 명령어로 무한 대기

### [kernel.ld](kernel.ld)
- 메모리 레이아웃을 정의하기 위한 링커 스크립트
- 진입점: `boot` 함수
- 메모리 배치: `0x80200000` 주소부터 시작
- 섹션 구성:
  - `.text`: 코드 영역 (부트 코드 우선 배치)
  - `.rodata`: 읽기 전용 데이터
  - `.data`: 초기화된 데이터
  - `.bss`: 초기화되지 않은 데이터 (`__bss`, `__bss_end` 심볼 제공)
- 스택: 128KB 할당 (`__stack_top` 심볼 제공)

## 커널 동작 순서

1. 부트로더가 커널을 `0x80200000`에 로드하고 `boot()` 실행
2. `boot()`에서 스택 포인터를 `__stack_top`으로 설정
3. `kernel_main()`으로 점프
4. "Hello, Kernel World!\n" 문자열을 순회하며 각 문자마다 `putchar()` 호출
5. `putchar()`에서 `sbi_call()`을 통해 SBI 콘솔 출력 함수 호출
6. WFI 명령어로 무한 대기 (절전 모드)

## SBI 콜 메커니즘

RISC-V에서 S-mode(커널)는 M-mode(펌웨어)의 서비스를 이용하기 위해 SBI를 사용합니다.

- **ecall 명령어**: S-mode에서 M-mode로 전환하여 SBI 함수 호출
- **레지스터 규약**:
  - `a0`~`a5`: 함수 인자 (arg0~arg5)
  - `a6`: 함수 ID (fid)
  - `a7`: 확장 ID (eid)
  - `a0`: 오류 코드 (반환)
  - `a1`: 반환 값 (반환)
- **콘솔 출력**: eid=1 (Legacy Console Putchar), fid=0

## 컴파일하기
```shell
clang \
  -std=c11 \
  -O2 \
  -g3 \
  -Wall \
  -Wextra \
  --target=riscv32-unknown-elf \
  -fuse-ld=lld \
  -fno-stack-protector \
  -ffreestanding \
  -nostdlib \
  -Wl,-Tkernel.ld \
  -Wl,-Map=kernel.map \
  -o kernel.elf \
    kernel.c
```
### 옵션 설명
 - `-std=c11`: C11 표준을 사용하고
 - `-O2`: 컴파일러의 최적화 레벨은 2단계로 하며
 - `-g3`: 최대한의 디버그 정보를 생성하고
 - `-Wall`: 핵심 경고를 활성화하고
 - `-Wextra`: 추가 경고도 활성화하고
 - `--target=riscv32-unknown-elf`: 타겟은 32비트 RISC-V로
 - `-fuse-ld=lld`: 링커는 LLVM 링커를 사용하고
 - `-fno-stack-protector`: 스택 보호기능을 활성화 하지 않는다. (clang 특정 버전부터 해당 옵션을 넣지 않으면 kernel_main 함수에 컴파일러가 임의의 코드를 삽입함.)
 - `-ffreestanding`: 호스트(개발 환경) 표준 라이브러리를 사용하지 않으며
 - `-nostdlib`: 표준 라이브러리를 링크하지 않는다.
 - `-Wl,-Tkernel.ld`: 링커 스크립트는 `kernel.ld`를 사용하고
 - `-Wl,-Map=kernel.map`: `kernel.map` 라는 이름으로 맵 파일을 생성한다.

## 실행 결과
 - OpenSBI 초기화 이후에 "Hello, Kernel World!" 문자열이 출력됨
 - 이후 WFI 명령어로 CPU가 절전 모드로 전환되어 대기

### 디버깅
1. elf 파일의 메모리 주소에 대한 명령어 확인
```
$> llvm-objdump -d kernel.elf
```
WFI 무한 루프의 어셈블리 코드를 확인할 수 있음
 - 타이머 인터럽트 등이 주기적으로 발생하기 때문에 WFI 명령어나 점프 명령어 위치를 가리키고 있음

2. QEMU 모니터 (C-a -> info registers) 또는 LLDB로 레지스터 상태 확인
   - PC가 WFI 명령어 또는 점프 명령어 위치를 가리키는지 확인
   - 여러 번 확인하면 두 위치를 번갈아가며 실행하는 것을 관찰 가능

3. 심볼 테이블 확인
```
$> llvm-nm kernel.elf
```
또는
```
$> cat kernel.map | grep __stack_top
```
