#pragma once

#include <QObject>
#include <QQmlEngine>
#include <qabstractitemmodel.h>
#include <security/pam_appl.h>

class Message 
//: public QObject
{
    // Q_OBJECT
public:
    inline QString message() const { return m_message; }
    inline bool error() const { return m_error; }

    // Message(QObject* parent = nullptr) : QObject(parent) {}

    Message(QString message, bool error)
      : m_message(message)
      , m_error(error)
    {
    }
    bool operator==(const Message &rhs) const
    {
        return m_message == rhs.m_message && m_error == rhs.m_error;
    }

private:
    QString m_message;
    bool m_error;
};

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AnimalRoles
    {
        MessageRole = Qt::UserRole + 1,
        ErrorRole
    };
    MessageModel(QObject *parent = nullptr);

    void addMessage(const Message &msg);

    void removeMessage(const Message &msg);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<Message> m_messages;
    //![2]
};

class PasswordInfo : public QObject
{
    Q_OBJECT

public:
    static PasswordInfo *instance();

    inline QString password() { return m_password; }

    void setPassword(const QString &password);

private:
    explicit PasswordInfo(QObject *parent = nullptr);

private:
    QString m_password;
};

class Commandline final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Commandline(QObject *parent = nullptr);

    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    inline QString password() { return m_password; }
    void setPassword(const QString &password);

    Q_PROPERTY(QString username READ username NOTIFY usernameChanged)
    inline QString username() { return m_username; }

    Q_PROPERTY(MessageModel* messages READ messages NOTIFY messagesChanged)
    inline MessageModel* messages() { return m_messages; }

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
    QString m_currentDate;
    QString m_password;
    QString m_username;
    MessageModel* m_messages = new MessageModel;
    pam_handle_t *m_handle;
    bool m_usePam;
    bool m_busy;
    QUrl m_backgroundImagePath;
    double m_opacity;
    bool m_exiting;
    int m_fadeIn;
    int m_fadeOut;
};
