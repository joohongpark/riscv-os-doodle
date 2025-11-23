# 해당 문서에서 RISC-V를 사용하는 이유
 - 명세가 단순함.
 - 명령어 셋이 트렌디함.
 - 문서화가 잘 되어 있음

## RISC-V 어셈블리
 - (https://godbolt.org)[https://godbolt.org] 에서 어떤 코드가 어떻게 어셈블리로 전환되는지 보면 좋음.

## RISC-V 어셈블리 기초
### 예제
 - `addi a0, a1, 123` : a1에 즉시값 123을 더해 a0에 저장함
 - `lw a0, (a1)` : 메모리 주소를 담은 레지스터 a1의 word(32비트) 길이의 값을 a0 레지스터에 저장
 - `sw a0, (a1)` : a0 레지스터의 word(32비트) 길이의 값을 메모리 주소를 담은 레지스터 a1의 메모리에 저장
### 레지스터
 - x0 (zero) : 항상 0인 레지스터
 - x1 (ra) : 함수 호출시 복귀 주소 저장
 - x2 (sp) : 스택 포인터
 - x5 ~ x7 (t0 ~ t2), x28 ~ x31 (t3 ~ t6) : 임시용 레지스터
 - x8 (fp) : 스택 프레임 포인터
 - x10 ~ x11 (a0 ~ a1) 함수 인자 or 반환값
 - x12 ~ x17 (a2 ~ a7) 함수 인자
 - x18 ~ x27 (s0 ~ s11) 함수 호출 사이에도 값이 보존되는 레지스터
### CPU 모드
 - M-mode: 펌웨어, 부트로더 등
 - S-mode: 커널 모드
 - U-mode: 유저 모드

# OpenSBI
## Supervisor Binary Interface(SBI)
 - RISC-V에서 M-mode(Machine mode)와 S-mode(Supervisor mode) 간의 표준 인터페이스 스펙
 - OS 커널(S-mode)이 특권 명령이 필요한 작업(타이머 설정, 프로세서 간 인터럽트, 시스템 리셋 등)을 할 때, ecall 명령으로 M-mode 펌웨어에 요청하는 방식을 정의함.
## OpenSBI
 - Supervisor Binary Interface의 오픈소스 구현체
 - BIOS/UEFI처럼 부팅 초기에 하드웨어를 초기화하고 ecall 명령으로 하드웨어에 직접 접근하지 않고도 동작할 수 있게 해줌.

# QEMU 실행
```bash
qemu-system-riscv32 \
  -machine virt \
  -bios qemu/opensbi-riscv32-generic-fw_dynamic.bin \
  -nographic \
  -serial mon:stdio \
  --no-reboot \
  -gdb tcp::1234 \
  -S \
  -kernel kernel.elf
```
 - machine: 머신 종류 (none, virt, opentitan 등이 있음)
 - bios: 펌웨어(OpenSBI) 지정
 - nographic: 그래픽 미사용
 - serial: 표준 입출력을 가상 머신의 시리얼 포트에 연결 (mon으로 Ctrl+A 이후 C를 눌러 QEMU 모니터로 전환할 수 있게 함.)
 - no-reboot: 가상 머신이 크래시되면 즉시 종료
 - kernel: 실행할 커널 이미지 (bzImage) 지정
 - (옵션) gdb or s: GDB 서버 실행
 - (옵션) S: 시작 시 CPU 중지 상태로 실행

## 실행
 - 위에서 (옵션) 항목을 빼고 실행하거나
 - 위대로 실행하고 C-a -> c를 눌러 실행하거나
 - 위대로 실행하고 gdb (lldb) 를 열고
   - gdb-remote localhost:1234
   - c (또는 continue)를 눌러 실행

### 참고 - LLDB 기본 명령어
```
# QEMU 연결
(lldb) gdb-remote localhost:1234
(lldb) gdb-remote 127.0.0.1:1234

# 실행 제어
(lldb) continue     # 또는 c - 계속 실행
(lldb) step         # 또는 s - 한 줄 실행 (함수 안으로)
(lldb) next         # 또는 n - 한 줄 실행 (함수 건너뜀)
(lldb) stepi        # 또는 si - 한 명령어 실행
(lldb) nexti        # 또는 ni - 한 명령어 실행 (call 건너뜀)

# 모든 범용 레지스터
(lldb) register read

# 특정 레지스터
(lldb) register read zero

# 레지스터 값 변경
(lldb) register write t2 0x1234

# 메모리 읽기 (기본: 32바이트)
(lldb) memory read 0x100000
(lldb) memory read $rip

# 형식 지정
(lldb) memory read --format hex --size 4 --count 16 0x100000
# 약어: -fx -s4 -c16

# 자주 쓰는 포맷들
(lldb) memory read -fx -c32 $rsp        # 스택 (hex, 4바이트씩 32개)
(lldb) memory read -fb -c64 0x1000      # 바이너리
(lldb) memory read -fc -c100 0x2000     # 문자열 (char)
(lldb) memory read -fi -c16 0x3000      # 정수

# 메모리 쓰기
(lldb) memory write 0x200000 0x12345678

# 중단
(lldb) process interrupt  # 또는 Ctrl+C
```

### 참고 - QEMU 모니터 기본 명령
```
C-a h    도움말 표시
C-a x    에뮬레이터 종료
C-a s    디스크 데이터를 파일에 저장(-snapshot 사용 시)
C-a t    콘솔 타임스탬프 토글
C-a b    break(매직 sysrq)
C-a c    콘솔과 모니터 간 전환
C-a C-a  C-a를 전송
```