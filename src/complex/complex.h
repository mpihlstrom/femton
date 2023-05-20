#pragma once
#include "node.h"
#include "trigon.h"
#include "flip.h"
#include <vector>
#include <list>
#include <iostream>
#include <unordered_map>

#define epadding(e) (e->t->type == Tri::Padding)
#define jpadding(e) (epadding(e->j))
#define ejpadding(e) (epadding((e)) || epadding((e)->j))
#define contour(e) ((e->t->color - e->j->t->color).dist() > ContourColorDist)

template<typename S> struct Euiterator {
    typedef std::list<S*> Eumetries;
    Euiterator(Eumetries* s, typename Eumetries::iterator const& a) : s(s), i(a) {} //potentially empty list
    bool operator!=(Euiterator const& a) const { return a.i != i; }
    bool operator==(Euiterator const& a) const { return a.i == i; }
    S* operator*() { while((static_cast<Eumetry*>(*i))->invalid()) ++i; return *i; } //first obj could be invalid
    Euiterator const& operator++() { do { ++i; } while(i != s->end() && (static_cast<Eumetry*>(*i))->invalid()); return *this; }
    Eumetries* s;
    typename Eumetries::iterator i;
};

template<typename S> struct Eulist {
    typedef std::list<S*> Eumetries;
    Eulist(Eumetries* s) : s(s) {}
    Euiterator<S> begin() { return Euiterator<S>(s, s->begin()); }
    Euiterator<S> end() { return Euiterator<S>(s, s->end()); }
    Eumetries* s;
};

typedef Eulist<Node> Eunodes;
typedef Eulist<Tri> Eutris;

struct Complex {
    enum Move {
        Concur = 0,
        Pseudo = 1,
        Consec = 2
    };

    const int Units;
    const double color_dist_th;
    const double ev_quant;
    const double ar_quant;
    const double purge_cos_thresh;

    friend struct State;
    Complex(bool configure = false);
    //Complex(string filename);
	~Complex();

    Eunodes ns;
    Eutris ts;

	void load_config();
    static Move str_to_move(std::string str);

    Node* split_label_edge(Edge* e, double q, Edge*& e1, Edge*& e2);
    Node* split(Tri& t, Vec3 u);
    Node* split(Tri& t, Vec2 coord);
    Node* split_edge(Tri& t, Vec2 coord);
    Node* split_edge(Edge const* e, double q);

    Node* line_split_node(Node* n, Vec2 p1);

    void move_nodes();

    double area() { return 2*_canvas.x * 2*_canvas.y; }

    void move_nodes_concur();
    bool move(Node &n, Vec2i np);
    void set_node_type();
    bool cross_case(Tri* t, Tri::Cross* c = nullptr);
    void flip(Edge* e);
    bool merge(Edge* e);
    Node* remove(Cross* c);
    static bool remove_self_folded_tri(Edge* e);
    static bool remove_double_linked_tri(Edge* e);
    Cross* pop_cross();
    void push_cross(Cross* c);
    void detach(Cross* c);

    Core::Result move_consec(Node& n, Vec2i p);
    void move_nodes_consec();

    void refract();
    bool automata();

    void waste();
    Tri* inside(Vec2i const& p) const;
    Vec2 canvas() const { return _canvas; }

	void delaunay(Node& n);
    void delaunify(bool constrained = true);
	void unconstr_delaunify();
    void color_to_line();

    void purge();
    void purge_nonlines();
    void purge_small_trigons();
    void purge_stars();
    void purge_straight_lines();
    void purge_spikes();

    void randomize();

    static bool clip(Vec2i& c, Vec2i& p);
    static void clipd(Vec2i c, Vec2& p);

	bool constrained(Edge* e);
	bool redelaunay(Edge* e);
    bool flip_delaunay(Edge* e, bool constrained = true);
    //bool flip_worse(Edge* e);

    bool do_rectify;

    Trigons trigons;
    Nodes nodes;

    Nodes mv_nodes;

    Move move_mode;

private:
	static bool jt_inside_circumcircle(Edge* e);
public:
    Core core;

    std::vector<Tri::Cross*> mv_tris;
    int cross_counter;
    double curt;
    Vec2i _canvas;

    std::vector<Concomp*> ccs;

    int count;
};

