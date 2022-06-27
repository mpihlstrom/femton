#pragma once
#include "GL/glew.h"
#include <string>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <iostream>
#include "common/common.h"

struct Tri_patch;

struct Framebufferobject : QOpenGLFramebufferObject {
    struct Binder {
        Binder(QOpenGLFramebufferObject* fbo) : fbo(fbo) { fbo->bind(); }
        ~Binder() { fbo->release(); }
        QOpenGLFramebufferObject* fbo;
    };

    Framebufferobject(Size const& sz) : QOpenGLFramebufferObject(QSize(sz.w, sz.h)) {}
    //Though the QT parent class supports attaching a depth buffer attachment, it does not provide a
    //way to get the depth buffer id and bind it to a texture. Hence these additional fields.
    GLuint depth_id,
           id_id,
           coordinate_id,
           color2_id,
           float_id;

    void attach_depth_buffer();
    void attach_id_buffer();
    void attach_coordinate_buffer();
    void attach_color2_buffer();
    void attach_float_buffer();
};

struct Shader_program : QOpenGLShaderProgram {
    struct Binder {
        Binder() {}
        Binder(Shader_program* p) : program(p) {
            program->bind();
        }
        ~Binder() {
            program->release();
        }

        Shader_program* program;
    };

    struct FBOBinder : Binder {
        FBOBinder(Shader_program* p, QOpenGLFramebufferObject* fbo) : framebuffer_object(fbo) {
            program = p;
            framebuffer_object->bind();
            program->bind();
        }
        ~FBOBinder() {
            program->release();
            framebuffer_object->release();
        }

        QOpenGLFramebufferObject* framebuffer_object;
    };

    Shader_program(std::string v_path, std::string tc_path, std::string te_path, std::string g_path, std::string f_path);

private:
    void add_shader(std::string path, QOpenGLShader::ShaderTypeBit type);
};

