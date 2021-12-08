#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QApplication>
#include <QWheelEvent>

#include <QDebug>

class GLWidgetData : public QSharedData
{
public:

};

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent)
  , data(new GLWidgetData)
  , m_cameraSpeed(0.1f)
{
    setFocusPolicy(Qt::ClickFocus);

    cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
}

GLWidget::GLWidget(const GLWidget &rhs) : data(rhs.data)
{

}

GLWidget::~GLWidget()
{
    cleanup();
}

void GLWidget::cleanup()
{
    if (m_program == nullptr)
        return;
    makeCurrent();
    m_vbo.destroy();
    delete m_program;
    m_program = 0;
    doneCurrent();
}

GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

GLenum indices[] = {
    0, 1, 3, // 第一个三角形
    1, 2, 3  // 第二个三角形
};

void GLWidget::initializeGL()
{
    //    QTimer *timer = new QTimer(this);
    //    timer->setInterval(10);
    //    connect(timer, &QTimer::timeout, [=]{
    //        t +=1;
    //        t = t%360;
    //        update();
    //    });
    //    timer->start();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    m_program = new QOpenGLShaderProgram;

    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertexShaderSource.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragmentShaderSource.frag");

    if(m_program->link())
    {
        qDebug("link success");
    }
    else
    {
        qDebug("link failed");
    }

    m_program->bind();
    m_modelLoc = m_program->uniformLocation("model");
    m_cameraLoc = m_program->uniformLocation("view");
    m_projLoc = m_program->uniformLocation("projection");

    m_vao.create();
    m_vbo.create();
    m_ebo.create();

    m_vao.bind();

    //定点属性绑定
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo.bufferId());
    glBufferData(GL_ARRAY_BUFFER,  sizeof(vertices), vertices, GL_STATIC_DRAW);

    //纹理相关代码初始化
    m_texture1 = new QOpenGLTexture(QImage(":/container.jpg"));
    glBindTexture(GL_TEXTURE_2D, m_texture1->textureId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_texture2 = new QOpenGLTexture(QImage(":/awesomeface.png").mirrored());
    glBindTexture(GL_TEXTURE_2D, m_texture2->textureId());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof (GLfloat)));

    m_vao.release();
    m_program->release();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    // 设置顺时针方向 CW : Clock Wind 顺时针方向
    // 默认是 GL_CCW : Counter Clock Wind 逆时针方向
    //glFrontFace(GL_CW);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    //绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture1->textureId());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->textureId());
    m_program->setUniformValue("texture1", 0);
    m_program->setUniformValue("texture2", 1);

    m_camera = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glUniformMatrix4fv(m_cameraLoc, 1, GL_FALSE, glm::value_ptr(m_camera));

    glUniformMatrix4fv(m_projLoc, 1, GL_FALSE, glm::value_ptr(m_proj));

    for(int i=0; i < 10; ++i)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);

        float angle = 20.0f * i;
        model = glm::rotate(model,  glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_program->release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj = glm::mat4(1.0f);
    m_proj = glm::perspective(glm::radians(45.0f), GLfloat(w) / h, 0.1f, 100.0f);
}

//旋转,可以沿着X Y Z轴旋转
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->localPos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float xoffset = event->x() - m_lastPos.x();
    float yoffset = m_lastPos.y() - event->y(); // reversed since y-coordinates go from bottom to top

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw = xoffset - 90.0f;
    m_pitch = yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    cameraFront = glm::normalize(front);

    update();
}

//前后移动
void GLWidget::wheelEvent(QWheelEvent *event)
{
    if(QApplication::keyboardModifiers () == Qt::ControlModifier)
    {
        event->delta() > 0  ? cameraPos += m_cameraSpeed * cameraFront : cameraPos -= m_cameraSpeed * cameraFront;
        update();
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}

//上下左右控制
void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        cameraPos += m_cameraSpeed * cameraUp;
        break;
    case Qt::Key_Down:
        cameraPos -= m_cameraSpeed * cameraUp;
        break;
    case Qt::Key_Left:
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * m_cameraSpeed;
        break;
    case Qt::Key_Right:
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * m_cameraSpeed;
        break;
    }
    update();
}
