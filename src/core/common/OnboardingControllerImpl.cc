#include "OnboardingControllerImpl.h"

#include "utils/Log.h"

#define TAG "OnboardingControllerImpl"

using namespace ajn;
using namespace services;

typedef enum {
    CIPHER_NONE,
    CIPHER_TKIP,
    CIPHER_CCMP,
    CIPHER_BOTH
}GroupCiphers;

/****	Auth Types
    WPA2_AUTO = -3,                            //!< WPA2_AUTO authentication
    WPA_AUTO = -2,                           //!< WPA_AUTO authentication
    ANY = -1,                           //!< ANY authentication
    OPEN = 0,                          //!< OPEN authentication
    WEP = 1,                           //!< WEP authentication
    WPA_TKIP = 2,                           //!< WPA_TKIP authentication
    WPA_CCMP = 3,                           //!< WPA_CCMP authentication
    WPA2_TKIP = 4,                            //!<WPA2_TKIP authentication
    WPA2_CCMP = 5,                        //!<WPA2_CCMP authentication
    WPS = 6,                          //!<WPS authentication
*/

#define CASE(_auth) case _auth: return # _auth
static const char* AuthText(short authType)
{
    switch (authType) {
        CASE(WPA2_AUTO);
        CASE(WPA_AUTO);
        CASE(OPEN);
        CASE(WEP);
        CASE(WPA_TKIP);
        CASE(WPA_CCMP);
        CASE(WPA2_TKIP);
        CASE(WPA2_CCMP);
        CASE(WPS);

    default:
        // This function directly feeds the configure scripts - it should return a valid value
        // if all else fails, rather than ANY.
        return "OPEN";
    }
}

OnboardingControllerImpl::OnboardingControllerImpl(OBConcurrency concurrency, BusAttachment& busAttachment) :
    mBus(&busAttachment), mConcurrency(concurrency), 
	mWifiController(new WifiController()), mState(CTRL_NOT_READY), mSoftApSSID(NULL)
{
	// if the mConcurrency values are out of range, set it to min
    if (mConcurrency < CONCURRENCY_MIN || mConcurrency > CONCURRENCY_MAX) {
        mConcurrency = CONCURRENCY_MIN;
    }

	mWifiController->AddListener(this);
	mState = GetWifiState();
	mWifiController->Init();
	mWifiController->Start();
}

OnboardingControllerImpl::~OnboardingControllerImpl()
{
    LOGD(TAG, "entered");

	if (mWifiController) {
		delete mWifiController;
		mWifiController = NULL;
	}

	if (mSoftApSSID) {
		free(mSoftApSSID);
		mSoftApSSID = NULL;
	}
}

void OnboardingControllerImpl::ConfigureWiFi(qcc::String SSID, qcc::String passphrase, short authType, short& status, qcc::String&  error, qcc::String& errorMessage) {
    LOGD(TAG, "entered");

    // Set the return value based on presence of fast switching feature
    status = mConcurrency;

	if (mWifiController)
	    mWifiController->Configure(SSID.c_str(), authType, passphrase.c_str());
}

void OnboardingControllerImpl::Connect()
{
    LOGD(TAG, "entered");
	if (mWifiController)
		mWifiController->SetState(CTRL_STATION);
}

void OnboardingControllerImpl::Offboard()
{
    LOGD(TAG, "entered");
	if (mWifiController) {
		mWifiController->SetSoftApSSID(mSoftApSSID);
		mWifiController->SetState(CTRL_SOFTAP);
	}
}

ControllerState OnboardingControllerImpl::GetWifiState()
{
    LOGD(TAG, "entered");

	if (mWifiController)
		return mWifiController->GetState();
	else
		return CTRL_NOT_READY;
}

short OnboardingControllerImpl::GetState()
{
	ControllerState state = GetWifiState();
	if (state == CTRL_STATION)
		return 1;
	else if (state == CTRL_SOFTAP)
		return 2;
	else
		return 0;

	mState = state;
}

void OnboardingControllerImpl::SetSoftApSSID(const char* SSID)
{
	mSoftApSSID = strdup(SSID);
}

const OBLastError& OnboardingControllerImpl::GetLastError()
{
	return mOBLastError;
}

void OnboardingControllerImpl::GetScanInfo(unsigned short& age, ajn::services::OBScanInfo*& scanInfoList, size_t& scanListNumElements)
{
	return;
}

void OnboardingControllerImpl::StateChanged(ConnectionType connType, ControllerState state)
{
	LOGD(TAG, "State chagned %d -> %d", mState, state);
	// Restart AJ daemon after switch station or softap
	if ((mState != state && state == CTRL_STATION) || (mState != state && state == CTRL_SOFTAP)) {
		pthread_t t;
		pthread_create(&t, NULL, OnboardingControllerImpl::RestartDaemonThread, this);
		pthread_detach(t);
	}
	
	mState = state;
}

void OnboardingControllerImpl::DeviceRemoved(ConnectionType connType, uint16_t nodeId)
{
}

void OnboardingControllerImpl::Reported(ConnectionType connType, RepDataT data)
{
}

void OnboardingControllerImpl::BatteryLevel(ConnectionType connType, int val)
{
}

void* OnboardingControllerImpl::RestartDaemonThread(void* arg)
{
	LOGD(TAG, "");
	int result = system("/etc/rc.d/S70alljoyn restart");
	LOGD(TAG, "result %d", result);
	return NULL;
}
