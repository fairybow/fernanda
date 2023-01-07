// startcop.h, Fernanda

#pragma once

#include <string>

#include <qsystemdetection.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QObject>
#include <QSharedMemory>
#include <QString>
#include <QStringList>
#include <QSystemSemaphore>

#ifdef Q_OS_WINDOWS

#include <windows.h>
#include <winuser.h>

#else

//

#endif

QStringList windowNames;

#ifdef Q_OS_WINDOWS

static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lParam)
{
    int length = GetWindowTextLength(hWnd);
    wchar_t* buffer = new wchar_t[length + 1];
    GetWindowText(hWnd, buffer, length + 1);
    std::wstring window_title(buffer);
    delete[] buffer;
    if (IsWindowVisible(hWnd) && length > 1) {
        windowNames << QString::fromStdWString(window_title);
    }
    return TRUE;
}

void setFocus_OS_Windows()
{
    EnumWindows(enumWindowCallback, 0);
    for (auto& window_name : windowNames)
    {
        if (window_name.contains("Fernanda"))
        {
            auto name = window_name.toStdString();
            auto wstr_name = std::wstring(name.begin(), name.end());
            auto handle = FindWindow(0, wstr_name.c_str());
            if (IsIconic(handle))
                ShowWindow(handle, SW_RESTORE);
            SetForegroundWindow(handle);
        }
    }
}

#else

void setFocus_OS_Linux()
{
    return;
}

#endif

class StartCop
{

public:
    StartCop(const QString& key)
        : key(key)
        , memoryLockKey(generateKeyHash(key, "_memoryLockKey"))
        , sharedMemoryKey(generateKeyHash(key, "_sharedMemoryKey"))
        , sharedMemory(sharedMemoryKey)
        , memoryLock(memoryLockKey, 1)
    {
        memoryLock.acquire();
        {
            QSharedMemory fix(sharedMemoryKey);
            fix.attach();
        }
        memoryLock.release();
    }

    ~StartCop() { release(); }

    bool exists()
    {
        if (alreadyRunning()) return true;
        memoryLock.acquire();
        const bool result = sharedMemory.create(sizeof(quint64));
        memoryLock.release();
        if (!result)
        {
            release();
            return true;
        }
        return false;
    }

    static void setFocus()
    {

#ifdef Q_OS_WINDOWS

        setFocus_OS_Windows();

#else

        setFocus_OS_Linux();

#endif
    
    }

private:
    const QString key;
    const QString memoryLockKey;
    const QString sharedMemoryKey;
    QSharedMemory sharedMemory;
    QSystemSemaphore memoryLock;

    QString generateKeyHash(const QString& key, const QString& salt)
    {
        QByteArray data;
        data.append(key.toUtf8());
        data.append(salt.toUtf8());
        data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
        return data;
    }

    bool alreadyRunning()
    {
        if (sharedMemory.isAttached()) return false;
        memoryLock.acquire();
        const bool is_running = sharedMemory.attach();
        if (is_running)
            sharedMemory.detach();
        memoryLock.release();
        return is_running;
    }

    void release()
    {
        memoryLock.acquire();
        if (sharedMemory.isAttached())
            sharedMemory.detach();
        memoryLock.release();
    }

    //Q_DISABLE_COPY(StartCop)
};

// startcop.h, Fernanda
