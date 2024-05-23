#include <math.h>
#include <stdio.h>
#include "polynomial.h"

int main()
{
    double a, b, c, d, e, x1, x2, x3, x4;

    printf("Enter a, b, c, d, e: ");
    scanf("%lf %lf %lf %lf %lf", &a, &b, &c, &d, &e);
    solve_quartic(a, b, c, d, e, &x1, &x2, &x3, &x4);
    printf("The roots are %lf, %lf, %lf, %lf\n", x1, x2, x3, x4);
    return 0;
}