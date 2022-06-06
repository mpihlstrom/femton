#pragma once
#include "common/vector.h"
#include "common/color.h"
#include "complex/edge.h"
#include "common/vector3.h"
#include "common/entity.h"
#include "iterator.h"

#include <list>
#include <vector>
#include <tuple>

#define nxtnpadding(e) ((e)->nxt->n->type == Node::Padding)

struct Node : Class, Eumetry {
	friend struct Complex;
	friend struct Core;
	friend struct Flip;
	friend struct Tri;
	friend struct Edge;
	friend struct State;

	typedef Node_iterator iterator;

	enum Status { Live, Dead };
	enum Type { Floating, Border, Corner, Padding };

	Node();
    Node(Vec2i const& p_, Edge* e);
    Node(Vec2i const& p_, Edge* e, Col const& c);
    Node(Vec2i const& p_, Type t, Col const& c);
    virtual ~Node() {}

    bool operator<(Node const& s);

    inline int64_t x() const { return cp.x; }
    inline int64_t y() const { return cp.y; }
    inline Vec2i p_() const { return cp; }
    inline Vec2 p() const { return cp; }
    bool invalid() const { return status == Dead || type == Padding; }
	Status state() const { return status; }
	Type type_() const { return type; }
    Col col() const { return color; }
    double c() const;
    iterator begin() const;
    iterator end() const;
    Edge* entry() const { return *entry_; }
    Edge* border() const;
    double ctrst() const { return contrast; }

    Vec2 ppcp(double t) const { return Vec2(pp)*(1-t) + Vec2(cp)*t; }

    Vec3 wnu_init() const;

    Vec3 wn() const;
    double w() const;
    Vec2 g() const;
    Vec2 min_edge() const;
    double area() const;

    std::tuple<double,double> calc_flip_t();

    virtual Class::Name name() const { return Class_node; }

    void swap_p();

    Vec2i cp; //current point/position
    Vec2i pp; //previous point/position
    Vec2i np; //new point/position

    Vec2 v;

    double flip_t;
    double flip_t_acc;

private:
	//Anything that can be used to corrupt (or is relevant only to) the complex is private.
	iterator operator++();
	iterator operator--();
public:
    Edge* setentry(Edge* e);

	iterator entry_;
	Type type;
public:
	Status status;    

public:
    mutable Col color;
    double w_; //make private?
    Vec2 g_;
    double contrast;
};

typedef std::list<Node*> Nodes;
typedef std::vector<Node*> NodesV;
typedef Nodes::iterator niterator;

