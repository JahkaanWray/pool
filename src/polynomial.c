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

double quartic_quadratic_newton_iterate(double a, double b, double c, double d, double e, double x)
{
    double f = evaluate_quartic(a, b, c, d, e, x);
    double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x);
    double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x);
    while (fabs(f) > 1e-10)
    {

        // y = 0.5 f''(x) (x-x0)^2 + f'(x) (x - x0) + f(x)
        // Expands to y = 0.5 a (x-x0)^2 + b (x - x0) + c
        // y = 0.5 a x^2 - a x0 x + 0.5 a x0^2 + b x - b x0 + c
        // y = 0.5 a x^2 + (b - a x0) x + 0.5 a x0^2 - b x0 + c

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x;
        double c1 = 0.5 * f_double_prime * x * x - f_prime * x + f;

        if (f_prime > 0)
        {
            // Must be on left side of curve, so choose root to the right
            if (f_double_prime > 0)
            {
                // Must be concave up, so choose root to the right
                x = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
            }
            else
            {
                // Must be concave down, so choose root to the left
                x = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
            }
        }
        else
        {
            // Must be on right side of curve, so choose root to the left
            if (f_double_prime > 0)
            {
                // Must be concave up, so choose root to the left
                x = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
            }
            else
            {
                // Must be concave down, so choose root to the right
                x = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
            }
        }
        f = evaluate_quartic(a, b, c, d, e, x);
        f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x);
        f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x);
    }

    return x;
}

double quartic_newton_iterate(double a, double b, double c, double d, double e, double x)
{
    double f = evaluate_quartic(a, b, c, d, e, x);
    double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x);

    while (fabs(f) > 1e-10)
    {
        x = x - f / f_prime;
        f = evaluate_quartic(a, b, c, d, e, x);
        f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x);
    }
    return x;
}

