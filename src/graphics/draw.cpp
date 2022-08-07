#include "graphics.h"
#include "tool/tools.h"
#include "globals.h"

//double scale = 10000;
float contrast_scale = 200;
float gaussian_size = 10;
float min_gaussian_q = 0.2f;

GLfloat quad_vertices[] = {
    -1.f, -1.f, 0.f, -1.f,  1.f, 0.f,  1.f, -1.f, 0.f,
     1.f,  1.f, 0.f,  1.f, -1.f, 0.f, -1.f,  1.f, 0.f
};

Vec3 bg_vel;

int render_counter = 0;
bool draw_contours = false;
bool draw_flat = false;

Composition_render::Composition_render() : Render(FBO_buffer::Color | FBO_buffer::Id) {
    program = new Shader_program("graphics/shaders/composition/vertex.glsl", "", "", "", "graphics/shaders/composition/fragment.glsl");
}

Gaussian_render::Gaussian_render(QOpenGLBuffer* pbuffer, Base_render* in_fbo_renderer, Direction dir)
    : Base_render(pbuffer, FBO_buffer::Color | FBO_buffer::Color2), in_fbo_renderer(in_fbo_renderer), dir(dir) {
    program = new Shader_program("graphics/shaders/gaussian/vertex.glsl", "", "", "", "graphics/shaders/gaussian/fragment.glsl");
}

Painting_render::Painting_render(QOpenGLBuffer* pbuffer) : Base_render(pbuffer, FBO_buffer::Color) {
    program = new Shader_program("graphics/shaders/painting/vertex.glsl", "", "", "", "graphics/shaders/painting/fragment.glsl");
}

Surface_render::Surface_render(uint8_t fbo_buffers) : Render(fbo_buffers) {}
Surface_render::Surface_render() : Render(FBO_buffer::Color | FBO_buffer::Depth | FBO_buffer::Id | FBO_buffer::Coordinate | FBO_buffer::Color2) {
    program = new Shader_program("graphics/shaders/surface/vertex.glsl","","","","graphics/shaders/surface/fragment.glsl");
}

Nodes_render::Nodes_render(QOpenGLBuffer* pbuffer) : Base_render(pbuffer, FBO_buffer::Color | FBO_buffer::Id  | FBO_buffer::Depth) {
    program = new Shader_program("graphics/shaders/points/vertex.glsl", "", "", "", "graphics/shaders/points/fragment.glsl");
}

Edges_render::Edges_render() : Render(FBO_buffer::Color | FBO_buffer::Depth | FBO_buffer::Id | FBO_buffer::Coordinate) {
    program = new Shader_program("graphics/shaders/edges/vertex.glsl","","","", "graphics/shaders/edges/fragment.glsl");
}

Control_net_render::Control_net_render(QOpenGLBuffer* pbuffer) : Base_render(pbuffer, FBO_buffer::Color | FBO_buffer::Depth) {
    program = new Shader_program("graphics/shaders/controlnet/vertex.glsl","","","", "graphics/shaders/controlnet/fragment.glsl");
}

Col node_color(Node* n) {
    if(n->type == Node::Padding) return Col::Black;
    Col nac;
    int count = 0;
    for(auto e : *n) {
        if(e->t->label(e) == Tri::A) nac += Col::Red;
        if(e->t->label(e) == Tri::B) nac += Col::Green;
        if(e->t->label(e) == Tri::C) nac += Col::Blue;
        ++count;
    }
    nac /= count;
    return nac;
}

