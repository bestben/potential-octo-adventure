#include "openglvertexarrayobject.h"

#include <glad/glad.h>
#include "../utility.h"


OpenGLVertexArrayObject::OpenGLVertexArrayObject( )
{
    m_VaoId = 0;
}

OpenGLVertexArrayObject::~OpenGLVertexArrayObject()
{
    if( m_VaoId != 0 )
        destroy();
}

void OpenGLVertexArrayObject::create() {
    MI_ASSERT( (m_VaoId == 0) );

    glGenVertexArrays(1, &m_VaoId);
}

void OpenGLVertexArrayObject::destroy() {
    MI_ASSERT( (m_VaoId != 0) );

    glDeleteVertexArrays(1, &m_VaoId);
    m_VaoId = 0;
}

bool OpenGLVertexArrayObject::isCreated() const {
    return m_VaoId != 0;
}

unsigned int OpenGLVertexArrayObject::objectId() const {
    return m_VaoId;
}

void OpenGLVertexArrayObject::bind() {
    MI_ASSERT( (m_VaoId !=0 ) );
    glBindVertexArray( m_VaoId );
}

void OpenGLVertexArrayObject::release() {
    MI_ASSERT( (m_VaoId !=0 ) );
    glBindVertexArray( 0 );
}
