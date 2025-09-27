#ifndef MY_ZIG_LIB_H
#define MY_ZIG_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MY_ZIG_LIB_EXPORT
#define ZIG_API __attribute__((visibility("default")))
#else
#define ZIG_API
#endif

ZIG_API int zig_add(int a, int b);

typedef struct {
    float x;
    float y;
} ZigPoint;

ZIG_API ZigPoint ZigPoint_create(float x, float y);
ZIG_API float ZigPoint_magnitude(const ZigPoint* self);

#ifdef __cplusplus
}
#endif

#endif // MY_ZIG_LIB_H
