#include "ShaderProgramPool.h"

ShaderProgramPool::ShaderProgramPool()
{

}

bool ShaderProgramPool::addShaderFromSourceCode(const QString &name, QOpenGLShader::ShaderType type, const char *source)
{
    QString objName(QString("%1_%2").arg(name).arg(type));
    QOpenGLShader* shader = new QOpenGLShader(type, this);
    if (shader->compileSourceCode(source))
    {
        QOpenGLShader* old = findChild<QOpenGLShader*>(objName);
        if (old)
        {
            delete old;
        }
        shader->setObjectName(objName);
    }
    else
    {
        fprintf(stderr, "addShaderFromSourceCode(%s) fail:%s\n", source, shader->log().toUtf8().data() );
        delete shader;
        return false;
    }
    return true;
}

bool ShaderProgramPool::addShaderFromSourceCode(const QString &name, QOpenGLShader::ShaderType type, const QByteArray &source)
{
    QString objName(QString("%1_%2").arg(name).arg(type));
    QOpenGLShader* shader = new QOpenGLShader(type, this);
    if (shader->compileSourceCode(source))
    {
        QOpenGLShader* old = findChild<QOpenGLShader*>(objName);
        if (old)
        {
            delete old;
        }
        shader->setObjectName(objName);
    }
    else
    {
        fprintf(stderr, "addShaderFromSourceCode(%s) fail:%s\n", source.data(), shader->log().toUtf8().data() );
        delete shader;
        return false;
    }
    return true;
}

bool ShaderProgramPool::addShaderFromSourceCode(const QString &name, QOpenGLShader::ShaderType type, const QString &source)
{
    QString objName(QString("%1_%2").arg(name).arg(type));
    QOpenGLShader* shader = new QOpenGLShader(type, this);
    if (shader->compileSourceCode(source))
    {
        QOpenGLShader* old = findChild<QOpenGLShader*>(objName);
        if (old)
        {
            delete old;
        }
        shader->setObjectName(objName);
    }
    else
    {
        fprintf(stderr, "addShaderFromSourceCode(%s) fail:%s\n", source.toUtf8().data(), shader->log().toUtf8().data() );
        delete shader;
        return false;
    }
    return true;
}

bool ShaderProgramPool::addShaderFromSourceFile(const QString &name, QOpenGLShader::ShaderType type, const QString &fileName)
{
    if (name.isEmpty()) return false;
    QString objName(QString("%1_%2").arg(name).arg(type));
    QOpenGLShader* shader = new QOpenGLShader(type, this);
    if (shader->compileSourceFile(fileName))
    {
        auto old = m_shaders.find(objName);
        if (old != m_shaders.end())
        {
            delete old.value();
            m_shaders.erase(old);
        }
        m_shaders[objName] = shader;
        //shader->setParent(this);
        //shader->setObjectName(objName);
        //QOpenGLShader* shader2 = findChild<QOpenGLShader*>();
        //qDebug() <<shader2;
    }
    else
    {
        fprintf(stderr, "addShaderFromSourceFile(%s) fail:%s\n", fileName.toUtf8().data(), shader->log().toUtf8().data() );
        delete shader;
        return false;
    }
    return true;
}

bool ShaderProgramPool::setProgramShaders(const QString &name, QVector<QPair<QString, QOpenGLShader::ShaderType>> shaders)
{
    if ( name.isEmpty() || shaders.empty() ) return false;
    QVector<QOpenGLShader*> lst;
    for (auto s = shaders.begin(); s != shaders.end(); ++s)
    {
        QString objName(QString("%1_%2").arg(s->first).arg(s->second));
        auto shader = m_shaders.find(objName);
        if (shader == m_shaders.end()) return false;
        lst.push_back(shader.value());
    }
    m_programs[name] = lst;
    return true;
}

QOpenGLShaderProgram *ShaderProgramPool::createProgram(const QString &name)
{
    auto it = m_programs.find(name);
    if (it != m_programs.end())
    {
        QOpenGLShaderProgram* prog = new QOpenGLShaderProgram(this);
        for (auto s:it.value())
        {
            prog->addShader(s);
        }
        prog->bindAttributeLocation("qt_Vertex", 0);
        prog->bindAttributeLocation("qt_MultiTexCoord0", 1);
        if (prog->link())
        {
            prog->setObjectName(name);
            return prog;
        }
        else
        {
            fprintf(stderr, "createProgram %s, link fail:%s\n", name.toUtf8().data(), prog->log().toUtf8().data());
            delete prog;
        }
    }
    return nullptr;
}
