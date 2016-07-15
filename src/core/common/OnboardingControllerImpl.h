#ifndef _ONBOARDINGCONTROLLERIMPL_H
#define _ONBOARDINGCONTROLLERIMPL_H

#include "plugins/ControllerDef.h"
#include "plugins/WifiController.h"

#include <alljoyn/onboarding/OnboardingControllerAPI.h>

class OnboardingControllerImpl : public ajn::services::OnboardingControllerAPI, public ControllerListener {

  public:

    OnboardingControllerImpl(ajn::services::OBConcurrency concurency, ajn::BusAttachment& busAttachment);
    virtual ~OnboardingControllerImpl();

    /**
     * ConfigureWiFi passing connection info to connect to WIFI
     * @param[in] SSID  of WIFI AP
     * @param[in] passphrase of WIFI AP in hex
     * @param[in] authType used by  WIFI AP
     * @param[out] status
     * @param[out] error
     * @param[out] errorMessage
     */
    virtual void ConfigureWiFi(qcc::String SSID, qcc::String passphrase, short authType, short& status, qcc::String& error, qcc::String& errorMessage);

    virtual void Connect();
    virtual void Offboard();

	void SetSoftApSSID(const char* SSID);

    ControllerState GetWifiState();
    
	// 0: none, 1: station, 2: softap
	virtual short GetState();
    virtual const ajn::services::OBLastError& GetLastError();
	virtual void GetScanInfo(unsigned short& age, ajn::services::OBScanInfo*& scanInfoList, size_t& scanListNumElements);

	// ControllerListener
	void StateChanged(ConnectionType connType, ControllerState state);
	void DeviceRemoved(ConnectionType connType, uint16_t nodeId);		// not use
	void Reported(ConnectionType connType, RepDataT data);				// not use
	void BatteryLevel(ConnectionType connType, int val);											// not use

	static void* RestartDaemonThread(void* arg);

  private:
    virtual OnboardingControllerImpl operator=(const OnboardingControllerImpl& otherOnboardingControllerAPI) { return *this; }
    OnboardingControllerImpl(const OnboardingControllerImpl& otherOnboardingControllerImpl) { }

    ajn::BusAttachment* mBus;
    ajn::services::OBConcurrency mConcurrency;

	ajn::services::OBLastError mOBLastError;

	WifiController* mWifiController;
	ControllerState mState;

	char* mSoftApSSID;
};

#endif
