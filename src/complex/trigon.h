#pragma once
#include <list>
#include <vector>
#include "common/color.h"
#include "common/vector3.h"
#include "common/entity.h"
#include "edge.h"
#include "mpir/mpir.h"
#include <set>
#include <map>

struct Node;
struct Tri;

typedef std::list<Tri*> Trigons;
typedef std::vector<Tri*> TrigonsV;
typedef Trigons::iterator titerator;

struct Concomp {
    TrigonsV ts;
    int64_t area;
    Vec2 mid;
    double peri;
    double cos;
    std::set<Concomp*> neighbors;
    Vec3 covar;
    Vec2 eigv;
    Vec2 eigvec1;
    Vec2 eigvec2;
    double r;
    Edge* cnt_entry;
    std::vector<Edge*> cnt;

    Edge* cntr(int i) {
        return cnt[(i + cnt.size() - 1) % cnt.size()];
    }
};

struct Tri : Class, Eumetry {
    struct Cross {
        struct Root {
            static Root root_t0;
            static Root root_t1;

            enum Kind {
                None = 0,
                Linear = 1,
                Square = 2,
                Imaginary = 3
            };

            Root(Root const& s);
            Root();
            Root(Tri const& t, Root const* cur, int sqrtsgn = 0);
            Root(int64_t A, int64_t B, int64_t C, Root const* cur, int sqrtsgn = 0);
            void init(Tri::Cross::Root const* cur   );
            void operator=(Root const& s);

            Root const& operator()(int s) { sqrtsgn = s; return *this; }
            int operator-(Root const& s) const;
            double root() const { return sqrtsgn <= 0? root1 : root2; }

            int init_sq_sgn_lh1_big();
            static int linlin_sgn_lh1_big(Root const& f, Root const& s);
            static int linsq_sgn_lh1_big(Root const& f, Root const& s);
            static int linsq_sgn_rh1_big(Root const& s);
            static int linsq_sgn_lh2_big(Root const& f, Root const& s);
            static int sqsq_sgn_lh1_big(Root const& f, Root const& s);
            static int sqsq_sgn_rh1_big(Root const& f, Root const& s);
            static int sqsq_sgn_lh2_big(Root const& f, Root const& s);
            //static int sqsq_sgn_rh2_big(Root const& f, Root const& s);
            static int sqsq_sgn_rh2_big_2(Root const& f, Root const& s);
            static int sqsq_sgn_lh3_big(Root const& f, Root const& s);
            static void init_mpzs();
            static void clear_mpzs();

            int64_t A;
            int64_t B;
            int64_t C;
            mpz_t A_;
            mpz_t B_;
            mpz_t C_;

            Kind kind;
            int sqrtsgn;
            //bool colin;

            double root1;
            double root2;

            static mpz_t t, t1, t2, t3, t4, t5, t6, t7, t8;
        };

        struct SqL2
        {
            SqL2(Edge* e, Root* t);
            SqL2(int64_t A, int64_t B, int64_t C, Root* t);
            SqL2 operator-(SqL2 const& s) const;
            void operator=(SqL2 const& s);
            int operator()() const;

            int64_t A;
            int64_t B;
            int64_t C;
            mpz_t A_;
            mpz_t B_;
            mpz_t C_;

            Root* t;
        };

        Cross(Tri* t) : t(t), detached(true) {}
        Cross() : t(nullptr), flip_e(nullptr), invalid(true), nodecross(false), count(0), iter(0), crossroot(Root()), overlap(false), detached(false) {}
        Cross(Tri* t, Edge* flip_e, int count, Root const& cr, int overlap = 0) :
            t(t), flip_e(flip_e), invalid(false), nodecross(false), count(count), iter(0), crossroot(cr), overlap(overlap), detached(false) {}
        static bool lt(Tri::Cross* f, Tri::Cross* s);
        Cross* operator++() { ++iter; return this; }

        Tri* t;
        Edge* flip_e;
        bool invalid;
        bool nodecross;
        int count;
        int iter;

        Root crossroot;

        int overlap;
        bool detached;
    };

    Tri::Cross* create_cross(Tri::Cross* curcr = nullptr);
    Cross* cross;

    typedef Trigon_iterator iterator;

	friend struct Complex;
	friend struct Core;
	friend struct Flip;
	friend struct State;

    enum Label { None = 0, A = 1, B = 2, C = 3, Aj = 4};
	enum Status { Dead, Live };
	enum Type {	Regular, Padding };

    Tri() : cross(nullptr), type(Regular), status(Dead), _area(1), entry(&a) {}
    Tri(Node* an, Edge* aj, Node* bn, Edge* bj, Node* cn, Edge* cj, Type type, Col const& col = Col());
    Tri(Node* an, Node* bn, Node* cn, Type t, Col const& col);
    virtual ~Tri() {}

    Vec2 operator*(Vec3 const& s) const;

    static int64_t deter(Node& a, Node& b, Node& c);
    static void clamp(Vec3& cp, Vec2 const& a, Vec2 const& b, double q);

    static Vec3 p2u(Vec2 const& a, Vec2 const& b, Vec2 const& c, Vec2 const& q);

    iterator begin() const { return entry; }
    iterator end() const { return entry; }

    Edge const& ea() const { return a; }
	Edge const& eb() const { return b; }
	Edge const& ec() const { return c; }
	Node const& na() const { return *a.n; }
	Node const& nb() const { return *b.n; }
	Node const& nc() const { return *c.n; }
    Col const& col() const { return color; }
	Type type_() const { return type; }
	Status state() const { return status; }
    bool invalid() const {
        return status == Dead || type == Padding;
    }
	Label label(Node const& n) const;
    Label label(Edge const* e) const;
    bool inside(Vec2i const& p) const;
    Vec2 centroid() const;
    void join_cps();
    Vec3 cntrst();

    void clamp_cps();

    Vec3 gcont_b111();

    Vec3 p2u(Vec2 const& p) const;

    Vec2 ab() const;
    Vec2 bc() const;
    Vec2 ca() const;
private:
    Vec2 clamp(Vec2 p); //clamp to trigon area
public:
    virtual Class::Name name() const { return Class_trigon; }

    //This value is intended to always be the correct area/determinant
    //int64_t area() const { return _area; }
    int64_t area() const;
    double area3() const;
    void u_2_e_q(Vec2 const& u2, Edge const** e, double* q) const;

    mutable double w;

    void retire();
    void retire_w_ns();
private:
	//Anything that can be used to corrupt the complex (or is relevant only to
	//the complex) is private.
	Edge* edge(Label l);

public:
    Type type;
    Status status;

    Edge a, b, c;

	//Prefixed underline means that these are temporary storage variables.
    Sint64 _area;

    Col color;

    iterator entry;

    Concomp* cc;
};

typedef Tri::Cross Cross;
typedef Tri::Cross::Root Root;
