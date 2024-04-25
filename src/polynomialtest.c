#include <math.h>
#include <stdio.h>
#include "polynomial.h"

int main()
{
    double a, b, c, x1, x2;
    printf("Enter a, b, c: ");
    scanf("%lf %lf %lf", &a, &b, &c);
    printf("Solving %fx^2 + %fx + %f\n", a, b, c);
    solve_quadratic(a, b, c, &x1, &x2);
    return 0;
}