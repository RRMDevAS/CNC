#include "vec3.h"




Vec3 zero() {
    Vec3 res = {
        .X = 0.0f,
        .Y = 0.0f,
        .Z = 0.0f
    };
    return res;
}
Vec3 unitX() {
    Vec3 res = {
        .X = 1.0f,
        .Y = 0.0f,
        .Z = 0.0f
    };
    return res;
}
Vec3 unitY() {
    Vec3 res = {
        .X = 0.0f,
        .Y = 1.0f,
        .Z = 0.0f
    };
    return res;
}
Vec3 unitZ() {
    Vec3 res = {
        .X = 0.0f,
        .Y = 0.0f,
        .Z = 1.0f
    };
    return res;
}
Vec3 newVec3(float x, float y, float z) {
    Vec3 res = {
        .X = x,
        .Y = y,
        .Z = z
    };
    return res;
}

Vec3 add(const Vec3* a, const Vec3* b) {
    Vec3 res = {
        .X = a->X + b->X,
        .Y = a->Y + b->Y,
        .Z = a->Z + b->Z,
    };
    return res;
}
Vec3 sub(const Vec3* a, const Vec3* b) {
    Vec3 res = {
        .X = a->X - b->X,
        .Y = a->Y - b->Y,
        .Z = a->Z - b->Z,
    };
    return res;
}
float dot(const Vec3* a, const Vec3* b) {
    return a->X * b->X + a->Y * b->Y + a->Z * b->Z;
}
Vec3 cross(const Vec3* a, const Vec3* b) {
    Vec3 res = {
        .X = a->Y * b->Z - a->Z * b->Y,
        .Y = a->Z * b->X - a->X * b->Z,
        .Z = a->X * b->Y - a->Y * b->X,
    };
    return res;
}
Vec3 divide(const Vec3* a, float b) {
    Vec3 res = {
        .X = a->X / b,
        .Y = a->Y / b,
        .Z = a->Z / b,
    };
    return res;
}

Vec3 unit(const Vec3* a) {
    Vec3 res = *a;
    normalize(&res);
    return res;
}
void normalize(Vec3* a) {
    float fLength = length(a);
    if (fLength != 0.0f ) {
        return divide(a, fLength);
    }
    return zero();
    
}
float length(const Vec3* a) {
    return sqrt(a->X * a->X + a->Y * a->Y + a->Z * a->Z);
}