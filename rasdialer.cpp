#include "rasdialer.h"

#include "rasdialerconnectthread.h"

#include <QDir>
#include <QThread>
#include <QMessageBox>
#include <QCoreApplication>

#include <Windows.h>

#include <string.h>
#include <RasError.h>

WCHAR k_vpnName[] = L"VPN Connection";

Q_DECLARE_METATYPE(HRASCONN)

RasDialer::RasDialer(QObject *parent)
	: QObject(parent)
{
    qRegisterMetaType<State>();

	auto result = ERROR_SUCCESS;
	RASCONN *lpRasConn = nullptr;
	DWORD dwSize = 0, dwConnections = 0;
	result = RasEnumConnectionsW(lpRasConn, &dwSize, &dwConnections);
	if (result == ERROR_BUFFER_TOO_SMALL)  {
		WCHAR szPhoneBook[PATHLEN];
		szPhoneBook[phonebookPath().toWCharArray(szPhoneBook)] = '\0';

		lpRasConn = (RASCONN*)calloc(1, dwSize);
		lpRasConn->dwSize = sizeof(RASCONN);
		result = RasEnumConnectionsW(lpRasConn, &dwSize, &dwConnections);
		Q_ASSERT(result == ERROR_SUCCESS);
		for (DWORD i = 0u; i < dwConnections; ++i) {
			if (wcscmp(lpRasConn[i].szPhonebook, szPhoneBook) == 0) {
				result = RasHangUpW(lpRasConn[i].hrasconn);
				Q_ASSERT(result == ERROR_SUCCESS);
			}
		}
	}
}

bool RasDialer::isConnected() const
{
    return m_handle != 0;
}

RasDialer::State RasDialer::state() const
{
    return m_state;
}

void RasDialer::connect(const QString &address, const QString &userName, const QString &password)
{
    if (m_state != NotConnected || m_handle)
        return;

	auto thread = new RasDialerConnectThread(phonebookPath(), address, userName, password, this);

	QObject::connect(thread, &RasDialerConnectThread::finished, [&](HRASCONN handle)
	{
        qRegisterMetaType<State>();

        m_handle = handle;
        m_state = Connected;
        emit stateChanged(m_state);
	});
	QObject::connect(thread, &RasDialerConnectThread::finished,
					 thread, &RasDialerConnectThread::deleteLater);
    QObject::connect(thread, &RasDialerConnectThread::error, this, &RasDialer::showError,
                     Qt::QueuedConnection);
    QObject::connect(thread, &RasDialerConnectThread::error,
                     thread, &RasDialerConnectThread::deleteLater);

	thread->start();
    m_state = Connecting;
    emit stateChanged(m_state);
}

void RasDialer::disconnect()
{
	if (m_handle) {
		auto result = RasHangUpW(m_handle);
		Q_ASSERT(result == ERROR_SUCCESS);
		if (result == ERROR_SUCCESS) {
            m_state = NotConnected;
            emit stateChanged(m_state);

            RASCONNSTATUS status;
            memset(&status, 0, sizeof(status));
            status.dwSize = sizeof(status);
            while (RasGetConnectStatusW(m_handle, &status) != ERROR_INVALID_HANDLE)
                QThread::currentThread()->wait(0);

            m_handle = 0;
        }
    }
}

void RasDialer::showError(const QString &error)
{
    m_state = NotConnected;
//    QMessageBox::critical(nullptr, qApp->applicationName(), error);
    emit errorReceived(error);
}

QString RasDialer::phonebookPath() const
{
	QDir dir(qApp->applicationDirPath());
	QFileInfo fi(dir.filePath("phonebook.pbk"));
	Q_ASSERT(fi.exists());

	auto path = QDir::toNativeSeparators(fi.absoluteFilePath());

	return path;
}
