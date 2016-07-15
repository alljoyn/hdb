#include "SrpKeyXListener.h"
#include "utils/Log.h"
#include <iostream>

#define TAG "SrpKeyXListener"

SrpKeyXListener::SrpKeyXListener(ajn::services::AboutDataStore* aboutDataStore) : 
	m_AboutDataStore(aboutDataStore), m_PassCode("000000"), m_GetPassCode(0)
{


}

SrpKeyXListener::~SrpKeyXListener()
{

}

void SrpKeyXListener::setPassCode(qcc::String const& passCode)
{
    m_PassCode = passCode;
}

void SrpKeyXListener::setGetPassCode(void (*getPassCode)(ajn::services::AboutDataStore&, qcc::String&))
{
    m_GetPassCode = getPassCode;
}

bool SrpKeyXListener::RequestCredentials(const char* authMechanism, const char* authPeer,
                                         uint16_t authCount, const char* userId, uint16_t credMask, Credentials& creds)
{
    QCC_UNUSED(userId);
    LOGD(TAG, "RequestCredentials for authenticating %s using mechanism %s", authPeer, authMechanism);
    if (strcmp(authMechanism, "ALLJOYN_SRP_KEYX") == 0 || strcmp(authMechanism, "ALLJOYN_ECDHE_PSK") == 0) {
        if (credMask & AuthListener::CRED_PASSWORD) {
            if (authCount <= 3) {
                qcc::String passCodeFromGet;
                if (m_GetPassCode) {
                    m_GetPassCode(*m_AboutDataStore, passCodeFromGet);
                }
                LOGD(TAG, "RequestCredentials setPasscode to %s", (m_GetPassCode ? passCodeFromGet.c_str() : m_PassCode.c_str()));
                creds.SetPassword(m_GetPassCode ? passCodeFromGet.c_str() : m_PassCode.c_str());
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void SrpKeyXListener::AuthenticationComplete(const char* authMechanism, const char* authPeer, bool success)
{
    QCC_UNUSED(authPeer);
    LOGD(TAG, "Authentication with %s %s", authMechanism, (success ? " was successful" : " failed"));
}
