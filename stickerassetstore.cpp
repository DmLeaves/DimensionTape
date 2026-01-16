#include "stickerassetstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>

StickerAssetStore::StickerAssetStore()
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    m_rootDir = QDir::cleanPath(appDir.filePath("data"));
    m_tapesDir = ensureSubdir("Tapes");
    m_modulesDir = ensureSubdir("Modules");
}

QString StickerAssetStore::tapesRoot() const
{
    return m_tapesDir;
}

QString StickerAssetStore::modulesRoot() const
{
    return m_modulesDir;
}

QString StickerAssetStore::importImage(const QString &sourcePath, QString *error) const
{
    if (sourcePath.trimmed().isEmpty()) {
        return QString();
    }

    const QFileInfo sourceInfo(sourcePath);
    const QString absoluteSource = QDir::cleanPath(sourceInfo.absoluteFilePath());
    if (isPathUnderRoot(absoluteSource, m_tapesDir)) {
        return absoluteSource;
    }

    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (error) {
            *error = QString("图片不存在: %1").arg(sourcePath);
        }
        return sourcePath;
    }

    QDir().mkpath(m_tapesDir);
    QString targetPath = QDir(m_tapesDir).filePath(sourceInfo.fileName());
    if (QFile::exists(targetPath)) {
        targetPath = uniqueFilePath(m_tapesDir, sourceInfo.fileName());
    }

    if (!QFile::copy(absoluteSource, targetPath)) {
        if (error) {
            *error = QString("复制图片失败: %1").arg(sourcePath);
        }
        return sourcePath;
    }

    return QDir::cleanPath(targetPath);
}

QString StickerAssetStore::importLive2DModel(const QString &modelJsonPath, QString *error) const
{
    if (modelJsonPath.trimmed().isEmpty()) {
        return QString();
    }

    const QFileInfo sourceInfo(modelJsonPath);
    const QString absoluteSource = QDir::cleanPath(sourceInfo.absoluteFilePath());
    if (isPathUnderRoot(absoluteSource, m_modulesDir)) {
        return absoluteSource;
    }

    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (error) {
            *error = QString("模型文件不存在: %1").arg(modelJsonPath);
        }
        return modelJsonPath;
    }

    const QString sourceDirPath = QDir::cleanPath(sourceInfo.absolutePath());
    const QString baseName = QFileInfo(sourceDirPath).fileName();
    QDir().mkpath(m_modulesDir);

    QString targetDir = QDir(m_modulesDir).filePath(baseName);
    if (QDir(targetDir).exists()) {
        targetDir = uniqueDirPath(m_modulesDir, baseName);
    }

    if (!copyDirRecursive(sourceDirPath, targetDir, error)) {
        return modelJsonPath;
    }

    QString targetModel = QDir(targetDir).filePath(sourceInfo.fileName());
    return QDir::cleanPath(targetModel);
}

QString StickerAssetStore::ensureSubdir(const QString &name) const
{
    QDir root(m_rootDir);
    const QString path = root.filePath(name);
    QDir().mkpath(path);
    return QDir::cleanPath(path);
}

QString StickerAssetStore::uniqueFilePath(const QString &dirPath, const QString &fileName) const
{
    QFileInfo info(fileName);
    QString base = info.completeBaseName();
    QString suffix = info.suffix();
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString uniqueName = suffix.isEmpty()
        ? QString("%1_%2").arg(base, token)
        : QString("%1_%2.%3").arg(base, token, suffix);
    return QDir(dirPath).filePath(uniqueName);
}

QString StickerAssetStore::uniqueDirPath(const QString &dirPath, const QString &baseName) const
{
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return QDir(dirPath).filePath(QString("%1_%2").arg(baseName, token));
}

bool StickerAssetStore::copyDirRecursive(const QString &sourceDir, const QString &targetDir, QString *error) const
{
    QDir source(sourceDir);
    if (!source.exists()) {
        if (error) {
            *error = QString("目录不存在: %1").arg(sourceDir);
        }
        return false;
    }

    QDir().mkpath(targetDir);
    const QFileInfoList entries = source.entryInfoList(
        QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        const QString srcPath = entry.absoluteFilePath();
        const QString dstPath = QDir(targetDir).filePath(entry.fileName());
        if (entry.isDir()) {
            if (!copyDirRecursive(srcPath, dstPath, error)) {
                return false;
            }
        } else if (entry.isFile()) {
            if (QFile::exists(dstPath)) {
                QFile::remove(dstPath);
            }
            if (!QFile::copy(srcPath, dstPath)) {
                if (error) {
                    *error = QString("复制文件失败: %1").arg(srcPath);
                }
                return false;
            }
        }
    }
    return true;
}

bool StickerAssetStore::isPathUnderRoot(const QString &path, const QString &root) const
{
    if (path.isEmpty() || root.isEmpty()) {
        return false;
    }
    QString cleanedRoot = QDir::fromNativeSeparators(QFileInfo(root).absoluteFilePath());
    QString cleanedPath = QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath());
    cleanedRoot = QDir::cleanPath(cleanedRoot);
    cleanedPath = QDir::cleanPath(cleanedPath);
    if (!cleanedRoot.endsWith('/')) {
        cleanedRoot += '/';
    }
    return cleanedPath.startsWith(cleanedRoot, Qt::CaseInsensitive);
}
