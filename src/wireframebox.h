#ifndef WIREFRAMEBOX_H
#define WIREFRAMEBOX_H

#include <QtGui/QVector3D>

class GameWindow;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;

class WireframeBox {
public:
    WireframeBox();
    ~WireframeBox();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void draw(GameWindow* gl);

    void setPosition(const QVector3D& position);

private:
    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer* m_indices;
    QOpenGLVertexArrayObject* m_vao;

    int m_posUniform;
    int m_matrixUniform;

    QVector3D m_position;
};

#endif // WIREFRAMEBOX_H
