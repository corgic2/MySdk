#include "QtCustomAPI.h"
#include <QCoreApplication>
#include <QDebug>

QtCustomAPI::QtCustomAPI(QObject* parent)
    : QObject(parent)
{
    m_process = new QProcess(this);
}

QtCustomAPI::~QtCustomAPI()
{
    // 确保进程被正确停止和清理
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(3000); // 等待3秒让进程结束
    }
}

ST_CommandResult QtCustomAPI::ExecuteCommand(const QString& command, const QStringList& arguments, int timeoutMs)
{
    ST_CommandResult result;
    
    if (!m_process)
    {
        result.errorMessage = "进程对象未初始化";
        return result;
    }
    
    // 确保进程不在运行中
    if (m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    
    // 启动进程
    m_process->start(command, arguments);
    
    if (!m_process->waitForStarted(5000))
    {
        result.errorMessage = "命令启动失败: " + m_process->errorString();
        return result;
    }
    
    // 等待完成
    bool finished = false;
    if (timeoutMs > 0)
    {
        finished = m_process->waitForFinished(timeoutMs);
    }
    else
    {
        finished = m_process->waitForFinished(-1);
    }
    
    if (!finished)
    {
        m_process->kill();
        m_process->waitForFinished(3000);
        result.errorMessage = "命令执行超时";
        return result;
    }
    
    // 获取结果
    result.exitCode = m_process->exitCode();
    result.output = QString::fromUtf8(m_process->readAllStandardOutput());
    result.error = QString::fromUtf8(m_process->readAllStandardError());
    
    if (m_process->exitStatus() == QProcess::NormalExit && result.exitCode == 0)
    {
        result.success = true;
    }
    else
    {
        result.errorMessage = "命令执行失败，退出代码: " + QString::number(result.exitCode);
    }
    
    return result;
}

ST_CommandResult QtCustomAPI::ExecuteFFmpegCommand(const QStringList& arguments, int timeoutMs)
{
    return ExecuteCommand("ffmpeg", arguments, timeoutMs);
}

ST_CommandResult QtCustomAPI::ExecuteCommandLine(const QString& commandLine, int timeoutMs)
{
    ST_CommandResult result;
    
    if (!m_process)
    {
        result.errorMessage = "进程对象未初始化";
        return result;
    }
    
    // 在Windows下使用cmd /c执行命令
#ifdef Q_OS_WIN
    QString program = "cmd";
    QStringList arguments;
    arguments << "/c" << commandLine;
#else
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << commandLine;
#endif
    
    return ExecuteCommand(program, arguments, timeoutMs);
}
