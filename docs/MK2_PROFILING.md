# MK2 - Whisper Bare-Metal Profiling

## Objective

Identify the performance bottleneck of Whisper Tiny
running on Raspberry Pi 5 with Circle bare metal.

## MK1 Baseline

| Metric | MK1 |
|---|---|
| Hardware | Raspberry Pi 5 |
| Model | Whisper Tiny |
| Input | Korean PCM, 16 kHz, 5 seconds |
| Threads | 1 |
| Inference time | 8720 ms |
| RTF | 1.744 |
| Korean STT | PASS |

Note: MK1 result is based on a single observed run.

## MK2-01: Internal Profiling

Changes:
- Add whisper_print_timings()
- Keep n_threads=1
- Keep the same model and audio

Measurements:
- Mel processing: Pending
- Encoder: Pending
- Decoder: Pending
- Other Whisper processing: Pending
- Total inference: Pending

## Optimization Process

1. Measure the baseline repeatedly
2. Identify the slowest operation
3. Analyze compiler output and ARM NEON usage
4. Optimize the identified bottleneck
5. Evaluate AArch64 assembly only where justified
6. Validate STT output and stability
7. Compare against MK1
8. Merge verified improvements into main

## Experiment History

| Experiment | Changes | Inference ms | RTF | Result |
|---|---|---|---|---|
| MK1 | Initial working STT | 8720 | 1.744 | PASS |
| MK2-01 | Internal profiling | Pending | Pending | Pending |

## Merge Conditions

- Bare-metal boot: PASS
- Model initialization: PASS
- Korean STT: PASS
- No new crashes or deadlocks
- Performance measured repeatedly
- Improvement demonstrated under matching conditions

## MK2-O3 Experiment

Hardware: Raspberry Pi 5
Runtime: Circle bare metal
Model: Whisper Tiny
Input: Korean PCM, 16 kHz, 5 seconds
Threads: 1

### Compiler modification

ggml CPU backend:
- Before: -O2 -mcpu=cortex-a76
- After:  -O3 -mcpu=cortex-a76

### Observed results

| Metric | MK2-O2 | MK2-O3 |
|---|---:|---:|
| Inference | 9809 ms | 5006 ms |
| Encoder | 9135.10 ms | 4484.59 ms |
| Decoder | 362.93 ms | 257.84 ms |
| Mel | 44.27 ms | 31.27 ms |
| RTF | 1.961 | 1.001 |

Observed inference-time reduction: 48.97%.

Note: These values are from individual runs.
Repeated measurements and output-text verification are pending.
