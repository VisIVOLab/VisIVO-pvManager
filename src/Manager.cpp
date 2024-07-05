#include "Manager.h"
#include "PVServer.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::Literals::StringLiterals;

Manager::Manager(const QString &pvserverFilePath, int port, QObject *parent)
    : QObject(parent), port(port)
{
    qInfo().noquote().nospace() << "Starting " << QCoreApplication::instance()->applicationName()
                                << " v" << QCoreApplication::instance()->applicationVersion();

    this->pvserver = new PVServer(pvserverFilePath, this);
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
    qInfo() << "Stopping" << QCoreApplication::instance()->applicationName();
}

void Manager::start()
{
    if (!this->http->listen(QHostAddress::LocalHost, this->port)) {
        qFatal() << "Could not listen on port" << this->port;
    }
    qInfo() << "Server listening on port" << this->port;

    // GET /server
    this->http->route(u"/server"_s, QHttpServerRequest::Method::Get,
                      [this](const QHttpServerRequest &req) { return this->routeGetServer(req); });
    qInfo() << "Configured GET /server";

    // POST /server
    this->http->route(u"/server"_s, QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest &req) { return this->routePostServer(req); });
    qInfo() << "Configured POST /server";
}

QHttpServerResponse Manager::routeGetServer(const QHttpServerRequest &req)
{
    QJsonObject body;
    auto state = this->pvserver->state();
    body["state"_L1] = QVariant::fromValue(state).toString();
    if (state == QProcess::Running) {
        body["start-time"_L1] = this->pvserver->startedAt().toString(Qt::ISODate);
        body["server-port"_L1] = this->pvserver->serverPort();
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
    if (serverPort <= 0) {
        bodyOut["message"_L1] = "Invalid server port value."_L1;
        return QHttpServerResponse(bodyOut, QHttpServerResponder::StatusCode::BadRequest);
    }

    this->pvserver->start(serverPort);
    bodyOut["message"_L1] = "Server process has started."_L1;
    return QHttpServerResponse(bodyOut);
}
