#!/bin/bash

# RISC-V 개발 환경 설정 스크립트
# Usage: source setup_env.sh

# 현재 스크립트 디렉토리
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# QEMU 관련 설정
export QEMU_PATH="${SCRIPT_DIR}/qemu"
export DYLD_LIBRARY_PATH="${QEMU_PATH}:${DYLD_LIBRARY_PATH:-}"
export PATH="${QEMU_PATH}:${PATH}"

# LLVM 관련 설정
export LLVM_PATH="${SCRIPT_DIR}/llvm"
export PATH="${LLVM_PATH}/bin:${PATH}"

# 환경변수 확인 함수
check_env() {
    echo "=== RISC-V 개발 환경 설정 완료 ==="
    echo "QEMU Path: $QEMU_PATH"
    echo "LLVM Path: $LLVM_PATH"
    echo "DYLD_LIBRARY_PATH: $DYLD_LIBRARY_PATH"
    echo "PATH에 LLVM bin 추가됨: ${LLVM_PATH}/bin"
    echo ""

    # 실행 파일 확인
    if [ -x "${QEMU_PATH}/qemu-system-riscv32" ]; then
        echo "✓ QEMU RISC-V32 찾음: ${QEMU_PATH}/qemu-system-riscv32"
    else
        echo "✗ QEMU RISC-V32 실행 파일을 찾을 수 없음"
    fi

    if [ -x "${LLVM_PATH}/bin/clang" ]; then
        echo "✓ LLVM Clang 찾음: ${LLVM_PATH}/bin/clang"
    else
        echo "✗ LLVM Clang 실행 파일을 찾을 수 없음"
    fi

    echo "=================================="
}

# 환경 설정 확인
check_env