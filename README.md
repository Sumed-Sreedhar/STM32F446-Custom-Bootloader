# STM32F446RE Custom Bootloader

A custom bootloader and application architecture for the STM32F446RE. The bootloader validates an application stored in internal Flash using application metadata and CRC-32 before transferring execution to it.

## Memory Layout

| Region | Start Address | Size |
|---|---:|---:|
| Bootloader | `0x08000000` | 32 KB |
| Application Header | `0x08008000` | 16 KB |
| Application Image | `0x0800C000` | 464 KB |

The Flash memory is divided into three regions. The bootloader occupies the first 32 KB, followed by a dedicated 16 KB application header region. The application image is linked to start at `0x0800C000`.

The memory layout is defined in `flash_layout.h` and configured in the application linker script.

## Application Header

The application header is placed in a dedicated `.header` linker section at `0x08008000`.

```c
typedef struct
{
    uint32_t magic;
    uint32_t size;
    uint32_t crc;
} app_header_t;
```

| Field | Description |
|---|---|
| `magic` | Identifies a valid application header. Expected value: `0xABCDEFAB`. |
| `size` | Application image size in bytes. |
| `crc` | Expected CRC-32 checksum of the application image. |

The header structure occupies 12 bytes. The surrounding 16 KB region is reserved in the Flash memory layout.

## Implemented Features

- Separate bootloader and application projects.
- Custom Flash memory partitioning.
- Application header stored at `0x08008000`.
- Application image linked at `0x0800C000`.
- Magic-number validation.
- Reset Handler address validation.
- Application size validation.
- Hardware CRC-32 verification using the STM32 CRC peripheral.
- UART messages for bootloader status and validation errors.
- Application handoff using the application's initial stack pointer and Reset Handler.
- Interrupt disabling and SysTick cleanup before handoff.
- Vector table relocation in the application.
- PA4 LED blinking when application validation fails.
- Python utility for calculating application size and CRC.

## Application Validation

The `bootloader_is_app_valid()` function validates the application before the bootloader attempts to execute it.

The checks are performed in the following order:

1. **Magic number check:** Reads the header from `APP_HEADER_ADDR` and compares its magic field against `APP_MAGIC`.
2. **Reset Handler check:** Reads the Reset Handler address from `APP_START_ADDR + 4` and checks that it lies within the configured application Flash region.
3. **Application size check:** Rejects an application whose size is zero or exceeds `APP_MAX_SIZE`.
4. **CRC check:** Calculates the CRC-32 over the application image and compares it with the CRC stored in the header.

The validation function returns a status code:

| Return Value | Meaning |
|---:|---|
| `0` | Application is valid |
| `1` | Magic number check failed |
| `2` | Reset Handler check failed |
| `3` | Application size check failed |
| `4` | CRC check failed |

If all checks pass, the bootloader calls `JumpToApplication()`. If validation fails, it prints the corresponding error over UART and remains in the bootloader failure state, blinking the PA4 LED.

## CRC Implementation

The bootloader uses the STM32F446RE hardware CRC peripheral through the STM32 HAL.

The CRC configuration uses:

- Polynomial: `0x04C11DB7`
- Initial value: `0xFFFFFFFF`
- Input data: 32-bit words
- Input size: Must be divisible by four bytes

The `CRC32_Calculate()` wrapper calls `HAL_CRC_Calculate()` to calculate the checksum over the application image.

The application CRC is calculated from `APP_START_ADDR`, using the size stored in the application header. The header is excluded from the CRC calculation because it resides in a separate Flash region.

## Application Binary and CRC Generation

The application ELF contains the application image and the `.header` section. A post-build command generates a header-free binary for CRC calculation:

```bash
arm-none-eabi-objcopy -O binary --remove-section=.header \
    "${BuildArtifactFileName}" \
    "${BuildArtifactFileBaseName}_app.bin"
```

The generated `_app.bin` file contains the application image without the `.header` section. The Python utility reads this binary and calculates its size and CRC using the same CRC convention as the STM32 hardware peripheral.

The resulting size and CRC values are written into the application header.

The CRC is calculated over the binary image beginning at `0x0800C000`, excluding the header stored at `0x08008000`.

## Bootloader-to-Application Handoff

After successful validation, `JumpToApplication()` transfers execution to the application.

The handoff procedure includes:

1. Reading the application's initial Main Stack Pointer (MSP) from `APP_START_ADDR`.
2. Reading the application's Reset Handler from `APP_START_ADDR + 4`.
3. Disabling interrupts.
4. Stopping SysTick.
5. Setting the MSP to the application's initial stack pointer.
6. Jumping to the application's Reset Handler.

The application sets `SCB->VTOR` to `APP_START_ADDR`, enables interrupts, initializes its peripherals, and enters its main application logic.

## Testing

The bootloader has been tested on the STM32F446RE hardware using UART output to observe its behaviour.

| Test | Expected Behaviour | Result |
|---|---|---|
| Valid application | Validation succeeds and execution transfers to the application. | Passed |
| Invalid CRC | Application is rejected and the CRC failure is reported. | Passed |
| Invalid magic number | Application is rejected and the magic-number failure is reported. | Passed |

Successful application boot produced:

```text
Bootloader is running...
Application is running...
```

The CRC failure test produced:

```text
Bootloader is running...
Failed to jump!
CRC check failed!
```

The invalid magic-number test was also performed and behaved as expected.

## Tools and Technologies

- STM32F446RE
- STM32CubeIDE
- STM32 HAL
- STM32 hardware CRC peripheral
- GNU Arm Embedded Toolchain
- `arm-none-eabi-objcopy`
- Python 3
- UART terminal for debugging and validation

## Current Limitations

- The bootloader validates an application already programmed into internal Flash.
- Firmware downloading and in-field application updates are not implemented.
- Automatic recovery and fallback to another application image are not implemented.
- The CRC wrapper currently rejects input sizes that are not divisible by four bytes.

