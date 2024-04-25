#include <math.h>
#include <stdio.h>
#include <complex.h>
#include "polynomial.h"

void solve_quadratic(double a, double b, double c, double *x1, double *x2)
{
    printf("Solving %fx^2 + %fx + %f\n", a, b, c);
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        *x1 = *x2 = 0;
        printf("No real roots\n");
    }
    else
    {
        printf("Two real roots\n");
        *x1 = (-b + sqrt(discriminant)) / (2 * a);
        *x2 = (-b - sqrt(discriminant)) / (2 * a);
        printf("Root 1: %f\n", *x1);
        printf("Root 2: %f\n", *x2);
    }
}

void solve_quartic(double a, double b, double c, double d, double e)
{
    printf("Solving %fx^4 + %fx^3 + %fx^2 + %fx + %f\n", a, b, c, d, e);
    double A = -3 * b * b / (8 * a * a) + c / a;
    double B = b * b * b / (8 * a * a * a) - b * c / (2 * a * a) + d / a;
    double C = -3 * b * b * b * b / (256 * a * a * a * a) + b * b * c / (16 * a * a * a) - b * d / (4 * a * a) + e / a;
    printf("Converted to depressed quartic: x^4 + %fx^2 + %fx + %f\n", A, B, C);
    double P = -A * A / 12 - C;
    double Q = -A * A * A / 108 + A * C / 3 - B * B / 8;
    printf("P: %f\n", P);
    printf("Q: %f\n", Q);
    double discriminant = Q * Q / 4 + P * P * P / 27;
    printf("Discriminant: %f\n", discriminant);
    double complex R = -Q / 2 + csqrt(discriminant);
    printf("R: %f + %fi\n", creal(R), cimag(R));
    double complex U = cpow(R, 1.0 / 3);
    printf("U: %f + %fi\n", creal(U), cimag(U));
    double y;
    if (U == 0)
    {
        y = -5 * A / 6 - cbrt(Q);
    }
    else
    {
        y = -5 * A / 6 - P / (3 * U) + U;
    }
    printf("y: %f\n", y);
    double W = sqrt(A + 2 * y);
    printf("W: %f\n", W);
    double d1 = -3 * A - 2 * y - 2 * B / W;
    double d2 = -3 * A - 2 * y + 2 * B / W;
    printf("d1: %f\n", d1);
    printf("d2: %f\n", d2);

    double x1 = (-b / (4 * a)) + (W - sqrt(d1)) / 2;
    double x2 = (-b / (4 * a)) + (W + sqrt(d1)) / 2;
    double x3 = (-b / (4 * a)) - (W - sqrt(d2)) / 2;
    double x4 = (-b / (4 * a)) - (W + sqrt(d2)) / 2;
    printf("Root 1: %f\n", x1);
    printf("Root 2: %f\n", x2);
    printf("Root 3: %f\n", x3);
    printf("Root 4: %f\n", x4);
}