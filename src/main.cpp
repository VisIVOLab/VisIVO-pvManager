#include "Manager.h"
#include "version.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>

#include <csignal>

using namespace Qt::Literals::StringLiterals;

void signalHandler(int signal)
{
    QCoreApplication::instance()->quit();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Set locale to C.UTF-8
    std::setlocale(LC_NUMERIC, "C");
    QLocale::setDefault(QLocale::c());

    // Set application info
    QCoreApplication::setOrganizationName(u"Osservatorio Astrofisico di Catania"_s);
    QCoreApplication::setOrganizationDomain(u"oact.inaf.it"_s);
    QCoreApplication::setApplicationName(u"VisIVO pvManager"_s);
    QCoreApplication::setApplicationVersion(QStringLiteral(VISIVO_MANAGER_VERSION_STRING));

    // Set signal handlers to stop the event loop
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);

    // --port <port> arg
    QCommandLineOption optPort(QStringList{ u"p"_s, u"port"_s });
    optPort.setDescription(u"The port the server listens on."_s);
    optPort.setValueName(u"port"_s);
    optPort.setDefaultValue(u"11110"_s);

    // <pvserver> arg
    QCommandLineParser parser;
    parser.addPositionalArgument(u"pvserver"_s, u"Path to the pvserver binary."_s);
    parser.addOption(optPort);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    bool okPort;
    const int port = parser.value(optPort).toInt(&okPort);
    if (!okPort) {
        qFatal() << "Invalid port value: " << port;
    }

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qFatal() << "You must specify a path to the pvserver executable.";
    }

    const QFileInfo filepath(parser.positionalArguments().first());
    if (!filepath.exists() || !filepath.isFile() || !filepath.isExecutable()) {
        qFatal() << "Invalid pvserver path:" << filepath.absoluteFilePath();
    }

    // Customize QDebug message format from now on
    qSetMessagePattern("%{time} "
                       "%{if-debug}D %{endif}"
                       "%{if-info}I %{endif}"
                       "%{if-warning}W %{endif}"
                       "%{if-critical}C %{endif}"
                       "%{if-fatal}F %{endif}"
#ifndef NDEBUG
                       "%{file} %{function}:%{line} "
#endif
                       "%{if-category}%{category} %{endif}%{message}");

    Manager mgr(filepath.absoluteFilePath(), port);
    mgr.start();

    return app.exec();
}
