#include "SessionListenerImpl.h"
#include "utils/Log.h"
#include <iostream>

#define TAG "SessionListenerImpl"

SessionListenerImpl::SessionListenerImpl(qcc::String const& inServiceNAme) :
    serviceName(inServiceNAme)
{
}

SessionListenerImpl::~SessionListenerImpl()
{
}

void SessionListenerImpl::SessionLost(ajn::SessionId sessionId, ajn::SessionListener::SessionLostReason reason)
{
    QCC_UNUSED(sessionId);
    QCC_UNUSED(reason);
    LOGD(TAG, "AboutClient session has been lost for %s", serviceName.c_str());
}
