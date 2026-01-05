#ifndef WINDOWATTACHMENTSERVICE_H
#define WINDOWATTACHMENTSERVICE_H

#include "windowrecognitionservice.h"

class QWidget;

class WindowAttachmentService
{
public:
    WindowAttachmentService();

    void attach(QWidget *widget, WindowHandle target) const;
    void detach(QWidget *widget) const;
    void ensureZOrder(QWidget *widget, WindowHandle target) const;

private:
#ifdef Q_OS_WIN
    static bool isTopMost(void *handle);
    static bool isTargetForeground(void *handle);
    static void *previousInSameGroup(void *handle);
#endif
};

#endif // WINDOWATTACHMENTSERVICE_H
