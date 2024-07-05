#ifndef PVSERVER_H
#define PVSERVER_H

#include <QDateTime>
#include <QObject>
#include <QProcess>

class PVServer : public QObject
{
    Q_OBJECT

public:
    PVServer(const QString &filepath, QObject *parent = nullptr);

    void start(int port);
    QProcess::ProcessState state() const;

private:
    QProcess *proc;
    uint port;
    QString filepath;
    QString logFile;
    QDateTime startTime;
};

#endif // PVSERVER_H
