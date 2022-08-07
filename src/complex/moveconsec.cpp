#include "flip.h"
#include "common/vector.h"
#include "complex/edge.h"
#include "complex/node.h"
#include "complex/trigon.h"
#include "complex/complex.h"

#include <algorithm>
#include <iostream>

#include "globals.h"

void Complex::move_nodes_consec()
{
    for(auto n : mv_nodes) {
        move_consec(*n, n->np);
    }
    mv_nodes.clear();
}

Core::Result Complex::move_consec(Node& n, Vec2i p) {
    if(n.invalid()) return Core::None;
    if(n.type == Node::Corner) return Core::None;

    if(n.type == Node::Border) {
        Vec2i nxtp(n.border()->nxt->n->cp);
        //I don't understand what I did here (line below). Todo: maybe remove.
        if(nxtp == n.cp) return Core::None;
        Vec2 b = nxtp - n.cp;
        //move p as projected onto the border
        p = n.cp + Vec2i((b*b.dot(Vec2(p - n.cp))/b.dot(b)));
    }

    clip(_canvas, p);

    n.pp = n.cp;
    n.cp = p;

    //if(!do_rectify) return Core::None;

    auto ret = core.rectify(n);
    n.pp = n.cp;
    return ret;
}

Core::Result Core::rectify(Node& n) {
    ri = NoFlip;
    flip_cursor = flip_buf;
    pp.clear();

    if(prime(n)) {
        rectify_primed();
        if(ri != EqPos)
            ri = Flipped;
    }

    return ri;
}

int e_count(Node* n)
{
    int c = 0;
    auto i = n->entry();
    do
    {
        ++c;
        i = i->snxt();
    } while(i != n->entry());
    //for(Node::iterator i = n->begin(); i != n->begin(); ++i, ++c);
    return c;
}

//Removes a pair of trigons sharing the edge e. The node e->n will be preserved
//while e->nxt->n is made Dead. Affected node entries are adjusted accordingly.
void Core::remove(Edge* e) {
    Node* n = e->n;
    Node* s = e->nxt->n;

    //N.B. we iterate through the entire star.
    for(Node::iterator i = s->begin(); i != s->begin(); ++i) {
        (*i)->n = n;
    }

    //Set n->entry to m->entry, unless m->entry points to some edge which is deleted.
    //This maneuver ensures entry-edge == null-edge for m if n is a Border node.
    if(s->entry() == e->j || s->entry() == e->nxt) n->setentry(e->snxt());
    else n->setentry(s->entry());

    Edge* j = e->j;

    //Change entry nodes if they are pointing to trigons to be deleted.
    if(e->prv->n->entry() == e->prv) --*e->prv->n;
    if(j->prv->n->entry() == j->prv) ++*j->prv->n;

    e->nxt->j->link(e->prv->j);
    j->nxt->j->link(j->prv->j);

    e->t->retire();
    j->t->retire();
    n->type = s->type;
    //n->n = s->n;
    n->color = s->color;
    //n->w_ = s->w_;
    s->status = Node::Dead;

    //myDebug() << "node removed.";
}

//Checks for node overlapping and amends if such is the case.
inline bool Core::eqpos_prv(Edge** pe) {
    Edge* e = *pe;
    if(e->n->cp != e->prv->n->cp) return false;
    *pe = e->snxt()->snxt();
    remove(e->snxt());
    ri = EqPos;
    return true;
}

//Checks for node overlapping and amends if such is the case.
inline bool Core::eqpos_nxt(Edge** pe) {
    Edge* e = *pe;
    if(e->n->cp != e->nxt->n->cp) return false;
    *pe = e->snxt();
    remove(e);
    ri = EqPos;
    return true;
}

/*
    d                d
   /#\              /|\
  /###\            j#|#\
 /#####\          /##|##\
a---e-->c  --->  a###|###c
 `i   .´         |`i#|#e´|
q  `b´  r        q--`b´--r
    ^                ^
    |                |
    |                |

Flipping a trigon pair, method 1. Maintains entry edges within the trigon pair. */
void Core::iflip(Edge* i) { //in-flip ("new flip")
    Edge *e = i->snxt(), *j = e->j;

    double acw = e->w, caw = e->j->w, daw = e->prv->w, bcw = j->prv->w;

    j->t->color = e->t->color;
    e->j->t->type = e->t->type;

    if(e->n->entry() == e) --*e->n;
    e->n = j->prv->n;
    if(j->n->entry() == j) --*j->n;
    j->n = e->prv->n;
    e->link(j->prv->j);
    j->link(e->prv->j);
    e->prv->link(j->prv);

    *i |= acw;
    *e |= bcw;
    *i->j |= caw;
    *e->j |= caw;
    j->w = daw;
    i->nxt->w = e->prv->w = 0;

    //myDebug() << "iflip";
}


