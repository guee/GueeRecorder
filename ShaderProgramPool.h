#ifndef SHADERPROGRAMPOOL_H
#define SHADERPROGRAMPOOL_H
#include<QtCore>
#include <QOpenGLShaderProgram>

class ShaderProgramPool : public QObject
{
public:
    ShaderProgramPool();
    
    bool addShaderFromSourceCode(const QString& name, QOpenGLShader::ShaderType type, const char *source);
    bool addShaderFromSourceCode(const QString& name, QOpenGLShader::ShaderType type, const QByteArray& source);
    bool addShaderFromSourceCode(const QString& name, QOpenGLShader::ShaderType type, const QString& source);
    bool addShaderFromSourceFile(const QString& name, QOpenGLShader::ShaderType type, const QString& fileName);

    bool setProgramShaders(const QString& name, QVector<QPair<QString, QOpenGLShader::ShaderType>> shaders);
    bool setProgramShaders(const QString& name, const QString& file_vert, const QString& file_frag);
    QOpenGLShaderProgram* createProgram(const QString& name);
private:
    QMap<QString, QOpenGLShader*> m_shaders;
    QMap<QString, QVector<QOpenGLShader*>> m_programs;
};

#endif // SHADERPROGRAMPOOL_H
