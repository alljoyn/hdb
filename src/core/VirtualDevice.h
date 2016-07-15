#ifndef _VIRTUALDEVICE_H
#define _VIRTUALDEVICE_H

#include "Options.h"
#include "EventActionDef.h"
#include "EventsAndActions.h"
#include "common/CommonUtil.h"
#include "common/SrpKeyXListener.h"
#include "common/AboutDataStore.h"
#include "common/AboutObjApi.h"
#include "common/ConfigServiceListenerImpl.h"
#include "common/OnboardingControllerImpl.h"

#include "plugins/ZigbeeController.h"

#include <alljoyn/onboarding/OnboardingService.h>
#include <alljoyn/notification/NotificationService.h>
#include <alljoyn/notification/Notification.h>
#include <alljoyn/notification/NotificationText.h>

#include <alljoyn/config/ConfigService.h>

#include <alljoyn/AboutIconObj.h>
#include <qcc/Thread.h>

enum VDeviceState {
	VD_NOT_READY		= 0,
	VD_READY,

	VD_INITIALIZED,
	VD_INIT_FAILED,

	VD_REMOVED
};

namespace qcc {
class Mutex;
class Thread;
}

namespace ajn {
namespace services {

class VirtualDeviceListener {
	public:
		virtual void StateChanged(ConnectionType connType, uint16_t id, VDeviceState state) = 0;
		virtual void NotiRequest(uint16_t id, char* deviceName, const char* msg) = 0;
};

class VirtualDevice : public ControllerListener, public ActionListener, public ConfigListener {
	public:
		VirtualDevice(ConnectionType connType, DeviceType devType, uint16_t id, const char* addtional=NULL);
		~VirtualDevice();

		QStatus Init();

		// ControllerListener
		void DeviceRemoved(ConnectionType connType, uint16_t nodeId);
		void StateChanged(ConnectionType connType, ControllerState state);
		void Reported(ConnectionType connType, RepDataT data);
		void BatteryLevel(ConnectionType connType, int val);

		// ActionListener
		void ActionCallback(const char* member, ajn::Message& msg);

		// ConfigListener
		void Restart();
		void FactoryReset();

		void AddListener(VirtualDeviceListener* listener);

		Options* GetOptions();
		BusAttachment* GetBusAttachment();
		bool IsDaemonDisconnected();

		ControllerIface* GetController();
		
		ControllerState GetNetworkState();

		void SendEvent(const char* name);

		ConfigServiceListenerImpl* GetConfigListenerImpl();

		NotificationService* 	GetNotiService();
		NotificationSender* 	GetNotiSender();

		OnboardingControllerImpl* GetOnbController();

		// TEST
		QStatus SendTestEvent();

	public:
		static bool mDaemonDisconnected;

	private:
		QStatus InitAllJoyn();

		QStatus	InitEvents();
		QStatus CreateEvents();
		QStatus	InitActions();
		QStatus CreateActions();
		QStatus RegisterEA();

		QStatus InitController();
		QStatus InitOnboarding();

		void SendState(VDeviceState state);
		void NotiRequstToBridge(const char* msg);
		
		static void AllJoynDaemonDisconnectCB();
		static void readPassword(AboutDataStore& aboutDataStore, qcc::String& passCode);

	private:
		Options*			mOptions;
		ControllerIface* 	mController;
		uint16_t 			mId;

		BusAttachment* 		mBus;
		SrpKeyXListener* 	mKeyListener;
		CommonBusListener* 	mBusListener;

		AboutDataStore* 	mAboutDataStore;
		AboutObj* 			mAboutObj;
		AboutObjApi*		mAboutApi;

		AboutIcon* 			mIcon;
		AboutIconObj* 		mAboutIconObj;

		OnboardingControllerImpl* 	mOnboardingController;
		OnboardingService*			mOnboardingService;

		ConfigService* 				mConfigService;
		ConfigServiceListenerImpl* 	mConfigServiceListenerImpl;

		NotificationService* 	mNotiService;
		NotificationSender* 	mNotiSender;

		EventsAndActions* 	mEventsAndActions;
		CustomEventMap	 	mCustomEvents;
		CustomActionMap		mCustomActions;

		SessionPort 		mServicePort;

		VirtualDeviceListener* mListener;

		RepDataT	mRepData;
};

} //namespace services
} //namespace ajn

#endif // _VIRTUALDEVICE_H
