#include <math.h>
#include <stdio.h>
#include "polynomial.h"

void solve_quadratic(double a, double b, double c, double *x1, double *x2)
{
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