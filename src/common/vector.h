#pragma once
#include <math.h>
#include <stdint.h>

template<typename S> struct Vector3;

template<typename S> struct Vector2 {

    Vector2() : x(0), y(0) {}
    Vector2(S s) : x(s), y(s) {}
    Vector2(S x, S y) : x(x), y(y) {}
    Vector2(Vector3<S> const& v) : x(v.x), y(v.y) {}
    Vector2(Vector2<int64_t> const& b) : x((S)b.x), y((S)b.y) {}
    Vector2(Vector2<double> const& b)  : x((S)b.x), y((S)b.y) {}
    Vector2(Vector2<int32_t> const& b) : x((S)b.x), y((S)b.y) {}

    Vector2 operator*(Vector2 const& b) const { return Vector2(x*b.x, y*b.y); }
    Vector2 operator/(Vector2 const& b) const { return Vector2(x/b.x, y/b.y); }
    Vector2 operator+(Vector2 const& b) const { return Vector2(x + b.x, y + b.y); }
    Vector2 operator-(Vector2 const& b) const { return Vector2(x - b.x, y - b.y); }
    S mx() const { return x >= y ? x : y; }
    S mn() const { return x <= y ? x : y; }

	//determinant, used for calculations of oriented area
    S operator^(Vector2 const& b) const { return x*b.y - y*b.x; }

    Vector2 operator*(S s) const { return Vector2(x*s, y*s); }
    Vector2 operator/(S s) const { return Vector2(x/s, y/s); }
    S operator&(Vector2 const& b) const { return x*b.x + y*b.y; }

    Vector2 unit() const { return *this / length(); }

    Vector2 round() const { return Vector2(::round(x), ::round(y)); }

    bool operator!=(Vector2 const& b) const { return x != b.x || y != b.y; }
    bool operator==(Vector2 const& b) const { return x == b.x && y == b.y; }
	bool operator!=(double const& b) const { return x != b || y != b; }
	bool operator==(double const& b) const { return x == b && y == b; }
	bool is_zero() const { return x == 0 && y == 0; }

	S dot() const { return x*x + y*y; }
	S hypot() const { return sqrt(this->dot()); }
	S l2() const { return sqrt(this->dot()); }
    S dot(Vector2 const& b) const { return x*b.x + y*b.y; }
	S length() const { return (S)sqrt((double)(x*x + y*y)); }

    Vector2 const& operator=(Vector2 const& b) { x = b.x; y = b.y; return *this; }
    void operator+=(Vector2 const& b) { x += b.x; y += b.y; }
    void operator-=(Vector2 const& b) { x -= b.x; y -= b.y; }
    void operator*=(Vector2 const& b) { x *= b.x; y *= b.y; }
    void operator/=(Vector2 const& b) { x /= b.x; y /= b.y; }
	void operator*=(S s) { x *= s; y *= s; }
	void operator/=(S s) { x /= s; y /= s; }

    static S area(Vector2 const& a, Vector2 const& b, Vector2 const& c);
    static Vector2 solve2x2_col(Vector2 const& c1, Vector2 const& c2, Vector2 const& c3);

    Vector2& norm();
    Vector2& rotate90();
    Vector2& rot90();
	//Vector const& rotate90_inv();
    Vector2 const& scalerotate(Vector2 const& m);
    Vector2 const& scalerotate_inv(Vector2 const& m);

    union {S x; S u; S a; S w;};
    union {S y; S v; S b; S h;};
};

typedef Vector2<int64_t> Vec2i;
typedef Vector2<double> Size;
typedef Vector2<double> Vec2;

template<typename S> struct RectS {
    RectS() : x(0), y(0), w(0), h(0) {}
    RectS(S const& x, S const& y, S const& w, S const& h) : x(x), y(y), w(w), h(h) {}
    RectS(S const& w, S const& h) : x(0), y(0), w(w), h(h) {}
    RectS(Vector2<S> const& b) : x(0), y(0), w(b.x), h(b.y) {}
    bool contains(Vector2<S> const& p) const;
    S x,y,w,h;
};

typedef RectS<int> Rect;
//typedef Rect<double> Rectd;

/* Definitions */
template<typename S> Vector2<S> & Vector2<S>::rotate90() {
	S tx = x;
	x = -y;
	y = tx;
	return *this;
}

template<typename S> Vector2<S> & Vector2<S>::rot90() {
	return rotate90();
}

/*
template<typename S> Vector<S> const& Vector<S>::rotate90_inv() {
	S tx = x;
	x = y;
	y = -tx;
	return *this;
}
*/

/*
template<typename S> Vector2<S> const& Vector2<S>::scalerotate(Vector2<S> const& m) {
	S px = x;
	x = (S)((double) x*m.x - (double)y*m.y);
	y = (S)((double)px*m.y + (double)y*m.x);
	return *this;
}
*/

template<typename S> Vector2<S> const& Vector2<S>::scalerotate(Vector2<S> const& m) {
    S px = x;
    x = (S)((double) x*m.x - (double)y*m.y);
    y = (S)((double)px*m.y + (double)y*m.x);
    return *this;
}

template<typename S> Vector2<S> const& Vector2<S>::scalerotate_inv(Vector2<S> const& m) {
    Vec2 fm(m);
    fm /= fm.dot();
    S px = x;
    x = (S)((double)x*fm.x + (double)y*fm.y);
    y = (S)((double)-px*fm.y + (double)y*fm.x);
    return *this;
}

template<typename S> Vector2<S> & Vector2<S>::norm() {
	if(x == 0 && y == 0) return *this;
	double d = length();
	x /= d;
	y /= d;
	return *this;
}

template<typename S> bool RectS<S>::contains(Vector2<S> const& p) const {
    return p.x >= x && p.x < w && p.y >= y && p.y < h;
}

template<typename S> Vector2<S> Vector2<S>::solve2x2_col(Vector2<S> const& a, Vector2<S> const& b, Vector2<S> const& c) {
    //return Vector2(c3^c1,c2^c3)/(c1^c2);
    return Vector2(c^b,a^c)/(a^b);

}

template<typename S> S Vector2<S>::area(Vector2 const& a, Vector2 const& b, Vector2 const& c) {
    return(b - a)^(c - a);
}
