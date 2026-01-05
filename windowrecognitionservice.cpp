#include "windowrecognitionservice.h"
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
#ifdef Q_OS_WIN
QString windowClassName(HWND hwnd)
{
    wchar_t buf[256] = {};
    if (GetClassNameW(hwnd, buf, 255) > 0) {
        return QString::fromWCharArray(buf);
    }
    return QString();
}

QString windowTitle(HWND hwnd)
{
    wchar_t buf[512] = {};
    int len = GetWindowTextW(hwnd, buf, 511);
    if (len > 0) {
        return QString::fromWCharArray(buf, len).trimmed();
    }
    return QString();
}

QString processName(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) {
        return QString();
    }

    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc) {
        return QString();
    }

    wchar_t path[MAX_PATH] = {};
    DWORD len = MAX_PATH;
    QString result;

    typedef BOOL (WINAPI *QueryFullProcessImageNameWFunc)(HANDLE, DWORD, LPWSTR, PDWORD);
    HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
    if (hKernel) {
        auto queryFunc = reinterpret_cast<QueryFullProcessImageNameWFunc>(
            GetProcAddress(hKernel, "QueryFullProcessImageNameW"));
        if (queryFunc && queryFunc(hProc, 0, path, &len)) {
            result = QString::fromWCharArray(path);
            int lastSlash = result.lastIndexOf('\\');
            if (lastSlash >= 0) {
                result = result.mid(lastSlash + 1);
            }
        }
    }

    CloseHandle(hProc);
    return result;
}

bool isTopMost(HWND hwnd)
{
    LONG ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    return (ex & WS_EX_TOPMOST) != 0;
}

QRect rectFromWinRect(const RECT &r)
{
    return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto *list = reinterpret_cast<QList<HWND>*>(lParam);
    if (IsWindow(hwnd)) {
        list->append(hwnd);
    }
    return TRUE;
}
#endif
}

WindowRecognitionService::WindowRecognitionService(QObject *parent)
    : QObject(parent)
{
}

QList<WindowInfo> WindowRecognitionService::listWindows(bool visibleOnly) const
{
    QList<WindowInfo> result;

#ifdef Q_OS_WIN
    QList<HWND> handles;
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&handles));

    for (HWND hwnd : handles) {
        if (visibleOnly && !IsWindowVisible(hwnd)) {
            continue;
        }
        WindowInfo info;
        info.handle = reinterpret_cast<WindowHandle>(hwnd);
        info.visible = IsWindowVisible(hwnd);
        info.minimized = IsIconic(hwnd);
        info.topMost = isTopMost(hwnd);
        info.title = windowTitle(hwnd);
        info.className = windowClassName(hwnd);
        info.processName = processName(hwnd);

        RECT rr;
        if (GetWindowRect(hwnd, &rr)) {
            info.rect = rectPhysicalToLogical(rectFromWinRect(rr), info.handle);
        }
        result.append(info);
    }
#else
    Q_UNUSED(visibleOnly)
#endif

    return result;
}

WindowInfo WindowRecognitionService::queryWindow(WindowHandle handle) const
{
    WindowInfo info;
    info.handle = handle;

#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(handle);
    if (!IsWindow(hwnd)) {
        return info;
    }
    info.visible = IsWindowVisible(hwnd);
    info.minimized = IsIconic(hwnd);
    info.topMost = isTopMost(hwnd);
    info.title = windowTitle(hwnd);
    info.className = windowClassName(hwnd);
    info.processName = processName(hwnd);

    RECT rr;
    if (GetWindowRect(hwnd, &rr)) {
        info.rect = rectPhysicalToLogical(rectFromWinRect(rr), handle);
    }
#endif

    return info;
}

bool WindowRecognitionService::isWindowValid(WindowHandle handle) const
{
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(handle);
    return IsWindow(hwnd) != FALSE;
#else
    Q_UNUSED(handle)
    return false;
#endif
}

qreal WindowRecognitionService::dpiScaleForWindow(WindowHandle handle)
{
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(handle);
    typedef UINT (WINAPI *GetDpiForWindowFunc)(HWND);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        auto f = reinterpret_cast<GetDpiForWindowFunc>(GetProcAddress(hUser32, "GetDpiForWindow"));
        if (f && IsWindow(hwnd)) {
            UINT dpi = f(hwnd);
            if (dpi >= 96) {
                return qreal(dpi) / 96.0;
            }
        }
    }
#endif

    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    qreal dpr = screen ? screen->devicePixelRatio() : qApp->devicePixelRatio();
    return (dpr > 0 ? dpr : 1.0);
}

QRectF WindowRecognitionService::rectPhysicalToLogical(const QRect &rect, WindowHandle handle)
{
    qreal dpr = dpiScaleForWindow(handle);
    if (dpr <= 0) {
        dpr = 1.0;
    }
    return QRectF(rect.left() / dpr,
                  rect.top() / dpr,
                  rect.width() / dpr,
                  rect.height() / dpr);
}
