#include "AsyncSessionJoiner.h"
#include "SessionListenerImpl.h"
#include "utils/Log.h"
#include <iostream>

#define TAG "AsyncSessionJoiner"

using namespace ajn;

AsyncSessionJoiner::AsyncSessionJoiner(const char* name, SessionJoinedCallback callback) :
    m_Busname(""), m_Callback(callback)
{
    if (name) {
        m_Busname.assign(name);
    }
}

AsyncSessionJoiner::~AsyncSessionJoiner()
{

}

void AsyncSessionJoiner::JoinSessionCB(QStatus status, SessionId id, const SessionOpts& opts, void* context)
{
    QCC_UNUSED(opts);
    if (status == ER_OK) {
        LOGD(TAG, "JoinSessionCB(%s) succeeded with id %d",m_Busname.c_str(), id);
        if (m_Callback) {
            LOGD(TAG, "Calling SessionJoiner Callback");
            m_Callback(m_Busname, id);
        }
    } else {
        LOGD(TAG, "JoinSessionCB(%s) failed with status: %s", m_Busname.c_str(), QCC_StatusText(status));
    }

    SessionListenerImpl* listener = (SessionListenerImpl*) context;
    delete listener;
    delete this;
}
