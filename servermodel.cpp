#include "servermodel.h"

#include "pingrunnable.h"

#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>
#include <QXmlDefaultHandler>
#include <QHostInfo>
#include <QDateTime>
#include <QThreadPool>

struct XmlHandler : public QXmlDefaultHandler
{
    decltype(ServerModel::m_entryList) *m_entryList = nullptr;
    QString m_currentText;
    QString m_errorString;
    bool m_serverTag = false;
    bool m_serversTag = false;
    ServerModel::EntryMember m_currentEntry = ServerModel::InvalidMember;

    XmlHandler(decltype(m_entryList) entryList)
        : QXmlDefaultHandler()
        , m_entryList(entryList)
    {}

    virtual bool startElement(const QString &namespaceURI, const QString &localName,
                              const QString &qName, const QXmlAttributes &atts) override
    {
        Q_UNUSED(namespaceURI);
        Q_UNUSED(localName);
        Q_UNUSED(atts);
        if (qName == "servers" && !m_serversTag)
            m_serversTag = !m_serversTag;
        if (qName == "server" && !m_serverTag) {
            m_serverTag = !m_serverTag;
            m_entryList->append(ServerModel::Entry());
        }
        if (m_currentEntry == ServerModel::InvalidMember) {
            if (qName == "address")
                m_currentEntry = ServerModel::Address;
            else if (qName == "username")
                m_currentEntry = ServerModel::UserName;
            else if (qName == "password")
                m_currentEntry = ServerModel::Password;
            m_currentText.clear();
        }
        return true;
    }
    virtual bool endElement(const QString &namespaceURI, const QString &localName,
                            const QString &qName) override
    {
        Q_UNUSED(namespaceURI);
        Q_UNUSED(localName);
        if (qName == "servers" && m_serversTag)
            m_serversTag = !m_serversTag;
        if (qName == "server" && m_serverTag)
            m_serverTag = !m_serverTag;
        if (m_currentEntry != ServerModel::InvalidMember) {
            if (qName == "address") {
                m_entryList->last().m_address.swap(m_currentText);
                m_currentEntry = ServerModel::InvalidMember;
            }
            else if (qName == "username") {
                m_entryList->last().m_username.swap(m_currentText);
                m_currentEntry = ServerModel::InvalidMember;
            }
            else if (qName == "password") {
                m_entryList->last().m_password.swap(m_currentText);
                m_currentEntry = ServerModel::InvalidMember;
            }
        }
        return true;
    }
    virtual bool characters(const QString &ch) override
    {
        m_currentText += ch;
        return true;
    }
    virtual bool fatalError(const QXmlParseException &exception) override
    {
        QMessageBox::critical(nullptr, qApp->applicationName(),
                              QObject::tr("Parse error at line %1, column %2:\n"
                                          "%3")
                              .arg(exception.lineNumber())
                              .arg(exception.columnNumber())
                              .arg(exception.message()));
        return false;
    }
    virtual QString errorString() const override
    {
        return m_errorString;
    }
};

ServerModel::ServerModel(QObject *parent)
    : QAbstractListModel(parent)
    , km_path(QDir(qApp->applicationDirPath()).filePath("servers.xml"))
    , m_watcher(QStringList{ km_path })
{
    readFile();
    m_timer.setInterval(10000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &ServerModel::ping);
    QMetaObject::invokeMethod(&m_timer, "start");

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ServerModel::readFile);
}

ServerModel::ServerModel(const QString &path, QObject *parent)
    : QAbstractListModel(parent)
    , km_path(path)
    , m_watcher(QStringList{ km_path })
{
    readFile();
    m_timer.setInterval(10000);
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &ServerModel::ping);
    QMetaObject::invokeMethod(&m_timer, "start");

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ServerModel::readFile);
}

int ServerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return -1;
    return m_entryList.size();
}

QVariant ServerModel::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    if (index.column() != 0 || index.row() >= m_entryList.size() || index.row() < 0)
        return QVariant();

    switch (role)
    {
    case AddressRole:
        return m_entryList[index.row()].m_address;
    case UserNameRole:
        return m_entryList[index.row()].m_username;
    case PasswordRole:
        return m_entryList[index.row()].m_password;
    case PingRole: {
        auto ping = m_entryList[index.row()].m_ping;
        if (ping == -1)
            return "Unknown";
        else
            return QString("%1 ms").arg(m_entryList[index.row()].m_ping);
    }
    case CombinedRole:
        return QString("%1:%2@%3").arg(data(index, UserNameRole).toString())
                .arg(data(index, PasswordRole).toString())
                .arg(data(index, AddressRole).toString());
    }
    return QVariant();
}

QHash<int, QByteArray> ServerModel::roleNames() const
{
    const decltype(roleNames()) roles{
        { AddressRole,  "address"   },
        { UserNameRole, "username"  },
        { PasswordRole, "password"  },
        { PingRole,     "ping"      },
        { CombinedRole, "combined"  },
    };
    return roles;
}

void ServerModel::readFile()
{
   beginResetModel();
   m_entryList.clear();

   QFile file(km_path);

   QXmlSimpleReader reader;
   XmlHandler handler(&m_entryList);
   reader.setContentHandler(&handler);
   reader.setErrorHandler(&handler);
   QXmlInputSource source(&file);
   auto ok = reader.parse(&source);

   if (ok)
       QMetaObject::invokeMethod(this, "ping");
   else
       qCritical() << "Failed to parse servers.xml";
   endResetModel();
}

void ServerModel::ping()
{
    for (const auto entry : m_entryList) {
        auto runnable = new PingRunnable(entry);
        connect(runnable, &PingRunnable::finished,
                this, &ServerModel::pingFinishedReceived);
        QThreadPool::globalInstance()->start(runnable);
    }
}

void ServerModel::pingFinishedReceived(
        const QString &address, const QSet<QHostAddress> &resolvedAddress,
        int ms)
{
    for (int i = 0; i < m_entryList.size(); ++i) {
        auto &entry = m_entryList[i];
        if (entry.m_address == address) {
            entry.m_resolvedAddresses.unite(resolvedAddress);
            if (entry.m_ping != ms) {
                auto idx = index(i, 0);
                emit dataChanged(idx, idx, { PingRole });
                entry.m_ping = ms;
            }
        }
    }
}
