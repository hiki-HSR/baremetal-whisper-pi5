# Raspberry Pi 5 Bare-Metal Whisper STT

Whisper Tiny speech recognition running on a Raspberry Pi 5
without Raspberry Pi OS or a Linux kernel.

## Project Overview

This project explores porting Whisper and ggml to a bare-metal
environment using the Circle framework.

The target language for STT verification is Korean.

## Hardware

- Raspberry Pi 5
- ARM Cortex-A76 CPU
- USB storage for Whisper model and audio
- INMP441 I2S microphone (planned for bare-metal integration)

## Software

- Circle bare-metal framework
- circle-stdlib
- whisper.cpp
- ggml CPU backend
- AArch64 bare-metal GNU toolchain

## Implemented Features

- Raspberry Pi 5 bare-metal boot
- USB filesystem support
- Whisper and ggml ARM64 compilation
- Static linking into Circle kernel
- Non-Apple ANE compatibility stub
- POSIX aligned memory allocation compatibility
- Experimental pthread compatibility layer
- ggml CPU backend initialization
- Whisper Tiny model initialization
- Korean speech-to-text inference from USB PCM
- ARM64 hardware-timer-based inference measurement
- UTF-8 transcription output to USB storage

## Verified Results

- Whisper Tiny model initialization: PASS
- ggml CPU backend registration: PASS
- Korean STT inference: PASS
- Audio input: 16 kHz, mono, 16-bit PCM
- Audio duration: 5 seconds
- Inference time: 8720 ms (single observed run)
- RTF: 1.744

These results do not establish a speed improvement over
Raspberry Pi OS. A controlled performance comparison is needed.

## Current Limitations

- STT input is loaded from USB, not captured live from INMP441.
- The pthread compatibility layer is experimental.
- Multithreaded inference has not been validated.
- Full reproducibility of the build has not yet been verified.
- Long-term stability has not been tested.

## Build Environment

The current project is built inside the circle-stdlib source tree.
The build scripts reference local Circle and whisper.cpp paths.

Required dependencies include:
- Raspberry Pi 5 compatible Circle version
- circle-stdlib with external libc++ threading
- whisper.cpp and ggml source trees
- aarch64-none-elf cross compiler

The exact upstream revisions and patches should be recorded
before treating this repository as a reproducible build.

## Files

- kernel.cpp / kernel.h: Circle application and Whisper STT test
- main.cpp: Bare-metal entry point
- circle-pthread.cpp: Experimental POSIX threading adapter
- circle-memory.c: Aligned allocation compatibility
- Build scripts: Cross-compilation and final-link experiments

## Status

Bare-metal Whisper Tiny model loading and Korean STT inference
have been demonstrated on Raspberry Pi 5 hardware.

Performance optimization and live I2S microphone integration
remain future work.
