#ifndef MANAGER_H
#define MANAGER_H

#include <QHttpServer>
#include <QObject>

class PVServer;

class Manager : public QObject
{
    Q_OBJECT

public:
    Manager(const QString &pvserverFilePath, int port, QObject *parent = nullptr);
    ~Manager() override;

    void start();

private:
    int port;
    QHttpServer *http;
    PVServer *pvserver;

    QHttpServerResponse routePostServer(const QHttpServerRequest &req);
};

#endif // MANAGER_H
