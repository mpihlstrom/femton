#include <unordered_map>
#include <string>

#include <math.h>
#include <time.h>
#include <vector>

#include <QApplication>

#include "GL/glew.h"
#include <QOpenGLShaderProgram>
#include <QMouseEvent>

#include "json/json.h"

#include "graphics.h"
#include "graphics.h"
#include "common/common.h"
#include "complex/complex.h"
#include "camera.h"
#include "init.h"
#include "util.h"
#include "globals.h"
#include "shader.h"
#include "globals.h"

float line_width;
int tessellation_level;
float point_size;
Col bg;
bool draw_cursor = false;


Base_render::Base_render(uint8_t fbo_buffers) : active(true), fbo(0), fbo_buffers(fbo_buffers), program(0)  {
    reset_fbo(scr_sz);
}

Base_render::Base_render(QOpenGLBuffer* pbuffer, uint8_t fbo_buffers) : pbuffer(pbuffer), active(true), fbo(0), fbo_buffers(fbo_buffers), program(0)  {
    reset_fbo(scr_sz);
}

Base_render::~Base_render() {
    if(program) delete program;
    if(fbo) delete fbo;
}

void Base_render::reset_fbo(Size const& size)  {
    if(fbo)
        delete fbo;

    if(!(fbo_buffers & FBO_buffer::Color)) {
        fbo = 0;
        return;
    }
    fbo = new Framebufferobject(size);

    if(fbo_buffers & FBO_buffer::Depth)
        fbo->attach_depth_buffer();
    if(fbo_buffers & FBO_buffer::Id)
        fbo->attach_id_buffer();
    if(fbo_buffers & FBO_buffer::Coordinate)
        fbo->attach_coordinate_buffer();
    if(fbo_buffers & FBO_buffer::Color2)
        fbo->attach_color2_buffer();
}

Render::Render(uint8_t fbo_buffers) : Base_render(fbo_buffers) {
    pbuffer = &buffer;
    pbuffer->create();
    pbuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
}

void Base_render::render() {
    if(!active)
        return;
    Shader_program::FBOBinder sp(program, fbo);
    QOpenGLVertexArrayObject vao;
    QOpenGLVertexArrayObject::Binder binder(&vao);
    Buffer_binder bufbinder(*pbuffer);
    draw();
}

void Base_render::render_to_screen() {
    Shader_program::Binder sp(program);
    QOpenGLVertexArrayObject vao;
    QOpenGLVertexArrayObject::Binder binder(&vao);
    Buffer_binder bufbinder(*pbuffer);
    draw();
}

void Graphics::composition_to_fbo() {
    makeCurrent(); //Very important, apparently, as toImage() will transform the entire window view otherwise.
    composition->render();
}

QImage Graphics::grab_render2(Size size) {
    makeCurrent(); //Very important, apparently, as toImage() will transform the entire window view otherwise.
    glViewport(0, 0, size.w, size.h);
    draw_scene();
    composition->render();
    QImage img(composition->color_to_image());
    return img;
}

QImage Graphics::grab_render(Size size) {
    makeCurrent(); //Very important, apparently, as toImage() will transform the entire window view otherwise.
    Size prev_size = render_sz;
    resizeGL(size);
    glViewport(0, 0, size.w, size.h);
    draw_scene();
    composition->render();
    QImage img(composition->color_to_image());
    resizeGL(prev_size);
    return img;
}

Graphics::Graphics(QWidget *parent) : QOpenGLWidget(parent), render_sz(scr_sz) {
    /*core = */QCoreApplication::arguments().contains(QStringLiteral("--coreprofile"));
    bool transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }
}

void Graphics::add_render(Base_render* render, char const* name) {
    render->name = name;
    renders[name] = render;
    renders_order.push_back(render);
}

void Graphics::initializeGL() {
    //initializeOpenGLFunctions();
    glewInit();

    composition = new Composition_render();
    composition->set_buffer();

    add_render(new Surface_render(), "surface");
    add_render(new Edges_render(), "edges");
    add_render(new Control_net_render(renders["edges"]->pbuffer), "control_net");
    add_render(new Nodes_render(renders["surface"]->pbuffer), "nodes");
    add_render(new Gaussian_h_render(composition->pbuffer, renders["surface"]), "gaussian_h");
    add_render(new Gaussian_v_render(composition->pbuffer, renders["gaussian_h"]), "gaussian_v");
    //add_render(new Contour_h_render(composition->pbuffer, renders["surface"]), "contour_h");
    //add_render(new Contour_v_render(composition->pbuffer, renders["contour_h"]), "contour_v");
    add_render(new Painting_render(composition->pbuffer), "painting");

    bg = Col(0.5,0.5,0.5);//Col::random();

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);

    //resizeGL(scr_sz.w, scr_sz.h);
    //resizeGL(scr_sz);
}

