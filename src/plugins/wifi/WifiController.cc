#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include "GlobalDef.h"
#include "utils/Log.h"
#include "plugins/WifiController.h"

#define TAG "WifiController"

WifiController::WifiController() :
	mState(CTRL_NOT_READY), mControllerListener(NULL)
{
	LOGD(TAG, "created.");
}

WifiController::~WifiController()
{
	pthread_cancel(mMonitorThread);
}

void WifiController::WifiSendCommand(const char *arg)
{
	// TODO
	// Send wifi command by wpa_cli system cmd.
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "wpa_cli %s", arg);
	system(cmd);

	return;
}

int WifiController::InitWifiConf()
{
	// TODO
	// Initialize wpa_supplicant.conf file.

	return 0;
}

int WifiController::InitSoftApConf()
{
	// TODO
	// Need to setting SSID.

	return SetSoftApSSID("AJ_Bridge-123456");
}

int WifiController::WifiStartSoftap()
{
	// TODO
	// Start soft ap
	LOGD(TAG,"*************** start softap mode");
	
	// Send current wifi state
	if (mControllerListener){
		sleep(1);
		LOGD(TAG,"mControllerListener::StateChaged(CTRL_SOFTAP)");
		mControllerListener->StateChanged(CONN_NONE, CTRL_SOFTAP);
	}

	return 0;
}

int WifiController::WifiStartStation()
{
	// TODO
	// Start station mode
	LOGD(TAG,"=============== start station mode");

	// Send current wifi state
	if ( mControllerListener){
		sleep(1);
		LOGD(TAG,"mControllerListener::StateChaged(CTRL_STATION)");
		mControllerListener->StateChanged(CONN_NONE, CTRL_STATION);
	}

	return 0;
}

ControllerState WifiController::Init()
{
	// TODO
	// Init wifi state & return currenct state.
	
	mState = CTRL_INITIALIZED;
	return mState;
}

void* WifiController::MonitorState(void *pthis_)
{
	LOGD(TAG, "Monitor thread create");

	int mode, prev_mode;
	WifiController *pthis = (WifiController *)pthis_;
	int WpaStateTimeout = WPA_TIMEOUT;
	
	prev_mode = pthis->GetState();

	while(1) {
		mode = pthis->GetState();

		// If no configured before, set soft ap.
		if (mode == CTRL_NOT_READY)
			mode = CTRL_SOFTAP;
		
		if (mode != prev_mode) {
			LOGD(TAG, "state mode change : %d -> %d", prev_mode, mode);
			if (mode == CTRL_STATION)
				pthis->WifiStartStation();
			else 
				pthis->WifiStartSoftap();

			prev_mode = mode;
		}
		else {
			if (mode == CTRL_STATION) {
				if (pthis->CheckWpaState()) {	// When connection is failed
					if (WpaStateTimeout == 0) {
						// TODO
						// Re-connect to current AP.
						
						// Re-connect every 5 seconds while error.
						WpaStateTimeout = WPA_TIMEOUT;
					}
					WpaStateTimeout--;				
				}
				else
					WpaStateTimeout = WPA_TIMEOUT;
			}
		}
		sleep(1);
	}

	return 0;
}

int WifiController::CheckWpaState()
{
	// TODO
	// Check current wifi connection and return 0(OK) or 1(Not OK).

	return 0;
}

ControllerState WifiController::Start()
{
	int state = GetState();

	LOGD(TAG, "initial state : %d", state);

	switch(state){
		case CTRL_STATION :
			if (mControllerListener){
				LOGD(TAG,"mControllerListener::StateChaged(CTRL_STATION)");
				mControllerListener->StateChanged(CONN_NONE, CTRL_STATION);
			}
			break;
		case CTRL_SOFTAP :
			if (mControllerListener){
				LOGD(TAG,"mControllerListener::StateChaged(CTRL_SOFTAP)");
				mControllerListener->StateChanged(CONN_NONE, CTRL_SOFTAP);
			}
			break;
		case CTRL_NOT_READY :
			WifiStartSoftap();
			mState = CTRL_SOFTAP;
			break;
	}

	pthread_create(&mMonitorThread, NULL, WifiController::MonitorState, (void *)this);

	return CTRL_INITIALIZED;
}

void WifiController::Final()
{
}

ControllerState WifiController::GetState()
{
	// TODO
	// Need to get currecnt state.

	mState = CTRL_STATION;
	return mState;
}

void WifiController::SetState(ControllerState state)
{
}

void WifiController::Configure(const char* SSID, const int authType, const char* passphrase)
{
	// TODO
	// Set wifi configuration by SSID, autyType and passphrase.
}


int WifiController::SetSoftApSSID(const char* SSID)
{
	// TODO
	// Set softap.conf file.
	return 0;
}

void WifiController::Reset()
{
	// TODO
	// This function called when factory reset.
}

void WifiController::AddListener(ControllerListener* listener)
{
	mControllerListener = listener;
}

void WifiController::RemoveListener(ControllerListener* listener)
{
	mControllerListener = NULL;
}

// not used
void WifiController::GetList()
{
}

// not used
void WifiController::AddConnListener(ConnectionListener* listener)
{
}

// not used
void WifiController::RemoveConnListener(ConnectionListener* listener)
{
}

