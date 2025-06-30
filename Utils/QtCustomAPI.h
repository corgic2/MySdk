#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include "../SDKCommonDefine/SDKCommonDefine.h"
#include "../SDKCommonDefine/SDK_Export.h"

/// <summary>
/// 命令执行结果结构体
/// </summary>
struct ST_CommandResult
{
    /// <summary>
    /// 是否执行成功
    /// </summary>
    bool success;

    /// <summary>
    /// 退出代码
    /// </summary>
    int exitCode;

    /// <summary>
    /// 标准输出内容
    /// </summary>
    QString output;

    /// <summary>
    /// 错误输出内容
    /// </summary>
    QString error;

    /// <summary>
    /// 错误信息
    /// </summary>
    QString errorMessage;

    /// <summary>
    /// 构造函数
    /// </summary>
    ST_CommandResult()
        : success(false), exitCode(-1), output(""), error(""), errorMessage("")
    {
    }
};

/// <summary>
/// Qt自定义API类，提供简单的命令行调用功能
/// </summary>
class SDK_API QtCustomAPI : public QObject
{
    Q_OBJECT

public:
    /// <summary>
    /// 构造函数
    /// </summary>
    /// <param name="parent">父对象</param>
    QtCustomAPI(QObject* parent = nullptr);

    /// <summary>
    /// 析构函数
    /// </summary>
    ~QtCustomAPI() override;

    /// <summary>
    /// 执行命令
    /// </summary>
    /// <param name="command">命令</param>
    /// <param name="arguments">参数列表</param>
    /// <param name="timeoutMs">超时时间（毫秒，0表示无限制）</param>
    /// <returns>命令执行结果</returns>
    ST_CommandResult ExecuteCommand(const QString& command, const QStringList& arguments = QStringList(), int timeoutMs = 30000);

    /// <summary>
    /// 执行FFmpeg命令（推荐使用，避免路径解析问题）
    /// </summary>
    /// <param name="arguments">FFmpeg参数列表</param>
    /// <param name="timeoutMs">超时时间（毫秒，0表示无限制）</param>
    /// <returns>命令执行结果</returns>
    ST_CommandResult ExecuteFFmpegCommand(const QStringList& arguments, int timeoutMs = 300000);

    /// <summary>
    /// 执行命令（字符串形式）
    /// </summary>
    /// <param name="commandLine">完整的命令行</param>
    /// <param name="timeoutMs">超时时间（毫秒，0表示无限制）</param>
    /// <returns>命令执行结果</returns>
    ST_CommandResult ExecuteCommandLine(const QString& commandLine, int timeoutMs = 30000);

private:
    /// <summary>
    /// 进程对象指针
    /// </summary>
    QProcess* m_process = nullptr;
};
