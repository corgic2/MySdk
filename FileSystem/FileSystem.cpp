#include "FileSystem.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <QDateTime>
#include <QTextStream>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include "SDKCommonDefine/SDKCommonDefine.h"

namespace my_sdk
{
    FileSystem::FileSystem()
    {
        // 设置全局的文本编码为UTF-8
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    }

    FileSystem::~FileSystem()
    {
    }

    bool FileSystem::WriteStringToFile(const std::string& filePath, const std::string& str, bool writeBom)
    {
        try
        {
            // 确保父目录存在
            QFileInfo fileInfo(QString::fromUtf8(filePath.c_str()));
            QDir dir = fileInfo.dir();
            if (!dir.exists())
            {
                dir.mkpath(".");
            }

            // 写入内容
            QFile file(QString::fromUtf8(filePath.c_str()));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                return false;
            }

            QTextStream out(&file);
            // 设置文本流编码为UTF-8
            out.setCodec("UTF-8");
            if (writeBom)
            {
                out.setGenerateByteOrderMark(true);
            }
            out << QString::fromUtf8(str.c_str());
            file.close();
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string FileSystem::ReadStringFromFile(const std::string& filePath, bool removeBOM)
    {
        try
        {
            QFile file(QString::fromUtf8(filePath.c_str()));
            if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                return "";
            }

            QTextStream in(&file);
            // 设置文本流编码为UTF-8
            in.setCodec("UTF-8");
            QString content = in.readAll();
            file.close();

            return content.toUtf8().constData();
        }
        catch (const std::exception&)
        {
            return "";
        }
    }

    std::string FileSystem::ConvertEncodingToUtf_8(const std::string& input, const std::string& to)
    {
        try
        {
            QTextCodec* codec = QTextCodec::codecForName(to.c_str());
            if (!codec)
            {
                return input;
            }
            QString text = codec->toUnicode(input.c_str());
            return text.toUtf8().constData();
        }
        catch (const std::exception&)
        {
            return input;
        }
    }

    ST_FileInfo FileSystem::GetFileInfo(const std::string& path)
    {
        ST_FileInfo info;
        try
        {
            QFileInfo fileInfo(QString::fromUtf8(path.c_str()));
            if (!fileInfo.exists())
            {
                return info;
            }

            info.m_name = fileInfo.fileName().toUtf8().constData();
            info.m_path = fileInfo.absoluteFilePath().toUtf8().constData();
            info.m_isDirectory = fileInfo.isDir();
            info.m_isReadOnly = !fileInfo.isWritable();

            if (!info.m_isDirectory)
            {
                info.m_size = fileInfo.size();
            }

            info.m_createTime = fileInfo.birthTime().toSecsSinceEpoch();
            info.m_modifyTime = fileInfo.lastModified().toSecsSinceEpoch();
            info.m_accessTime = fileInfo.lastRead().toSecsSinceEpoch();
        }
        catch (const std::exception&)
        {
        }
        return info;
    }

