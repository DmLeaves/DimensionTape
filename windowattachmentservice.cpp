#include "windowattachmentservice.h"
#include <QWidget>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

WindowAttachmentService::WindowAttachmentService()
{
}

void WindowAttachmentService::attach(QWidget *widget, WindowHandle target) const
{
#ifdef Q_OS_WIN
    if (!widget || target == 0) {
        return;
    }

    HWND self = reinterpret_cast<HWND>(widget->winId());
    HWND hwndTarget = reinterpret_cast<HWND>(target);
    if (!IsWindow(self) || !IsWindow(hwndTarget)) {
        return;
    }

    SetWindowLongPtrW(self, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(hwndTarget));
    ensureZOrder(widget, target);
#else
    Q_UNUSED(widget)
    Q_UNUSED(target)
#endif
}

void WindowAttachmentService::detach(QWidget *widget) const
{
#ifdef Q_OS_WIN
    if (!widget) {
        return;
    }
    HWND self = reinterpret_cast<HWND>(widget->winId());
    if (!IsWindow(self)) {
        return;
    }
    SetWindowLongPtrW(self, GWLP_HWNDPARENT, 0);
#else
    Q_UNUSED(widget)
#endif
}

void WindowAttachmentService::ensureZOrder(QWidget *widget, WindowHandle target) const
{
#ifdef Q_OS_WIN
    if (!widget || target == 0) {
        return;
    }

    HWND self = reinterpret_cast<HWND>(widget->winId());
    HWND hwndTarget = reinterpret_cast<HWND>(target);
    if (!IsWindow(self) || !IsWindow(hwndTarget)) {
        return;
    }

    bool targetTop = isTopMost(hwndTarget);
    SetWindowPos(self, targetTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    HWND prev = reinterpret_cast<HWND>(previousInSameGroup(hwndTarget));
    if (prev && prev != self) {
        SetWindowPos(self, prev, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#else
    Q_UNUSED(widget)
    Q_UNUSED(target)
#endif
}

#ifdef Q_OS_WIN
bool WindowAttachmentService::isTopMost(void *handle)
{
    HWND hwnd = reinterpret_cast<HWND>(handle);
    LONG ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    return (ex & WS_EX_TOPMOST) != 0;
}

bool WindowAttachmentService::isTargetForeground(void *handle)
{
    HWND target = reinterpret_cast<HWND>(handle);
    if (!IsWindow(target)) {
        return false;
    }
    HWND fg = GetForegroundWindow();
    if (!fg) {
        return false;
    }
    HWND fgRoot = GetAncestor(fg, GA_ROOTOWNER);
    HWND targetRoot = GetAncestor(target, GA_ROOTOWNER);
    return fgRoot == targetRoot;
}

void *WindowAttachmentService::previousInSameGroup(void *handle)
{
    HWND start = reinterpret_cast<HWND>(handle);
    if (!IsWindow(start)) {
        return nullptr;
    }
    bool targetTop = isTopMost(start);
    HWND prev = GetWindow(start, GW_HWNDPREV);
    while (prev) {
        if (isTopMost(prev) == targetTop) {
            return prev;
        }
        prev = GetWindow(prev, GW_HWNDPREV);
    }
    return nullptr;
}
#endif
