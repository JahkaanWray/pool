#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "polynomial.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <number of iterations>\n", argv[0]);
        return 1;
    }

    int iterations = atoi(argv[1]);

    for (int i = 0; i < iterations; i++)
    {
        double coeffs[5];
        for (int j = 0; j < 5; j++)
        {
            coeffs[j] = rand() % 10;
        }
        double roots[4];
        solve_quartic(coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4], &roots[0], &roots[1], &roots[2], &roots[3]);
    }
}