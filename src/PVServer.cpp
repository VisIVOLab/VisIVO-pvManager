#include "PVServer.h"

#include <QDir>

using namespace Qt::Literals::StringLiterals;

PVServer::PVServer(const QString &filepath, const QString &workDir, QObject *parent)
    : QObject(parent), filepath(filepath), workDir(workDir)
{
    this->proc = new QProcess;
    this->proc->setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(this->proc, &QProcess::started, this, [this]() {
        qInfo().nospace() << "pvserver(" << this->proc->processId() << ") started";
    });
    QObject::connect(this->proc, &QProcess::finished, this->proc,
                     [](int exitCode) { qInfo() << "pvserver finished, exit code" << exitCode; });

    QObject::connect(this, &QObject::destroyed, this->proc, &QProcess::terminate);
}

QString PVServer::absoluteFilePath() const
{
    return this->filepath;
}

void PVServer::start(int port)
{
    if (this->proc->state() == QProcess::Running) {
        return;
    }

    QStringList args;
    args << u"--server-port"_s << QString::number(port);
    this->proc->setProgram(this->filepath);
    this->proc->setArguments(args);
    this->port = port;
    this->mpi = false;
    this->startTime = QDateTime::currentDateTimeUtc();
    this->logFile =
            QDir(this->workDir)
                    .absoluteFilePath(
                            QString("pvserver-%1.log").arg(this->startTime.toString(Qt::ISODate)));
    this->proc->setStandardOutputFile(this->logFile);
    this->proc->start();
    this->proc->waitForStarted();
}

void PVServer::startMPI(int port, const QString &mpirun, int processes)
{
    if (this->proc->state() == QProcess::Running) {
        return;
    }

    QStringList args;
    args << u"-n"_s << QString::number(processes) << this->filepath << u"--server-port"_s
         << QString::number(port);
    this->proc->setProgram(mpirun);
    this->proc->setArguments(args);
    this->port = port;
    this->mpi = true;
    this->processes = processes;
    this->startTime = QDateTime::currentDateTimeUtc();
    this->logFile =
            QDir(this->workDir)
                    .absoluteFilePath(
                            QString("pvserver-%1.log").arg(this->startTime.toString(Qt::ISODate)));
    this->proc->setStandardOutputFile(this->logFile);
    this->proc->start();
    this->proc->waitForStarted();
}

QProcess::ProcessState PVServer::state() const
{
    return this->proc->state();
}

QDateTime PVServer::startedAt() const
{
    return this->startTime;
}

int PVServer::serverPort() const
{
    return this->port;
}

bool PVServer::usingMPI() const
{
    return this->mpi;
}

int PVServer::processesMPI() const
{
    return this->processes;
}

QByteArray PVServer::logs() const
{
    QFile logsFile(this->logFile);
    if (logsFile.exists()) {
        logsFile.open(QIODevice::ReadOnly);
        const auto logs = logsFile.readAll();
        logsFile.close();
        return logs;
    }

    return {};
}
