#ifndef POSTPROCESS_H
#define POSTPROCESS_H

#include <QTime>
#include <QString>
#include <QtGui/QOpenGLFunctions>

class GameWindow;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QString;

class PostProcess {
public:
    PostProcess(const QString& fragmentShader);
    ~PostProcess();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);

    void render(GameWindow* gl);

private:
    QString m_fragmentShader;

    QOpenGLShaderProgram* m_program;
    // Le vertex array object
    QOpenGLVertexArrayObject* m_vao;

    QTime m_time;
    int m_timeUniformLocation;
};

#endif // POSTPROCESS_H
