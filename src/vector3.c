#include <math.h>
#include <stdio.h>
#include <raylib.h>
#include "vector3.h"

void Vector3_print(Vector3 v)
{
    printf("(%f, %f, %f)\n", v.x, v.y, v.z);
}

double Vector3_dot(Vector3 v, Vector3 w)
{
    return v.x * w.x + v.y * w.y + v.z * w.z;
}

Vector3 Vector3_scalar_multiply(Vector3 v, double s)
{
    Vector3 result = {v.x * s, v.y * s, v.z * s};
    return result;
}

Vector3 Vector3_add(Vector3 v, Vector3 w)
{
    Vector3 result = {v.x + w.x, v.y + w.y, v.z + w.z};
    return result;
}

Vector3 Vector3_subtract(Vector3 v, Vector3 w)
{
    Vector3 result = {v.x - w.x, v.y - w.y, v.z - w.z};
    return result;
}

Vector3 Vector3_cross(Vector3 v, Vector3 w)
{
    Vector3 result = {
        v.y * w.z - v.z * w.y,
        v.z * w.x - v.x * w.z,
        v.x * w.y - v.y * w.x};
    return result;
}

double Vector3_mag(Vector3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 Vector3_normalize(Vector3 v)
{
    double mag = Vector3_mag(v);
    if (mag == 0)
    {
        return v;
    }
    return Vector3_scalar_multiply(v, 1.0 / mag);
}