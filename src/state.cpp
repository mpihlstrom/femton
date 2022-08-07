#include "state.h"
#include "common/common.h"
#include "camera.h"
#include "info.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include "util.h"

#include "globals.h"

bool State::export_to(Complex const& cmplx, std::string file) {
    State state(cmplx);
    return state.export_to(file);
}

Complex* State::load_from(std::string file) {
	State state;
    if(state.import_from(file)) {
        return state.load();
	}
    return 0;
}

State::State(std::string const& file) {
    import_from(file);
}

State::State(Complex const& cmplx) {
    save(cmplx);
}

void State::save(Complex const& cmplx) {

	//used to associate each node's memory address with the node's index in the list
    std::unordered_map<Node*, uint32_t> node_index_map;
    std::unordered_map<Tri*, uint32_t> trigon_index_map; //same for trigons

	//complex.waste(); //Not necessary.

	nodes.clear();
    nodes.reserve(cmplx.nodes.size()); //potentially dead nodes also counted
	trigons.clear();
    trigons.reserve(cmplx.trigons.size()); //potentially dead trigons also counted

	//set up trigon address-index map first
	uint32_t index = 0;
    for(Trigons::const_iterator i = cmplx.trigons.begin(); i != cmplx.trigons.end(); ++i) {
		Tri* t = *i;
		if(t->status == Tri::Dead) continue;
		trigon_index_map[t] = index;
		++index;
	}

	//write node and set up address-index map
	index = 0;
    for(Nodes::const_iterator i = cmplx.nodes.begin(); i != cmplx.nodes.end(); ++i) {
        Node& n = **i;
        if(n.status == Node::Dead) continue;
        node_index_map[&n] = index;
        Edge* e = n.entry();
        nodes.push_back(File::Node(n.cp.x, n.cp.y, n.pp.x, n.pp.y, (uint8_t)n.type, trigon_index_map[(n.entry())->t], (uint8_t)e->t->label(e), n.color.to32(), n.w_, n.contrast));
		++index;
	}

	index = 0;
    for(Trigons::const_iterator i = cmplx.trigons.begin(); i != cmplx.trigons.end(); ++i) {
		Tri& t = **i;
		if(t.status == Tri::Dead) continue;

		uint32_t aji, bji, cji;
		Tri::Label ajl, bjl, cjl;

		aji = bji = cji = -1; //default
		ajl = bjl = cjl = Tri::None; //default

		if(t.a.j) {
			aji = trigon_index_map[t.a.j->t];
			ajl = t.a.j->t->label(t.a.j);

		}
		if(t.b.j) {
			bji = trigon_index_map[t.b.j->t];
			bjl = t.b.j->t->label(t.b.j);
		}
		if(t.c.j) {
			cji = trigon_index_map[t.c.j->t];
			cjl = t.c.j->t->label(t.c.j);
		}

        trigons.push_back(File::Tri(index, t.type, t.color.to32(),
                node_index_map[t.a.n], node_index_map[t.b.n], node_index_map[t.c.n],
                aji, bji, cji,
                ajl, bjl, cjl,
                t.a.w, t.b.w, t.c.w));
		++index;
	}

    header = File::Header(3, (uint32_t)nodes.size(), (uint32_t)trigons.size(), cmplx._canvas);

    ready_to_load = true;
}

Complex* State::load() {

    if(!ready_to_load) return 0;

    Complex* cmplx = new Complex();

    cmplx->_canvas = header.canvas;

	//associate each node's index with the node's memory address in the list
    std::vector<Node*> index_node_map(header.num_nodes);
    std::vector<Tri*> index_trigon_map(header.num_trigons);

	for(uint32_t i = 0; i < header.num_nodes; ++i) {
        File::Node const& fn = nodes[i];
        Node* n = new Node(Vec2i(fn.x, fn.y), (Node::Type)fn.type, Col(fn.color));
        n->pp = Vec2i(fn.px, fn.py);
        n->w_ = fn.w;
        n->contrast = fn.ctrst;
        cmplx->nodes.push_back(n);
        index_node_map[i] = n;
	}

	for(uint32_t i = 0; i < header.num_trigons; ++i) {
        File::Tri const& ft = trigons[i];
        Tri* t = new Tri(index_node_map[ft.node_a], index_node_map[ft.node_b], index_node_map[ft.node_c], (Tri::Type)ft.type, Col(ft.color));
        cmplx->trigons.push_back(t);
        index_trigon_map[i] = t;
	}

	//link trigons with its adjacent trigons
	for(uint32_t i = 0; i < header.num_trigons; ++i) {
		Tri& t = *index_trigon_map[i];
        File::Tri& ft = trigons[i];
        t.a.j = ft.aj_index == (uint32_t)-1? 0 : index_trigon_map[ft.aj_index]->edge((Tri::Label)ft.aj_label);
        t.b.j = ft.bj_index == (uint32_t)-1? 0 : index_trigon_map[ft.bj_index]->edge((Tri::Label)ft.bj_label);
        t.c.j = ft.cj_index == (uint32_t)-1? 0 : index_trigon_map[ft.cj_index]->edge((Tri::Label)ft.cj_label);

        t.a.w = ft.aw;
        t.b.w = ft.bw;
        t.c.w = ft.cw;
    }

	//link the nodes' entry edges
	for(uint32_t i = 0; i < header.num_nodes; ++i) {
		Node& n = *index_node_map[i];
        File::Node filenode = nodes[i];
		n.setentry(index_trigon_map[filenode.entry_ti]->edge((Tri::Label)filenode.entry_label));
	}

    return cmplx;
}

bool State::export_to(std::string const& filename) {

    if(!ready_to_load) return false;

    std::ofstream outfile(filename, std::ofstream::binary);
    if(!outfile.is_open()) {
        std::cout << "Could not open file \"" << filename << "\"" << std::endl;
		return false;
	}

    outfile.write((char*)&header, sizeof(header));
    outfile.write((char*)&nodes[0], sizeof(File::Node)*nodes.size());
    outfile.write((char*)&trigons[0], sizeof(File::Tri)*trigons.size());

    return true;
}

bool State::import_from(std::string const& filename) {

    std::ifstream infile(filename, std::ifstream::binary);
	infile.read((char*)&header, sizeof(header));
	if(!infile.is_open()) return false;

	nodes.resize(header.num_nodes);
	trigons.resize(header.num_trigons);

    //This data is read into a buffer as it will be used multiple times in different iterations.
    infile.read((char*)&nodes[0], sizeof(File::Node)*header.num_nodes);
    infile.read((char*)&trigons[0], sizeof(File::Tri)*header.num_trigons);
	
    return ready_to_load = true;
}
