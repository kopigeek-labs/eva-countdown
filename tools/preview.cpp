// SPDX-License-Identifier: Apache-2.0
#include "render.h"
#include <cstdio>
#include <cstdlib>
int main(int argc, char **argv) {
    const int64_t remaining = argc > 1 ? std::atoll(argv[1]) : 300000000;
    const bool running = argc > 2 ? std::atoi(argv[2]) : false;
    const int64_t now = argc > 3 ? std::atoll(argv[3]) : 0;
    eva::Frame f;
    eva::render(f, remaining, running, now);
    std::printf("P4\n400 300\n");
    for (auto b : f.data)
        std::putchar(b ^ 0xff); // PBM uses 1 for black.
}
