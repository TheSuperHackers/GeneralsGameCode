/*
 * Times GameMath against the system math library, function by function.
 *
 * For every function four timings are taken: the GameMath double entry point,
 * the system double one, and the same pair in float. Each timing is the best of
 * several rounds, because the scheduler can move the thread to a slower core.
 *
 * Results are accumulated into a volatile sink so the calls cannot be optimised
 * away, and every configuration runs the same input set the same number of
 * times.
 *
 * On 32-bit x86 the whole table runs twice, once with the x87 precision control
 * set to _PC_24 and once with _PC_53, since the game build sets _PC_24 in
 * setFPMode(). Only two x87 instructions change speed with that setting, fdiv
 * and fsqrt, so the difference is expected to sit in the functions that divide
 * or take roots. Elsewhere there is no x87 precision control and the table runs
 * once.
 *
 * The mode order can be given on the command line - "bench_game_math pc53 pc24"
 * - so the same pair can be measured both ways round and the ordering ruled out
 * as the cause of a difference.
 *
 * Output goes to a file named after the platform, the architecture, the /fp
 * model and the precision control, and to stdout.
 *
 * macOS:
 *   cc -O2 -ffp-contract=off -Wno-macro-redefined Benchmarks/GameMath/Source/bench_game_math.c \
 *      -I build/macos/_deps/gamemath-src/include \
 *      build/macos/_deps/gamemath-build/libgm.a -lm -o bench_game_math
 *   ./bench_game_math
 *
 * win32 x86, from a Developer Command Prompt:
 *   cl /O2 /fp:precise /MD /wd4005 Benchmarks\GameMath\Source\bench_game_math.c ^
 *      /I build\win32\_deps\gamemath-src\include ^
 *      build\win32\_deps\gamemath-build\Release\gm.lib ^
 *      /Fe:bench_game_math.exe
 *   bench_game_math.exe
 */

#ifdef _WIN32
#include <windows.h>
#include <float.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "gmath.h"

#define NV     6
#define CALLS  1000000L
#define REPS   (CALLS / NV)
#define ROUNDS 3

static volatile double g_sink_d;
static volatile float  g_sink_f;

/* ---------- clock ---------- */

#ifdef _WIN32
static double now_ns(void)
{
    static LARGE_INTEGER freq;
    LARGE_INTEGER c;

    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1e9 / (double)freq.QuadPart;
}
#else
static double now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}
#endif

/* ---------- output ---------- */

static FILE *g_out;

static void emit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    if (!g_out) {
        return;
    }
    va_start(ap, fmt);
    vfprintf(g_out, fmt, ap);
    va_end(ap);
}

/* ---------- input sets ---------- */

/* general values */
static const double v_gen[]  = { 0.5, -0.5, 2.7, 187.66, 0.000123, 100.5 };
/* positive only */
static const double v_pos[]  = { 0.5, 1.0, 2.7, 187.66, 0.000123, 100.5 };
/* inside [-1, 1] */
static const double v_unit[] = { -0.9, -0.5, 0.0, 0.25, 0.5, 0.9 };
/* one and above */
static const double v_ge1[]  = { 1.0, 1.5, 2.7, 10.0, 187.66, 1000.0 };

static const double v_pair[][2] = {
    { 0.4, 1.3 }, { 2.7, 11.9 }, { 187.66, -59.13 },
    { 0.000123, 9999.5 }, { 1.0, -1.0 }, { -3.5, 2.0 }
};

/*
 * The timed expressions read their inputs from here rather than from the
 * constant arrays above. Without this the compiler folds calls to the system
 * functions at compile time, since it knows what sin(0.5) is, while the
 * GameMath calls stay opaque and the comparison becomes meaningless.
 */
static volatile double vin[NV];
static volatile double vinx[NV];
static volatile double viny[NV];

static void load1(const double *s)
{
    int i;
    for (i = 0; i < NV; ++i) vin[i] = s[i];
}

