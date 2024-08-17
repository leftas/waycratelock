#include "Commandline.h"
#include <_pam_types.h>
#include <glob.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandinputdevice_p.h>
#include <private/qwaylandwindow_p.h>
#include <qguiapplication.h>
#include <qinputdevice.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qwindow.h>

#ifdef DEBUG_MODE
#define NON_DEBUG(EXP)                                                                             \
    do {                                                                                           \
    } while (0);
#else
#define NON_DEBUG(EXP) EXP
#include <SessionLockQt/command.h>
#endif

#include <QCoreApplication>
#include <QDate>
#include <QGuiApplication>
#include <QLocale>
#include <QTimer>
#include <QtConcurrent>

#include <xkbcommon/xkbcommon-names.h>
#include <xkbcommon/xkbcommon.h>

#include "access_private.hpp"
#include <format>
#include <mutex>
#include <string>
#include <toml++/toml.h>
#include <unistd.h>

constexpr static std::string CONFIG_FILE = "config.toml";
constexpr static std::string CONFIGDIR   = "waycratelock";

static QString get_config_path() {
    return QString::fromStdString(std::format(
        "{}/{}/{}", QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(),
        CONFIGDIR, CONFIG_FILE));
}

enum PamStatus {
    PamFailed,
    Successful,
    AuthFailed,
};

static int handle_conversation(int num_msg, const struct pam_message** msg,
                               struct pam_response** resp, void* data) {
    /* PAM expects an array of responses, one for each message */
    auto thisptr = static_cast<Commandline*>(data);

    struct pam_response* pam_reply =
        static_cast<struct pam_response*>(calloc(num_msg, sizeof(struct pam_response)));
    if (pam_reply == nullptr) {
        return PAM_ABORT;
    }

    *resp = pam_reply;
    for (int i = 0; i < num_msg; ++i) {
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON:
            pam_reply[i].resp = strdup(thisptr->password().toLocal8Bit().data()); // PAM clears and
                                                                                  // frees this
            if (pam_reply[i].resp == NULL) {
                return PAM_ABORT;
            }
            break;
        case PAM_ERROR_MSG:
            thisptr->showTimedMessage(msg[i]->msg, true);
        case PAM_TEXT_INFO:
            thisptr->showTimedMessage(msg[i]->msg, false, 6000);
            break;
        }
    }
    return PAM_SUCCESS;
}

// Some hacky way of getting xkb state of wayland keyboard.
namespace access_private {
template struct access<&QtWaylandClient::QWaylandInputDevice::Keyboard::mXkbState>;
}

void Commandline::checkCapsLock() {
    auto win = qGuiApp->focusWindow();
    if (!win)
        return;

    auto wWin = win->nativeInterface<QtWaylandClient::QWaylandWindow>();
    if (!wWin)
        return;

    auto display = wWin->display();
    if (!display)
        return;

    auto inputDevice = display->currentInputDevice();
    if (!inputDevice)
        return;

    auto keyboard = inputDevice->keyboard();
    if (!keyboard)
        return;

    auto& xkbstate = access_private::accessor<"mXkbState">(keyboard);

    bool isCapsLocked =
        xkb_state_mod_name_is_active(xkbstate.get(), XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED);

    setCapsLocked(isCapsLocked);
}

Commandline::Commandline(QObject* parent)
    : QObject(parent), m_usePam(true), m_backgroundImagePath(QUrl("qrc:/image/gangdamu.png")),
      m_opacity(1) {
    m_username = QString::fromStdString(getlogin());
    readConfig();
    auto app = qGuiApp;
    QObject::connect(
        app, &QGuiApplication::focusWindowChanged, app,
        [this](QWindow* window) { checkCapsLock(); }, Qt::SingleShotConnection);
    if (!m_usePam) {
        return;
    }
    startPam();
}

Commandline::~Commandline() {
    for (auto handle : m_handles.asKeyValueRange()) {
        pam_end(handle.second, PAM_SUCCESS);
        handle.second = nullptr;
    }
    for (auto guard : m_guards.values()) {
        delete guard;
    }
}

