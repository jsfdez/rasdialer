#ifndef SERVERMODEL_H
#define SERVERMODEL_H

#include <QTimer>
#include <QHostAddress>
#include <QAbstractListModel>
#include <QFileSystemWatcher>

class ServerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ServerRoles {
        AddressRole = Qt::UserRole + 1,
        UserNameRole,
        PasswordRole,
        PingRole,
        CombinedRole,
    };

    Q_ENUMS(ServerRoles)

    explicit ServerModel(QObject *parent = 0);
    explicit ServerModel(const QString &path, QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

public slots:
    void ping();

protected:
    void readFile();

protected slots:
    void pingFinishedReceived(const QString &address, const QSet<QHostAddress> &resolvedAddress,
                              int ms);

private:
    friend struct XmlHandler;
    friend class PingRunnable;

    struct Entry {
        Entry() {}
        Entry(const QString &address)
            : m_address(address)
        {}

        QString m_address;
        QString m_username;
        QString m_password;
        QSet<QHostAddress> m_resolvedAddresses;
        int m_ping = -1;
    };

    enum EntryMember {
        InvalidMember = -1,
        Address,
        UserName,
        Password,
    };

    QList<Entry> m_entryList;
    const QString km_path;
    QTimer m_timer;
    QFileSystemWatcher m_watcher;
};

#endif // SERVERMODEL_H