static void load2(void)
{
    int i;
    for (i = 0; i < NV; ++i) { vinx[i] = v_pair[i][0]; viny[i] = v_pair[i][1]; }
}

static void row(const char *name, double gd, double sd, double gf, double sf)
{
    emit("%-12s %9.1f %9.1f %7.2fx %9.1f %9.1f %7.2fx\n",
         name, gd, sd, (sd > 0.0 ? gd / sd : 0.0),
               gf, sf, (sf > 0.0 ? gf / sf : 0.0));
}

/* Best of ROUNDS, in nanoseconds per call. */
#define TIME_D(expr, out)                                                   \
    do {                                                                    \
        int rd; double best = 1e30;                                         \
        for (rd = 0; rd < ROUNDS; ++rd) {                                   \
            double acc = 0.0; long r; int i;                                \
            double t0 = now_ns();                                           \
            for (r = 0; r < REPS; ++r)                                      \
                for (i = 0; i < NV; ++i) acc += (expr);                     \
            { double t1 = now_ns();                                         \
              double ns = (t1 - t0) / (double)(REPS * NV);                  \
              if (ns < best) best = ns; }                                   \
            g_sink_d = acc;                                                 \
        }                                                                   \
        (out) = best;                                                       \
    } while (0)

#define TIME_F(expr, out)                                                   \
    do {                                                                    \
        int rd; double best = 1e30;                                         \
        for (rd = 0; rd < ROUNDS; ++rd) {                                   \
            float acc = 0.0f; long r; int i;                                \
            double t0 = now_ns();                                           \
            for (r = 0; r < REPS; ++r)                                      \
                for (i = 0; i < NV; ++i) acc += (expr);                     \
            { double t1 = now_ns();                                         \
              double ns = (t1 - t0) / (double)(REPS * NV);                  \
              if (ns < best) best = ns; }                                   \
            g_sink_f = acc;                                                 \
        }                                                                   \
        (out) = best;                                                       \
    } while (0)