void Surface_render::set_buffer() {
    std::vector<G::Tri_patch> patches;

    //for(auto t0 : com->ts) { Tri const& t = *t0;
    for(auto t0 = com->trigons.cbegin(); t0 != com->trigons.cend(); ++t0) { Tri const& t = **t0;

        double curt = interpolate_move? com->curt : manual_curt;
        auto tc = t.color;

        /*auto na = t.a.n->type == Node::Floating? Col::White : t.a.n->type == Node::Border? Col::Black : Col::Red; //t.a.n->entry()->t->color;
        auto nb = t.b.n->type == Node::Floating? Col::White : t.b.n->type == Node::Border? Col::Black : Col::Red; //t.b.n->entry()->t->color;
        auto nc = t.c.n->type == Node::Floating? Col::White : t.c.n->type == Node::Border? Col::Black : Col::Red; //t.c.n->entry()->t->color;*/

        auto na = t.a.n->entry()->t->color;
        auto nb = t.b.n->entry()->t->color;
        auto nc = t.c.n->entry()->t->color;

        na = node_color(t.a.n);
        nb = node_color(t.b.n);
        nc = node_color(t.c.n);

        auto cea = Col::Red; auto ceb = Col::Green; auto cec = Col::Blue;

        /*if(&t.a == flipped_e || &t.b == flipped_e || &t.c == flipped_e) tc = Col::Black;
        if(&t.a == flipped_j || &t.b == flipped_j || &t.c == flipped_j) tc = Col::White;
        if(t.a.n == flipped_en) na = Col::Black; if(t.a.n == flipped_jn) na = Col::White;
        if(t.b.n == flipped_en) nb = Col::Black; if(t.b.n == flipped_jn) nb = Col::White;
        if(t.c.n == flipped_en) nc = Col::Black; if(t.c.n == flipped_jn) nc = Col::White;*/
        //if(t.area() > 0) na.a = nb.a = nc.a = 0;

        int t_id =  t.id;//t.type_() != Tri::Regular ? 0 : t.id;
        patches.push_back(G::Tri_patch(t_id, G::Node(t.na().id, G::Vec3(t.na().ppcp(curt)), na, t.na().w(), t.na().c()), 0, tc, G::Vec2(t.na().g())));
        patches.push_back(G::Tri_patch(t_id, G::Node(t.nb().id, G::Vec3(t.nb().ppcp(curt)), nb, t.nb().w(), t.nb().c()), 1, tc, G::Vec2(t.nb().g())));
        patches.push_back(G::Tri_patch(t_id, G::Node(t.nc().id, G::Vec3(t.nc().ppcp(curt)), nc, t.nc().w(), t.nc().c()), 2, tc, G::Vec2(t.nc().g())));
        //patches.push_back(G::Tri_patch(t.id, G::Node(t.na().id, G::Vec3(t.na().pp(curt)), t.na().type_(), t.na().w(), t.na().c()), 0, tc));
        //patches.push_back(G::Tri_patch(t.id, G::Node(t.nb().id, G::Vec3(t.nb().pp(curt)), t.nb().type_(), t.nb().w(), t.nb().c()), 1, tc));
        //patches.push_back(G::Tri_patch(t.id, G::Node(t.nc().id, G::Vec3(t.nc().pp(curt)), t.nc().type_(), t.nc().w(), t.nc().c()), 2, tc));
    }

    Buffer_binder bufbinder(*pbuffer);
    pbuffer->allocate(patches.data(), (int)patches.size()*sizeof(G::Tri_patch));
}

