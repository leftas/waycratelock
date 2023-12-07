#ifdef DEBUG_MODE
#define NON_DEBUG(EXP)                                                                             \
    do {                                                                                           \
    } while (0)
#else
#define NON_DEBUG(EXP) EXP
#include <SessionLockQt/command.h>
#include <SessionLockQt/shell.h>
#include <SessionLockQt/window.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickStyle>
#include <QQuickItem>
#include <QWindow>

#include <algorithm>
#include <cstdlib>

int
main(int argc, char *argv[])
{
    NON_DEBUG(ExtSessionLockV1Qt::Shell::useExtSessionLock();)

    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    QQmlApplicationEngine engine;
    auto screens = QGuiApplication::screens();
    const QUrl url(u"qrc:/WayCrateLock/qml/main.qml"_qs);

    QObject::connect(&app,
                     &QGuiApplication::focusWindowChanged,
                     &app,
                     [&app](QWindow* window) {
                        // window->findChild<QQuickItem*>("root")->forceActiveFocus(Qt::FocusReason::ActiveWindowFocusReason);
                     });
    QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreated,
      &app,
      [url](QObject *obj, const QUrl &objUrl) {
          if (!obj && url == objUrl)
              QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

    auto connectScreen = [&engine, url](auto screen) -> void {
        engine.load(url);
        if (QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().last())) {
            window->setScreen(screen);
            window->setGeometry(screen->geometry());
            NON_DEBUG(ExtSessionLockV1Qt::Window::registerWindowFromQtScreen(window, screen);)
            window->show();
        } else {
            qDebug() << "Cannot get window";
            exit(0);
        }
    };
    std::reverse(screens.begin(), screens.end());
    for (auto screen : screens) {
        connectScreen(screen);
    }

    QObject::connect(&app, &QGuiApplication::screenAdded, &app, [connectScreen](auto screen) {
        connectScreen(screen);
        NON_DEBUG(ExtSessionLockV1Qt::Command::instance()->LockScreen();)
    });

    NON_DEBUG(ExtSessionLockV1Qt::Command::instance()->LockScreen();)

    return app.exec();
}
