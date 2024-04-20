#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct
{
    double x;
    double y;
    double z;
} Vector3;

double Vector3_dot(Vector3 v, Vector3 w)
{
    return v.x * w.x + v.y * w.y + v.z * w.z;
}

int main()
{
    printf("Hello, World!\n");
    Vector3 v = {1.0, 2.0, 0};
    Vector3 w = {3.0, 4.0, 0};
    printf("v.x = %f, v.y = %f\n", v.x, v.y);
    printf("w.x = %f, w.y = %f\n", w.x, w.y);
    printf("v dot w = %f\n", Vector3_dot(v, w));
    return 0;
}