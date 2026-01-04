#ifndef STICKERREPOSITORY_H
#define STICKERREPOSITORY_H

#include <QString>
#include <QList>
#include "StickerData.h"

class StickerRepository
{
public:
    StickerRepository();

    bool load(QList<StickerConfig> &outConfigs, bool &hasData) const;
    bool save(const QList<StickerConfig> &configs) const;
    bool clear() const;

    QString configFilePath() const;

private:
    void ensureDataDirectory();

    QString m_configDirectory;
    QString m_defaultConfigFile;
};

#endif // STICKERREPOSITORY_H
