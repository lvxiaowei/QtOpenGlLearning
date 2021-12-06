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
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aTexCoord;"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;"
        "uniform mat4 model;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        "void main() {\n"
        "   gl_Position = projection * view * model * vec4(posVertex, 1.0f);\n"
        "   ourColor = aColor;\n"
        "   TexCoord = aTexCoord;"
        "}\n";

static const char *fragmentShaderSource =
        " #version 330\n"
        " out vec4 fragColor;"
        " in vec3 ourColor;"
        " in vec2 TexCoord;"
        " uniform sampler2D texture1;"
        " uniform sampler2D texture2;"
        " void main() {"
        "   fragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);"
        " }";

GLfloat vertices[] = {
    //---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
    0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
    0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上
};

GLenum indices[] = {
    0, 1, 3, // 第一个三角形
    1, 2, 3  // 第二个三角形
};

void GLWidget::initializeGL()
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(50);
    connect(timer, &QTimer::timeout, [=]{
        t +=10;
        t = t%360;
        update();
    });
    timer->start();

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
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo.bufferId());
    glBufferData(GL_ARRAY_BUFFER,  sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo.bufferId());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof (GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof (GLfloat)));

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

    //绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture1->textureId());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->textureId());
    m_program->setUniformValue("texture1", 0);
    m_program->setUniformValue("texture2", 1);


    //设置变换矩阵
    glm::mat4 model = glm::mat4(1.0f);;
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    unsigned int transformLoc_i = m_program->uniformLocation("model");
    glUniformMatrix4fv(transformLoc_i, 1, GL_FALSE, glm::value_ptr(model));

    //观察矩阵，将场景往前移动一点
    glm::mat4 view = glm::mat4(1.0f);
    // 注意，我们将矩阵向我们要进行移动场景的反方向移动。
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    unsigned int view_i = m_program->uniformLocation("view");
    glUniformMatrix4fv(view_i, 1, GL_FALSE, glm::value_ptr(view));

    //投影矩阵
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), GLfloat(width()) / height(), 0.1f, 100.0f);
    unsigned int projection_i = m_program->uniformLocation("projection");
    glUniformMatrix4fv(projection_i, 1, GL_FALSE, glm::value_ptr(projection));

    //glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    m_program->release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}