void solve_quartic_j(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
{
    double stationary_points[3];
    double sorted_stationary_points[3];
    solve_cubic(4 * a, 3 * b, 2 * c, d, &stationary_points[0], &stationary_points[1], &stationary_points[2]);
    // Sort the stationary points
    if (stationary_points[0] < stationary_points[1] && stationary_points[0] < stationary_points[2])
    {
        // stationary_points[0] is the smallest
        sorted_stationary_points[0] = stationary_points[0];
        if (stationary_points[1] < stationary_points[2])
        {
            sorted_stationary_points[1] = stationary_points[1];
            sorted_stationary_points[2] = stationary_points[2];
        }
        else
        {
            sorted_stationary_points[1] = stationary_points[2];
            sorted_stationary_points[2] = stationary_points[1];
        }
    }
    else if (stationary_points[1] < stationary_points[0] && stationary_points[1] < stationary_points[2])
    {
        // stationary_points[1] is the smallest
        sorted_stationary_points[0] = stationary_points[1];
        if (stationary_points[0] < stationary_points[2])
        {
            sorted_stationary_points[1] = stationary_points[0];
            sorted_stationary_points[2] = stationary_points[2];
        }
        else
        {
            sorted_stationary_points[1] = stationary_points[2];
            sorted_stationary_points[2] = stationary_points[0];
        }
    }
    else
    {
        // stationary_points[2] is the smallest
        sorted_stationary_points[0] = stationary_points[2];
        if (stationary_points[0] < stationary_points[1])
        {
            sorted_stationary_points[1] = stationary_points[0];
            sorted_stationary_points[2] = stationary_points[1];
        }
        else
        {
            sorted_stationary_points[1] = stationary_points[1];
            sorted_stationary_points[2] = stationary_points[0];
        }
    }
    double ds[3];
    for (int i = 0; i < 3; i++)
    {
        ds[i] = evaluate_quartic(a, b, c, d, e, sorted_stationary_points[i]);
    }

    if (isnan(sorted_stationary_points[0]) && isnan(sorted_stationary_points[1]))
    {
        // Only one stationary point;
        if (ds[2] > 0)
        {
            // No solutions;
            *x1 = *x2 = *x3 = *x4 = nan("0");
            return;
        }
        // 2 solutions
        double x0 = sorted_stationary_points[2];
        double f = evaluate_quartic(a, b, c, d, e, x0);
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x0;
        double c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root1 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root1 = quartic_newton_iterate(a, b, c, d, e, root1);

        double root2 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root2 = quartic_newton_iterate(a, b, c, d, e, root2);

        *x1 = root1;
        *x2 = root2;
        *x3 = *x4 = nan("0");
        return;
    }

    if (ds[0] > 0 && ds[1] > 0 && ds[2] > 0)
    {
        // No solutions
        *x1 = *x2 = *x3 = *x4 = nan("0");
        return;
    }
    if (ds[0] < 0 && ds[1] < 0 && ds[2] < 0)
    {
        // 2 solutions, find starting points by approximating with quadratic
        // y = 0.5 f''(x) (x-x0)^2 + f'(x) (x - x0) + f(x)
        // Expands to y = 0.5 a (x-x0)^2 + b (x - x0) + c
        // y = 0.5 a x^2 - a x0 x + 0.5 a x0^2 + b x - b x0 + c
        // y = 0.5 a x^2 + (b - a x0) x + 0.5 a x0^2 - b x0 + c

        double x0 = sorted_stationary_points[0];
        double f = evaluate_quartic(a, b, c, d, e, x0);
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x0;
        double c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root1 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root1 = quartic_newton_iterate(a, b, c, d, e, root1);

        x0 = sorted_stationary_points[3];
        f = evaluate_quartic(a, b, c, d, e, x0);
        f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        a1 = 0.5 * f_double_prime;
        b1 = f_prime - f_double_prime * x0;
        c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root2 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root2 = quartic_newton_iterate(a, b, c, d, e, root2);

        *x1 = root1;
        *x2 = root2;
        *x3 = *x4 = nan("0");
        return;
    }
    if (ds[0] < 0 && ds[1] > 0 && ds[2] < 0)
    {
        // 4 solutions
        double x0 = sorted_stationary_points[0];

        double f = evaluate_quartic(a, b, c, d, e, x0);
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x0;
        double c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root1 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root1 = quartic_newton_iterate(a, b, c, d, e, root1);

        double root2 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root2 = quartic_newton_iterate(a, b, c, d, e, root2);

        x0 = sorted_stationary_points[2];
        f = evaluate_quartic(a, b, c, d, e, x0);
        f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        a1 = 0.5 * f_double_prime;
        b1 = f_prime - f_double_prime * x0;
        c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root3 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root3 = quartic_newton_iterate(a, b, c, d, e, root3);

        double root4 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root4 = quartic_newton_iterate(a, b, c, d, e, root4);

        *x1 = root1;
        *x2 = root2;
        *x3 = root3;
        *x4 = root4;
        return;
    }
    if (ds[0] < 0 && ds[1] > 0 && ds[2] > 0)
    {
        // 2 Solutions
        double x0 = sorted_stationary_points[0];
        double f = evaluate_quartic(a, b, c, d, e, x0);
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x0;
        double c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root1 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root1 = quartic_newton_iterate(a, b, c, d, e, root1);

        double root2 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root2 = quartic_newton_iterate(a, b, c, d, e, root2);

        *x1 = root1;
        *x2 = root2;
        *x3 = *x4 = nan("0");
        return;
    }
    if (ds[0] > 0 && ds[1] > 0 && ds[2] < 0)
    {
        // 2 Solutions
        double x0 = sorted_stationary_points[2];
        double f = evaluate_quartic(a, b, c, d, e, x0);
        double f_prime = evaluate_cubic(4 * a, 3 * b, 2 * c, d, x0);
        double f_double_prime = evaluate_quadratic(12 * a, 6 * b, 2 * c, x0);

        double a1 = 0.5 * f_double_prime;
        double b1 = f_prime - f_double_prime * x0;
        double c1 = 0.5 * f_double_prime * x0 * x0 - f_prime * x0 + f;

        double root1 = (-b1 - sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root1 = quartic_newton_iterate(a, b, c, d, e, root1);

        double root2 = (-b1 + sqrt(b1 * b1 - 4 * a1 * c1)) / (2 * a1);
        root2 = quartic_newton_iterate(a, b, c, d, e, root2);

        *x1 = root1;
        *x2 = root2;
        *x3 = *x4 = nan("0");
        return;
    }
}

void solve_quartic(double a, double b, double c, double d, double e, double *x1, double *x2, double *x3, double *x4)
{
    solve_quartic_j(a, b, c, d, e, x1, x2, x3, x4);
}

void solve_cubic(double a, double b, double c, double d, double *x1, double *x2, double *x3)
{
    solve_cubic_newton(a, b, c, d, x1, x2, x3);
}