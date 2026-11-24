//
// kernel.cpp
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <cstdint>
#include "../../../whisper.cpp/include/whisper.h"
#include "../../../whisper.cpp/ggml/include/ggml-backend.h"
#include "../../../whisper.cpp/ggml/include/ggml-cpu.h"

#include <algorithm>
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#include <climits>
#include <limits>

namespace
{

void cxx_test(void);

}

CKernel::CKernel(void) : CStdlibAppStdio("09-usb-std", "USB:")
{
    mActLED.Blink(5); // show we are alive
}

bool CKernel::Initialize(void)
{
    if (!CStdlibAppScreen::Initialize())
    {
        return false;
    }

    mLogger.Write(GetKernelName(), LogNotice,
                  "Initializing USB...");

    if (!mUSBHCI.Initialize())
    {
        mLogger.Write(GetKernelName(), LogError,
                      "USB initialization FAILED");
        return false;
    }

    mLogger.Write(GetKernelName(), LogNotice,
                  "Mounting USB filesystem...");

    if (f_mount(&mFileSystem, "USB:", 1) != FR_OK)
    {
        mLogger.Write(GetKernelName(), LogError,
                      "USB filesystem mount FAILED");
        return false;
    }

    if (!mConsole.Initialize())
    {
        return false;
    }

    CGlueStdioInit(mConsole);

    mLogger.Write(GetKernelName(), LogNotice,
                  "USB INITIALIZATION PASSED");

    return true;
}

CStdlibApp::TShutdownMode CKernel::Run(void)
{
    std::cout << std::endl;
    std::cout << "=== BARE METAL KOREAN STT TEST ==="
              << std::endl;

    // 1. Audio file
    const char *audio_path = "USB:/AUDIO.PCM";

    std::cout << "[1] Opening Korean audio..."
              << std::endl;

    std::ifstream file(
        audio_path,
        std::ios::binary | std::ios::ate
    );

    if (!file.is_open()) {
        std::cout << "[FAIL] Audio file not found"
                  << std::endl;
        return ShutdownHalt;
    }

    std::streamoff size = file.tellg();

    if (size <= 0 ||
        size % sizeof(int16_t) != 0 ||
        size < 16000 * 2 ||
        size > 16000 * 2 * 10) {
        std::cout << "[FAIL] Invalid audio size"
                  << std::endl;
        return ShutdownHalt;
    }

    size_t samples_count =
        static_cast<size_t>(size) / sizeof(int16_t);

    std::vector<int16_t> pcm(samples_count);

    file.seekg(0, std::ios::beg);

    file.read(
        reinterpret_cast<char *>(pcm.data()),
        static_cast<std::streamsize>(size)
    );

    if (!file || file.gcount() != size) {
        std::cout << "[FAIL] Audio read error"
                  << std::endl;
        return ShutdownHalt;
    }

    file.close();

    std::vector<float> audio(samples_count);

    for (size_t i = 0; i < samples_count; ++i) {
        audio[i] =
            static_cast<float>(pcm[i]) / 32768.0f;
    }

    std::cout << "[PASS] Audio loaded" << std::endl;
    std::cout << "Samples: " << samples_count
              << std::endl;

    // 2. Initialize Whisper with inference state
    std::cout << "[2] Loading Whisper Tiny..."
              << std::endl;

    whisper_context_params ctx_params =
        whisper_context_default_params();

    ctx_params.use_gpu = false;

    whisper_context *ctx =
        whisper_init_from_file_with_params(
            "USB:/TINY.BIN",
            ctx_params
        );

    if (!ctx) {
        std::cout << "[FAIL] Whisper initialization"
                  << std::endl;
        return ShutdownHalt;
    }

    std::cout << "[PASS] Whisper initialized"
              << std::endl;

    // 3. Configure inference
    whisper_full_params params =
        whisper_full_default_params(
            WHISPER_SAMPLING_GREEDY
        );

    params.language = "ko";
    params.translate = false;
    params.n_threads = 1;
    params.no_timestamps = true;
    params.print_progress = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    params.print_special = false;

    std::cout << "[3] Running Korean STT..."
              << std::endl;


    // ARM64 generic hardware timer
    uint64_t timer_start = 0;
    uint64_t timer_end = 0;
    uint64_t timer_freq = 0;

    asm volatile("isb" ::: "memory");
    asm volatile("mrs %0, cntfrq_el0" : "=r"(timer_freq));
    asm volatile("mrs %0, cntvct_el0" : "=r"(timer_start));

    int result = whisper_full(
        ctx,
        params,
        audio.data(),
        static_cast<int>(audio.size())
    );


    asm volatile("isb" ::: "memory");
    asm volatile("mrs %0, cntvct_el0" : "=r"(timer_end));

    if (timer_freq > 0 && timer_end >= timer_start) {
        uint64_t elapsed_ms =
            ((timer_end - timer_start) * 1000ULL) / timer_freq;

        std::cout << "[TIME] Inference: "
                  << elapsed_ms << " ms" << std::endl;

        std::cout << "[TIME] RTF x1000: "
                  << (elapsed_ms * 1000ULL) /
                     (audio.size() / 16)
                  << std::endl;
    }

    if (result != 0) {
        std::cout << "[FAIL] Whisper inference: "
                  << result << std::endl;
        whisper_free(ctx);
        return ShutdownHalt;
    }

    std::cout << "[PASS] Whisper inference"
              << std::endl;

    // 4. Print transcription
    std::cout << "=== TRANSCRIPTION ==="
              << std::endl;

    std::ofstream result_file(
        "USB:/result.txt",
        std::ios::binary | std::ios::trunc
    );

    int segments = whisper_full_n_segments(ctx);

    std::cout << "Segments: " << segments << std::endl;

    for (int i = 0; i < segments; ++i) {
        const char *text =
            whisper_full_get_segment_text(ctx, i);

        if (text) {
            std::cout << "Segment " << i
                      << " processed." << std::endl;

            if (result_file.is_open()) {
                result_file << text << "\n";
            }
        }
    }

    if (result_file.is_open()) {
        result_file.flush();

        if (result_file.good()) {
            std::cout << "[PASS] Korean result saved"
                      << std::endl;
        } else {
            std::cout << "[FAIL] Result write error"
                      << std::endl;
        }

        result_file.close();
    } else {
        std::cout << "[FAIL] Cannot create result.txt"
                  << std::endl;
    }

    std::cout << "=== STT TEST FINISHED ==="
              << std::endl;

    whisper_free(ctx);

    return ShutdownHalt;
}

