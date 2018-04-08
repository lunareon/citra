// Copyright 2018 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <glad/glad.h>
#include "video_core/renderer_opengl/gl_resource_manager.h"
#include "video_core/renderer_opengl/gl_shader_gen.h"
#include "video_core/renderer_opengl/pica_to_gl.h"

enum class UniformBindings : GLuint { Common, VS, GS };

struct LightSrc {
    alignas(16) GLvec3 specular_0;
    alignas(16) GLvec3 specular_1;
    alignas(16) GLvec3 diffuse;
    alignas(16) GLvec3 ambient;
    alignas(16) GLvec3 position;
    alignas(16) GLvec3 spot_direction; // negated
    GLfloat dist_atten_bias;
    GLfloat dist_atten_scale;
};

/// Uniform structure for the Uniform Buffer Object, all vectors must be 16-byte aligned
// NOTE: Always keep a vec4 at the end. The GL spec is not clear wether the alignment at
//       the end of a uniform block is included in UNIFORM_BLOCK_DATA_SIZE or not.
//       Not following that rule will cause problems on some AMD drivers.
struct UniformData {
    GLint framebuffer_scale;
    GLint alphatest_ref;
    GLfloat depth_scale;
    GLfloat depth_offset;
    GLint scissor_x1;
    GLint scissor_y1;
    GLint scissor_x2;
    GLint scissor_y2;
    alignas(16) GLvec3 fog_color;
    alignas(8) GLvec2 proctex_noise_f;
    alignas(8) GLvec2 proctex_noise_a;
    alignas(8) GLvec2 proctex_noise_p;
    alignas(16) GLvec3 lighting_global_ambient;
    LightSrc light_src[8];
    alignas(16) GLvec4 const_color[6]; // A vec4 color for each of the six tev stages
    alignas(16) GLvec4 tev_combiner_buffer_color;
    alignas(16) GLvec4 clip_coef;
};

static_assert(
    sizeof(UniformData) == 0x460,
    "The size of the UniformData structure has changed, update the structure in the shader");
static_assert(sizeof(UniformData) < 16384,
              "UniformData structure must be less than 16kb as per the OpenGL spec");

struct PicaUniformsData {
    void SetFromRegs(const Pica::ShaderRegs& regs, const Pica::Shader::ShaderSetup& setup);

    struct {
        alignas(16) GLuint b;
    } bools[16];
    alignas(16) std::array<GLuvec4, 4> i;
    alignas(16) std::array<GLvec4, 96> f;
};

struct VSUniformData {
    PicaUniformsData uniforms;
};
static_assert(
    sizeof(VSUniformData) == 1856,
    "The size of the VSUniformData structure has changed, update the structure in the shader");
static_assert(sizeof(VSUniformData) < 16384,
              "VSUniformData structure must be less than 16kb as per the OpenGL spec");

struct GSUniformData {
    PicaUniformsData uniforms;
};
static_assert(
    sizeof(GSUniformData) == 1856,
    "The size of the GSUniformData structure has changed, update the structure in the shader");
static_assert(sizeof(GSUniformData) < 16384,
              "GSUniformData structure must be less than 16kb as per the OpenGL spec");

class ShaderProgramManager {
public:
    ShaderProgramManager(bool separable);
    ~ShaderProgramManager();

    void UseProgrammableVertexShader(const GLShader::PicaVSConfig& config,
                                     const Pica::Shader::ShaderSetup setup);

    void UseTrivialVertexShader();

    void UseProgrammableGeometryShader(const GLShader::PicaGSConfig& config,
                                       const Pica::Shader::ShaderSetup setup);

    void UseFixedGeometryShader(const GLShader::PicaGSConfigCommon& config);

    void UseTrivialGeometryShader();

    void UseFragmentShader(const GLShader::PicaShaderConfig& config);

    void ApplyTo(OpenGLState& state);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};