#include "common.h"
#include "common/color.h"
#include "util.h"

const Col Col::Red(1,0,0), Col::Yellow(1,1,0), Col::Green(0,1,0), Col::Cyan(0,1,1), Col::Blue(0,0,1), Col::Magenta(1,0,1), Col::White(1,1,1),  Col::Black(0,0,0);
const Col Col::Orange(1,0.5,0);
const Col Col::Gray(0.5,0.5,0.5);

Col::Col(Uint32 c) {
	r = (double)GET_R(c)/255.0;
	g = (double)GET_G(c)/255.0;
	b = (double)GET_B(c)/255.0;
	a = (double)GET_A(c)/255.0;
}

Uint32 Col::to32() const {
	return PUT_RGBA(r*255.0, g*255.0, b*255.0, a*255.0);
}

double Col::i() const {
	return intensity();
}
double Col::intensity() const {
	return 0.21267*r + 0.71516*g + 0.07217*b;
}

void Col::delta(double d) {
	r = r + d;
	g = g + d;
	b = b + d;
//	clamp();
}



double Col::dist() const {
    return sqrt(0.21267*0.21267*r*r + 0.71516*0.71516*g*g + 0.07217*0.07217*b*b);
}

bool Col::operator==(Col const& b) const {
    return dist(b) < 0.0001;
}

double Col::dist(Col const& s) const {
    //return sqrt(0.21267*0.21267*(s.r-r)*(s.r-r) + 0.71516*0.71516*(s.g-g)*(s.g-g) + 0.07217*0.07217*(s.b-b)*(s.b-b) + (s.a-a)*(s.a-a));
    return sqrt(((s.r*s.a-r*a)*(s.r*s.a-r*a) + (s.g*s.a-g*a)*(s.g*s.a-g*a) + (s.b*s.a-b*a)*(s.b*s.a-b*a))/3.0);

}

Col Col::h120() const {
	return Col(b, r, g, a);
}

Col Col::inv() const {
	return Col(1.0 - r, 1.0 - g, 1.0 - b, a);
}

Col Col::semi() const {
	return Col((double)(Uint8)(128.0 + 255.0*r)/255.0, (double)(Uint8)(128.0 + 255.0*g)/255.0, (double)(Uint8)(128.0 + 255.0*b)/255.0, a);
}

Col Col::random() {
    return Col(rand_uni(), rand_uni(), rand_uni(), 1.0);
}

Col Col::random_a() {
    return Col(rand_uni(), rand_uni(), rand_uni(), rand_uni());
}

Col Col::mod() {
	r = fmod(r, 1.0);
	g = fmod(g, 1.0);
	b = fmod(b, 1.0);
	return *this;
}

void Col::clamp() {
    if(r > 1.0)      r = 1.0;
    else if(r < 0.0) r = 0.0;
    if(g > 1.0)      g = 1.0;
    else if(g < 0.0) g = 0.0;
    if(b > 1.0)      b = 1.0;
    else if(b < 0.0) b = 0.0;
    if(a > 1.0)      a = 1.0;
    else if(a < 0.0) a = 0.0;
}

Col& Col::alpha_over(Col const& s) {
	//alpha compositing
	double oa = 1.0 - (1.0 - s.a)*(1.0 - a);
	double q = s.a/oa;
	double cq = 1.0 - s.a/oa;
	r = s.r*q + r*cq;
	g = s.g*q + g*cq;
	b = s.b*q + b*cq;
	a = oa;
	return *this;
}

Col& Col::blend(Col const& s, double q) {
	double cq = 1.0 - q;
	r = s.r*q + r*cq;
	g = s.g*q + g*cq;
	b = s.b*q + b*cq;
	a = s.a*q + a*cq;
	return *this;
}

std::ostream& operator<<(std::ostream &out, Col &c) {
	out << "(" << c.r << ", " << c.g << ", " << c.b << ")";
	return out;
}


Col::Col(double i) : Vec3(i, i, i), a(1.0) {
	clamp();
}



/*
void Color::operator/=(double s) {
    *this = *this / s;
}

void Color::operator*=(double s) {
    *this = *this * s;
}
*/

/*
double Color::dist(Color const& f, Color const& s) {
    return (s.r-f.r)*(s.r-f.r) + (s.g-f.g)*(s.g-f.g) + (s.b-f.b)*(s.b-f.b) + (s.a-f.a)*(s.a-f.a);
}*/

/*
Color Color::operator*(double q) const {
    return Color(r*q, g*q, b*q, a*q);
}

Color Color::operator/(double d) const {
    return Color(r/d, g/d, b/d, a/d);
}

Color Color::operator-(Color const& s) const {
    return Color(r - s.r, g - s.g, b - s.b, a - s.a);
}

Color Color::operator+(Color const& s) const {
    return Color(r + s.r, g + s.g, b + s.b, a + s.a);
}

Color Color::operator-(int c) const {
    return Color(r - c, g - c, b - c, a - c);
}

Color Color::operator+(int c) const {
    return Color(r + c, g + c, b + c, a + c);
}
*/
