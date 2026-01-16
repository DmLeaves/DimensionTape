#ifndef STICKERASSETSTORE_H
#define STICKERASSETSTORE_H

#include <QString>

class StickerAssetStore
{
public:
    StickerAssetStore();

    QString tapesRoot() const;
    QString modulesRoot() const;

    QString importImage(const QString &sourcePath, QString *error = nullptr) const;
    QString importLive2DModel(const QString &modelJsonPath, QString *error = nullptr) const;

private:
    QString ensureSubdir(const QString &name) const;
    QString uniqueFilePath(const QString &dirPath, const QString &fileName) const;
    QString uniqueDirPath(const QString &dirPath, const QString &baseName) const;
    bool copyDirRecursive(const QString &sourceDir, const QString &targetDir, QString *error) const;
    bool isPathUnderRoot(const QString &path, const QString &root) const;

    QString m_rootDir;
    QString m_tapesDir;
    QString m_modulesDir;
};

#endif // STICKERASSETSTORE_H
