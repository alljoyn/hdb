#include "CommonBusListener.h"
#include "utils/Log.h"
#include <iostream>
#include <algorithm>

#define TAG "CommonBusListener"

using namespace ajn;

CommonBusListener::CommonBusListener(ajn::BusAttachment* bus, void(*daemonDisconnectCB)()) :
    BusListener(), SessionPortListener(), m_SessionPort(0), m_Bus(bus), m_DaemonDisconnectCB(daemonDisconnectCB)
{
}

CommonBusListener::~CommonBusListener()
{
}

void CommonBusListener::setSessionPort(ajn::SessionPort sessionPort)
{
    m_SessionPort = sessionPort;
}

SessionPort CommonBusListener::getSessionPort()
{
    return m_SessionPort;
}

bool CommonBusListener::AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts)
{
    QCC_UNUSED(joiner);
    QCC_UNUSED(opts);
    if (sessionPort != m_SessionPort) {
        return false;
    }

    LOGD(TAG, "Accepting JoinSessionRequest");
    return true;
}

void CommonBusListener::SessionJoined(SessionPort sessionPort, SessionId id, const char* joiner)
{
    QCC_UNUSED(sessionPort);
    QCC_UNUSED(joiner);
    LOGD(TAG, "Session has been joined successfully");
    if (m_Bus) {
        m_Bus->SetSessionListener(id, this);
    }
    m_SessionIds.push_back(id);
}

void CommonBusListener::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    QCC_UNUSED(reason);
    LOGD(TAG, "Session has been lost");
    std::vector<SessionId>::iterator it = std::find(m_SessionIds.begin(), m_SessionIds.end(), sessionId);
    if (it != m_SessionIds.end()) {
        m_SessionIds.erase(it);
    }
}

void CommonBusListener::BusDisconnected()
{
    LOGD(TAG, "Bus has been disconnected");
    if (m_DaemonDisconnectCB) {
        m_DaemonDisconnectCB();
    }
}

const std::vector<SessionId>& CommonBusListener::getSessionIds() const
{
    return m_SessionIds;
}
