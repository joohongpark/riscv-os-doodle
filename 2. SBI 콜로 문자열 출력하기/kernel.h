typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

#pragma once

/**
 * @brief SBI 호출의 반환 값을 나타내는 구조체입니다.
 */
struct sbiret
{
    long error;
    long value;
};