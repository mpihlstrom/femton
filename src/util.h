#pragma once

#include "common/vector.h"
#include <string>

#define GET_A(X) ((X & 0xff000000) >> 24)
#define GET_R(X) ((X & 0x00ff0000) >> 16)
#define GET_G(X) ((X & 0x0000ff00) >> 8)
#define GET_B(X) (X & 0x000000ff)

#define WGET_R(X) ((Uint32)((X & 0x00ff0000) >> 16))
#define WGET_G(X) ((Uint32)((X & 0x0000ff00) >> 8))
#define WGET_B(X) ((Uint32)(X & 0x000000ff))

#define PUT_A(X) ((X) << 24)
#define PUT_R(X) ((X) << 16)
#define PUT_G(X) ((X) << 8)
#define PUT_B(X) (X)

#define PUT_RGB(X, Y, Z) ((((Uint8)(X)) << 16) | (((Uint8)(Y)) << 8) | (Uint8)(Z))
#define PUT_RGBA(X, Y, Z, W) ((((Uint8)(W)) << 24) | (((Uint8)(X)) << 16) | (((Uint8)(Y)) << 8) | (Uint8)(Z))

#define SWAP(A, B, T) T = A; A = B; B = T

template<typename S> S area(Vector2<S> a, Vector2<S> b, Vector2<S> c) {
	Vector2<S> ab = b - a;
	Vector2<S> ac = c - a;
	return ab.x*ac.y - ab.y*ac.x;
}

template<typename S> Vector2<S> rot90(Vector2<S> const& a) {
	return Vector2<S>(-a.y, a.x);
}

std::string new_filename(char const* path, char const* ext);
std::string get_file_contents(const char *filename);