namespace
{
struct ooops : std::exception
{
    const char *what() const noexcept
    {
        return "Ooops!";
    }
};

void barf(void)
{
    std::cerr << "Throwing exception..." << std::endl;
    throw ooops();
}

struct a
{
    static unsigned long counter;
    a()
    {
        counter += 1;
    }
    ~a()
    {
        assert(counter > 0);
        counter -= 1;
        if (counter == 0)
        {
            std::cout << "all 'struct a' instances cleaned up..." << std::endl;
        }
    }
    int filler[5];
};

unsigned long a::counter = 0;

void cxx_test(void)
{
    std::vector<std::string> const v = {"vector entry 1", "vector entry 2"};

    std::cout << "Opening file via std::ofstream..." << std::endl;

    std::ofstream ofs("USB:/test.txt", std::ofstream::out);
    if (!ofs.is_open())
    {
        std::cerr << "Failed to open file 'test.txt'!" << std::endl;
        return;
    }

    try
    {
        barf();
    }
    catch (std::exception &e)
    {
        std::cerr << "Caught exception..." << std::endl;
        ofs << "lorem ipsum" << std::endl;
        std::string s(e.what());
        ofs << s.c_str() << std::endl;
    }
    std::cout << "Use <algorithm>..." << std::endl;
    for_each(v.begin(), v.end(),
             [&](std::string const &s) { ofs << s.c_str() << std::endl; });

    // Test out-of-memory condition
    try
    {
        std::cout << "size of a: " << sizeof(a) << std::endl;

        std::vector<std::unique_ptr<a[]>> ptrs;
        while (true)
        {
            // provoke out-of-memory error, destructors of "a" and of the vector
            // must be called
            std::cout << "Allocating large array of 'a' instances" << std::endl;
            ptrs.emplace_back(new a[10000000U]);
            std::cout << "Allocated pointer " << std::hex << ptrs.back().get()
                      << std::endl;
        }
    }
    catch (std::bad_alloc &ba)
    {
        std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
    }

    ofs.close();

    std::wstring const ws = L"Hello, world with wchar_t!";
    std::wcout << ws.c_str() << std::endl;

    std::string_view const s{"Hello, world with std::string_view!"};
    std::cout << s << std::endl;

    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 9);

    // Generate and print 10 random numbers in the range [0, 9].
    std::cout << "Random numbers:";
    for (int i = 0; i < 10; ++i)
    {
        std::cout << " " << dist(rd);
    }
    std::cout << std::endl;

    std::cout << "Limits:" << std::endl;
    std::cout << "  long long: " << std::numeric_limits<long long>::min()
              << " to " << std::numeric_limits<long long>::max() << std::endl;
    
    std::cout << "Limit macros:" << std::endl;
    std::cout << "  LLONG_MIN: " << LLONG_MIN << std::endl;
    std::cout << "  LLONG_MAX: " << LLONG_MAX << std::endl;
}
} // namespace
