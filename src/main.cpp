#include "Manager.h"
#include "version.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

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

    // --help
    QCommandLineOption optHelp(QStringList{ u"h"_s, u"help"_s });
    optHelp.setDescription(u"Displays help on commandline options."_s);

    // --port <port>
    QCommandLineOption optPort(QStringList{ u"p"_s, u"port"_s });
    optPort.setDescription(u"The port the manager listens on (default 11110)."_s);
    optPort.setValueName(u"port"_s);
    optPort.setDefaultValue(u"11110"_s);

    // --workdir <workdir>
    QCommandLineOption optWorkDir(QStringList{ u"d"_s, u"workdir"_s });
    optWorkDir.setDescription(
            u"Directory where to save logs and cutouts (default $TMPDIR or /tmp)."_s);
    optWorkDir.setValueName(u"workdir"_s);
    optWorkDir.setDefaultValue(QDir::tempPath());

    QCommandLineParser parser;
    parser.setApplicationDescription(u"VisIVO Manager for pvserver instances."_s);
    parser.addPositionalArgument(u"pvserver"_s, u"Path to pvserver."_s);
    parser.addOption(optPort);
    parser.addOption(optWorkDir);
    parser.addOption(optHelp);
    parser.addVersionOption();
    parser.process(app);

    if (parser.isSet(optHelp)) {
        parser.showHelp();
        // showHelp also exits the program
    }

    bool okPort;
    const int port = parser.value(optPort).toInt(&okPort);
    if (!okPort || port <= 0 || port > 65535) {
        qCritical() << "Invalid port value:" << port;
        return 1;
    }

    const QFileInfo workDir(parser.value(optWorkDir));
    if (!workDir.exists() || !workDir.isDir() || !workDir.isWritable()) {
        qCritical().noquote() << "Invalid working directory:" << workDir.absoluteFilePath();
        return 1;
    }

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qCritical() << "You must specify a path to the pvserver executable.";
        parser.showHelp(1);
    }

    const QFileInfo pvserver(parser.positionalArguments().first());
    if (!pvserver.exists() || !pvserver.isFile() || !pvserver.isExecutable()) {
        qCritical().noquote() << "Invalid pvserver path:" << pvserver.absoluteFilePath();
        return 1;
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

    Manager mgr(port, pvserver.absoluteFilePath(), workDir.absoluteFilePath());
    mgr.start();

    return app.exec();
}
