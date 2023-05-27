HEADERS = \
    camera.h \
    graphics/shader.h \
    init.h \
    state.h \
    complex/flip.h \
    complex/complex.h \
    complex/edge.h \
    complex/node.h \
    complex/trigon.h \
    common/common.h \
    graphics/graphics.h \
    globals.h \
    window.h \
    graphics/shader.h \
    common/color.h \
    camera.h \
    GL/eglew.h \
    GL/glew.h \
    GL/glxew.h \
    GL/wglew.h \
    common/entity.h \
    tool/tools.h \
    common/vector.h \
    common/vector3.h \

SOURCES = \
    camera.cpp \
    common/common.cpp \
    complex/canvas.cpp \
    complex/equations.cpp \
    complex/flip.cpp \
    complex/moveconcur.cpp \
    complex/moveconsec.cpp \
    graphics/shader.cpp \
    main.cpp \
    init.cpp \
    jsoncpp.cpp \
    state.cpp \
    util.cpp \
    complex/complex.cpp \
    common/color.cpp \
    complex/trigon.cpp \
    complex/edge.cpp \
    complex/node.cpp \
    complex/config.cpp \
    window.cpp \
    globals.cpp \
    glew.c \
    tool/tools.cpp \
    graphics/graphics.cpp \
    graphics/draw.cpp \
    input.cpp

QT += widgets
QT += opengl
QT += openglwidgets

DISTFILES += \
    graphics/shaders/composition/fragment.glsl \
    graphics/shaders/composition/vertex.glsl \
    graphics/shaders/surface/fragment.glsl \
    graphics/shaders/surface/vertex.glsl \
    graphics/shaders/points/fragment.glsl \
    graphics/shaders/points/vertex.glsl \
    graphics/shaders/controlnet/fragment.glsl \
    graphics/shaders/controlnet/vertex.glsl \
    graphics/shaders/edges/fragment.glsl \
    graphics/shaders/edges/vertex.glsl \
    graphics/shaders/gaussian/fragment.glsl \
    graphics/shaders/gaussian/vertex.glsl \
    graphics/shaders/painting/fragment.glsl \
    graphics/shaders/painting/vertex.glsl

win32 {
    LIBS += libopengl32
    LIBS += mpir.lib
}

win32:LIBS += -luser32
msvc: LIBS += -luser32

<code>QMAKE_CXXFLAGS += -Wall</code>

