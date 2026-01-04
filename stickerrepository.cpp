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

bool StickerRepository::load(QList<StickerConfig> &outConfigs, bool &hasData) const
{
    outConfigs.clear();
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
    if (root.contains("stickers") && root["stickers"].isArray()) {
        QJsonArray stickersArray = root["stickers"].toArray();
        for (const QJsonValue &value : stickersArray) {
            if (!value.isObject()) {
                continue;
            }
            StickerConfig config;
            config.fromJson(value.toObject());
            outConfigs.append(config);
        }
        hasData = !outConfigs.isEmpty();
        return true;
    }

    if (root.contains("sticker") && root["sticker"].isObject()) {
        StickerConfig config;
        config.fromJson(root["sticker"].toObject());
        outConfigs.append(config);
        hasData = true;
        return true;
    }

    return true;
}

bool StickerRepository::save(const QList<StickerConfig> &configs) const
{
    QJsonObject root;
    root["version"] = "3.0";
    QJsonArray stickersArray;
    for (const StickerConfig &config : configs) {
        stickersArray.append(config.toJson());
    }
    root["stickers"] = stickersArray;

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
