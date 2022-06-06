#include "shader.h"
#include "globals.h"

void Framebufferobject::attach_depth_buffer() {

    glGenTextures(1, &depth_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size().width(), size().height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    //functions->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,d GL_CLAMP);
    //functions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_id, 0);
    release();
    glBindTexture(GL_TEXTURE_2D, 0); //reset to default texture
}

void Framebufferobject::attach_id_buffer() {

    glGenTextures(1, &id_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, size().width(), size().height(), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, id_id, 0);
    release();
    glBindTexture(GL_TEXTURE_2D, 0); //reset to default texture
}

void Framebufferobject::attach_coordinate_buffer() {

    glGenTextures(1, &coordinate_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, coordinate_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size().width(), size().height(), 0, GL_RG, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, coordinate_id, 0);
    release();
    glBindTexture(GL_TEXTURE_2D, 0); //reset to default texture
}

void Framebufferobject::attach_color2_buffer() {
    glGenTextures(1, &color2_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color2_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size().width(), size().height(), 0, GL_RGBA, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color2_id, 0);
    release();
    glBindTexture(GL_TEXTURE_2D, 0); //reset to default texture
}

void Framebufferobject::attach_float_buffer() {
    glGenTextures(1, &float_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, float_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size().width(), size().height(), 0, GL_RED, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, float_id, 0);
    release();
    glBindTexture(GL_TEXTURE_2D, 0); //reset to default texture
}

Shader_program::Shader_program(std::string v_path, std::string c_path, std::string e_path, std::string g_path, std::string f_path) {

    if(!v_path.empty()) add_shader(v_path, QOpenGLShader::Vertex);
    if(!c_path.empty()) add_shader(c_path, QOpenGLShader::TessellationControl);
    if(!e_path.empty()) add_shader(e_path, QOpenGLShader::TessellationEvaluation);
    if(!g_path.empty()) add_shader(g_path, QOpenGLShader::Geometry);
    if(!f_path.empty()) add_shader(f_path, QOpenGLShader::Fragment);

    if(!this->link()) qCritical() << QObject::tr( "Could not link shader program. Log:" ) << this->log();
}

void Shader_program::add_shader(std::string path, QOpenGLShader::ShaderTypeBit type) {
    if(!this->addShaderFromSourceFile(type, path.c_str())) qCritical() << QObject::tr("Could not compile shader. Log:") << this->log();
}



