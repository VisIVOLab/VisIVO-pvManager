#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QUrl>
#include <QDateTime>

struct Cutout
{
    QUrl url;
    QString filename;
    int status;
    QDateTime submissionTime;
    QDateTime endTime;
};

class Service : public QObject
{
    Q_OBJECT
public:
    explicit Service(const QString &workDir, QObject *parent = nullptr);
    ~Service() override;

    QString workingDirectory() const;

private:
    const QString workDir;
    QList<Cutout> cutouts;
};

#endif // SERVICE_H
