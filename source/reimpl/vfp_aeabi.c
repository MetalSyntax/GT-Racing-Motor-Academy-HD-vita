/**
 * @file  vfp_aeabi.c
 * @brief VFP replacements for the soft-float __aeabi_* helpers imported by libGTRacing.so.
 *
 * The .so is armeabi (ARMv5, no FPU): every float op is a PLT call into the
 * toolchain's integer-only libgcc emulation (~10k call sites; fdiv is the worst).
 * Built with -mfloat-abi=softfp, each of these becomes vmov + one VFP op + vmov
 * on the Cortex-A9. Compiled WITHOUT -ffast-math (see CMakeLists.txt) so NaN
 * comparisons and rounding keep the exact AEABI semantics.
 */

float vfp_fadd(float a, float b) { return a + b; }
float vfp_fsub(float a, float b) { return a - b; }
float vfp_fmul(float a, float b) { return a * b; }
float vfp_fdiv(float a, float b) { return a / b; }
float vfp_i2f(int a) { return (float) a; }
float vfp_ui2f(unsigned a) { return (float) a; }
int vfp_f2iz(float a) { return (int) a; }  // vcvt saturates like libgcc

int vfp_fcmpeq(float a, float b) { return a == b; }
int vfp_fcmplt(float a, float b) { return a < b; }
int vfp_fcmple(float a, float b) { return a <= b; }
int vfp_fcmpgt(float a, float b) { return a > b; }
int vfp_fcmpge(float a, float b) { return a >= b; }

double vfp_f2d(float a) { return (double) a; }
float vfp_d2f(double a) { return (float) a; }
double vfp_i2d(int a) { return (double) a; }
int vfp_d2iz(double a) { return (int) a; }
double vfp_dadd(double a, double b) { return a + b; }
double vfp_dsub(double a, double b) { return a - b; }
double vfp_dmul(double a, double b) { return a * b; }
double vfp_ddiv(double a, double b) { return a / b; }
