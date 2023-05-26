#pragma once
#include <list>
#include "common/vector.h"
#include "common/color.h"
#include <vector>

struct Tri;
struct Node;
struct Contour;

struct Edge {
	Edge();
	Edge(Edge const& e);
    Edge(Node* en, Edge* ej, Edge* nxt, Edge* prv, Tri* t);

	Edge* snxt() const { return prv->j; }
    Edge* sprv() const { return j->nxt; }
    Vec2 v() const;
    bool line() const;

	void link(Edge* e);
    void operator^=(double w);
    void operator|=(double w);

	Node* n;
	Edge* j; //adjacent (side-to-side) edge.
	Edge* nxt,* prv;
	Tri* t;

    //Vec3 p2u(Vec2 const& p) const;

    double ang() const;
    double cos() const;
    double sin() const;

    mutable double w; //weight

    mutable Col c;

    mutable double o1;
    mutable double o2;
    mutable double cost;

    Contour *cntr;
};


struct Edge_iterator {
    Edge_iterator() : first(true), e(0) {}
    Edge_iterator(Edge_iterator const& i) : first(i.first), e(i.e) {}
    Edge_iterator(Edge* e) : first(true), e(e) {}
    bool operator!=(Edge_iterator const& i) const;
    bool operator==(Edge_iterator const& i) const;
    Edge* operator*() const { return e; }

    virtual Edge_iterator const& operator++() = 0;
    virtual Edge_iterator const& operator--() = 0;
protected:
    bool first;
    Edge* e;
};

struct Node_iterator : Edge_iterator {
    Node_iterator() {}
    Node_iterator(Edge_iterator const& i) : Edge_iterator(i) {}
    Node_iterator(Edge* e) : Edge_iterator(e) {}
    Edge_iterator const&  operator++();
    Edge_iterator const&  operator--();
};

struct Trigon_iterator : Edge_iterator {
    Trigon_iterator() {}
    Trigon_iterator(Edge_iterator const& i) : Edge_iterator(i) {}
    Trigon_iterator(Edge* e) : Edge_iterator(e) {}
    Edge_iterator const&  operator++();
    Edge_iterator const&  operator--();
};
