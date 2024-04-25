#include <math.h>
#include <stdio.h>
#include "polynomial.h"

int main()
{
    double a, b, c, d, e, x1, x2;
    printf("Enter a, b, c: ");
    scanf("%lf %lf %lf", &a, &b, &c);
    solve_quadratic(a, b, c, &x1, &x2);

    printf("Enter a, b, c, d, e: ");
    scanf("%lf %lf %lf %lf %lf", &a, &b, &c, &d, &e);
    solve_quartic(a, b, c, d, e);
    return 0;
}