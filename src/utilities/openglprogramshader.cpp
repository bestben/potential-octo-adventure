#include "openglprogramshader.h"

#include "../utility.h"

#include <fstream>
#include <iostream>

#include "../glm/gtc/type_ptr.hpp"
#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"

#define INVALID_PROGRAM_SHADER_ID (unsigned int)0

OpenglProgramShader::OpenglProgramShader() {
    m_programId = glCreateProgram();
}

OpenglProgramShader::~OpenglProgramShader() {
    glDeleteProgram( m_programId );
}

bool OpenglProgramShader::isValid() const {
    return (m_programId != INVALID_PROGRAM_SHADER_ID);
}

bool OpenglProgramShader::addShaderFromSourceFile(OpenGLShaderType type, const std::string& fileName) {
    std::ifstream file( fileName.c_str() );

    MI_ASSERT( file.good() );

    file.seekg( 0, std::ios::end );
    std::streamoff fileSize = file.tellg();
    file.seekg( 0, std::ios::beg );
    std::string shaderContent;
    shaderContent.resize(fileSize);

    file.read( &shaderContent[0], fileSize );
    const char* source = shaderContent.c_str();
    addShaderFromSourceCode( type, source );

    return true;
}

bool OpenglProgramShader::addShaderFromSourceCode(OpenGLShaderType type, const char* source) {
    MI_ASSERT( source != nullptr );

    GLenum shaderType;
    switch(type) {
    case OpenGLShaderType::Fragment:
        shaderType = GL_FRAGMENT_SHADER;
        break;
    case OpenGLShaderType::Vertex:
        shaderType = GL_VERTEX_SHADER;
        break;
    case OpenGLShaderType::Geometry:
        shaderType = GL_GEOMETRY_SHADER;
        break;
     default:
        MI_ASSERT( false );
        return false;
    }

    GLuint shader = glCreateShader( shaderType );
    glShaderSource( shader, 1, &source, nullptr );
    glCompileShader( shader );
    glAttachShader( m_programId, shader );
    return true;
}

int OpenglProgramShader::attributeLocation(const char* name) const {
    MI_ASSERT( isValid() );
    return glGetAttribLocation( m_programId, name );
}

int OpenglProgramShader::attributeLocation(const std::string& name) const {
    MI_ASSERT( isValid() );
    return glGetAttribLocation( m_programId, name.c_str() );
}

int OpenglProgramShader::uniformLocation(const char* name) const {
    MI_ASSERT( isValid() );
    return glGetUniformLocation( m_programId, name );
}

int OpenglProgramShader::uniformLocation(const std::string& name) const {
    MI_ASSERT( isValid() );
    return glGetUniformLocation( m_programId, name.c_str() );
}

bool OpenglProgramShader::link() {
    MI_ASSERT( isValid() );
    glLinkProgram( m_programId );
    return true;
}

void OpenglProgramShader::bind() {
    MI_ASSERT( isValid() );
    glUseProgram( m_programId );
}

void OpenglProgramShader::release() {
    glUseProgram( 0 );
}

unsigned int OpenglProgramShader::getProgramId() const {
    return m_programId;
}

void OpenglProgramShader::enableAttributeArray(int attrib) {
    MI_ASSERT( isValid() );
    glEnableVertexAttribArray( attrib );
}

void OpenglProgramShader::setUniformValue(int location, float value) {
    MI_ASSERT( isValid() );
    glUniform1f( location, value );
}
void OpenglProgramShader::setUniformValue(int location, int value) {
    MI_ASSERT( isValid() );
    glUniform1i( location, value );
}
void OpenglProgramShader::setUniformValue(int location, unsigned int value) {
    MI_ASSERT( isValid() );
    glUniform1ui( location, value );
}
void OpenglProgramShader::setUniformValue(int location, const glm::vec2& value) {
    MI_ASSERT( isValid() );
    glUniform2fv( location, 1, glm::value_ptr(value) );
}
void OpenglProgramShader::setUniformValue(int location, const glm::vec3& value) {
    MI_ASSERT( isValid() );
    glUniform3fv( location, 1, glm::value_ptr(value) );
}
void OpenglProgramShader::setUniformValue(int location, const glm::vec4& value) {
    MI_ASSERT( isValid() );
    glUniform4fv( location, 1, glm::value_ptr(value) );
}
void OpenglProgramShader::setUniformValue(int location, const glm::mat4x4& value) {
    MI_ASSERT( isValid() );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr(value) );
}
void OpenglProgramShader::setUniformValue(const char* name, float value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, int value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, unsigned int value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, const glm::vec2& value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, const glm::vec3& value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, const glm::vec4& value) {
    setUniformValue( uniformLocation(name), value );
}
void OpenglProgramShader::setUniformValue(const char* name, const glm::mat4x4& value) {
    setUniformValue( uniformLocation(name), value );
}

