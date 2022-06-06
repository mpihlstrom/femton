#pragma once

#include "GL/glew.h"
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>
#include <string>
#include <unordered_map>
#include "common/common.h"
#include "common/color.h"
#include "shader.h"
#include "complex/node.h"
#include "complex/trigon.h"
#include "common/entity.h"

#pragma pack (push, 1)

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

extern int tessellation_level;
extern float point_size;
extern Col bg;
extern bool draw_cursor;

namespace G {

    struct Col {
        Col() : r(0), g(0), b(0), a(0) {}
        Col(float r, float g, float b) : r(r), g(g), b(b), a(1.0) {}
        Col(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        Col(::Col const& c) : r((float)c.r), g((float)c.g), b((float)c.b), a((float)c.a) {}
        float r, g, b, a;
    };

    struct Vec2 {
        Vec2() {}
        //Vec2(Vec2i const& v) : x((float)v.x), y((float)v.y) {}
        Vec2(::Vec2 const& v) : x((float)v.x), y((float)v.y) {}
        Vec2(float const& x, float const& y) : x(x), y(y) {}
        float x, y;
    };

    struct Vec3 {
        Vec3() {}
        Vec3(Vec2i const& v) : x((float)v.x), y((float)v.y), z(0) {}
        Vec3(::Vec2 const& v) : x((float)v.x), y((float)v.y), z(0) {}
        Vec3(Vec3 const& v) : x((float)v.x), y((float)v.y), z((float)v.z) {}
        Vec3(float const& x, float const& y, float const& z) : x(x), y(y), z(z) {}
        float x, y, z;
    };

    struct Vec3w : Vec3 {
        Vec3w() {}
        Vec3w(Vec3 const& v, float w) : Vec3(v), w(w) {}
        float w;
    };

    struct Tri {
        Tri() {}
        Tri(Vec3 const& a, Vec3 const& b, Vec3 const& c) : a(a), b(b), c(c) {}
        Vec3 a, b, c;
    };

    struct Edge : Entity {
        Edge() {}
        Edge(unsigned int id, Vec3 const& p, Col const& col, uint32_t l, Tri const& t, Vec3 const& ew, Vec3 const& nw)
            : Entity(id), p(p), col(col), l(l), t(t), ew(ew), nw(nw) {}
        Vec3 p;
        Col col;
        uint32_t l;
        Tri t;
        Vec3 ew;
        Vec3 nw;
    };

    struct Node : Entity {
        Node(uint32_t id, Vec3 const& p, Col const& c1, float w, float ctrst) : Entity(id), p(p), c1(c1), w(w), ctrst(ctrst) {}
        Node(uint32_t id, Vec3 const& p, ::Node::Type t, float w, float ctrst) : Entity(id), p(p), w(w), ctrst(ctrst) {
            if(t == ::Node::Type::Corner) c1 = Col(0,0,0);
            else if(t == ::Node::Type::Border) c1 = Col(1,1,0);
            else c1 = Col(1,1,1);
        }
        Node(Vec3 const& p, Col const& c1, float w) : p(p), c1(c1),w(w) {}
        Vec3 p;
        Col c1;
        float w;
        float ctrst;
    };

    struct Tri_patch : Entity {
        Tri_patch(unsigned int id, Node const& v, uint32_t l, Col const& c, Vec2 const& g) : Entity(id), v(v), l(l), c(c), g(g) {}
        Node v;
        uint32_t l;
        Col c;
        Vec2 g;
    };
}

struct Base_render {
    enum FBO_buffer {
        Color = 1,
        Depth = 2,
        Id = 4,
        Coordinate = 8,
        Color2 = 16,
        Float = 32,
        None = 0
    };

    Base_render(QOpenGLBuffer* pbuffer, uint8_t fbo_buffers);
    Base_render(uint8_t fbo_buffers = (FBO_buffer::Color | FBO_buffer::Depth | FBO_buffer::Id | FBO_buffer::Coordinate));
    virtual ~Base_render();
    virtual void set_buffer() {}
    void reset_fbo(Size const& size);
    virtual void render();
    virtual void render_to_screen();
    GLuint fbo_color() const { return fbo->texture(); }
    GLuint fbo_depth() const { return fbo->depth_id; }
    GLuint fbo_id() const { return fbo->id_id; }
    GLuint fbo_coordinate() const { return fbo->coordinate_id; }
    GLuint fbo_color2() const { return fbo->color2_id; }
    GLuint fbo_float() const { return fbo->float_id; }
    uint8_t fbos() const { return fbo_buffers; }
    QImage color_to_image() { return fbo->toImage(true); }

