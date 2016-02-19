#ifndef RASDIALER_H
#define RASDIALER_H

#include <QObject>

#include <Windows.h>
#include <Ras.h>

class RasDialer : public QObject
{
	Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    enum State {
        NotConnected,
        Connecting,
        Connected
    };
    Q_ENUMS(State)

    explicit RasDialer(QObject *parent = 0);

	bool isConnected() const;
    State state() const;

public slots:
	void connect(const QString &address, const QString &userName, const QString &password);
	void disconnect();

signals:
    void stateChanged(int state);
    void errorReceived(const QString &error);

protected slots:
    void showError(const QString &error);

private:
	QString phonebookPath() const;

    HRASCONN m_handle = nullptr;
    State m_state = NotConnected;
};

Q_DECLARE_METATYPE(RasDialer::State)

#endif // RASDIALER_H
