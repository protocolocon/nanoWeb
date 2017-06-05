/*  -*- mode: c++; coding: utf-8; c-file-style: "stroustrup"; -*-

    Contributors: Asier Aguirre

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "render.h"
#include <cassert>

#define DBG(x, ...)          //x, ##__VA_ARGS__

namespace render {

    void errorCallback(int error, const char* description) {
        LOG("error: glfw %d: %s", error, description);
    }

    DIAG(Render::~Render() {
            finish();
        });


    bool Render::init() {
        // init glfw
        glfwSetErrorCallback(errorCallback);
        if (!glfwInit()) {
            LOG("error: cannot init glfw");
            return false;
        }

        // create window
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        glfwWindowHint(GLFW_ALPHA_BITS, 0);   // remove alpha channel from window
        glfwWindowHint(GLFW_DEPTH_BITS, 0);   // depth buffer not required
        glfwWindowHint(GLFW_STENCIL_BITS, 0); // not using stencil either

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        int width(defaultWidth());
        int height(defaultHeight());
        if (!(win = glfwCreateWindow(width, height, "NanoWeb", nullptr, nullptr))) {
            LOG("error: cannot create glfw window");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(win);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // render first frame as soon as possible
        glClear(GL_COLOR_BUFFER_BIT);

        shaderLoad( // GL ES 2.0
            "uniform vec2 invViewSize2;"
            "attribute vec2 vertex;"
            "attribute vec2 texture;"
            "attribute vec4 color;"
            "varying vec2 vtexture;"
            "varying vec4 vcolor;"
            "void main(void) {"
            "  vtexture = texture;"
            "  vcolor = color;"
            "  gl_Position = vec4(vertex.x * invViewSize2.x - 1.0, 1.0 - vertex.y * invViewSize2.y, 0.0, 1.0);"
            "}",
            // -----------------------------------------------------
            "precision highp float;"
            "uniform sampler2D texSampler;"
            "varying vec2 vtexture;"
            "varying vec4 vcolor;"
            "void main(void) {"
            "  gl_FragColor = (texture2D(texSampler, vtexture) * vcolor) * 2.0;" // amplification capability
            "}");

        setWindowSize(width, height);

        // enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // debug GL status
        DBG(
            GLint caps[] = { GL_BLEND, GL_CULL_FACE, GL_DEPTH_TEST, GL_DITHER, GL_POLYGON_OFFSET_FILL, GL_SAMPLE_ALPHA_TO_COVERAGE,
                    GL_SAMPLE_COVERAGE, GL_SCISSOR_TEST, GL_STENCIL_TEST };
            for (auto cap: caps)
                LOG("Cap %x: %s", cap, glIsEnabled(cap) ? "on" : "off"));

        return checkError();
    }

    DIAG(void Render::finish() {
            glDeleteProgram(shaderProgram);
            glfwTerminate();
        });

    void Render::setWindowSize(int width, int height) {
        windowSize[0] = width;
        windowSize[1] = height;
        glViewport(0, 0, width, height);

        // use program and ser uniforms
        glUseProgram(shaderProgram);
        GLfloat invView[2] = { 2.f / float(width), 2.f / float(height) };
        glUniform2fv(glGetUniformLocation(shaderProgram, "invViewSize2"), 1, invView);
        glUniform1i(glGetUniformLocation(shaderProgram, "texSampler"), 0);
    }

    void Render::swapBuffers() {
        glfwSwapBuffers(win);
    }

    bool Render::checkError() {
        GLuint error(glGetError());
        if (error != GL_NO_ERROR) {
            LOG("GL error: %u", error);
            assert(false && "checkError sync on GL errors");
            return false;
        }
        return true;
    }

    bool Render::shaderLoad(const char* vertexShader, const char* fragmentShader) {
        GLint success;
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexShader, nullptr);
        glCompileShader(vs);
        glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(vs, sizeof(infoLog), nullptr, infoLog);
            LOG("error compiling VS: %s", infoLog);
            return false;
        }
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentShader, nullptr);
        glCompileShader(fs);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(fs, sizeof(infoLog), nullptr, infoLog);
            LOG("error compiling FS: %s", infoLog);
            return false;
        }

        glDeleteProgram(shaderProgram);
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, fs);
        glAttachShader(shaderProgram, vs);

        // no interface locations binding

        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
            LOG("error linking shader program: %s", infoLog);
            glDeleteShader(fs);
            glDeleteShader(vs);
            shaderProgram = 0;
            return false;
	}
        glValidateProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
            LOG("error invalid shader program: %s", infoLog);
            glDeleteShader(fs);
            glDeleteShader(vs);
            shaderProgram = 0;
            return false;
        }

        // dump attributes
        DBG(
            GLint maxLength, nAttribs;
            glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &nAttribs);
            glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
            GLchar* name((GLchar *)malloc(maxLength));
            GLint written, size, location;
            GLenum type;
            LOG("Index Name");
            for( int i = 0; i < nAttribs; i++ ) {
                glGetActiveAttrib(shaderProgram, i, maxLength, &written, &size, &type, name);
                location = glGetAttribLocation(shaderProgram, name);
                LOG("%5d %s", location, name);
            }
            free(name));

        // delete intermediate objects
        glDeleteShader(fs);
        glDeleteShader(vs);
        return checkError();
    }

}
