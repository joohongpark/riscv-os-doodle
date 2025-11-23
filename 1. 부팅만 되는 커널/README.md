# 1. 부팅만 되는 커널

아무 동작도 하지 않지만, 커널 이미지를 직접 만들어 빌드하고 QEMU에서 실행해 보기

## 주요 구성

### [kernel.c](kernel.c)
- `boot()`: 부트로더가 가장 먼저 실행하는 함수
  - 스택 포인터 설정 후 `kernel_main`으로 점프
  - `.text.boot` 섹션에 배치되어 진입점 역할
- `kernel_main()`: 커널 메인 함수
  - `memset()` 함수로 BSS 섹션 0으로 초기화
  - 이후 무한 루프를 실행해 아무 동작도 하지 않음.

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
3. `kernel_main()`으로 점프하여 BSS 초기화
4. 무한 루프 진입

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
 - OpenSBI 초기화 이후에 아무 동작 없어보임.
 - QEMU 모니터 (C-a -> info register) 또는 LLDB로 레지스터를 관찰해보면 특정 위치의 명령어를 실행 중임을 알 수 있음
 - `llvm-objdump` 명령어를 이용해 `kernel.elf` 파일을 확인해보면 PC 레지스터가 가리키는 주소의 명령어를 실행 중임을 알 수 있음.
1. elf 파일의 메모리 주소에 대한 명령어 확인
```
$> llvm-objdump -d kernel.elf
kernel.elf:     file format elf32-littleriscv

Disassembly of section .text:

80200000 <boot>:
80200000: 37 05 22 80   lui     a0, 0x80220
80200004: 13 05 45 04   addi    a0, a0, 0x44
80200008: 2a 81         mv      sp, a0
8020000a: 6f 00 60 01   j       0x80200020 <kernel_main>

8020000e <memset>:
8020000e: 01 ca         beqz    a2, 0x8020001e <memset+0x10>
80200010: 2a 96         add     a2, a2, a0
80200012: aa 86         mv      a3, a0
80200014: 23 80 b6 00   sb      a1, 0x0(a3)
80200018: 85 06         addi    a3, a3, 0x1
8020001a: e3 9d c6 fe   bne     a3, a2, 0x80200014 <memset+0x6>
8020001e: 82 80         ret

80200020 <kernel_main>:
80200020: 37 05 20 80   lui     a0, 0x80200
80200024: 13 05 45 04   addi    a0, a0, 0x44
80200028: b7 05 20 80   lui     a1, 0x80200
8020002c: 93 85 45 04   addi    a1, a1, 0x44
80200030: 33 86 a5 40   sub     a2, a1, a0
80200034: 11 c6         beqz    a2, 0x80200040 <kernel_main+0x20>
80200036: 23 00 05 00   sb      zero, 0x0(a0)
8020003a: 05 05         addi    a0, a0, 0x1
8020003c: e3 1d b5 fe   bne     a0, a1, 0x80200036 <kernel_main+0x16>
80200040: 01 a0         j       0x80200040 <kernel_main+0x20>
```
2. QEMU 모니터/LLDB를 통한 PC = 80200040
3. `80200040: 01 a0         j       0x80200040 <kernel_main+0x20>` 를 보면 무한루프를 돌고 있음을 알 수 있음.
- kernel.ld 파일에 선언된 스택포인터(SP)도 잘 초기화 된 것을 알 수 있음
  - PC = 80220044
  - `cat kernel.map | grep 80220044` -> `80220044 80220044        0     1 __stack_top = .`
  - 또는, `llvm-nm kernel.elf` 실행해 `__stack_top` 영역 확인
