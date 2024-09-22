#pragma once

#include <Qt>
#include <qabstractitemmodel.h>

class Message {
  public:
    inline QString message() const { return m_message; }
    inline bool error() const { return m_error; }

    Message(QString message, bool error) : m_message(message), m_error(error) {}
    bool operator==(const Message& rhs) const {
        return m_message == rhs.m_message && m_error == rhs.m_error;
    }

  private:
    QString m_message;
    bool m_error;
};

class MessageModel : public QAbstractListModel {
    Q_OBJECT
  public:
    enum AnimalRoles { MessageRole = Qt::UserRole + 1, ErrorRole };
    MessageModel(QObject* parent = nullptr);

    void addMessage(const Message& msg);

    void removeMessage(const Message& msg);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  protected:
    QHash<int, QByteArray> roleNames() const;

  private:
    QList<Message> m_messages{};
};
