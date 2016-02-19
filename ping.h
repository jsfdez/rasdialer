#ifndef PING_H
#define PING_H

#include <QObject>

class Ping : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)

public:
    explicit Ping(QObject *parent = 0);

    QString host() const;
    void setHost(const QString &host);

public slots:
    void ping();

signals:
    void hostChanged(const QString &host);
    void finished(int ms);

private:
    QString m_host;
    bool m_running = false;
};

#endif // PING_H
