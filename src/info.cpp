#include "info.h"


double minmax_angle_ratio(fVector const& av, fVector const& bv, fVector const& cv)
{
	fVector a = (bv - av).norm();
	fVector b = (cv - bv).norm();
	fVector c = (av - cv).norm();
	double ab = acos(-a.dot(b));
	double bc = acos(-b.dot(c));
	double ca = acos(-c.dot(a));
	double max, min;
	max = min = ab;
	if(bc > max) max = bc;
	if(ca > max) max = ca;
	if(bc < min) min = bc;
	if(ca < min) min = ca;
	return min/max;
}

double minmax_length_ratio(fVector const& av, fVector const& bv, fVector const& cv)
{
	double a = (bv - av).length();
	double b = (cv - bv).length();
	double c = (av - cv).length();
	double max, min;
	max = min = a;
	if(b > max) max = b;
	if(c > max) max = c;
	if(b < min) min = b;
	if(c < min) min = c;
	return min/max;
}

double inscribedcircum_ratio(fVector const& av, fVector const& bv, fVector const& cv)
{
	double a = (bv - av).length();
	double b = (cv - bv).length();
	double c = (av - cv).length();
	double s = 0.5*(a+b+c);
	double in_r = sqrt((s-a)*(s-b)*(s-c)/s);
	double out_r = (a*b*c)/sqrt((a+b+c)*(-a+b+c)*(a-b+c)*(a+b-c));
	double r = in_r/out_r;
	return r*r;
}