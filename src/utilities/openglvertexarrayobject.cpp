#include "openglvertexarrayobject.h"

#include "../gamewindow.h"

OpenGLVertexArrayObject::OpenGLVertexArrayObject( GameWindow* gl )
{
    MI_ASSERT( gl != nullptr );
    m_gl = gl;
    m_VaoId = 0;
}

OpenGLVertexArrayObject::~OpenGLVertexArrayObject()
{
    if( m_VaoId != 0 )
        destroy();
}

void OpenGLVertexArrayObject::create() {
    MI_ASSERT( (m_gl != nullptr) && (m_VaoId == 0) );

    m_gl->glGenVertexArrays(1, &m_VaoId);
}

void OpenGLVertexArrayObject::destroy() {
    MI_ASSERT( (m_gl != nullptr) && (m_VaoId != 0) );

    m_gl->glDeleteVertexArrays(1, &m_VaoId);
    m_VaoId = 0;
}

bool OpenGLVertexArrayObject::isCreated() const {
    return m_VaoId != 0;
}

unsigned int OpenGLVertexArrayObject::objectId() const {
    return m_VaoId;
}

void OpenGLVertexArrayObject::bind() {
    MI_ASSERT( (m_gl != nullptr) && (m_VaoId !=0 ) );
    m_gl->glBindVertexArray( m_VaoId );
}

void OpenGLVertexArrayObject::release() {
    MI_ASSERT( (m_gl != nullptr) && (m_VaoId !=0 ) );
    m_gl->glBindVertexArray( 0 );
}
