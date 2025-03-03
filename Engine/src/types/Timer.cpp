#include "types.h"
Timer::Timer(const char* name) : name(name), start(std::chrono::high_resolution_clock::now()) {
    printf("Timer '%s' started.\n", name);
}

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(end - start).count();
    printf("Timer '%s' ended. Duration: %.3f ms.\n", name, duration);
}
