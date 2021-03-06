#include "recurrence.h"

#ifdef __AVX__
    void horner_AVX(const int n, const double * c, const int incc, const int m, double * x, double * f) {
        HORNER_KERNEL(double, double4, 4, 16, vloadu4, vstoreu4, vmuladd, vall4)
    }
    void horner_AVXf(const int n, const float * c, const int incc, const int m, float * x, float * f) {
        HORNER_KERNEL(float, float8, 8, 16, vloadu8f, vstoreu8f, vmuladd, vall8f)
    }
    void clenshaw_AVX(const int n, const double * c, const int incc, const int m, double * x, double * f) {
        CLENSHAW_KERNEL(double, double4, 4, 8, vloadu4, vstoreu4, vmuladd, vall4)
    }
    void clenshaw_AVXf(const int n, const float * c, const int incc, const int m, float * x, float * f) {
        CLENSHAW_KERNEL(float, float8, 8, 8, vloadu8f, vstoreu8f, vmuladd, vall8f)
    }
    void orthogonal_polynomial_clenshaw_AVX(const int n, const double * c, const int incc, const double * A, const double * B, const double * C, const int m, double * x, double * phi0, double * f) {
        ORTHOGONAL_POLYNOMIAL_CLENSHAW_KERNEL(double, double4, 4, 8, vloadu4, vstoreu4, vmuladd, vmulsub, vall4)
    }
    void orthogonal_polynomial_clenshaw_AVXf(const int n, const float * c, const int incc, const float * A, const float * B, const float * C, const int m, float * x, float * phi0, float * f) {
        ORTHOGONAL_POLYNOMIAL_CLENSHAW_KERNEL(float, float8, 8, 8, vloadu8f, vstoreu8f, vmuladd, vmulsub, vall8f)
    }
    void eigen_eval_AVX(const int n, const double * c, const int incc, const double * A, const double * B, const double * C, const int m, double * x, const int sign, double * f) {
        EIGEN_EVAL_KERNEL(double, double4, 4, 8, vloadu4, vstoreu4, vmuladd, vmulsub, vall4, vsqrt4, vmovemask4, eps()/floatmin(), sqrt)
    }
    void eigen_eval_AVXf(const int n, const float * c, const int incc, const float * A, const float * B, const float * C, const int m, float * x, const int sign, float * f) {
        EIGEN_EVAL_KERNEL(float, float8, 8, 8, vloadu8f, vstoreu8f, vmuladd, vmulsub, vall8f, vsqrt8f, vmovemask8f, epsf()/floatminf(), sqrtf)
    }
#else
    void horner_AVX(const int n, const double * c, const int incc, const int m, double * x, double * f) {horner_SSE2(n, c, incc, m, x, f);}
    void horner_AVXf(const int n, const float * c, const int incc, const int m, float * x, float * f) {horner_SSEf(n, c, incc, m, x, f);}
    void clenshaw_AVX(const int n, const double * c, const int incc, const int m, double * x, double * f) {clenshaw_SSE2(n, c, incc, m, x, f);}
    void clenshaw_AVXf(const int n, const float * c, const int incc, const int m, float * x, float * f) {clenshaw_SSEf(n, c, incc, m, x, f);}
    void orthogonal_polynomial_clenshaw_AVX(const int n, const double * c, const int incc, const double * A, const double * B, const double * C, const int m, double * x, double * phi0, double * f) {orthogonal_polynomial_clenshaw_SSE2(n, c, incc, A, B, C, m, x, phi0, f);}
    void orthogonal_polynomial_clenshaw_AVXf(const int n, const float * c, const int incc, const float * A, const float * B, const float * C, const int m, float * x, float * phi0, float * f) {orthogonal_polynomial_clenshaw_SSEf(n, c, incc, A, B, C, m, x, phi0, f);}
    void eigen_eval_AVX(const int n, const double * c, const int incc, const double * A, const double * B, const double * C, const int m, double * x, const int sign, double * f) {eigen_eval_SSE2(n, c, incc, A, B, C, m, x, sign, f);}
    void eigen_eval_AVXf(const int n, const float * c, const int incc, const float * A, const float * B, const float * C, const int m, float * x, const int sign, float * f) {eigen_eval_SSEf(n, c, incc, A, B, C, m, x, sign, f);}
#endif
