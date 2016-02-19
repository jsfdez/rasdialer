#include "rasdialerconnectthread.h"

extern WCHAR k_vpnName[];

Q_DECLARE_METATYPE(HRASCONN)

RasDialerConnectThread::RasDialerConnectThread(const QString &phonebook, const QString &address,
											   const QString &username, const QString &password,
											   QObject *parent)
	: QThread(parent)
	, mk_phonebook(phonebook)
	, mk_address(address)
	, mk_username(username)
	, mk_password(password)
{
    qRegisterMetaType<HRASCONN>();
}

void RasDialerConnectThread::run()
{
	auto result = ERROR_SUCCESS;
	const auto path = mk_phonebook;

	WCHAR szPhoneBook[PATHLEN];
	szPhoneBook[path.toWCharArray(szPhoneBook)] = '\0';

	HRASCONN handle = 0;
	BOOL bPassword;
	RASDIALPARAMS params;
	memset(&params, 0, sizeof(params));
	wcscpy(params.szEntryName, k_vpnName);
	params.dwSize = sizeof(RASDIALPARAMS);
	result = RasGetEntryDialParamsW(szPhoneBook, &params, &bPassword);
    if (result != ERROR_SUCCESS) {
        emit error(QString("Error %1").arg(result));
        return;
    }
	params.szPhoneNumber[mk_address.toWCharArray(params.szPhoneNumber)] = '\0';
	params.szUserName[mk_username.toWCharArray(params.szUserName)] = '\0';
	params.szPassword[mk_password.toWCharArray(params.szPassword)] = '\0';
	result = RasDialW(NULL, szPhoneBook, &params, 0, NULL, &handle);
    if (result != ERROR_SUCCESS) {
        emit error(QString("Error %1").arg(result));
        if (handle != 0) {
            result = RasHangUpW(handle);
            RASCONNSTATUS status;
            memset(&status, 0, sizeof(status));
            status.dwSize = sizeof(status);
            while (RasGetConnectStatusW(handle, &status) != ERROR_INVALID_HANDLE)
                QThread::currentThread()->wait(0);
        }
    }
    else
        emit finished(handle);
}
