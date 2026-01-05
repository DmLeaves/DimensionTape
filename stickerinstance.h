#ifndef STICKERINSTANCE_H
#define STICKERINSTANCE_H

#include <QString>
#include "StickerData.h"

class StickerWidget;

struct StickerInstance {
    QString instanceId;
    QString templateId;
    StickerConfig config;
    StickerWidget *widget = nullptr;
    bool syncToTemplate = true;
};

#endif // STICKERINSTANCE_H
