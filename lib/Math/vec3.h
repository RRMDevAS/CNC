#ifndef VEC3_H
#define VEC3_H


struct Vec3 {
    union
    {
        struct {
            float X;
            float Y;
            float Z;
        };
        float x[3];
    };
    
};
typedef struct Vec3 Vec3;

Vec3 add(const Vec3* a, const Vec3* b);
Vec3 sub(const Vec3* a, const Vec3* b);
float dot(const Vec3* a, const Vec3* b);
Vec3 cross(const Vec3* a, const Vec3* b);
Vec3 divide(const Vec3* a, float b);

Vec3 unit(const Vec3* a);
void normalize(Vec3* a);
float length(const Vec3* a);

#endif