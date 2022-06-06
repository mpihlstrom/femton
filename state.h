#pragma once
#include "common/color.h"
#include "complex/complex.h"
#include "camera.h"
#include <fstream>

#pragma pack(push, 1)

namespace File {

    struct Header {
        Header() : version(3), num_nodes(0), num_trigons(0) {}
        Header(uint32_t version, uint32_t num_nodes, uint32_t num_trigons, Vec2i const& canvas) :
            version(version), num_nodes(num_nodes), num_trigons(num_trigons), canvas(canvas){}

        uint32_t version;
        uint32_t num_nodes;
        uint32_t num_trigons;
        Vec2i canvas;
    };

    struct Node {
        Node() : x(0), y(0), type(0), entry_ti(0), entry_label(0), color(0) {}
        Node(int64_t x, int64_t y, int64_t px, int64_t py, uint8_t t, uint32_t eti, uint8_t el, uint32_t c, double w, double ctrst) :
            x(x), y(y), px(px), py(py), type(t), entry_ti(eti), entry_label(el), color(c), w(w), ctrst(ctrst) {}

        int64_t x, y;
        int64_t px, py;
        double z;
        uint8_t type;
        uint32_t entry_ti; //entry trigon index
        uint8_t entry_label; //entry label
        uint32_t color;
        Vec2 n;
        double w;
        double ctrst;
    };

    struct Tri {
        Tri() : index(0), type(0), color(0), node_a(0), node_b(0), node_c(0), aj_index(0), bj_index(0), cj_index(0), aj_label(0), bj_label(0), cj_label(0) {}
        Tri(uint32_t i, uint8_t t, uint32_t color, uint32_t na, uint32_t bn, uint32_t nc, uint32_t aj, uint32_t bj, uint32_t cj, uint8_t al, uint8_t bl, uint8_t cl,
            double aw, double bw, double cw) :
            index(i), type(t), color(color), node_a(na), node_b(bn), node_c(nc), aj_index(aj), bj_index(bj), cj_index(cj), aj_label(al), bj_label(bl), cj_label(cl),
            aw(aw), bw(bw), cw(cw) {}

        uint32_t index;
        uint8_t type;
        uint32_t color;
        uint32_t node_a, node_b, node_c;
        uint32_t aj_index, bj_index, cj_index; //The index of the adjacent trigon. -1 means no adjacent trigon.
        uint8_t  aj_label, bj_label, cj_label; //The label of the adjacent trigon edge
        double aw, bw, cw;
    };
}

#pragma pack(pop)

struct State {
    //somewhat unintuitively we have here that:
    //export == load (to file)
    //import == save (from file)

    State() : ready_to_load(false) {}
    State(std::string const& filename);
    State(Complex const& cmplx);
    Complex* load();

    bool export_to(std::string const& filename);
    static bool export_to(Complex const& cmplx, std::string file);
    static Complex* load_from(std::string file);

private:
    bool import_from(std::string const& filename);
    void save(Complex const& cmplx);

    File::Header header;
    std::vector<File::Node> nodes;
    std::vector<File::Tri> trigons;
    bool ready_to_load;
};
