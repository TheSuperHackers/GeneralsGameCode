/*
 * Cast matrix for GameMath, run on the same input value four ways:
 *
 *   .d     double function, double result          gm_f(x)
 *   .f     float function, float result            gm_ff((float)x)
 *   .f2d   float function, result widened          (double)gm_ff((float)x)
 *   .d2f   double function, result narrowed        (float)gm_f(x)
 *
 * The .f2d row is what our WWMath wrappers do, so it is the one that matters
 * for the game. Comparing the four rows across platforms shows whether a
 * divergence comes from the function itself or from the conversion around it.
 *
 * On Windows the whole matrix runs twice, once with the x87 precision control
 * set to _PC_24 and once with _PC_53, writing math-win-PC24.txt and
 * math-win-PC53.txt. On macOS there is no x87 precision control, so it runs
 * once and writes math-mac.txt.
 *
 * Files are written to the working directory. Output is line oriented and
 * stable, so runs can be compared with a plain diff.
 *
 * macOS:
 *   cc -O2 -ffp-contract=off Tests/GameMath/Source/verify_game_math.c \
 *      -I build/macos/_deps/gamemath-src/include \
 *      build/macos/_deps/gamemath-build/libgm.a -o verify_game_math
 *   ./verify_game_math
 *
 * win32 x86, from a Developer Command Prompt:
 *   cl /O2 /fp:precise Tests\GameMath\Source\verify_game_math.c ^
 *      /I build\win32\_deps\gamemath-src\include ^
 *      build\win32\_deps\gamemath-build\Release\gm.lib ^
 *      /Fe:verify_game_math.exe
 *   verify_game_math.exe
 */

#include <stdio.h>
#include <string.h>
#include "gmath.h"

#ifdef _WIN32
#include <float.h>
#endif

/* ---------- output ---------- */

static FILE *g_out;

static void put_d(const char *name, const char *arg, double v)
{
    unsigned long long u;
    memcpy(&u, &v, sizeof u);
    fprintf(g_out, "%-20s %-46s %016llX\n", name, arg, u);
}

static void put_f(const char *name, const char *arg, float v)
{
    unsigned int u;
    memcpy(&u, &v, sizeof u);
    fprintf(g_out, "%-20s %-46s %08X\n", name, arg, u);
}

static void section(const char *title)
{
    fprintf(g_out, "---- %s ----\n", title);
}

static int open_out(const char *path, const char *title)
{
    g_out = fopen(path, "w");
    if (!g_out) {
        printf("cannot write %s\n", path);
        return 0;
    }
    fprintf(g_out, "================ %s ================\n", title);
    printf("writing %s\n", path);
    return 1;
}

/* ---------- input sets ---------- */

/* general values */
static const double v_gen[] = { 0.5, -0.5, 2.7, 187.66, 0.000123, 100.5 };
/* positive only: log, sqrt, gamma, bessel y */
static const double v_pos[] = { 0.5, 1.0, 2.7, 187.66, 0.000123, 100.5 };
/* inside [-1, 1]: asin, acos, atanh, erf */
static const double v_unit[] = { -0.9, -0.5, 0.0, 0.25, 0.5, 0.9 };
/* one and above: acosh */
static const double v_ge1[] = { 1.0, 1.5, 2.7, 10.0, 187.66, 1000.0 };

static const double v_pair[][2] = {
    { 0.4, 1.3 }, { 2.7, 11.9 }, { 187.66, -59.13 },
    { 0.000123, 9999.5 }, { 1.0, -1.0 }, { -3.5, 2.0 }
};

#define NV 6

/* ---------- one argument ---------- */