void Commandline::startPam() {
    const struct pam_conv conv = {
        .conv        = &handle_conversation,
        .appdata_ptr = this,
    };
    bool strict = true; // First one should be waycratelock
    for (auto name : m_handles.keys()) {
        auto* handle = initPam(
            name.toLocal8Bit().data(), &conv,
            m_username.toLocal8Bit().data()); // pam_start copies and clears username pointer.
        if (handle != nullptr) {
            m_handles[name] = handle;
            m_guards.insert(name, new std::mutex());
            // if (!strict)
            //     QTimer::singleShot(
            //       5, this, [this, handle, name] { runPamUnlock(handle, m_guards[name], true); });
        } else {
            if (strict) {
                qWarning() << "Cannot start pam";
                QTimer::singleShot(0, this, [this] { this->unlock(); });
                return;
            }
            qWarning() << "Cannot start pam for" << name;
            m_handles.remove(name);
        }
        strict = false;
    }
}

pam_handle_t*
Commandline::initPam(const char* pam_name, const struct pam_conv* conv, const char* username) {
    pam_handle_t* handle = nullptr;
    if (pam_start(pam_name, username, conv, &handle) != PAM_SUCCESS)
        return nullptr;
    return handle;
}

QString Commandline::parsePath(const QString& string) {
    QString result;
    glob_t globbuf;
    if (glob(string.toStdString().c_str(), GLOB_TILDE, NULL, &globbuf) == 0) {
        if (globbuf.gl_pathc > 0) {
            result = QString(globbuf.gl_pathv[0]);
        }
    }
    globfree(&globbuf);
    return result;
}

void Commandline::readConfig() {
    QString configpath = get_config_path();
    if (!QFile(configpath).exists()) {
        return;
    }
    try {
        auto tbl                              = toml::parse_file(configpath.toStdString());
        std::optional<bool> usePam            = tbl["general"]["needPassword"].value<bool>();
        std::optional<std::string> pamConfigs = tbl["general"]["pamConfigs"].value<std::string>();
        std::optional<std::string> background = tbl["background"]["path"].value<std::string>();
        std::optional<double> opacity         = tbl["background"]["opacity"].value<float>();
        std::optional<int> fadeIn             = tbl["background"]["fadein"].value<int>();
        std::optional<int> fadeOut            = tbl["background"]["fadeout"].value<int>();

        m_opacity = opacity.value_or(1);
        m_usePam  = usePam.value_or(true);
        m_fadeIn  = fadeIn.value_or(0);
        m_fadeOut = fadeOut.value_or(0);

        if (pamConfigs.has_value()) {
            auto names = QString::fromStdString(pamConfigs.value());
            for (auto name : names.split(",")) {
                m_handles.insert(name.trimmed(), nullptr);
            }
        }

        if (background.has_value()) {
            auto backgroundImg    = parsePath(QString::fromStdString(background.value()));
            m_backgroundImagePath = QUrl::fromLocalFile(backgroundImg);
        }
    } catch (const toml::parse_error& err) {
        showTimedMessage("Something error with config file", true);
        qWarning() << err.what();
    }
}

void Commandline::setPassword(const QString& password) {
    m_password = password;
    Q_EMIT passwordChanged();
}

void Commandline::unlock() {
    setExiting(true);
    QTimer::singleShot(fadeOut(), qApp,
                       [] { NON_DEBUG(ExtSessionLockV1Qt::Command::instance()->unlockScreen();) });
    QTimer::singleShot(fadeOut() + 1, qApp, [] { QGuiApplication::quit(); });
}

void Commandline::showTimedMessage(QString message, bool error, int timeSeconds) {
    auto msg = Message(message, error);
    messages()->addMessage(msg);
    QTimer::singleShot(timeSeconds, this, [this, msg] { messages()->removeMessage(msg); });
}

void Commandline::runPamUnlock(auto* handle, std::mutex* mutex, bool silent) {
    if (!silent)
        setBusy(true);

    QtConcurrent::run([mutex, handle, silent] {
        std::lock_guard guard{*mutex};
        int pam_status = pam_authenticate(handle, silent ? PAM_DISALLOW_NULL_AUTHTOK : 0);
        if (pam_status != PAM_SUCCESS) {
            return PamStatus::AuthFailed;
        }
        pam_setcred(handle, PAM_REFRESH_CRED);
        return PamStatus::Successful;
    }).then([this, silent](PamStatus value) {
        if (!silent)
            setBusy(false);
        switch (value) {
        case PamFailed: {
            if (!silent)
                showTimedMessage("Pam end failed!", true);
        }
        case Successful: {
            unlock();
            break;
        }
        case AuthFailed: {
            if (!silent)
                showTimedMessage("Failed to unlock. Check your password", true);
            break;
        }
        }
    });
}

void Commandline::requestUnlock() {
    if (!m_usePam) {
        unlock();
        return;
    }

    runPamUnlock(m_handles["waycratelock"], m_guards["waycratelock"], false);
}