    bool FileSystem::Copy(const std::string& source, const std::string& destination, bool overwrite)
    {
        try
        {
            QString srcPath = QString::fromUtf8(source.c_str());
            QString destPath = QString::fromUtf8(destination.c_str());

            QFileInfo srcInfo(srcPath);
            if (!srcInfo.exists())
            {
                return false;
            }

            QFileInfo destInfo(destPath);
            if (destInfo.exists() && !overwrite)
            {
                return false;
            }

            if (srcInfo.isDir())
            {
                QDir dir;
                if (!dir.mkpath(destPath))
                {
                    return false;
                }

                QDir sourceDir(srcPath);
                QStringList files = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
                for (const QString& file : files)
                {
                    QString srcFilePath = srcPath + "/" + file;
                    QString destFilePath = destPath + "/" + file;
                    if (!Copy(srcFilePath.toUtf8().constData(), destFilePath.toUtf8().constData(), overwrite))
                    {
                        return false;
                    }
                }
            }
            else
            {
                if (destInfo.exists() && overwrite)
                {
                    QFile::remove(destPath);
                }
                if (!QFile::copy(srcPath, destPath))
                {
                    return false;
                }
            }
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::Move(const std::string& source, const std::string& destination)
    {
        try
        {
            QString srcPath = QString::fromUtf8(source.c_str());
            QString destPath = QString::fromUtf8(destination.c_str());

            QFile file(srcPath);
            if (!file.exists())
            {
                return false;
            }

            return file.rename(destPath);
        }
        catch (const std::exception&)
        {
            // 如果跨设备移动失败，尝试复制后删除
            if (Copy(source, destination, true))
            {
                return Delete(source);
            }
            return false;
        }
    }

    bool FileSystem::Delete(const std::string& path)
    {
        try
        {
            QString qPath = QString::fromUtf8(path.c_str());
            QFileInfo fileInfo(qPath);
            if (!fileInfo.exists())
            {
                return true;
            }

            if (fileInfo.isDir())
            {
                QDir dir(qPath);
                return dir.removeRecursively();
            }
            else
            {
                QFile file(qPath);
                return file.remove();
            }
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::CreateWindowsDirectory(const std::string& path)
    {
        try
        {
            QDir dir;
            return dir.mkpath(QString::fromUtf8(path.c_str()));
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::Exists(const std::string& path)
    {
        try
        {
            return QFileInfo::exists(QString::fromUtf8(path.c_str()));
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    size_t FileSystem::GetFileSize(const std::string& path)
    {
        try
        {
            QFileInfo fileInfo(QString::fromUtf8(path.c_str()));
            if (fileInfo.exists() && fileInfo.isFile())
            {
                return fileInfo.size();
            }
        }
        catch (const std::exception&)
        {
        }
        return 0;
    }

    std::vector<std::string> FileSystem::GetFiles(const std::string& dir, bool recursive)
    {
        std::vector<std::string> files;
        try
        {
            QDir directory(QString::fromUtf8(dir.c_str()));
            if (!directory.exists())
            {
                return files;
            }

            QStringList entries = directory.entryList(QDir::Files);
            for (const QString& entry : entries)
            {
                files.push_back(directory.absoluteFilePath(entry).toUtf8().constData());
            }

            if (recursive)
            {
                QStringList subdirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                for (const QString& subdir : subdirs)
                {
                    QString subdirPath = directory.absoluteFilePath(subdir);
                    std::vector<std::string> subFiles = GetFiles(subdirPath.toUtf8().constData(), true);
                    files.insert(files.end(), subFiles.begin(), subFiles.end());
                }
            }
        }
        catch (const std::exception&)
        {
        }
        return files;
    }

    std::vector<std::string> FileSystem::GetDirectories(const std::string& dir, bool recursive)
    {
        std::vector<std::string> directories;
        try
        {
            QDir directory(QString::fromUtf8(dir.c_str()));
            if (!directory.exists())
            {
                return directories;
            }

            QStringList entries = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& entry : entries)
            {
                directories.push_back(directory.absoluteFilePath(entry).toUtf8().constData());
            }

            if (recursive)
            {
                for (const QString& subdir : entries)
                {
                    QString subdirPath = directory.absoluteFilePath(subdir);
                    std::vector<std::string> subdirs = GetDirectories(subdirPath.toUtf8().constData(), true);
                    directories.insert(directories.end(), subdirs.begin(), subdirs.end());
                }
            }
        }
        catch (const std::exception&)
        {
        }
        return directories;
    }

    bool FileSystem::SetFileAttributes(const std::string& path, bool readOnly)
    {
        try
        {
            QFile file(QString::fromUtf8(path.c_str()));
            if (!file.exists())
            {
                return false;
            }

            QFile::Permissions permissions = file.permissions();
            if (readOnly)
            {
                permissions &= ~(QFile::WriteOwner | QFile::WriteUser | QFile::WriteGroup | QFile::WriteOther);
            }
            else
            {
                permissions |= (QFile::WriteOwner | QFile::WriteUser);
            }

            return file.setPermissions(permissions);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string FileSystem::GetExtension(const std::string& path)
    {
        try
        {
            QFileInfo fileInfo(QString::fromUtf8(path.c_str()));
            return fileInfo.suffix().toUtf8().constData();
        }
        catch (const std::exception&)
        {
            return "";
        }
    }

    std::string FileSystem::GetFileNameWithoutExtension(const std::string& path)
    {
        try
        {
            QFileInfo fileInfo(QString::fromUtf8(path.c_str()));
            return fileInfo.baseName().toUtf8().constData();
        }
        catch (const std::exception&)
        {
            return "";
        }
    }

    void FileSystem::ListDirectory(const std::string& dir)
    {
        try
        {
            QDir directory(QString::fromUtf8(dir.c_str()));
            if (!directory.exists())
            {
                throw std::runtime_error("目录无效: " + dir);
            }

            std::cout << "目录内容 (" << dir << "):\n";
            QFileInfoList entries = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (const QFileInfo& entry : entries)
            {
                QString type = entry.isDir() ? "[目录]" : entry.isFile() ? "[文件]" : "[其他]";
                std::cout << "  " << type.toUtf8().constData() << " " << entry.fileName().toUtf8().constData() << "\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "列出目录内容失败: " << e.what() << std::endl;
        }
    }

    void FileSystem::GetDirectoryContents(const std::string& dir, std::vector<std::string>& files, std::vector<std::string>& directories)
    {
        try
        {
            QDir directory(QString::fromUtf8(dir.c_str()));
            if (!directory.exists())
            {
                return;
            }

            QFileInfoList entries = directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (const QFileInfo& entry : entries)
            {
                if (entry.isFile())
                {
                    files.push_back(entry.absoluteFilePath().toUtf8().constData());
                }
                else if (entry.isDir())
                {
                    directories.push_back(entry.absoluteFilePath().toUtf8().constData());
                    GetDirectoryContents(entry.absoluteFilePath().toUtf8().constData(), files, directories);
                }
            }
        }
        catch (const std::exception&)
        {
        }
    }

    EM_JsonOperationResult FileSystem::WriteJsonToFile(const std::string& filePath, const std::string& jsonStr, bool pretty)
    {
        try
        {
            // 首先验证JSON格式
            if (!ValidateJsonString(jsonStr))
            {
                return EM_JsonOperationResult::InvalidJson;
            }

            // 将std::string转换为QString时显式指定UTF-8编码
            QString jsonQStr = QString::fromUtf8(jsonStr.c_str());
            QJsonDocument doc = QJsonDocument::fromJson(jsonQStr.toUtf8());
            if (doc.isNull())
            {
                return EM_JsonOperationResult::InvalidJson;
            }

            QFile file(QString::fromUtf8(filePath.c_str()));
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  // 添加Text标志以确保正确的换行符
            {
                return EM_JsonOperationResult::WriteError;
            }

            // 使用QTextStream确保正确的UTF-8编码输出
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out.setGenerateByteOrderMark(true);  // 添加UTF-8 BOM
            out << doc.toJson(pretty ? QJsonDocument::Indented : QJsonDocument::Compact);
            file.close();

            return EM_JsonOperationResult::Success;
        }
        catch (const std::exception&)
        {
            return EM_JsonOperationResult::WriteError;
        }
    }

    EM_JsonOperationResult FileSystem::ReadJsonFromFile(const std::string& filePath, std::string& jsonStr)
    {
        try
        {
            QFile file(QString::fromUtf8(filePath.c_str()));
            if (!file.exists())
            {
                return EM_JsonOperationResult::FileNotFound;
            }

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  // 添加Text标志以确保正确的换行符
            {
                return EM_JsonOperationResult::ReadError;
            }

            // 使用QTextStream确保正确的UTF-8编码读取
            QTextStream in(&file);
            in.setCodec("UTF-8");
            QString jsonQStr = in.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(jsonQStr.toUtf8());
            if (doc.isNull())
            {
                return EM_JsonOperationResult::InvalidJson;
            }

            // 确保输出的字符串是UTF-8编码
            jsonStr = jsonQStr.toUtf8().constData();
            return EM_JsonOperationResult::Success;
        }
        catch (const std::exception&)
        {
            return EM_JsonOperationResult::ReadError;
        }
    }

    bool FileSystem::ValidateJsonString(const std::string& jsonStr)
    {
        try
        {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(jsonStr.c_str()).toUtf8());
            return !doc.isNull();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::FormatJsonString(const std::string& jsonStr, std::string& formattedStr)
    {
        try
        {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(jsonStr.c_str()).toUtf8());
            if (doc.isNull())
            {
                return false;
            }

            formattedStr = QString(doc.toJson(QJsonDocument::Indented)).toUtf8().constData();
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string FileSystem::QtPathToStdPath(const std::string& qtPath)
    {
        try
        {
            QString path = QString::fromUtf8(qtPath.c_str());
            std::string stdPath = path.toUtf8().constData();
            std::replace(stdPath.begin(), stdPath.end(), '/', '\\');
            return stdPath;
        }
        catch (const std::exception&)
        {
            return qtPath;
        }
    }

    std::string FileSystem::StdPathToQtPath(const std::string& stdPath)
    {
        try
        {
            QString path = QString::fromUtf8(stdPath.c_str());
            std::string qtPath = path.toUtf8().constData();
            std::replace(qtPath.begin(), qtPath.end(), '\\', '/');
            return qtPath;
        }
        catch (const std::exception&)
        {
            return stdPath;
        }
    }
}
