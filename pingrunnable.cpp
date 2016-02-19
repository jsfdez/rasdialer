#include "pingrunnable.h"

#include <QThread>
#include <QHostInfo>

#include <Windows.h>
#include <IPHlpApi.h>
#include <IcmpAPI.h>

Q_DECLARE_METATYPE(QSet<QHostAddress>)

PingRunnable::PingRunnable(const ServerModel::Entry &entry, QObject *parent)
    : QObject(parent)
    , km_entry(entry)
{
    qRegisterMetaType<QSet<QHostAddress>>();
    setAutoDelete(true);
}

PingRunnable::PingRunnable(const QString &host, QObject *parent)
    : QObject(parent)
    , km_entry(ServerModel::Entry(host))
{
}

void PingRunnable::run()
{
    m_addresses = km_entry.m_resolvedAddresses;
    if (km_entry.m_resolvedAddresses.empty())
        m_addresses = QHostInfo::fromName(km_entry.m_address).addresses().toSet();

    if (m_addresses.empty()) {
        emit finished(km_entry.m_address);
        return;
    }
    HANDLE hIcmp;
    char SendData[] = "ICMP SEND DATA";
    DWORD dwRetVal;
    PICMP_ECHO_REPLY pIcmpEchoReply;

    hIcmp = IcmpCreateFile();

    QVector<quint8> replyBuffer(sizeof(ICMP_ECHO_REPLY) + strlen(SendData) + 1, 0);
    pIcmpEchoReply = (PICMP_ECHO_REPLY)replyBuffer.data();
    const auto address = *m_addresses.begin();
    const auto string = address.toString();
    auto addr = inet_addr(string.toLatin1().data());

    dwRetVal = IcmpSendEcho(hIcmp, addr, SendData, strlen(SendData), NULL,
                            &replyBuffer[0], replyBuffer.size(), 1000);
    if (dwRetVal != 0)
      emit finished(km_entry.m_address, m_addresses, pIcmpEchoReply->RoundTripTime);
    else
      emit finished(km_entry.m_address, m_addresses);
    IcmpCloseHandle(hIcmp);
}
