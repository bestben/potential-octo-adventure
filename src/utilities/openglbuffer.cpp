#include "openglbuffer.h"

#include "../utility.h"

static unsigned int g_CurrentBoundBuffer = 0;

OpenGLBuffer::OpenGLBuffer(OpenGLBuffer::Type type)
{
    m_bufferId = 0;
    m_type = type;
    m_size = 0;
    m_usagePattern = StaticDraw;
}

OpenGLBuffer::~OpenGLBuffer()
{
    if( m_bufferId != 0 )
        destroy();
}

OpenGLBuffer::Type OpenGLBuffer::type() const {
    return m_type;
}

OpenGLBuffer::UsagePattern OpenGLBuffer::usagePattern() const {
    return m_usagePattern;
}

void OpenGLBuffer::setUsagePattern(OpenGLBuffer::UsagePattern value) {
    m_usagePattern = value;
}

void OpenGLBuffer::create() {
     MI_ASSERT( (m_bufferId == 0) );
     glGenBuffers(1, &m_bufferId);
}

bool OpenGLBuffer::isCreated() const {
    return m_bufferId != 0;
}

void OpenGLBuffer::destroy() {
    MI_ASSERT( (m_bufferId != 0) );
    glDeleteBuffers( 1, &m_bufferId );
    m_bufferId = 0;
    m_size = 0;
}

void OpenGLBuffer::bind() {
    MI_ASSERT( (m_bufferId != 0) );

    //if( g_CurrentBoundBuffer != m_bufferId ) {
        g_CurrentBoundBuffer = m_bufferId;
        glBindBuffer( m_type, m_bufferId );
    //}
}

void OpenGLBuffer::release() {
    MI_ASSERT( (m_bufferId != 0) );
    //MI_ASSERT( g_CurrentBoundBuffer == m_bufferId );

     //g_CurrentBoundBuffer = 0;
     glBindBuffer( m_type, 0 );
}

unsigned int OpenGLBuffer::bufferId() const {
    return m_bufferId;
}

int OpenGLBuffer::size() const {
    return m_size;
}

void OpenGLBuffer::read(int offset, void* data, int count) {
    MI_ASSERT( (m_bufferId != 0) && (data != nullptr) );
    //MI_ASSERT( g_CurrentBoundBuffer == m_bufferId );

    glGetBufferSubData( m_type, offset, count, data );
}

void OpenGLBuffer::write(int offset, const void* data, int count) {
    MI_ASSERT( (m_bufferId != 0) );
    //MI_ASSERT( g_CurrentBoundBuffer == m_bufferId );

    if( (count + offset) > (int)m_size )
        allocate( count + offset );

    glBufferSubData( m_type, offset, count, data );
}

void OpenGLBuffer::allocate(const void* data, int count) {
    MI_ASSERT( (m_bufferId != 0) );
    //MI_ASSERT( g_CurrentBoundBuffer == m_bufferId );

    glBufferData( m_type, count, data, m_usagePattern );
}

void* OpenGLBuffer::map(OpenGLBuffer::Access access) {
    MI_ASSERT( (m_bufferId != 0) );
    //MI_ASSERT( g_CurrentBoundBuffer == m_bufferId );
    return glMapBuffer( m_type, access );
}

bool OpenGLBuffer::unmap() {
     MI_ASSERT( (m_bufferId != 0) );
     return glUnmapBuffer( m_type ) == GL_TRUE;
}
