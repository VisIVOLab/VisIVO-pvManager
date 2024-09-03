#ifndef MANAGER_H
#define MANAGER_H

#include <QHttpServer>
#include <QObject>

class PVServer;
class Service;

class Manager : public QObject
{
    Q_OBJECT

public:
    Manager(int port, const QString &pvserverFilePath, const QString &workDirPath,
            QObject *parent = nullptr);
    ~Manager() override;

    void start();

private:
    QHttpServer *http;
    const int port;

    // mpirun absolute path
    QString mpirun;

    PVServer *pvserver;
    Service *service;

    void configureMPI();
    bool canRunMPI() const;

    // Manager routes
    QHttpServerResponse routeGetInfo(const QHttpServerRequest &req);
    QHttpServerResponse routeGetServer(const QHttpServerRequest &req);
    QHttpServerResponse routePostServer(const QHttpServerRequest &req);
    QHttpServerResponse routeGetServerLogs(const QHttpServerRequest &req);
    QHttpServerResponse routeGetCutouts(const QHttpServerRequest &req);
};

#endif // MANAGER_H
