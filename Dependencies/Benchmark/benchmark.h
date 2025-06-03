#pragma once

// Cheeky stub that just returns "good" results.
inline int RunBenchmark(int argc, char *argv[], float *floatResult, float *intResult, float *memResult)
{
    if (floatResult) {
        *floatResult = 10.0f;
    }
    
    if (intResult) {
        *intResult = 10.0f;
    }
    
    if (memResult) {
        *memResult = 10.0f;
    }

    return 1;
}
