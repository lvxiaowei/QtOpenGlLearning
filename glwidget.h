#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QSharedDataPointer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>

class GLWidgetData;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    GLWidget(const GLWidget &);
    GLWidget &operator=(const GLWidget &);
    ~GLWidget();

public slots:
    void cleanup();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    QSharedDataPointer<GLWidgetData> data;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;
    QOpenGLTexture *m_texture;
    QOpenGLShaderProgram *m_program;

    QMatrix4x4 m_camera;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_world;
};

#endif // GLWIDGET_H
