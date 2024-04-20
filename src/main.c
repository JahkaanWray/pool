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
    return Vector3_scalar_multiply(v, 1.0 / mag);
}

int main()
{
    printf("Hello, World!\n");
    double R = 1.0;
    double mu_slide = 0.5;
    double mu_roll = 0.3;
    double g = 9.8;
    Vector3 v = {1.0, 2.0, 0};
    Vector3 w = {3.0, 4.0, 0};
    Vector3 k = {0, 0, 1};
    Vector3 contact_point_v = Vector3_add(v, Vector3_scalar_multiply(Vector3_cross(w, k), -1 * R));
    Vector3 contact_point_v_normalized = Vector3_normalize(contact_point_v);
    double slide_time = 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    printf("Contact point velocity = (%f, %f, %f)\n", contact_point_v.x, contact_point_v.y, contact_point_v.z);
    printf("Time spent sliding = %f\n", slide_time);
    return 0;
}