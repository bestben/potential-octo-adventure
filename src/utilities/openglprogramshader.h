#pragma once

#include <string>

#include "glm/fwd.hpp"

#include <glad/glad.h>

enum class OpenGLShaderType {
    Vertex,
    Fragment,
    Geometry
};

class OpenglProgramShader
{
public:
    OpenglProgramShader();
    ~OpenglProgramShader();

    OpenglProgramShader( const OpenglProgramShader& copy ) = delete;
    OpenglProgramShader& operator=( const OpenglProgramShader& copy ) = delete;

    bool        isValid() const;

    bool        addShaderFromSourceFile(OpenGLShaderType type, const std::string& fileName);
    bool        addShaderFromSourceCode(OpenGLShaderType type, const char* source);

    int         attributeLocation(const char* name) const;
    int         attributeLocation(const std::string& name) const;

    int         uniformLocation(const char* name) const;
    int         uniformLocation(const std::string& name) const;

    bool        link();
    void        bind();
    void        release();

    unsigned int getProgramId() const;

    void    enableAttributeArray(int attrib);

    void    setUniformValue(int location, float value);
    void 	setUniformValue(int location, int value);
    void 	setUniformValue(int location, unsigned int value);
    void 	setUniformValue(int location, const glm::vec2& value);
    void 	setUniformValue(int location, const glm::vec3& value);
    void 	setUniformValue(int location, const glm::vec4& value);
    void 	setUniformValue(int location, const glm::mat4x4& value);

    void 	setUniformValue(const char* name, float value);
    void 	setUniformValue(const char* name, int value);
    void 	setUniformValue(const char* name, unsigned int value);
    void 	setUniformValue(const char* name, const glm::vec2& value);
    void 	setUniformValue(const char* name, const glm::vec3& value);
    void 	setUniformValue(const char* name, const glm::vec4& value);
    void 	setUniformValue(const char* name, const glm::mat4x4& value);

private:
    unsigned int    m_programId;
};