void Graphics::set_selectables() {
    selectable.clear();
    selectable[void_selection.id] = &void_selection;

    for(auto t0 : com->ts) { Tri const& t = *t0;
    //for(auto i = com->trigons.begin(); i != com->trigons.end(); ++i) {  auto t = **i;
        selectable[t.id] = &t;
        selectable[t.na().id] = &t.na();
        selectable[t.nb().id] = &t.nb();
        selectable[t.nc().id] = &t.nc();
    }
}

uint32_t* Base_render::fbo_buffer(FBO_buffer buffer, bool redraw) {
    if(!(fbo_buffers & buffer))
        return nullptr;
    ::draw->makeCurrent();
    int w = fbo->width();
    int h = fbo->height();
    uint32_t* pixel = new uint32_t[w*h];
    Shader_program::FBOBinder sp(program, fbo);

    if(redraw) {
        Buffer_binder bufbinder(*pbuffer);
        QOpenGLVertexArrayObject vao;
        QOpenGLVertexArrayObject::Binder vaobinder(&vao);
        draw();
    }

    GLenum mode, format, type;
    switch(buffer) {
    case FBO_buffer::Color:
        mode = GL_COLOR_ATTACHMENT0;
        format = GL_BGRA;
        type = GL_UNSIGNED_BYTE;
        break;
    case FBO_buffer::Id:
        mode = GL_COLOR_ATTACHMENT1;
        format = GL_RED_INTEGER;
        type = GL_UNSIGNED_INT;
        break;
    case FBO_buffer::Depth:
        mode = GL_DEPTH_ATTACHMENT;
        format = GL_DEPTH_COMPONENT;
        type = GL_FLOAT;
        break;
    case FBO_buffer::Coordinate:
        mode = GL_COLOR_ATTACHMENT2;
        format = GL_RG;
        type = GL_UNSIGNED_SHORT;
        break;
    default:
        return nullptr;
    }

    glReadBuffer(mode);
    glReadPixels(0, 0, w, h, format, type, pixel);
    return pixel;
}

uint32_t Base_render::id_at(int x, int y) {
    uint32_t* pixel = fbo_buffer(FBO_buffer::Id, true);
    int w = scr_sz.w,
        h = scr_sz.h;
    uint32_t id = *(pixel + (h-y)*w + x);
    delete[] pixel;
    return id;
}

Vec2 Base_render::coordinate_at(int x, int y) {
    uint32_t* pixel = fbo_buffer(FBO_buffer::Coordinate, true);
    int w = scr_sz.w,
        h = scr_sz.h;
    uint32_t p = *(pixel + (h-y)*w + x);
    uint16_t* coord = (uint16_t*)&p;
    delete[] pixel;
    return Vec2((double)coord[0], (double)coord[1]) / 65535.0;
}

void Graphics::resizeGL(Size const& sz) {
    render_sz = sz;
    composition->reset_fbo(render_sz);
    for(auto i : renders)
        i.second->reset_fbo(render_sz);
}

QSize Graphics::minimumSizeHint() const {
    return QSize(init_scr_sz.w, init_scr_sz.h) / 8;
}

QSize Graphics::sizeHint() const {
    auto ret = QSize(init_scr_sz.w, init_scr_sz.h);
    init_scr_sz *= pixelratio;
    scr_sz *= pixelratio;
    return ret;
}

void Graphics::resizeGL(int w, int h) {
    auto sz = Size(w*pixelratio, h*pixelratio);
    init_scr_sz = scr_sz = sz;
    resizeGL(sz);
    //resizeGL(Size(scr_sz.w, scr_sz.w));
}

Graphics::~Graphics() {
    cleanup();
}

void Graphics::cleanup() {
    makeCurrent();
    for(auto i : renders)
        delete i.second;
    doneCurrent();
}
