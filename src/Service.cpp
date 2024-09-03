#include "Service.h"

#include <QDebug>

Service::Service(const QString &workDir, QObject *parent) : QObject(parent), workDir(workDir)
{
    qInfo() << Q_FUNC_INFO;
}

Service::~Service()
{
    qInfo() << Q_FUNC_INFO;
}

QString Service::workingDirectory() const
{
    return this->workDir;
}
