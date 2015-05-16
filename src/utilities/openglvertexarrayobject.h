#pragma once

class OpenGLVertexArrayObject
{
public:
    OpenGLVertexArrayObject( );
    ~OpenGLVertexArrayObject();

    OpenGLVertexArrayObject( const OpenGLVertexArrayObject& copy ) = delete;
    OpenGLVertexArrayObject& operator=( const OpenGLVertexArrayObject& copy ) = delete;

    void            create();
    void            destroy();
    bool            isCreated() const;
    unsigned int    objectId() const;
    void            bind();
    void            release();

private:
    unsigned int    m_VaoId;
};
