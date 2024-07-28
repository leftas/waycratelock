#pragma once

#include <MessageModel.hpp>
#include <QObject>
#include <QQmlEngine>
#include <qabstractitemmodel.h>
#include <security/pam_appl.h>

class Commandline final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Commandline(QObject *parent = nullptr);
     ~Commandline();

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    inline QString password() { return m_password; }
    void setPassword(const QString &password);

    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    inline QString username() { return m_username; }

    Q_PROPERTY(MessageModel *messages READ messages NOTIFY messagesChanged)
    inline MessageModel *messages() { return m_messages; }

    Q_PROPERTY(bool usePam READ usePam NOTIFY usePamChanged)
    inline bool usePam() { return m_usePam; }

    Q_PROPERTY(QUrl background READ background NOTIFY backgroundChanged)
    inline QUrl background() { return m_backgroundImagePath; }

    Q_PROPERTY(double opacity READ opacity NOTIFY opacityChanged)
    inline double opacity() { return m_opacity; }

    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    inline double busy() { return m_busy; }

    Q_PROPERTY(bool exiting READ exiting WRITE setExiting NOTIFY exitingChanged)
    inline double exiting() { return m_exiting; }

    Q_PROPERTY(int fadeIn READ fadeIn NOTIFY fadeInChanged)
    inline int fadeIn() { return m_fadeIn; }

    Q_PROPERTY(int fadeOut READ fadeOut NOTIFY fadeOutChanged)
    inline int fadeOut() { return m_fadeOut; }

    Q_INVOKABLE void unlock();
    Q_INVOKABLE void requestUnlock();

    void showTimedMessage(QString message, bool error, int timeSeconds = 3000);

private:
    void readConfig();
    QString parsePath(const QString &command);

    void setBusy(bool value)
    {
        m_busy = value;
        emit busyChanged();
    }
    void setExiting(bool value)
    {
        m_exiting = value;
        emit exitingChanged();
    }
    void runPamUnlock(auto *handle, std::mutex *mutex, bool silent);
    void startPam();
    pam_handle_t *initPam(const char *pam_name, const struct pam_conv *conv, const char *username);

signals:
    void currentDateChanged();
    void passwordChanged();
    void usernameChanged();
    void messagesChanged();
    void usePamChanged();
    void opacityChanged();
    void backgroundChanged();
    void busyChanged();
    void exitingChanged();
    void fadeInChanged();
    void fadeOutChanged();

private:
    MessageModel *m_messages = new MessageModel;
    QString m_currentDate;
    QString m_password;
    QString m_username;
    QMap<QString, pam_handle_t *> m_handles{{"waycratelock", nullptr}};
    QMap<QString, std::mutex *> m_guards;
    QUrl m_backgroundImagePath;
    double m_opacity;
    int m_fadeIn;
    int m_fadeOut;
    bool m_usePam;
    bool m_busy    = false;
    bool m_exiting = false;
};
