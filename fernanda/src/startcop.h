// startcop.h, Fernanda

#pragma once

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <Qt>
#include <QString>
#include <QWidget>

class StartCop : public QObject
{
    Q_OBJECT

public:
    StartCop(const QString& name)
        : name(name) {}

    bool exists()
    {
        if (serverExists()) return true;
        startServer();
        return false;
    }

private:
    const QString name;

    bool serverExists()
    {
        QLocalSocket socket;
        socket.connectToServer(name);
        bool exists = socket.isOpen();
        socket.close();
        return exists;
    }

    void startServer()
    {
        QLocalServer* server = new QLocalServer;
        server->setSocketOptions(QLocalServer::WorldAccessOption);
        server->listen(name);
        connect(server, &QLocalServer::newConnection, this, &StartCop::focusMainWindow);
    }

private slots:
    void focusMainWindow()
    {
        for (auto& widget : QApplication::allWidgets())
        {
            if (widget->objectName() != "mainWindow") continue;
            if (widget->windowState() == Qt::WindowMinimized)
                widget->setWindowState((widget->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            widget->activateWindow();
            break;
        }
    }
};

// startcop.h, Fernanda
