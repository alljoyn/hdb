#ifndef _WIFICONTROLLER_H
#define _WIFICONTROLLER_H

#include <pthread.h>

#include "ControllerDef.h"
#include "ControllerIface.h"

#define WPA_TIMEOUT			5

class WifiController : public ControllerIface {
	public:
		WifiController();
		~WifiController();

		// ControllerIface functions
		ControllerState Init();
		ControllerState Start();
		void Final();

		ControllerState GetState();
		void SetState(ControllerState state);
		
		// not used in wifi
		void GetList();

		void Reset();

		void AddConnListener(ConnectionListener* listener);
		void RemoveConnListener(ConnectionListener* listener);

		// not used in wifi
		void AddListener(ControllerListener* listener);
		void RemoveListener(ControllerListener* listener);

		// WifiController's API
		void Configure(const char* SSID, const int authType, const char* passphrase);
		int SetSoftApSSID(const char* SSID);

		void WifiSendCommand(const char* arg);

		int InitWifiConf();		
		int InitSoftApConf();
		int WifiStartSoftap();	
		int WifiStartStation();

		static void *MonitorState(void *arg);
		int CheckWpaState();

	private:
		ControllerState mState;
		ControllerListener* mControllerListener;

		pthread_t mMonitorThread;
};

#endif // _WIFICONTROLLER_H

