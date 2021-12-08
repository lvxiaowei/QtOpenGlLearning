#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QSharedDataPointer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QTimer>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLWidgetData;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    GLWidget(const GLWidget &);
    ~GLWidget();

public slots:
    void cleanup();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QSharedDataPointer<GLWidgetData> data;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ebo;
    QOpenGLTexture *m_texture1, *m_texture2;
    QOpenGLShaderProgram *m_program;

    int t;
    QPointF m_lastPos;
    float m_cameraSpeed;
    float m_yaw;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float m_pitch;
    float m_fov;
    int m_modelLoc, m_cameraLoc, m_projLoc;
    glm::vec3 cameraPos, cameraFront, cameraUp;
    glm::mat4 m_camera;
    glm::mat4 m_proj;
    QMatrix4x4 m_world;
};

#endif // GLWIDGET_H
