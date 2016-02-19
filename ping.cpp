#include "ping.h"

#include "pingrunnable.h"

#include <QThreadPool>

Ping::Ping(QObject *parent) : QObject(parent)
{
}

QString Ping::host() const
{
    return m_host;
}

void Ping::setHost(const QString &host)
{
    if (host != m_host) {
        m_host = host;
        emit hostChanged(m_host);
    }
}

void Ping::ping()
{
    if (m_running)
        return;

    auto runnable = new PingRunnable(m_host);
    QThreadPool::globalInstance()->start(runnable);
    m_running = true;
    connect(runnable, &PingRunnable::finished,
            [&](const QString &, const QSet<QHostAddress> &, int ms)
    {
        emit finished(ms);
    });

}
