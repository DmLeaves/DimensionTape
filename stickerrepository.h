#ifndef STICKERREPOSITORY_H
#define STICKERREPOSITORY_H

#include <QString>
#include "StickerData.h"

class StickerRepository
{
public:
    StickerRepository();

    bool load(StickerConfig &outConfig, bool &hasData) const;
    bool save(const StickerConfig &config) const;
    bool clear() const;

    QString configFilePath() const;

private:
    void ensureDataDirectory();

    QString m_configDirectory;
    QString m_defaultConfigFile;
};

#endif // STICKERREPOSITORY_H
