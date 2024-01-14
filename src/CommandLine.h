#pragma once

#include <QObject>
#include <QQmlEngine>
#include <security/pam_appl.h>

class PassWordInfo : public QObject
{
    Q_OBJECT

public:
    static PassWordInfo *instance();

    inline QString password() { return m_password; }

    void setPassword(const QString &password);

private:
    explicit PassWordInfo(QObject *parent = nullptr);

private:
    QString m_password;
};

class CommandLine final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit CommandLine(QObject *parent = nullptr);

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    inline QString password() { return m_password; }
    void setPassword(const QString &password);

    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    inline QString userName() { return m_userName; }

    Q_PROPERTY(
      QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged)
    inline QString errorMessage() { return m_errorMessage; }

    Q_PROPERTY(
    bool errorMessageVisible READ errorMessageVisible WRITE setErrorMessageVisibility NOTIFY errorMessageVisibilityChanged)
    inline bool errorMessageVisible() { return m_errorMessageVisible; }

    Q_PROPERTY(bool usePam READ usePam NOTIFY usePamChanged)
    inline bool usePam() { return m_usePam; }

    Q_PROPERTY(QUrl background READ background NOTIFY backgroundChanged)
    inline QUrl background() { return m_backgroundImagePath; }

    Q_PROPERTY(double opacity READ opacity NOTIFY opacityChanged)
    inline double opacity() { return m_opacity; }

    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
    inline double busy() { return m_busy; }

    Q_INVOKABLE void UnLock();
    Q_INVOKABLE void RequestUnlock();

private:
    void readConfig();

    void setErrorMessage(QString message)
    {
        m_errorMessage = message;
        emit errorMessageChanged();
    }
    void setErrorMessageVisibility(bool visible){
        m_errorMessageVisible = visible;
        emit errorMessageVisibilityChanged();
    }
    void setBusy(bool value)
    {
        m_busy = value;
        emit busyChanged();
    }
    void showErrorMessage(QString message, int timeSeconds=3000);

signals:
    void currentDateChanged();
    void passwordChanged();
    void userNameChanged();
    void errorMessageChanged();
    void errorMessageVisibilityChanged();
    void usePamChanged();
    void opacityChanged();
    void backgroundChanged();
    void busyChanged();

private:
    QString m_currentDate;
    QString m_password;
    QString m_userName;
    QString m_errorMessage;
    bool m_errorMessageVisible;
    pam_handle_t *m_handle;
    bool m_usePam;
    bool m_busy = false;
    QUrl m_backgroundImagePath;
    double m_opacity;
};
