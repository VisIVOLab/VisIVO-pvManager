#ifndef PVSERVER_H
#define PVSERVER_H

#include <QDateTime>
#include <QObject>
#include <QProcess>

class PVServer : public QObject
{
    Q_OBJECT

public:
    PVServer(const QString &filepath, const QString &workDir, QObject *parent = nullptr);

    QString absoluteFilePath() const;

    void start(int port);
    void startMPI(int port, const QString &mpirun, int processes);
    QProcess::ProcessState state() const;
    QDateTime startedAt() const;
    int serverPort() const;
    bool usingMPI() const;
    int processesMPI() const;
    QByteArray logs() const;

private:
    QString filepath;
    QString workDir;

    QProcess *proc;
    int port;
    bool mpi;
    int processes;
    QString logFile;
    QDateTime startTime;
};

#endif // PVSERVER_H
