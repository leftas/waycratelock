#include "src/Command.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <SessionLockQt/command.h>
#include <SessionLockQt/shell.h>
#include <SessionLockQt/window.h>

void
registGlobTypes()
{
    qmlRegisterSingletonType<CommandLine>(
      "MimeApp.Command", 1, 0, "CommandLine", [](QQmlEngine *, QJSEngine *) -> QObject * {
          return new CommandLine;
      });
}

int
main(int argc, char *argv[])
{
    ExtSessionLockV1Qt::Shell::useExtSessionLock();

    QGuiApplication app(argc, argv);
    auto screens = QGuiApplication::screens();

    QQmlApplicationEngine engine;
    registGlobTypes();
    const QUrl url(u"qrc:/MimeApp/qml/main.qml"_qs);
    QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreated,
      &app,
      [url](QObject *obj, const QUrl &objUrl) {
          if (!obj && url == objUrl)
              QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
    engine.load(url);
    if (QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().at(0))) {
        ExtSessionLockV1Qt::Window::registerWindowFromQtScreen(window, screens[0]);
        window->show();
    } else {
        return 0;
    }

    return app.exec();
}