/*
    |                |
    |                |
    V                V
q  .d.  r        q--.d.--r
 .´   `.         |.´#|#j.|
a---e-->c  --->  a###|###c
|\#####/|        |\##|##/|
| \###/ |        | e#|#/ |
|  \#/  |        |  \|/  |
t---b---s        t---b---s

Flipping a trigon pair, method 2. Maintains entry edges within the trigon pair. */
void Core::oflip(Edge* e) { //out-flip ("old flip")
    Edge* j = e->j;

    double acw = e->w, caw = j->w, abw = j->nxt->w, cdw = e->nxt->w;

    e->t->type = j->t->type;
    //e->t->color = j->t->color*0.85 + Col::random()*0.15;//e->t->color*0.35;
    e->t->color = j->t->color;

    if(j->n->entry() == e->nxt) ++*j->n;
    e->nxt->n = j->prv->n;
    if(e->n->entry() == j->nxt) ++*e->n;
    j->nxt->n = e->prv->n;
    e->link(j->nxt->j);
    j->link(e->nxt->j);
    e->nxt->link(j->nxt);

    *e->prv->j |= acw;
    *j->j |= acw;
    *j |= cdw;
    *e->prv |= caw;
    e->w = abw;
    e->nxt->w = j->nxt->w = 0;

    //myDebug() << "oflip";
}

inline void Core::pushflip(Flip* f) {
    fh.push_back(f);
    push_heap(fh.begin(), fh.end(), &Flip::comp);
}

inline Flip* Core::popflip() {
    Flip* f = fh.front();
    pop_heap(fh.begin(), fh.end(), &Flip::comp);
    fh.pop_back();
    return f;
}

inline void Core::pushflip(Flip* f, int i, int g) {
    f->i = i;
    f->g = g;
    pushflip(f);
}

//To be treated as "= new Flip(e)"
inline Flip* Core::newflip(Edge* e) {
    ++flip_cursor;
    if(flip_cursor - flip_buf >= MAX_FLIPS) throw "bad max flips";
    flip_cursor->e = e;
    return flip_cursor;
}

inline bool Core::antimid() {
    for(unsigned int i = 0; i < pp.size(); ++i) {
        if(Flip::cross(pp[i]) == Flip::AntiMid) return true;
    }
    return false;
}

//Scans the edges of the node star in one revolution populating the flip heap
//with flip cases and identifying the AntiMid edge which is used as a starting point
//for the flip case index count.
bool Core::prime(Node& n) {
    Edge* h = n.entry();
    unsigned int s = 0;
    int i = 0;
    bool anti = false;

    if(!eqpos_prv(&h)) eqpos_nxt(&h);

    Flip* f = newflip(h);
    do {
        if(f->ndc()) { //flip-case?
            if(eqpos_prv(&f->e)) continue;

            //If this is the first non-positive trigon then /now/ we search the
            //positives for the Antimid case. This is an optimization shortcut
            //so that the Antimid calculations are not done unnecessarily when
            //there are no flip-cases.
            if(fh.empty()) anti = antimid();

            //If the AntiMid case hasn't been found yet, these Flip-indices will
            //be set later on anyway.
            if(anti) {
                f->i = i;
                i++;
            }

            //This assignment should not be necessary, but at the moment of writing
            //it is not the time to test this hypothesis.
            f->g = 0;

            fh.push_back(f);
            f = newflip(f->e);
        }
        else {
            if(fh.empty()) pp.push_back(f->e);
            else if(!anti && Flip::cross(f->e) == Flip::AntiMid) {
                s = (unsigned int)fh.size();
                anti = true;
            }
        }

        f->e = f->e->snxt();
    } while(f->e != h);

    if(fh.empty()) return false;

    //Retroactively setting the priority index on all entries that were added
    //before the Antimid edge was identified.
    for(unsigned int j = 0; j < s; ++j, ++i) {
        fh[j]->i = i;
    }

    make_heap(fh.begin(), fh.end(), &Flip::comp);

    return true;
}

