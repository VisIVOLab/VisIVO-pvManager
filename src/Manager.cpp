#include "Manager.h"

#include "PVServer.h"
#include "Service.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;

Manager::Manager(int port, const QString &pvserverFilePath, const QString &workDirPath,
                 QObject *parent)
    : QObject(parent), port(port)
{
    qInfo().noquote().nospace() << "Starting " << QCoreApplication::instance()->applicationName()
                                << " v" << QCoreApplication::instance()->applicationVersion();

    this->configureMPI();

    this->pvserver = new PVServer(pvserverFilePath, workDirPath, this);
    this->service = new Service(workDirPath, this);
    this->http = new QHttpServer(this);
    this->http->afterRequest([](const QHttpServerRequest &req, QHttpServerResponse &&resp) {
        qInfo().noquote() << "[" << req.remoteAddress().toString() << "]"
                          << QVariant::fromValue(resp.statusCode()).toInt()
                          << QVariant::fromValue(req.method()).toString().toUpper()
                          << req.url().path();
        return std::move(resp);
    });
}

Manager::~Manager()
{
    qInfo().noquote() << "Stopping" << QCoreApplication::instance()->applicationName();
}

void Manager::start()
{
    if (!this->http->listen(QHostAddress::AnyIPv4, this->port)) {
        qFatal() << "Could not listen on port" << this->port;
    }
    qInfo() << "Manager listening on port" << this->port;

    // GET /info
    this->http->route(u"/info"_s, QHttpServerRequest::Method::Get,
                      [this](const QHttpServerRequest &req) { return this->routeGetInfo(req); });
    qInfo() << "Configured GET /info";

    // GET /server
    this->http->route(u"/server"_s, QHttpServerRequest::Method::Get,
                      [this](const QHttpServerRequest &req) { return this->routeGetServer(req); });
    qInfo() << "Configured GET /server";

    // GET /server/logs
    this->http->route(
            u"/server/logs"_s, QHttpServerRequest::Method::Get,
            [this](const QHttpServerRequest &req) { return this->routeGetServerLogs(req); });
    qInfo() << "Configured GET /server/logs";

    // POST /server
    this->http->route(u"/server"_s, QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest &req) { return this->routePostServer(req); });
    qInfo() << "Configured POST /server";
}

void Manager::configureMPI()
{
    QProcess whereis;
    whereis.setProgram(u"whereis"_s);

    QStringList args;
    args << u"-b"_s << u"mpirun"_s;
    whereis.setArguments(args);
    whereis.start();

    if (whereis.waitForFinished()) {
        // output = "mpirun: [path]"
        this->mpirun = whereis.readAllStandardOutput().simplified().split(':')[1].simplified();
        if (!this->mpirun.isEmpty()) {
            qInfo().noquote() << "Found mpirun:" << this->mpirun;
        } else {
            qWarning() << "mpirun not found: it will not be possible to run pvserver using MPI.";
        }
    } else {
        qWarning() << "Could not detect if MPI tools are installed.";
    }
}

bool Manager::canRunMPI() const
{
    return !this->mpirun.isEmpty();
}

QHttpServerResponse Manager::routeGetInfo(const QHttpServerRequest &req)
{
    QJsonObject body;
    body["pvserver"_L1] = this->pvserver->absoluteFilePath();
    body["workdir"_L1] = this->service->workingDirectory();
    body["mpi"_L1] = this->canRunMPI();
    return QHttpServerResponse(body);
}

QHttpServerResponse Manager::routeGetServer(const QHttpServerRequest &req)
{
    QJsonObject body;
    auto state = this->pvserver->state();
    body["state"_L1] = QVariant::fromValue(state).toString();
    if (state == QProcess::Running) {
        body["start-time"_L1] = this->pvserver->startedAt().toString(Qt::ISODate);
        body["server-port"_L1] = this->pvserver->serverPort();

        const bool mpi = this->pvserver->usingMPI();
        body["mpi"_L1] = mpi;
        if (mpi) {
            body["processes"_L1] = this->pvserver->processesMPI();
        }
    }

    return QHttpServerResponse(body);
}

QHttpServerResponse Manager::routePostServer(const QHttpServerRequest &req)
{
    QJsonObject bodyOut;
    if (this->pvserver->state() == QProcess::Running) {
        bodyOut["message"_L1] = "A server instance is already running."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    QJsonObject bodyIn = QJsonDocument::fromJson(req.body()).object();

    if (bodyIn.isEmpty()) {
        bodyOut["message"_L1] = "Invalid request body."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    int serverPort = bodyIn.value("server-port"_L1).toInt();
    if (serverPort <= 0 || serverPort > 65535) {
        bodyOut["message"_L1] = "Invalid server port value."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    bool mpi = bodyIn.value("mpi"_L1).toBool();
    if (mpi && !this->canRunMPI()) {
        bodyOut["message"_L1] = "Cannot run MPI."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    int processes = bodyIn.value("processes"_L1).toInt();
    if (mpi && processes <= 0) {
        bodyOut["message"_L1] = "Invalid processes value."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    if (mpi) {
        this->pvserver->startMPI(serverPort, this->mpirun, processes);
    } else {
        this->pvserver->start(serverPort);
    }
    bodyOut["message"_L1] = "Server process has started."_L1;
    return QHttpServerResponse(bodyOut);
}

QHttpServerResponse Manager::routeGetServerLogs(const QHttpServerRequest &req)
{
    return QHttpServerResponse(this->pvserver->logs());
}

QHttpServerResponse Manager::routeGetCutouts(const QHttpServerRequest &req)
{
    return QHttpServerResponse(QHttpServerResponder::StatusCode::NotImplemented);
}