void Edges_render::set_buffer() {
    std::vector<G::Edge> edges;
    //for(auto t0 : com->ts) { Tri const& t = *t0;
    for(auto t0 = com->trigons.begin(); t0 != com->trigons.end(); ++t0) { Tri const& t = **t0;

        double curt = interpolate_move? com->curt : manual_curt;
        G::Tri tri = G::Tri(G::Vec3(t.na().ppcp(curt)), G::Vec3(t.nb().ppcp(curt)), G::Vec3(t.nc().ppcp(curt)));

        //auto cea = t.na().entry()->t->color; auto ceb = t.nb().entry()->t->color; auto cec = t.nc().entry()->t->color;
        //auto cea = t.color; auto ceb = t.color; auto cec = t.color;
        auto cea = Col::Red; auto ceb = Col::Green; auto cec = Col::Blue;
        //auto cea = Col::Gray; auto ceb = Col::Gray; auto cec = Col::Gray;

        edges.push_back(G::Edge(t.id, G::Vec3(t.na().ppcp(curt)), cea, 0, tri, G::Vec3(t.ea().w, t.eb().w, t.ec().w), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
        edges.push_back(G::Edge(t.id, G::Vec3(t.nb().ppcp(curt)), ceb, 1, tri, G::Vec3(t.ea().w, t.eb().w, t.ec().w), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
        edges.push_back(G::Edge(t.id, G::Vec3(t.nc().ppcp(curt)), cec, 2, tri, G::Vec3(t.ea().w, t.eb().w, t.ec().w), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
        //edges.push_back(G::Edge(t.id, t.na().pp(curt), cea, 0, tri, G::Vec3(ow,ow,ow), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
        //edges.push_back(G::Edge(t.id, t.nb().pp(curt), ceb, 1, tri, G::Vec3(ow,ow,ow), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
        //edges.push_back(G::Edge(t.id, t.nc().pp(curt), cec, 2, tri, G::Vec3(ow,ow,ow), G::Vec3(t.na().w(),t.nb().w(), t.nc().w())));
    }

    Buffer_binder bufbinder(*pbuffer);
    pbuffer->allocate(edges.data(), (int)edges.size()*sizeof(G::Edge));
}

void Composition_render::set_buffer() {
    Buffer_binder bufbinder(*pbuffer);
    pbuffer->allocate(quad_vertices, (int)sizeof(quad_vertices));
}

void model_view_projection(Shader_program* program) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    QMatrix4x4 modelview;
    QMatrix4x4 projection;
    projection.setToIdentity();
    projection.ortho(-com->canvas().x, com->canvas().x, -com->canvas().y, com->canvas().y, -com->canvas().x*10, com->canvas().x*10);
    //projection.perspective(0.0, 1.0, 50, -50);
    modelview.setToIdentity();
    //modelview.rotate(0, 1, 0, 0);
    //modelview.rotate(0, 0, 1, 0);
    modelview.rotate(view->rotz(), 0, 0, 1);
    modelview.translate(view->pos.x, view->pos.y);
    modelview.translate(0, 0, -.5);
    modelview.data()[15] = 1.0/view->zoom();
    program->setUniformValue(program->uniformLocation("modelview"), modelview);
    program->setUniformValue(program->uniformLocation("projection"), projection);

}


void globals(Shader_program* program) {
    program->setUniformValue(program->uniformLocation("W"), (int)((double)draw->render_sz.w));
    program->setUniformValue(program->uniformLocation("H"), (int)((double)draw->render_sz.h));
    program->setUniformValue(program->uniformLocation("screen_scale_x"), ((float)::draw->render_sz.w / (float)init_scr_sz.w));
    program->setUniformValue(program->uniformLocation("screen_scale_y"), ((float)::draw->render_sz.h / (float)init_scr_sz.h));
    glUniform1f(program->uniformLocation("zoom"), (float)view->zoom());
}

void Surface_render::draw() {
    model_view_projection(program);
    program->setUniformValue(program->uniformLocation("line_width"), line_width);
    program->setUniformValue(program->uniformLocation("tessellation_level"), tessellation_level);
    program->setUniformValue(program->uniformLocation("selected"), selecttool.id());

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffers);

    uint64_t offset = 0;
    int stride = sizeof(G::Tri_patch);

    program->enableAttributeArray("id");
    GLuint attribute_loc = glGetAttribLocation(program->programId(), "id");
    glVertexAttribIPointer(attribute_loc, 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(Entity) * 2;
    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("nc");
    program->setAttributeBuffer("nc", GL_FLOAT, offset, 4, stride);
    offset += sizeof(G::Col);
    program->enableAttributeArray("weight");
    program->setAttributeBuffer("weight", GL_FLOAT, offset, 1, stride);
    offset += sizeof(float);
    program->enableAttributeArray("contrast");
    program->setAttributeBuffer("contrast", GL_FLOAT, offset, 1, stride);
    offset += sizeof(float);
    program->enableAttributeArray("l");
    glVertexAttribIPointer(glGetAttribLocation(program->programId(), "l"), 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(uint32_t);
    program->enableAttributeArray("tc");
    program->setAttributeBuffer("tc", GL_FLOAT, offset, 4, stride);
    offset += sizeof(G::Col);
    program->enableAttributeArray("g");
    program->setAttributeBuffer("g", GL_FLOAT, offset, 2, stride);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(pbuffer->size())/stride);
}

void Control_net_render::draw() {
    model_view_projection(program);
    globals(program);
    program->setUniformValue(program->uniformLocation("line_width"), line_width * ((float)::draw->render_sz.w / (float)init_scr_sz.w));

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, buffers);

    int stride = sizeof(G::Edge);
    uint64_t offset = 0;

    offset += sizeof(Entity);
    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("color");
    program->setAttributeBuffer("color", GL_FLOAT, offset, 4, stride);
    offset += sizeof(G::Col);
    program->enableAttributeArray("l");
    glVertexAttribIPointer(glGetAttribLocation(program->programId(), "l"), 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(uint32_t);
    program->enableAttributeArray("v0");
    program->setAttributeBuffer("v0", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("v1");
    program->setAttributeBuffer("v1", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("v2");
    program->setAttributeBuffer("v2", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(pbuffer->size())/stride);
}

void Edges_render::draw() {
    model_view_projection(program);
    globals(program);
    program->setUniformValue(program->uniformLocation("line_width"), line_width * ((float)::draw->render_sz.w / (float)init_scr_sz.w));

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffers);

    int stride = sizeof(G::Edge);

    uint64_t offset = 0;
    program->enableAttributeArray("id");
    glVertexAttribIPointer(glGetAttribLocation(program->programId(), "id"), 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(Entity);
    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("color");
    program->setAttributeBuffer("color", GL_FLOAT, offset, 4, stride);
    offset += sizeof(G::Col);
    program->enableAttributeArray("l");
    glVertexAttribIPointer(glGetAttribLocation(program->programId(), "l"), 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(uint32_t);
    program->enableAttributeArray("v0");
    program->setAttributeBuffer("v0", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("v1");
    program->setAttributeBuffer("v1", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("v2");
    program->setAttributeBuffer("v2", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("ew");
    program->setAttributeBuffer("ew", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("nw");
    program->setAttributeBuffer("nw", GL_FLOAT, offset, 3, stride);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(pbuffer->size())/stride);
}

void Nodes_render::draw() {
    model_view_projection(program);
    program->setUniformValue(program->uniformLocation("point_size"), point_size * ((float)::draw->render_sz.w / (float)init_scr_sz.w));

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    int stride = sizeof(G::Tri_patch);
    uint64_t offset = sizeof(Entity);

    program->enableAttributeArray("id");
    GLuint attribute_loc = glGetAttribLocation(program->programId(), "id");
    glVertexAttribIPointer(attribute_loc, 1, GL_UNSIGNED_INT, stride, (void*)offset);
    offset += sizeof(Entity);
    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, offset, 3, stride);
    offset += sizeof(G::Vec3);
    program->enableAttributeArray("nc");
    program->setAttributeBuffer("nc", GL_FLOAT, offset, 4, stride);
    offset += sizeof(G::Col);
    program->enableAttributeArray("weight");
    program->setAttributeBuffer("weight", GL_FLOAT, offset, 1, stride);
    offset += sizeof(float);
    program->enableAttributeArray("contrast");
    program->setAttributeBuffer("contrast", GL_FLOAT, offset, 1, stride);
    offset += sizeof(float);
    offset += sizeof(uint32_t);
    offset += sizeof(G::Col);
    program->enableAttributeArray("g");
    program->setAttributeBuffer("g", GL_FLOAT, offset, 2, stride);

    glDrawArrays(GL_POINTS, 0, static_cast<unsigned int>(pbuffer->size())/stride);
}

void Gaussian_render::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3}; //<-- look over this

    glDrawBuffers(4, buffers);

    globals(program);

    int i = 0;
    glUniform1i(program->uniformLocation("in_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, in_fbo_renderer->fbo_color());

    glUniform1i(program->uniformLocation("in_color2_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, in_fbo_renderer->fbo_color2());

    glUniform1i(program->uniformLocation("surface_color2_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["surface"]->fbo_color2());

    glUniform1i(program->uniformLocation("surface_id_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["surface"]->fbo_id());

    program->setUniformValue(program->uniformLocation("W"), (int)::draw->render_sz.w);
    program->setUniformValue(program->uniformLocation("H"), (int)::draw->render_sz.h);

    program->setUniformValue(program->uniformLocation("dir"), dir);

    glUniform1f(program->uniformLocation("gaussian_size"), (float)gaussian_size);
    glUniform1f(program->uniformLocation("min_gaussian_q"), (float)min_gaussian_q);

    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Painting_render::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, buffers);

    int i = 0;
    glUniform1i(program->uniformLocation("surface_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["surface"]->fbo_color());

    glUniform1i(program->uniformLocation("surface_id_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["surface"]->fbo_id());

    glUniform1i(program->uniformLocation("surface_color2_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["surface"]->fbo_color2());

    glUniform1i(program->uniformLocation("gaussian_v_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["gaussian_v"]->fbo_color());

    glUniform1i(program->uniformLocation("gaussian_v_color2_buffer"), i);
    glActiveTexture(GL_TEXTURE0 + i++);
    glBindTexture(GL_TEXTURE_2D, ::draw->renders["gaussian_v"]->fbo_color2());

    globals(program);
    glUniform1i(program->uniformLocation("draw_gaussian_v"), (int)::draw->renders["gaussian_v"]->active);
    glUniform1f(program->uniformLocation("gaussian_size"), (float)gaussian_size);
    glUniform1f(program->uniformLocation("min_gaussian_q"), (float)min_gaussian_q);
    glUniform1f(program->uniformLocation("contrast_scale"), (float)contrast_scale);

    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}



void Composition_render::draw() {
    /*
    const double Acceleration_factor = 0.00001;
    const double Bounce_velocity_loss = 0.95;
    bg_vel += (Vec3(normrand, normrand, normrand) - 0.5) * 2.0 * Acceleration_factor;
    bg += bg_vel;
    if(bg.r < 0 || bg.r > 1) bg_vel.r = -bg_vel.r*Bounce_velocity_loss;
    if(bg.g < 0 || bg.g > 1) bg_vel.g = -bg_vel.g*Bounce_velocity_loss;
    if(bg.b < 0 || bg.b > 1) bg_vel.b = -bg_vel.b*Bounce_velocity_loss;
    bg.clamp();
    */

    glClearColor((float)bg.r, (float)bg.g, (float)bg.b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);

    int i = 0;
    for(auto r : ::draw->renders_order) {
        std::string name(r->name + "_buffer");

        glUniform1i(program->uniformLocation(name.c_str()), i);
        glActiveTexture(GL_TEXTURE0 + i++);
        glBindTexture(GL_TEXTURE_2D, r->fbo_color());

        if(r->fbos() & Render::FBO_buffer::Depth) {
            std::string depth_name(r->name + "_depth_buffer");
            glUniform1i(program->uniformLocation(depth_name.c_str()), i);
            glActiveTexture(GL_TEXTURE0 + i++);
            glBindTexture(GL_TEXTURE_2D, r->fbo_depth());
        }

        if(r->fbos() & Render::FBO_buffer::Id) {
            std::string id_name(r->name + "_id_buffer");
            glUniform1i(program->uniformLocation(id_name.c_str()), i);
            glActiveTexture(GL_TEXTURE0 + i++);
            glBindTexture(GL_TEXTURE_2D, r->fbo_id());
        }

        if(r->fbos() & Render::FBO_buffer::Coordinate) {
            std::string coordinate_name(r->name + "_coordinate_buffer");
            glUniform1i(program->uniformLocation(coordinate_name.c_str()), i);
            glActiveTexture(GL_TEXTURE0 + i++);
            glBindTexture(GL_TEXTURE_2D, r->fbo_coordinate());
        }

        if(r->fbos() & Render::FBO_buffer::Color2) {
            std::string color2_name(r->name + "_color2_buffer");
            glUniform1i(program->uniformLocation(color2_name.c_str()), i);
            glActiveTexture(GL_TEXTURE0 + i++);
            glBindTexture(GL_TEXTURE_2D, r->fbo_color2());
        }

        std::string draw_name(std::string("draw_") + r->name);
        glUniform1i(program->uniformLocation(draw_name.c_str()), (int)r->active);
    }

    glUniform1i(program->uniformLocation("render_counter"), ++render_counter);
    glUniform1f(program->uniformLocation("contrast_scale"), (float)contrast_scale);
    glUniform3f(program->uniformLocation("bg_color"), (float)bg.r, (float)bg.g, (float)bg.b);
    glUniform2f(program->uniformLocation("cursor"), (float)view->scr_uni_p(cursor).x, (float)view->scr_uni_p(cursor).y);
    glUniform1i(program->uniformLocation("draw_cursor"), (int)draw_cursor);
    glUniform1i(program->uniformLocation("draw_contours"), (int)draw_contours);
    glUniform1i(program->uniformLocation("draw_flat"), (int)draw_flat);

    globals(program);

    glUniform1f(program->uniformLocation("line_width"), line_width*(float)view->zoom()*((float)::draw->render_sz.w  / (float)init_scr_sz.w));
    glUniform1f(program->uniformLocation("gaussian_scale"), gaussian_size);

    glUniform1f(program->uniformLocation("cursor_radius"), (float)mousetool->radius);
    int mousetool_index = mousetool == &cameratool? 1 : 0;
    program->setUniformValue(program->uniformLocation("mousetool"), (int)mousetool_index);
    program->setUniformValue(program->uniformLocation("mouse_left"), (int)mousetool->left);
    program->setUniformValue(program->uniformLocation("mouse_right"), (int)mousetool->right);

    program->enableAttributeArray("position");
    program->setAttributeBuffer("position", GL_FLOAT, 0, 3);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Graphics::draw_scene() {
    com->waste();
    set_selectables();
    if(set_buffers)
        for(auto r : renders_order)
            r->set_buffer();
    for(auto r : renders_order)
        r->render();
}

void Graphics::paintGL() {
    draw_scene();
    composition->render_to_screen();
}

