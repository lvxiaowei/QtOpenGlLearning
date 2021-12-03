#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QDebug>

class GLWidgetData : public QSharedData
{
public:

};

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent)
  , data(new GLWidgetData)
{

}

GLWidget::GLWidget(const GLWidget &rhs) : data(rhs.data)
{

}

GLWidget &GLWidget::operator=(const GLWidget &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
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

static const char *vertexShaderSource =
        "#version 330\n"
        "layout (location = 0) in vec3 posVertex;\n"
        "void main() {\n"
        "   gl_Position = vec4(posVertex, 1.0);\n"
        "}\n";

static const char *fragmentShaderSource =
        " #version 330\n"
        " out vec4 fragColor;"
        " void main() {"
        "   fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
        " }";

GLfloat vertices[] = {
    0.5f, 0.5f, 0.0f,   // 右上角
    0.5f, -0.5f, 0.0f,  // 右下角
    -0.5f, -0.5f, 0.0f, // 左下角
    -0.5f, 0.5f, 0.0f   // 左上角
};

GLenum indices[] = {
    0, 1, 3, // 第一个三角形
    1, 2, 3  // 第二个三角形
};

void GLWidget::initializeGL()
{
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,  vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);

    if(m_program->link())
    {
        qDebug("link success");
    }
    else
    {
        qDebug("link failed");
    }

    m_vao.create();
    m_vbo.create();
    m_ebo.create();

    m_vao.bind();

    //定点属性绑定
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo.bufferId());
    f->glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo.bufferId());
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(GLenum), indices, GL_STATIC_DRAW);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    m_vao.release();
    m_program->release();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // 设置顺时针方向 CW : Clock Wind 顺时针方向
    // 默认是 GL_CCW : Counter Clock Wind 逆时针方向
    glFrontFace(GL_CW);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();

    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_program->release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}
