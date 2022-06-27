#pragma once
#include "complex/edge.h"
#include <cstdint>
#include <vector>

#define MAX_FLIPS 0x10000

//The structure for sorting the flip cases. There are four orders of priority,
//the first and main one being where on the travel line the trigon became non-positive.
//Biggest first.
struct Flip {

	/*
	UPDATE: The below comment is nonsense. Basically the degenerate case conditionals in the predicate take care of this.
	I'm keeping the comment for historical reasons and because it is an interesting case worth keeping in mind.
	-----
	The order in Cross is important. It must be that Left < Right or else there will be a degenerate like this:
	
	     *.
	    /: `.
	   / :   `.
	  /  X     `.
	 /   :       `.
	*----*....*....*
			 
		 O
	*/

	enum Cross {
		AntiMid = 0,
		Left = 1,
		Right = 2,
		Mid = 3,
	};

	Flip() : e(0), n(0), d(0), c(AntiMid), i(0), g(0) {}
	Flip(Edge* e) : e(e), n(0), d(0), c(AntiMid), i(0), g(0) {}

	//Computes n first, which is essentially the area. If it's positive, it returns false. Otherwise it proceeds to compute d.
	bool ndc();

	//Calculates c for an Edge. The function is used for finding the AntiMid case on positive trigons.
	static Cross cross(Edge* e);
	
	//The compare predicate function. It uses several attributes to define an order. For instance, if a hull crossing of two edges occur at the same t' = n/d,
	//the function will proceed to check what type of crossing the two cases are for determining the predicate outcome. If further still the type of crossing is 
	//also the same, the index will be compared, and so on.
	static bool comp(Flip *f, Flip *s);

	Edge* e;
	//We have that the quotient n/d = q \in [0, 1] represents where/when the hull was crossed.
	//This is the primary sorting value. q is reversed, bigger is earlier.
	int64_t n, d;
	Cross c; //The type of star hull crossing. Secondary sorting value.
	int i; //This is basically the index of the edge as ordered circularly around the star. Tertiary sorting value.
	int g; //The generation, increases for each time the star hull is penetrated and expands by means of a mid-flip. Quaternary sorting value.
};

//The machinery that deals with re-triangulation.
struct Core {

    enum Result {
        None = 0,
        NoFlip = 1,
        EqPos = 2,
        Flipped = 3,
    };

    Core() : flip_cursor(), ri(None) {}

    Result rectify(Node& n);
    Result rectify_set(Node& n);

    static void remove(Edge* e);

    static bool remove_star(Node* n);
    static void iflip(Edge* e);
    static void oflip(Edge* e);

private:
    bool prime(Node& n);
    void rectify_primed();
    void rectify_primed_set();
    Flip* newflip(Edge* e);
    Flip* popflip();
    void pushflip(Flip* f);
    void pushflip(Flip* f, int i, int g);
    bool eqpos_prv(Edge** pe);
    bool eqpos_nxt(Edge** pe);
    bool antimid();

    Flip flip_buf[MAX_FLIPS]; //Buffer for flips.
    Flip* flip_cursor;
    std::vector<Flip*> fh; //flip heap
    //Pre-negative Positives. This buffer is used for the case when the star
    //trigons are all positive, to avoid the overhead of finding the AntiMid edge.
    std::vector<Edge*> pp;
    Result ri; //storage for returning information from rectify()
};