void Core::rectify_primed() {
    Node const* n = fh.front()->e->n;
    int g = 0;

    while(!fh.empty()) {

        Flip* f = popflip();

        //A flip-case may become irrelevant and ostracized from the star by some other flip.
        if(f->e->n != n) continue;

        switch(f->c) {
        case Flip::Right:
            oflip(f->e);
            if(f->ndc()) pushflip(f);
            break;

        case Flip::Left:
            iflip(f->e);
            if(f->ndc()) pushflip(f);
            break;

        case Flip::Mid:
            {
                if(jpadding(f->e->nxt)) f->e->n->type = Node::Border;

                oflip(f->e->nxt);
                //A Mid-flip results in an addition to the star that we need to
                //check for vertex overlap.
                if(eqpos_prv(&f->e)) break;

                ++g;
                Flip* h = newflip(f->e->snxt());
                if(h->ndc()) pushflip(h, f->i, g);
                if(f->ndc()) pushflip(f, f->i, g);
                break;
            }
        default: break; //Does not happen.
        }
    }
}

//Biggest first.
bool Flip::comp(Flip* f, Flip* s) {
    //The actual value of n/d is not needed when comparing the two quotients.
    //This allows us to compute n_1*d_2 - n_2*d_1 instead. [todo?: big integer]
    int64_t qd = f->n*s->d - s->n*f->d;
    if(qd != 0) return qd < 0;

    //This is the second time I've derived these tests of degenerate cases.
    //I believe they are needed [*]. Evidently a bad heap structure can cause
    //problems; otherwise I see no reason why these tests could not be skipped.
    if((f->d == 0 && f->n == 0) && !(s->d == 0 && s->n == 0)) return true;
    else if((s->d == 0 && s->n == 0) && !(f->d == 0 && f->n == 0)) return false;
    //

    if(f->c != s->c) return f->c > s->c;
    if(f->i == s->i) return f->g > s->g;
    if(f->c == Left) return f->i < s->i;
    return f->i > s->i;
}

/*
[*] A degenerate case occurs when the node has traveled through a vertex
such that the following Right-flip case A first causes another
Right-flip case B to become ostracized and then remains a Right-flip case such
that the flip results in a hull edge precisely parallel with the travel line.
Let C be the Mid-flip case following the vertex crossing. Between B and C we get by the
n/d-test that B < C; between A and B we get by i-test (since the n/d-test fails)
that A < B; and between A and C we get by c-test (n/d-test fails again) that C < A.
Thus we end up with a scenario where A < B < C < A, that is, an invalid heap.

--- Old version. I think I was wrong here. --
A degenerate predicate case occurs when the node has traveled through a vertex
such that the Right-flip following on the inclined edge A first causes another
Right-flip edge B to become ostracized and then triggers another Right-flip on A
such that its hull edge is precisely parallel with the travel line. Let C be
the Mid-flip edge following the vertex crossing. If by cross time B < C,
we end up with a scenario where A < B < C < A.
*/

Flip::Cross Flip::cross(Edge* e) {
    Vec2i h1p0(e->n->pp - e->nxt->n->cp);
    Vec2i h1p1(e->n->cp - e->nxt->n->cp);
    Vec2i h2p0(e->n->pp - e->prv->n->cp);
    Vec2i h2p1(e->n->cp - e->prv->n->cp);

    //return (Cross)(((h1p1^h1p0) >= 0) || ((h2p0^h2p1) >  0) << 1);
    if((h1p1^h1p0) >= 0) return ((h2p0^h2p1) > 0)? Mid : Right;
    else return ((h2p0^h2p1) > 0)? Left : AntiMid;
}

bool Flip::ndc() {
    Vec2i h1h2(e->prv->n->cp - e->nxt->n->cp);
    Vec2i h1p1(e->n->cp - e->nxt->n->cp);
    n = h1h2^h1p1;

    if(n > 0) { //Positive area, we are done.
        e->t->_area = n; //Saving this calculation, though it is further irrelevant for this algorithm.
        return false;
    }

    Vec2i p0p1(e->n->cp - e->n->pp);
    d = h1h2^p0p1;

    if(d == 0 && n == 0) {
        //This is the special case when the node has traveled through a hull vertex resulting in a hull edge becoming parallel with the travel line after a flip.
        //This can occur after a mid-flip and after a right-flip. Both occurrences happen to be covered by the below test.
        c = (h1p1.dot() < h1h2.dot())? Mid : Right;
    } else {
        //Standard case. We could use cross() but we will shortcut a determinant calculation here (AntiMid can never occur).
        Vec2i h1p0(e->n->pp - e->nxt->n->cp);
        if((h1p1^h1p0) >= 0) {
            Vec2i h2p1(e->n->cp - e->prv->n->cp);
            Vec2i h2p0(e->n->pp - e->prv->n->cp); //<-- is this correct, "e->prv->n->cp"? i don't want to change this, seems to work.
            c = ((h2p0^h2p1) > 0)? Mid : Right;
        }
        else c = Left;
    }

    return true;
}
