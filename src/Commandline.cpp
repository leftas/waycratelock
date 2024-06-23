#include "Commandline.h"
#include <glob.h>
#include <qlogging.h>
#include <qnamespace.h>

#ifdef DEBUG_MODE
#define NON_DEBUG(EXP)                                                                             \
    do {                                                                                           \
    } while (0);
#else
#define NON_DEBUG(EXP) EXP
#include <SessionLockQt/command.h>
#endif

#include <QDate>
#include <QGuiApplication>
#include <QLocale>
#include <QTimer>
#include <QtConcurrent>

#include <format>
#include <mutex>
#include <string>
#include <toml++/toml.h>
#include <unistd.h>

constexpr static std::string CONFIG_FILE = "config.toml";
constexpr static std::string CONFIGDIR   = "waycratelock";

static std::mutex PAM_GUARD;

static QString
get_config_path()
{
    return QString::fromStdString(
      std::format("{}/{}/{}",
                  QStandardPaths::writableLocation(QStandardPaths::ConfigLocation).toStdString(),
                  CONFIGDIR,
                  CONFIG_FILE));
}

static PasswordInfo *PASSWORDINFO_INSTANCE = nullptr;

enum PamStatus
{
    PamEndFailed,
    Successful,
    Failed,

};

PasswordInfo::PasswordInfo(QObject *parent)
  : QObject(parent)
  , m_password(QString())
{
}

PasswordInfo *
PasswordInfo::instance()
{
    if (!PASSWORDINFO_INSTANCE) {
        PASSWORDINFO_INSTANCE = new PasswordInfo;
    }
    return PASSWORDINFO_INSTANCE;
}

void
PasswordInfo::setPassword(const QString &password)
{
    m_password = password;
}

MessageModel::MessageModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

static int
handle_conversation(int num_msg,
                    const struct pam_message **msg,
                    struct pam_response **resp,
                    void *data)
{
    /* PAM expects an array of responses, one for each message */
    auto thisptr = static_cast<Commandline *>(data);
    struct pam_response *pam_reply =
      static_cast<struct pam_response *>(calloc(num_msg, sizeof(struct pam_response)));
    if (pam_reply == NULL) {
        return PAM_ABORT;
    }
    *resp = pam_reply;
    for (int i = 0; i < num_msg; ++i) {
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON:
            pam_reply[i].resp =
              strdup(PasswordInfo::instance()->password().toLocal8Bit().data()); // PAM clears and
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

Commandline::Commandline(QObject *parent)
  : QObject(parent)
  , m_usePam(true)
  , m_backgroundImagePath(QUrl("qrc:/image/gangdamu.png"))
  , m_opacity(1)
{
    m_username = QString::fromStdString(getlogin());
    readConfig();
    if (!m_usePam) {
        return;
    }
    const struct pam_conv conv = {
      .conv        = &handle_conversation,
      .appdata_ptr = this,
    };
    if (pam_start("waycratelock", m_username.toLocal8Bit().data(), &conv, &m_handle) !=
        PAM_SUCCESS) {
        qWarning() << "Cannot start pam";
        QTimer::singleShot(0, this, [this] { this->unlock(); });
        return;
    }
}

QString
Commandline::parsePath(const QString &string)
{
    glob_t globbuf;
    if (glob(string.toStdString().c_str(), GLOB_TILDE, NULL, &globbuf) == 0) {
        if (globbuf.gl_pathc > 0) {
            auto result = QString(globbuf.gl_pathv[0]);
            globfree(&globbuf);
            return result;
        }
    }
    return QString();
}

void
Commandline::readConfig()
{
    QString configpath = get_config_path();
    if (!QFile(configpath).exists()) {
        return;
    }
    try {
        auto tbl                              = toml::parse_file(configpath.toStdString());
        std::optional<bool> usePam            = tbl["general"]["needPassword"].value<bool>();
        std::optional<std::string> background = tbl["background"]["path"].value<std::string>();
        std::optional<double> opacity         = tbl["background"]["opacity"].value<float>();
        std::optional<int> fadeIn             = tbl["background"]["fadein"].value<int>();
        std::optional<int> fadeOut            = tbl["background"]["fadeout"].value<int>();

        m_opacity = opacity.value_or(1);
        m_usePam  = usePam.value_or(true);
        m_fadeIn  = fadeIn.value_or(0);
        m_fadeOut = fadeOut.value_or(0);

        if (background.has_value()) {
            auto backgroundImg    = parsePath(QString::fromStdString(background.value()));
            m_backgroundImagePath = QUrl::fromLocalFile(backgroundImg);
        }
    } catch (const toml::parse_error &err) {
        showTimedMessage("Something error with config file", true);
        qWarning() << err.what();
    }
}

void
Commandline::setPassword(const QString &password)
{
    m_password = password;
    PasswordInfo::instance()->setPassword(password);
    Q_EMIT passwordChanged();
}

void
Commandline::unlock()
{
    setExiting(true);
    QTimer::singleShot(
      fadeOut(), qApp, [] { NON_DEBUG(ExtSessionLockV1Qt::Command::instance()->unlockScreen();) });
    QTimer::singleShot(fadeOut() + 1, qApp, [] { QGuiApplication::quit(); });
}

void
Commandline::showTimedMessage(QString message, bool error, int timeSeconds)
{
    auto msg = Message(message, error);
    messages()->addMessage(msg);
    QTimer::singleShot(timeSeconds, this, [this, msg] { messages()->removeMessage(msg); });
}

void
Commandline::requestUnlock()
{
    if (!m_usePam) {
        unlock();
        return;
    }

    setBusy(true);

    QtConcurrent::run([this] {
        std::lock_guard<std::mutex> guard(PAM_GUARD);
        int pam_status = pam_authenticate(m_handle, 0);
        if (pam_status != PAM_SUCCESS) {
            return PamStatus::Failed;
        }
        pam_setcred(m_handle, PAM_REFRESH_CRED);
        if (pam_end(m_handle, pam_status) != PAM_SUCCESS) {
            return PamStatus::PamEndFailed;
        }
        return PamStatus::Successful;
    }).then([this](PamStatus value) {
        setBusy(false);
        switch (value) {
        case PamEndFailed: {
            showTimedMessage("Pam end failed!", true);
        }
        case Successful: {
            unlock();
            break;
        }
        case Failed: {
            showTimedMessage("Failed to unlock. Check your password", true);
            break;
        }
        }
    });
}
