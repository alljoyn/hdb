#include "AnnounceHandlerImpl.h"
#include "utils/Log.h"
#include <iostream>

#define TAG "AnnounceHandlerImpl"

using namespace ajn;

AnnounceHandlerImpl::AnnounceHandlerImpl(BasicAnnounceHandlerCallback basicCallback, FullAnnounceHandlerCallback fullCallback) :
    m_BasicCallback(basicCallback), m_FullCallback(fullCallback)
{

}

AnnounceHandlerImpl::~AnnounceHandlerImpl()
{
}

void AnnounceHandlerImpl::Announced(const char* busName, uint16_t version, SessionPort port, const MsgArg& objectDescriptionArg, const MsgArg& aboutDataArg)
{
    LOGD(TAG, "*********************************************************************************");
    LOGD(TAG, "Announce signal discovered");
    LOGD(TAG, "\tFrom bus %s", busName);
    LOGD(TAG, "\tAbout version %d", version);
    LOGD(TAG, "\tSessionPort %d", port);
    LOGD(TAG, "\tObjectDescription %s", objectDescriptionArg.ToString().c_str());
    LOGD(TAG, "\tAboutData: %s", aboutDataArg.ToString().c_str());
    LOGD(TAG, "*********************************************************************************");

    LOGD(TAG, "AnnounceHandlerImpl::Announced()");
    if (m_BasicCallback) {
        LOGD(TAG, "Calling AnnounceHandler Callback");
        m_BasicCallback(busName, port);
    }

    if (m_FullCallback) {
        LOGD(TAG, "Calling AnnounceHandler Callback");
        AboutData aboutData;
        aboutData.CreatefromMsgArg(aboutDataArg);
        AboutObjectDescription aboutObjectDescription;
        aboutObjectDescription.CreateFromMsgArg(objectDescriptionArg);
        m_FullCallback(busName, version, port, aboutObjectDescription, aboutData);
    }
}

