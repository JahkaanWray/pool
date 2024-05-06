#include <math.h>

void Vector3_print(Vector3 v);

double Vector3_dot(Vector3 v, Vector3 w);

Vector3 Vector3_scalar_multiply(Vector3 v, double s);

Vector3 Vector3_add(Vector3 v, Vector3 w);

Vector3 Vector3_subtract(Vector3 v, Vector3 w);

Vector3 Vector3_cross(Vector3 v, Vector3 w);

double Vector3_mag(Vector3 v);

Vector3 Vector3_normalize(Vector3 v);
