#include <math.h>
#include <stdio.h>
#include <complex.h>
#include "polynomial.h"

double evaluate_cubic(double a, double b, double c, double d, double x)
{
    return a * x * x * x + b * x * x + c * x + d;
}

double evaluate_quadratic(double a, double b, double c, double x)
{
    return a * x * x + b * x + c;
}

double evaluate_quartic(double a, double b, double c, double d, double e, double x)
{
    return a * x * x * x * x + b * x * x * x + c * x * x + d * x + e;
}

void solve_cubic_newton(double a, double b, double c, double d, double *x1, double *x2, double *x3)
{
    double x = 0;
    double f;

    for (int i = 0; i < 1000; i++)
    {
        f = evaluate_cubic(a, b, c, d, x);
        if (fabs(f) < 1e-10)
        {
            break;
        }
        double f_prime = evaluate_quadratic(3 * a, 2 * b, c, x);
        x = x - f / f_prime;
    }

    if (f < 1e-10)
    {

        *x1 = x;
    }
    else
    {
        *x1 = nan("0");
        *x2 = nan("0");
        *x3 = nan("0");
        return;
    }

    printf("New coefficient: %lf %lf %lf\n", a, b + a * x, -d / x);
    solve_quadratic(a, b + a * x, -d / x, x2, x3);
}

void solve_quadratic(double a, double b, double c, double *x1, double *x2)
{
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        *x1 = *x2 = nan("0");
    }
    else
    {
        *x1 = (-b + sqrt(discriminant)) / (2 * a);
        *x2 = (-b - sqrt(discriminant)) / (2 * a);
    }
}

void solve_quartic_newton(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
{
    double x = 0;
    double f;

    for (int i = 0; i < 1000; i++)
    {
        f = evaluate_quartic(a, b, c, d, e, x);
        if (fabs(f) < 1e-10)
        {
            break;
        }
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x);
        x = x - f / f_prime;
    }

    if (f < 1e-10)
    {
        *x1 = x;
    }
    else
    {
        *x1 = nan("0");
        *x2 = nan("0");
        *x3 = nan("0");
        *x4 = nan("0");
        return;
    }

    printf("New coefficient: %lf %lf %lf %lf\n", a, b + a * x, c + b * x + a * x * x, -e / x);
    solve_cubic(a, b + a * x, c + b * x + a * x * x, -e / x, x2, x3, x4);
}

void solve_quartic_analytical(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
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

void solve_quartic(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
{
    solve_quartic_newton(a, b, c, d, e, x1, x2, x3, x4);
}

void solve_cubic(double a, double b, double c, double d, double *x1, double *x2, double *x3)
{
    solve_cubic_newton(a, b, c, d, x1, x2, x3);
}