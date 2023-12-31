#include "CommandLine.h"
#include "MediaPlayerBackend.h"
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#ifdef DEBUG_MODE
#define NON_DEBUG(EXP)                                                                             \
    do {                                                                                           \
    } while (0);
#else
#define NON_DEBUG(EXP) EXP
#include <SessionLockQt/command.h>
#include <SessionLockQt/shell.h>
#include <SessionLockQt/window.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>

QtWaylandClient::QWaylandWindow *oldWindow = nullptr;

int
main(int argc, char *argv[])
{
    NON_DEBUG(ExtSessionLockV1Qt::Shell::useExtSessionLock();)
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");
    auto screens = QGuiApplication::screens();
    const QUrl url(u"qrc:/WayCrateLock/qml/main.qml"_qs);
    QQmlApplicationEngine engine;
    QObject::connect(
      &engine,
      &QQmlApplicationEngine::objectCreated,
      &app,
      [url](QObject *obj, const QUrl &objUrl) {
          if (!obj && url == objUrl)
              QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

    qmlRegisterSingletonType<CommandLine>(
      "WayCrateLock", 1, 0, "CommandLine", [](QQmlEngine *, QJSEngine *) -> QObject * {
          return new CommandLine();
      });
    qmlRegisterSingletonType<MediaPlayerBackend>(
      "WayCrateLock", 1, 0, "MediaPlayerBackend", [](QQmlEngine *, QJSEngine *) -> QObject * {
          return new MediaPlayerBackend();
      });
    auto connectScreen = [&engine, url](auto screen) -> void {
        engine.load(url);
        if (QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().last())) {
            auto input = window->findChild<QQuickItem *>("input");
            QObject::connect(input, &QQuickItem::focusChanged, input, [input](auto focus) {
                if (focus) {
                    auto focusWindow = input->window();
                    auto wFocusWindow =
                      dynamic_cast<QtWaylandClient::QWaylandWindow *>(focusWindow->handle());
                    wFocusWindow->display()->handleWindowActivated(wFocusWindow);
                    if (oldWindow && oldWindow != wFocusWindow) {
                        oldWindow->display()->handleWindowDeactivated(oldWindow);
                    }
                    oldWindow = wFocusWindow;
                }
            });
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
