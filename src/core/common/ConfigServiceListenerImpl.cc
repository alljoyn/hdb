#include "ConfigServiceListenerImpl.h"
#include "AboutObjApi.h"

#include "utils/Log.h"

#define TAG "ConfigServiceListenerImpl"

using namespace ajn;
using namespace services;

ConfigServiceListenerImpl::ConfigServiceListenerImpl(AboutDataStore& store, ajn::BusAttachment& bus, 
	CommonBusListener& busListener, AboutObjApi& aboutApi, OnboardingControllerImpl& onbCont) :
    ConfigService::Listener(), m_AboutDataStore(&store), m_Bus(&bus), 
	m_BusListener(&busListener), m_AboutApi(&aboutApi), m_OnboardingController(&onbCont),
	mConfigListener(NULL)
{
}

QStatus ConfigServiceListenerImpl::Restart()
{
    LOGD(TAG, "Restart has been called !!!");
	if (mConfigListener)
		mConfigListener->Restart();
    return ER_OK;
}

QStatus ConfigServiceListenerImpl::FactoryReset()
{
    LOGD(TAG, "FactoryReset has been called!!!");

    QStatus status = ER_OK;
    m_AboutDataStore->FactoryReset();
    LOGD(TAG, "Clearing Key Store");
    m_Bus->ClearKeyStore();

    if (m_AboutApi) {
        status = m_AboutApi->Announce();
        LOGD(TAG, "Announce for %s = %s", m_Bus->GetUniqueName().c_str(),  QCC_StatusText(status));
    }

	m_OnboardingController->Offboard();

	if (mConfigListener)
		mConfigListener->FactoryReset();
    return status;
}

QStatus ConfigServiceListenerImpl::SetPassphrase(const char* daemonRealm, size_t passcodeSize, const char* passcode, ajn::SessionId sessionId)
{
    qcc::String passCodeString(passcode, passcodeSize);
    LOGD(TAG, "SetPassphrase has been called daemonRealm=%s passcode=%s passcodeLength=%d",
		   	daemonRealm, passCodeString.c_str(), passcodeSize);

    PersistPassword(daemonRealm, passCodeString.c_str());

    LOGD(TAG, "Clearing Key Store");
    m_Bus->ClearKeyStore();
    m_Bus->EnableConcurrentCallbacks();

    std::vector<SessionId> sessionIds = m_BusListener->getSessionIds();
    for (size_t i = 0; i < sessionIds.size(); i++) {
        if (sessionIds[i] == sessionId) {
            continue;
        }
        m_Bus->LeaveSession(sessionIds[i]);
        LOGD(TAG, "Leaving session with id: %d", sessionIds[i]);
    }
    m_AboutDataStore->write();
    return ER_OK;
}

ConfigServiceListenerImpl::~ConfigServiceListenerImpl()
{
}

void ConfigServiceListenerImpl::AddListener(ConfigListener* listener)
{
	mConfigListener = listener;
}

void ConfigServiceListenerImpl::RemoveListener()
{
	mConfigListener = NULL;
}

void ConfigServiceListenerImpl::PersistPassword(const char* daemonRealm, const char* passcode)
{
    MsgArg argPasscode;
    MsgArg argDaemonrealm;
    argPasscode.Set("s", passcode);
    argDaemonrealm.Set("s", daemonRealm);
    m_AboutDataStore->SetField("Passcode", argPasscode);
    m_AboutDataStore->SetField("Daemonrealm", argDaemonrealm);
}