#define BENCH1(base, set)                                                   \
    do {                                                                    \
        double gd, sd, gf, sf;                                              \
        load1(set);                                                         \
        TIME_D(gm_##base(vin[i]), gd);                                      \
        TIME_D(base(vin[i]), sd);                                           \
        TIME_F(gm_##base##f((float)vin[i]), gf);                            \
        TIME_F(base##f((float)vin[i]), sf);                                 \
        row(#base, gd, sd, gf, sf);                                         \
    } while (0)

#define BENCH2(base)                                                        \
    do {                                                                    \
        double gd, sd, gf, sf;                                              \
        load2();                                                            \
        TIME_D(gm_##base(vinx[i], viny[i]), gd);                            \
        TIME_D(base(vinx[i], viny[i]), sd);                                 \
        TIME_F(gm_##base##f((float)vinx[i], (float)viny[i]), gf);           \
        TIME_F(base##f((float)vinx[i], (float)viny[i]), sf);                \
        row(#base, gd, sd, gf, sf);                                         \
    } while (0)

static void run_all(void)
{
    emit("%-12s %9s %9s %8s %9s %9s %8s\n",
         "", "gm dbl", "sys dbl", "ratio", "gm flt", "sys flt", "ratio");
    emit("%-12s %9s %9s %8s %9s %9s %8s\n",
         "------------", "---------", "---------", "--------",
         "---------", "---------", "--------");

    BENCH1(sin,   v_gen);
    BENCH1(cos,   v_gen);
    BENCH1(tan,   v_gen);
    BENCH1(asin,  v_unit);
    BENCH1(acos,  v_unit);
    BENCH1(atan,  v_gen);
    BENCH2(atan2);

    BENCH1(sinh,  v_gen);
    BENCH1(cosh,  v_gen);
    BENCH1(tanh,  v_gen);
    BENCH1(asinh, v_gen);
    BENCH1(acosh, v_ge1);
    BENCH1(atanh, v_unit);

    BENCH1(exp,   v_gen);
    BENCH1(exp2,  v_gen);
    BENCH1(expm1, v_gen);
    BENCH1(log,   v_pos);
    BENCH1(log10, v_pos);
    BENCH1(log1p, v_pos);
    BENCH1(logb,  v_pos);
    BENCH2(pow);

    BENCH1(sqrt,  v_pos);
    BENCH1(cbrt,  v_gen);
    BENCH2(hypot);

    BENCH1(ceil,  v_gen);
    BENCH1(floor, v_gen);
    BENCH1(trunc, v_gen);
    BENCH1(round, v_gen);
    BENCH1(rint,  v_gen);

    BENCH1(fabs,  v_gen);
    BENCH2(fmod);
    BENCH2(remainder);
    BENCH2(copysign);
    BENCH2(fmax);
    BENCH2(fmin);

    BENCH1(erf,   v_unit);
    BENCH1(erfc,  v_unit);
    BENCH1(lgamma, v_pos);

    /* j0 and y0 are left out: macOS libm has no j0f or y0f to compare against. */
}

/* ---------- what we are running on ----------
 *
 * The file name carries the configuration, so several builds can drop their
 * results side by side without overwriting each other. Same scheme as
 * verify_game_math.c.
 */

#if defined(_M_IX86) || defined(__i386__)
#define ARCH_TAG "x86"
#elif defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
#define ARCH_TAG "x64"
#elif defined(_M_ARM64) || defined(__aarch64__) || defined(__arm64__)
#define ARCH_TAG "arm64"
#else
#define ARCH_TAG "unknown"
#endif

#ifdef _WIN32
#if defined(_M_FP_STRICT)
#define FP_TAG "strict"
#elif defined(_M_FP_FAST)
#define FP_TAG "fast"
#elif defined(_M_FP_PRECISE)
#define FP_TAG "precise"
#else
#define FP_TAG "fpdefault"
#endif
#define PLAT_TAG "win"
#else
#define FP_TAG "clang"
#define PLAT_TAG "mac"
#endif

#define BASE_TAG PLAT_TAG "-" ARCH_TAG "-" FP_TAG

/* x87 precision control only exists on 32-bit x86. */
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
#define HAS_X87_PC 1
#else
#define HAS_X87_PC 0
#endif

static void run_to_file(const char *path, const char *title)
{
    g_out = fopen(path, "w");
    if (!g_out) {
        printf("cannot write %s\n", path);
        return;
    }

    emit("%s\n", title);
    emit("Nanoseconds per call, best of %d rounds of %ld calls.\n",
         ROUNDS, CALLS);
    emit("Lower is better. The ratio is GameMath divided by system.\n\n");

    run_all();

    fclose(g_out);
    g_out = NULL;
    printf("wrote %s\n\n", path);
}

int main(int argc, char **argv)
{
#if HAS_X87_PC
    static const char *default_order[] = { "pc24", "pc53" };
    const char **order = default_order;
    int nmodes = 2;
    unsigned int cw;
    int m;

    if (argc > 1) {
        order = (const char **)(argv + 1);
        nmodes = argc - 1;
    }

    for (m = 0; m < nmodes; ++m) {
        if (strcmp(order[m], "pc24") == 0) {
            _controlfp_s(&cw, _PC_24, _MCW_PC);
            run_to_file("bench-" BASE_TAG "-PC24.txt", BASE_TAG ", _PC_24");
        }
        else if (strcmp(order[m], "pc53") == 0) {
            _controlfp_s(&cw, _PC_53, _MCW_PC);
            run_to_file("bench-" BASE_TAG "-PC53.txt", BASE_TAG ", _PC_53");
        }
        else {
            printf("unknown mode: %s (expected pc24 or pc53)\n", order[m]);
            return 1;
        }
    }
#else
    (void)argc;
    (void)argv;
    run_to_file("bench-" BASE_TAG ".txt", BASE_TAG);
#endif
    puts("done");
    return 0;
}
