# STM32F446RE Custom Bootloader

A custom bootloader and application architecture developed for the STM32F446RE.

## Memory Layout

| Region             | Address      |
| ------------------ | -----------: |
| Bootloader         | `0x08000000` |
| Application Header | `0x08008000` |
| Application        | `0x0800C000` |

The application header occupies a dedicated 16 KB region between the bootloader and application image.

The header stores application metadata and is currently used to validate the application before attempting a bootloader-to-application handoff.

## Current Features

- Separate bootloader and application projects
- Custom Flash memory partitioning
- Dedicated 16 KB application header region
- Application linked to `0x0800C000`
- Application header placed at `0x08008000`
- Magic-number based application validation
- Application Reset Handler address validation
- Bootloader reads the application's initial MSP and Reset Handler
- Bootloader-to-application handoff
- MSP initialization before application jump
- SysTick cleanup before handoff
- Interrupt disabling before application jump
- Vector table relocation
- Application startup through its Reset Handler
- STM32 HAL / STM32CubeIDE

## Application Validation

Before jumping to the application, the bootloader validates:

1. The application header contains the expected magic number.
2. The application's Reset Handler points to the STM32 Flash memory region.

If validation fails, the bootloader does not attempt to jump to the application and instead enters a failure state.

The current application header contains:

```c
typedef struct
{
    uint32_t magic;
    uint32_t app_size;
    uint32_t version;
    uint32_t crc;
} AppHeader;
