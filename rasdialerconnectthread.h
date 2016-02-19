#ifndef RASDIALERCONNECTTHREAD_H
#define RASDIALERCONNECTTHREAD_H

#include <QThread>

#include <Windows.h>
#include <Ras.h>

class RasDialerConnectThread : public QThread
{
	Q_OBJECT
public:
	explicit RasDialerConnectThread(const QString &phonebook, const QString &address,
									const QString &username, const QString &password,
									QObject *parent = 0);

signals:
    void error(const QString &errorString);
	void finished(HRASCONN handle);

protected:
	virtual void run() override;

private:
	const QString mk_phonebook;
	const QString mk_address;
	const QString mk_username;
	const QString mk_password;
};

#endif // RASDIALERCONNECTTHREAD_H
