#pragma once
#include <math.h>
#include <stdint.h>
#include "vector.h"

template<typename S> struct Vector3 {

	Vector3() : x(0), y(0), z(0) {}
    Vector3(S s) : x(s), y(s), z(s) {}
	Vector3(S x, S y, S z) : x(x), y(y), z(z) {}
    Vector3(const Vector2<S>& b, S c) : x(b.x), y(b.y), z(c) {}
	Vector3(const Vector3<int64_t>& b) : x((S)b.x), y((S)b.y), z((S)b.z) {}
	Vector3(const Vector3<double>& b)  : x((S)b.x), y((S)b.y), z((S)b.z) {}
	Vector3(const Vector3<int32_t>& b) : x((S)b.x), y((S)b.y), z((S)b.z) {}

	Vector3 operator*(Vector3 const& b) const { return Vector3(x*b.x, y*b.y, z*b.z); }
	Vector3 operator/(Vector3 const& b) const { return Vector3(x/b.x, y/b.y, z/b.z); }
	Vector3 operator+(Vector3 const& b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
	Vector3 operator-(Vector3 const& b) const { return Vector3(x - b.x, y - b.y, z - b.z); }
    Vector3 operator+(S const& s) const { return Vector3(x + s, y + s, z + s); }
    Vector3 operator-(S const& s) const { return Vector3(x - s, y - s, z - s); }
    S operator&(Vector3 const& b) const { return x*b.x + y*b.y + z*b.z; }
    Vector3 unit() const { return *this / length(); }

    Vector3 operator^(Vector3 const& s) const { return Vector3(y*s.z-z*s.y, z*s.x-x*s.z, x*s.y-y*s.x); }
	Vector3 operator*(S s) const { return Vector3(x*s, y*s, z*s); }
	Vector3 operator/(S s) const { return Vector3(x/s, y/s, z/s); }

    Vector3 const& rotate_x(double a);
    Vector3 const& rotate_y(double a);
    Vector3 const& rotate_z(double a);
    void mix(Vector3 const& b, double q) { *this = *this*(1-q) + b*q; }

	bool operator!=(Vector3 const& b) const { return x != b.x || y != b.y || z != b.z; }
	bool operator!=(double const& b) const { return x != b || y != b || z != b; }
	bool operator==(Vector3 const& b) const { return x == b.x && y == b.y && z == b.z; }
	bool operator==(double const& b) const { return x == b && y == b && z == b; }

	S dot() const { return x*x + y*y + z*z; }
	S l2() const { return length(); }
	S dot(Vector3 const& b) const { return x*b.x + y*b.y + z*b.z; }
	S length() const { return (S)sqrt((double)(x*x + y*y + z*z)); }

    static S mix(Vector3 const& a, Vector3 const& b, double q) { return a*(1-q) + b(q); }
    static S deter(Vector3 const& r1, Vector3 const& r2, Vector3 const& r3);
    static S deter_col(Vector3 const& c1, Vector3 const& c2, Vector3 const& c3);
    static Vector3 solve3x3(Vector3 const& r1, Vector3 const& r2, Vector3 const& r3, Vector3 const& c);
    static Vector3 solve3x3_col(Vector3 const& c1, Vector3 const& c2, Vector3 const& c3, Vector3 const& c4);
    static Vector3 planes_intersection(Vector3 const& n1, Vector3 const& p1,
                                       Vector3 const& n2, Vector3 const& p2,
                                       Vector3 const& n3, Vector3 const& p3);

	void operator+=(Vector3 const& b) { x += b.x; y += b.y; z += b.z; }
	void operator-=(Vector3 const& b) { x -= b.x; y -= b.y; z -= b.z; }
	void operator*=(Vector3 const& b) { x *= b.x; y *= b.y; z *= b.z; }
	void operator/=(Vector3 const& b) { x /= b.x; y /= b.y; z /= b.z; }
	void operator*=(S s) { x *= s; y *= s; z *= s; }
	void operator/=(S s) { x /= s; y /= s; z /= s; }
    S operator[](int i) const {
        switch(i % 3) {
        case 0: return x;
        case -2: case 1: return y;
        case -1: case 2: return z;
        default: return 0;
        }
    }

	Vector3& norm();

    union {S x; S r; S u;};
    union {S y; S g; S v;};
    union {S z; S b; S w;};
};

typedef Vector3<double> Vec3;

/* Definitions */

template<typename S> Vector3<S> & Vector3<S>::norm() {
	if(x == 0 && y == 0 && z==0) return *this;
	double d = length();
	x /= d;
	y /= d;
	z /= d;
	return *this;
}

template<typename S> Vector3<S> const& Vector3<S>::rotate_x(double a) {
    S py = y;
    y = (S)((double) y*cos(a) - (double)z*sin(a));
    z = (S)((double)py*sin(a) + (double)z*cos(a));
    return *this;
}

template<typename S> Vector3<S> const& Vector3<S>::rotate_y(double a) {
    S pz = z;
    z = (S)((double) z*cos(a) - (double)x*sin(a));
    x = (S)((double)pz*sin(a) + (double)x*cos(a));
    return *this;
}

template<typename S> Vector3<S> const& Vector3<S>::rotate_z(double a) {
    S px = x;
    x = (S)((double) x*cos(a) - (double)y*sin(a));
    y = (S)((double)px*sin(a) + (double)y*cos(a));
    return *this;
}

template<typename S> S Vector3<S>::deter(Vector3<S> const& r1, Vector3<S> const& r2, Vector3<S> const& r3) {
    //r1.x r1.y r1.z
    //r2.x r2.y r2.z
    //r3.x r3.y r3.z

    return r1.x * (r2.y * r3.z - r2.z * r3.y) -
           r1.y * (r2.x * r3.z - r2.z * r3.x) +
           r1.z * (r2.x * r3.y - r2.y * r3.x);
}

template<typename S> S Vector3<S>::deter_col(Vector3<S> const& c1, Vector3<S> const& c2, Vector3<S> const& c3) {
    //c1.x c2.x c3.x
    //c1.y c2.y c3.y
    //c1.z c2.z c3.z
    return c1.x * (c2.y * c3.z - c3.y * c2.z) -
           c2.x * (c1.y * c3.z - c3.y * c1.z) +
           c3.x * (c1.y * c2.z - c2.y * c1.z);
}

template<typename S> Vector3<S> Vector3<S>::solve3x3(Vector3 const& r1, Vector3 const& r2, Vector3 const& r3, Vector3 const& c) {
    double d =  deter(r1,r2,r3);

    if(d >= 0 && d <= 0)
        return Vector3();

    double dx = deter(Vector3(c.x, r1.y, r1.z),
                      Vector3(c.y, r2.y, r2.z),
                      Vector3(c.z, r3.y, r3.z));
    double dy = deter(Vector3(r1.x, c.x, r1.z),
                      Vector3(r2.x, c.y, r2.z),
                      Vector3(r3.x, c.z, r3.z));
    double dz = deter(Vector3(r1.x, r1.y, c.x),
                      Vector3(r2.x, r2.y, c.y),
                      Vector3(r3.x, r3.y, c.z));

    return Vector3(dx, dy, dz) / d;
}

template<typename S> Vector3<S> Vector3<S>::solve3x3_col(Vector3 const& c1, Vector3 const& c2, Vector3 const& c3, Vector3 const& c4) {
    double d =  deter_col(c1,c2,c3);

    if(d >= 0 && d <= 0)
        return Vector3();

    double
           dx = deter_col(c4,c2,c3),
           dy = deter_col(c1,c4,c3),
           dz = deter_col(c1,c2,c4);

    return Vector3(dx, dy, dz) / d;
}

template<typename S> Vector3<S> Vector3<S>::planes_intersection(Vector3 const& n1, Vector3 const& p1,
                                                                Vector3 const& n2, Vector3 const& p2,
                                                                Vector3 const& n3, Vector3 const& p3) {
    return solve3x3(n1, n2, n3, Vector3(n1.dot(p1), n2.dot(p2), n3.dot(p3)));
}
