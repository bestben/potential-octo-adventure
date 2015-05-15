#pragma once

class GameWindow;

class OpenGLVertexArrayObject
{
public:
    OpenGLVertexArrayObject( GameWindow* gl );
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
    GameWindow*     m_gl;
    unsigned int    m_VaoId;
};
