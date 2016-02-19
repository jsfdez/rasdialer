#ifndef PINGTHREAD_H
#define PINGTHREAD_H

#include <QObject>
#include <QRunnable>

#include "servermodel.h"

class PingRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit PingRunnable(const ServerModel::Entry &entry, QObject *parent = 0);
    explicit PingRunnable(const QString &host, QObject *parent = 0);

    virtual void run() override;

signals:
    void finished(const QString &address,
                  const QSet<QHostAddress> &resolvedAddresses = QSet<QHostAddress>(), int ms = -1);

private:
    const ServerModel::Entry km_entry;
    QSet<QHostAddress> m_addresses;
};

#endif // PINGTHREAD_H
