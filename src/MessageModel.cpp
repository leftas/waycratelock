#include "MessageModel.hpp"

MessageModel::MessageModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

void
MessageModel::addMessage(const Message &message)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_messages << message;
    endInsertRows();
}

void
MessageModel::removeMessage(const Message &message)
{
    auto idx = m_messages.indexOf(message);
    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_messages.removeAt(idx);
        endRemoveRows();
    }
}

int
MessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_messages.count();
}

QVariant
MessageModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_messages.count())
        return QVariant();

    const Message &message = m_messages[index.row()];
    if (role == MessageRole)
        return QVariant::fromValue(message.message());
    else if (role == ErrorRole)
        return QVariant::fromValue(message.error());
    return QVariant();
}

//![0]
QHash<int, QByteArray>
MessageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[MessageRole] = "message";
    roles[ErrorRole]   = "error";
    return roles;
}