#define MATRIX1(base, set)                                                  \
    do {                                                                    \
        int i;                                                              \
        for (i = 0; i < NV; ++i) {                                          \
            double x = set[i];                                              \
            float  xf = (float)x;                                           \
            char a[48];                                                     \
            snprintf(a, sizeof a, "%.17g", x);                              \
            put_d(#base ".d",   a, gm_##base(x));                           \
            put_f(#base ".f",   a, gm_##base##f(xf));                       \
            put_d(#base ".f2d", a, (double)gm_##base##f(xf));               \
            put_f(#base ".d2f", a, (float)gm_##base(x));                    \
        }                                                                   \
        for (i = 0; i < NV; ++i) {                                          \
            double d1 = gm_##base(set[i]);                                  \
            double d2 = gm_##base(set[(i + 1) % NV]);                       \
            double d3 = gm_##base(set[(i + 2) % NV]);                       \
            float  s1 = gm_##base##f((float)set[i]);                        \
            float  s2 = gm_##base##f((float)set[(i + 1) % NV]);             \
            float  s3 = gm_##base##f((float)set[(i + 2) % NV]);             \
            char a[48];                                                     \
            snprintf(a, sizeof a, "%.17g", set[i]);                         \
            put_d(#base ".mul.d", a, d1 * d2);                              \
            put_f(#base ".mul.f", a, s1 * s2);                              \
            put_d(#base ".inv.d", a, 1.0 / d1);                             \
            put_f(#base ".inv.f", a, 1.0f / s1);                            \
            put_d(#base ".mad.d", a, d1 * d2 + d3);                         \
            put_f(#base ".mad.f", a, s1 * s2 + s3);                         \
        }                                                                   \
    } while (0)

/* ---------- two arguments ----------
 *
 * An argument may reach the call as a full double or as a value that has
 * already been through a float, so both arguments are varied independently,
 * and every combination is recorded with the result kept as double and with
 * the result narrowed to float:
 *
 *   .dd     gm_f(x, y)
 *   .dd2f   (float)gm_f(x, y)
 *   .df     gm_f(x, (double)(float)y)
 *   .df2f   (float)gm_f(x, (double)(float)y)
 *   .fd     gm_f((double)(float)x, y)
 *   .fd2f   (float)gm_f((double)(float)x, y)
 *   .ff     gm_f((double)(float)x, (double)(float)y)
 *   .ff2f   (float)gm_f((double)(float)x, (double)(float)y)
 *   .f      gm_ff((float)x, (float)y)
 *   .f2d    (double)gm_ff((float)x, (float)y)
 */

#define MATRIX2(base)                                                       \
    do {                                                                    \
        int i;                                                              \
        for (i = 0; i < NV; ++i) {                                          \
            double x = v_pair[i][0], y = v_pair[i][1];                      \
            float  xf = (float)x, yf = (float)y;                            \
            double xd = (double)xf, yd = (double)yf;                        \
            char a[64];                                                     \
            snprintf(a, sizeof a, "%.17g, %.17g", x, y);                    \
            put_d(#base ".dd",   a, gm_##base(x, y));                       \
            put_f(#base ".dd2f", a, (float)gm_##base(x, y));                \
            put_d(#base ".df",   a, gm_##base(x, yd));                      \
            put_f(#base ".df2f", a, (float)gm_##base(x, yd));               \
            put_d(#base ".fd",   a, gm_##base(xd, y));                      \
            put_f(#base ".fd2f", a, (float)gm_##base(xd, y));               \
            put_d(#base ".ff",   a, gm_##base(xd, yd));                     \
            put_f(#base ".ff2f", a, (float)gm_##base(xd, yd));              \
            put_f(#base ".f",    a, gm_##base##f(xf, yf));                  \
            put_d(#base ".f2d",  a, (double)gm_##base##f(xf, yf));          \
        }                                                                   \
        for (i = 0; i < NV; ++i) {                                          \
            int j = (i + 1) % NV, k = (i + 2) % NV;                         \
            double d1 = gm_##base(v_pair[i][0], v_pair[i][1]);              \
            double d2 = gm_##base(v_pair[j][0], v_pair[j][1]);              \
            double d3 = gm_##base(v_pair[k][0], v_pair[k][1]);              \
            float  s1 = gm_##base##f((float)v_pair[i][0],                   \
                                     (float)v_pair[i][1]);                  \
            float  s2 = gm_##base##f((float)v_pair[j][0],                   \
                                     (float)v_pair[j][1]);                  \
            float  s3 = gm_##base##f((float)v_pair[k][0],                   \
                                     (float)v_pair[k][1]);                  \
            char a[64];                                                     \
            snprintf(a, sizeof a, "%.17g, %.17g",                           \
                     v_pair[i][0], v_pair[i][1]);                           \
            put_d(#base ".mul.d", a, d1 * d2);                              \
            put_f(#base ".mul.f", a, s1 * s2);                              \
            put_d(#base ".inv.d", a, 1.0 / d1);                             \
            put_f(#base ".inv.f", a, 1.0f / s1);                            \
            put_d(#base ".mad.d", a, d1 * d2 + d3);                         \
            put_f(#base ".mad.f", a, s1 * s2 + s3);                         \
        }                                                                   \
    } while (0)

/* ---------- the run ---------- */

static void run_all(void)
{
    section("trigonometric");
    MATRIX1(sin, v_gen);
    MATRIX1(cos, v_gen);
    MATRIX1(tan, v_gen);
    MATRIX1(asin, v_unit);
    MATRIX1(acos, v_unit);
    MATRIX1(atan, v_gen);
    MATRIX2(atan2);

    section("hyperbolic");
    MATRIX1(sinh, v_gen);
    MATRIX1(cosh, v_gen);
    MATRIX1(tanh, v_gen);
    MATRIX1(asinh, v_gen);
    MATRIX1(acosh, v_ge1);
    MATRIX1(atanh, v_unit);

    section("exponential and logarithmic");
    MATRIX1(exp, v_gen);
    MATRIX1(exp2, v_gen);
    MATRIX1(expm1, v_gen);
    MATRIX1(log, v_pos);
    MATRIX1(log10, v_pos);
    MATRIX1(log1p, v_pos);
    MATRIX1(logb, v_pos);
    MATRIX2(pow);

    section("roots");
    MATRIX1(sqrt, v_pos);
    MATRIX1(cbrt, v_gen);
    MATRIX2(hypot);

    section("rounding");
    MATRIX1(ceil, v_gen);
    MATRIX1(floor, v_gen);
    MATRIX1(trunc, v_gen);
    MATRIX1(round, v_gen);
    MATRIX1(rint, v_gen);

    section("sign and remainder");
    MATRIX1(fabs, v_gen);
    MATRIX2(fmod);
    MATRIX2(remainder);
    MATRIX2(drem);
    MATRIX2(copysign);
    MATRIX2(fmax);
    MATRIX2(fmin);

    section("special");
    MATRIX1(erf, v_unit);
    MATRIX1(erfc, v_unit);
    MATRIX1(lgamma, v_pos);
    MATRIX1(gamma, v_pos);
    MATRIX1(significand, v_pos);
    MATRIX1(j0, v_pos);
    MATRIX1(y0, v_pos);
}

/* ---------- what we are running on ----------
 *
 * The file name carries the configuration, so several builds can drop their
 * dumps side by side without overwriting each other.
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

int main(void)
{
    /* x87 precision control only exists on 32-bit x86. */
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
    unsigned int cw;

    if (open_out("math-" BASE_TAG "-PC24.txt", BASE_TAG ", _PC_24")) {
        _controlfp_s(&cw, _PC_24, _MCW_PC);
        run_all();
        fclose(g_out);
    }

    if (open_out("math-" BASE_TAG "-PC53.txt", BASE_TAG ", _PC_53")) {
        _controlfp_s(&cw, _PC_53, _MCW_PC);
        run_all();
        fclose(g_out);
    }
#else
    if (open_out("math-" BASE_TAG ".txt", BASE_TAG)) {
        run_all();
        fclose(g_out);
    }
#endif
    puts("done");
    return 0;
}
