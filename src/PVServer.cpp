#include "PVServer.h"

using namespace Qt::Literals::StringLiterals;

PVServer::PVServer(const QString &filepath, QObject *parent) : QObject(parent), filepath(filepath)
{
    this->proc = new QProcess;
    this->proc->setProcessChannelMode(QProcess::MergedChannels);
    this->proc->setProgram(this->filepath);
    QObject::connect(this->proc, &QProcess::started, this, [this]() {
        qDebug() << "pvserver [" << this->proc->processId() << "] started";
    });
    QObject::connect(this->proc, &QProcess::finished, this->proc,
                     [](int exitCode) { qDebug() << "pvserver finished, exit code" << exitCode; });

    QObject::connect(this, &QObject::destroyed, this->proc, &QProcess::terminate);
}

void PVServer::start(int port)
{
    if (this->proc->state() == QProcess::Running) {
        return;
    }

    QStringList args;
    args << u"--server-port"_s << QString::number(port);
    this->proc->setArguments(args);

    this->startTime = QDateTime::currentDateTimeUtc();
    this->logFile = QString("pvserver-%1.log").arg(this->startTime.toString(Qt::ISODate));
    this->proc->setStandardOutputFile(this->logFile);
    this->proc->start();
    this->proc->waitForStarted();
}

QProcess::ProcessState PVServer::state() const
{
    return this->proc->state();
}
