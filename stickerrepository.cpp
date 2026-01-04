#include "StickerRepository.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDebug>

StickerRepository::StickerRepository()
{
    ensureDataDirectory();
}

void StickerRepository::ensureDataDirectory()
{
    m_configDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_configDirectory);
    m_defaultConfigFile = QDir(m_configDirectory).filePath("sticker.json");
}

QString StickerRepository::configFilePath() const
{
    return m_defaultConfigFile;
}

bool StickerRepository::load(StickerConfig &outConfig, bool &hasData) const
{
    hasData = false;
    QFile file(m_defaultConfigFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "配置文件解析错误:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    if (root.contains("sticker") && root["sticker"].isObject()) {
        outConfig.fromJson(root["sticker"].toObject());
        hasData = true;
        return true;
    }

    if (root.contains("stickers") && root["stickers"].isArray()) {
        QJsonArray stickersArray = root["stickers"].toArray();
        if (!stickersArray.isEmpty() && stickersArray.first().isObject()) {
            outConfig.fromJson(stickersArray.first().toObject());
            hasData = true;
            return true;
        }
    }

    return true;
}

bool StickerRepository::save(const StickerConfig &config) const
{
    QJsonObject root;
    root["version"] = "2.0";
    root["sticker"] = config.toJson();

    QJsonDocument doc(root);

    QFile file(m_defaultConfigFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法写入配置文件:" << m_defaultConfigFile;
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

bool StickerRepository::clear() const
{
    if (!QFile::exists(m_defaultConfigFile)) {
        return true;
    }
    return QFile::remove(m_defaultConfigFile);
}
