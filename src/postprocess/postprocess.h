#pragma once
#include <QTime>
#include <QString>
#include <QtGui/QOpenGLFunctions>

class GameWindow;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QString;

/**
 * @brief Classe permettant d'afficher un effet de post-process sur tout l'écran.
 */
class PostProcess {
public:
    PostProcess(const QString& fragmentShader);
    ~PostProcess();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);

    void render(GameWindow* gl);

private:
    // Le nom du fragment shader à utiliser
    QString m_fragmentShader;
    // Le shader affichant l'effet
    QOpenGLShaderProgram* m_program;
    // Le vertex array object
    QOpenGLVertexArrayObject* m_vao;

    QTime m_time;
    int m_timeUniformLocation;
};