    uint32_t* id_buffer();
    uint32_t* depth_buffer();
    uint32_t* coord_buffer();

    uint32_t* fbo_buffer(FBO_buffer buffer, bool redraw = false);
    uint32_t id_at(int x, int y);
    Vec2 coordinate_at(int x, int y);

    void render_to_buffer(QOpenGLBuffer* in_buf);

    std::string name;

    QOpenGLBuffer* pbuffer;
    bool active;

protected:
    virtual void draw() = 0;
    Framebufferobject* fbo;
    uint8_t fbo_buffers;
    Shader_program* program;
};

struct Render : Base_render {
    Render(uint8_t fbo_buffers = (FBO_buffer::Color | FBO_buffer::Depth | FBO_buffer::Id | FBO_buffer::Coordinate));
private:
    QOpenGLBuffer buffer;
};

struct Surface_render : Render {
    Surface_render(uint8_t fbo_buffers);
    Surface_render();
protected:
    virtual void draw();
private:
    void set_buffer();
};

struct Composition_render : Render {
    Composition_render();

    void set_buffer();
private:
    void draw();
};

struct Gaussian_render : Base_render {
    enum Direction {
        Horizontal = 0,
        Vertical = 1
    };

    Gaussian_render(QOpenGLBuffer* pbuffer, Base_render* in_fbo_renderer, Direction dir);
protected:
    void draw();
    Base_render* in_fbo_renderer;
    Direction dir;
};

struct Gaussian_h_render : Gaussian_render {
    Gaussian_h_render(QOpenGLBuffer* pbuffer, Base_render* in_fbo_renderer) : Gaussian_render(pbuffer, in_fbo_renderer, Horizontal) {}
};

struct Gaussian_v_render : Gaussian_render {
    Gaussian_v_render(QOpenGLBuffer* pbuffer, Base_render* in_fbo_renderer) : Gaussian_render(pbuffer, in_fbo_renderer, Vertical) {}
};

struct Painting_render : Base_render {
    Painting_render(QOpenGLBuffer* pbuffer);
protected:
    void draw();
};

struct Nodes_render : Base_render {
    Nodes_render(QOpenGLBuffer* pbuffer);
private:
    void draw();
};

struct Edges_render : Render {
    Edges_render();
private:
    void draw();
    void set_buffer();
};

struct Control_net_render : Base_render {
    Control_net_render(QOpenGLBuffer* pbuffer);
private:
    void draw();
};

struct Buffer_binder {
    Buffer_binder(QOpenGLBuffer& buf) : buf(buf) { buf.bind(); }
    ~Buffer_binder() { buf.release(); }
    QOpenGLBuffer& buf;
};

class Graphics : public QOpenGLWidget {
    Q_OBJECT
public:
    Graphics(QWidget *parent = 0);
    ~Graphics();

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;

    void render_active(std::string s, bool b) { renders[s]->active = b; update(); }
    void toggle_render(std::string s) { renders[s]->active = !renders[s]->active; update(); }
    bool is_render_active(std::string s) { return renders[s]->active; }

    QImage grab_render(Size size);
    QImage grab_render2(Size size);
    uint32_t surface_id_at(int x, int y) { return ((Surface_render*)renders["surface"])->id_at(x, y); }
    uint32_t composition_id_at(int x, int y) { return composition->id_at(x, y); }
    Vec2 surface_coordinate_at(int x, int y) { return ((Surface_render*)renders["surface"])->coordinate_at(x, y); }

public slots:
    void update() { QWidget::update(); }
    void cleanup();

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(Size const& size);
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;


private:
    void hover_mouse_event(QHoverEvent* me);
    void add_render(Base_render* render, char const* name);

public:
    void draw_scene();

    void composition_to_fbo();

    void set_selectables();
    std::map<std::string, Base_render*> renders;
    std::list<Base_render*> renders_order;
    Composition_render* composition;
    Size render_sz;
};

#pragma pack(pop)




