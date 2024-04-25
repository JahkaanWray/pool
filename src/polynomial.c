#include <math.h>
#include <stdio.h>
#include <complex.h>
#include "polynomial.h"

void solve_quadratic(double a, double b, double c, double *x1, double *x2)
{
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        *x1 = *x2 = 0;
    }
    else
    {
        *x1 = (-b + sqrt(discriminant)) / (2 * a);
        *x2 = (-b - sqrt(discriminant)) / (2 * a);
    }
}

void solve_quartic(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
{
    double A = -3 * b * b / (8 * a * a) + c / a;
    double B = b * b * b / (8 * a * a * a) - b * c / (2 * a * a) + d / a;
    double C = -3 * b * b * b * b / (256 * a * a * a * a) + b * b * c / (16 * a * a * a) - b * d / (4 * a * a) + e / a;
    double P = -A * A / 12 - C;
    double Q = -A * A * A / 108 + A * C / 3 - B * B / 8;
    double discriminant = Q * Q / 4 + P * P * P / 27;
    double complex R = -Q / 2 + csqrt(discriminant);
    double complex U = cpow(R, 1.0 / 3);
    double y;
    if (U == 0)
    {
        y = -5 * A / 6 - cbrt(Q);
    }
    else
    {
        y = -5 * A / 6 - P / (3 * U) + U;
    }
    double W = sqrt(A + 2 * y);
    double d1 = -3 * A - 2 * y - 2 * B / W;
    double d2 = -3 * A - 2 * y + 2 * B / W;

    double root1 = (-b / (4 * a)) + (W - sqrt(d1)) / 2;
    double root2 = (-b / (4 * a)) + (W + sqrt(d1)) / 2;
    double root3 = (-b / (4 * a)) - (W - sqrt(d2)) / 2;
    double root4 = (-b / (4 * a)) - (W + sqrt(d2)) / 2;

    *x1 = root1;
    *x2 = root2;
    *x3 = root3;
    *x4 = root4;
}