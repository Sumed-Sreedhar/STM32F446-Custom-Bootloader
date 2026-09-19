# STM32F446RE Custom Bootloader

A custom bootloader and application architecture developed for the STM32F446RE.

## Memory Layout

| Region | Address |
|---|---:|
| Bootloader | `0x08000000` |
| Application Header | `0x08008000` |
| Application | `0x0800C000` |

The application header occupies the memory region between the bootloader and application image and is intended to store application metadata.

## Current Features

- Separate bootloader and application projects
- Custom Flash memory partitioning
- Application linked to `0x0800C000`
- Bootloader reads the application's initial MSP and Reset Handler
- Vector table relocation
- Bootloader-to-application handoff
- STM32 HAL / STM32CubeIDE

## Hardware

- STM32F446RE

## Project Structure

```text
STM32F446RE-Custom-Bootloader/
├── bootloader/
├── application/
├── docs/
├── .gitignore
└── README.md
