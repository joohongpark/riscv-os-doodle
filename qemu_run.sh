#!/bin/bash
set -xue

# QEMU 실행 파일 경로
QEMU=./qemu/qemu-system-riscv32

# BIOS 파일 경로
BIOS=./qemu/opensbi-riscv32-generic-fw_dynamic.bin

# dylib 라이브러리 경로 설정
export DYLD_LIBRARY_PATH=./qemu:${DYLD_LIBRARY_PATH:-}

# QEMU 실행
$QEMU -machine virt -bios $BIOS -nographic -serial mon:stdio --no-reboot