#include "VirtualDevice.h"

#include "Icons.h"
#include "common/AJInitializer.h"
#include "utils/Log.h"
#include "utils/FileUtil.h"

#include <alljoyn/AllJoynStd.h>
#include <alljoyn/services_common/LogModulesNames.h>

#define TAG "VirtualDevice"

using namespace ajn;
using namespace qcc;
using namespace std;

namespace ajn {
namespace services {
	
bool VirtualDevice::mDaemonDisconnected = false;

	VirtualDevice::VirtualDevice(ConnectionType connType, DeviceType devType, uint16_t id, const char* addtional) :
		mOptions(NULL), mController(NULL), mId(id), 
		mBus(NULL), mKeyListener(NULL), mBusListener(NULL), 
		mAboutDataStore(NULL), mAboutObj(NULL), mAboutApi(NULL), 
		mIcon(NULL), mAboutIconObj(NULL),
		mOnboardingController(NULL), mOnboardingService(NULL),
		mConfigService(NULL), mConfigServiceListenerImpl(NULL),
		mNotiService(NULL), mNotiSender(NULL),
		mEventsAndActions(NULL),
		mServicePort(900), mListener(NULL)
	{
		VDLOGD(TAG, id, "new virtual device creating for connType:%d, devType: %d, addtional: %s", connType, devType, addtional);

		mOptions = new Options(connType, devType, id, addtional);
	}

	VirtualDevice::~VirtualDevice() 
	{
		VDLOGD(TAG, mId, "");

		if (mController)
			delete mController;

		if (mAboutApi) {
			delete mAboutApi;
			mAboutApi = NULL;
		}

		if (mKeyListener) {
			delete mKeyListener;
			mKeyListener = NULL;
		}

		if (mAboutIconObj) {
			delete mAboutIconObj;
			mAboutIconObj = NULL;
		}

		if (mIcon) {
			delete mIcon;
			mIcon = NULL;
		}

		if (mAboutDataStore) {
			delete mAboutDataStore;
			mAboutDataStore = NULL;
		}

		if (mAboutObj) {
			delete mAboutObj;
			mAboutObj = NULL;
		}

		if (mBusListener) {
			if (mBus) {
				mBus->UnregisterBusListener(*mBusListener);
			}
			delete mBusListener;
			mBusListener = NULL;
		}

		if (mConfigService) {
			delete mConfigService;
			mConfigService = NULL;
		}

		if (mConfigServiceListenerImpl) {
			delete mConfigServiceListenerImpl;
			mConfigServiceListenerImpl = NULL;
		}
		
		if (mOnboardingController) {
			delete mOnboardingController;
			mOnboardingController = NULL;
		}

		if (mOnboardingService) {
			delete mOnboardingService;
			mOnboardingService = NULL;
		}

		if (mEventsAndActions) {
			delete mEventsAndActions;
			mEventsAndActions = NULL;
		}

		if (mNotiService) {
			mNotiService->shutdown();
			mNotiService = NULL;
		}

		if (mNotiSender) {
			delete mNotiSender;
			mNotiSender = NULL;
		}
		
		if (mBus) {
			delete mBus;
			mBus = NULL;
		}

		if (mOptions) {
			delete mOptions;
			mOptions = NULL;
		}
	}

	QStatus VirtualDevice::Init()
	{
		QStatus status = ER_OK;

		VDLOGD(TAG, mId, "");

    	status = mOptions->ParseResult();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "ParseResult() returns fail.");
			return status;
		}

		status = InitAllJoyn();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "InitAllJoyn() returns fail.");
			return status;
		}

		status = InitController();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "InitController() returns fail.");
			return status;
		}

		VDLOGD(TAG, mId, "");
		return ER_OK;
	}

	void VirtualDevice::DeviceRemoved(ConnectionType connType, uint16_t nodeId) 
	{
		VDLOGD(TAG, mId, "");
		if (mListener) {

			if (CONN_ZIGBEE == mOptions->GetConnectionType()) {
				FU_RemoveFile(mOptions->GetConfigFile().c_str());
				FU_RemoveFile(mOptions->GetFactoryConfigFile().c_str());
			}

			mListener->StateChanged(mOptions->GetConnectionType(), mId, VD_REMOVED);
		}
		else {
			VDLOGD(TAG, mId, "mListener is null.");
		}
	}

	void VirtualDevice::StateChanged(ConnectionType connType, ControllerState state)
	{
		VDLOGD(TAG, mId, "");
	}

	void VirtualDevice::Reported(ConnectionType connType, RepDataT data)
	{
		VDLOGD(TAG, mId, "");
		ConnectionType conn = mOptions->GetConnectionType();
		DeviceType dev = mOptions->GetDeviceType();

		// Zigbee
		if (CONN_ZIGBEE == conn) {
			// door
			if (DEV_DOOR == dev) {
				if (data.bin.flag)
					SendEvent("Opened");
				else
					SendEvent("Closed");
			} 
			// temperture & humidity
			else if (DEV_TEMP_HUMIDITY == dev) {
				int temp = data.th.temp;
				int humi = data.th.humi;

				if (temp != mRepData.th.temp)
					SendEvent("TemperatureChanged");
				if (humi != mRepData.th.humi)
					SendEvent("HumidityChanged");

				if (temp > 30 && mRepData.th.temp <= 30)
					SendEvent("TemperatureOver30");
				else if (temp < 20 && mRepData.th.temp >= 20)
					SendEvent("TemperatureUnder20");
				
				if (humi > 60 && mRepData.th.humi <= 60)
					SendEvent("HumidityOver60");
				else if (humi < 45 && mRepData.th.humi >= 45)
					SendEvent("HumidityUnder45");
			}
			// plug
			else if (DEV_PLUG == dev) {
				if (data.bin.flag)
					SendEvent("TurnedOn");
				else
					SendEvent("TurnedOff");
			}
		}
		else {
			VDLOGD(TAG, mId, "Not implemented connection type %d", conn);
		}
		
		mRepData = data;
	}

	void VirtualDevice::BatteryLevel(ConnectionType connType, int val)
	{
		VDLOGD(TAG, mId, "connType: %d, val: %d", connType, val);

		if (val <= ZIGBEE_LOW_BATTERY_LEVEL)
			NotiRequstToBridge("Battery is low.");
	}

	void VirtualDevice::ActionCallback(const char* member, ajn::Message& msg)
	{
		VDLOGD(TAG, mId, "Action member %s called", member);

		ConnectionType connType = mOptions->GetConnectionType();
		DeviceType devType = mOptions->GetDeviceType();

		if (CONN_ZIGBEE == connType) {
			ZigbeeController* cont = (ZigbeeController*)mController;
			// remove
			if (strcmp(ACTION_MEMBER_REMOVE, member) == 0) {
				VDLOGD(TAG, mId, "Call Zigbee %s", member);
				cont->RemoveNode();
			}

			if (DEV_PLUG == devType) {
				if (strcmp("TurnOn", member) == 0) 
					cont->SetBinary(true);
				else if (strcmp("TurnOff", member) == 0) 
					cont->SetBinary(false);
			}
		}
	}

	void VirtualDevice::AddListener(VirtualDeviceListener* listener)
	{
		mListener = listener;
	}

	Options* VirtualDevice::GetOptions()
	{
		return mOptions;
	}

	BusAttachment* VirtualDevice::GetBusAttachment()
	{
		return mBus;
	}

	bool VirtualDevice::IsDaemonDisconnected()
	{
		return mDaemonDisconnected;
	}

	void VirtualDevice::readPassword(AboutDataStore& aboutDataStore, qcc::String& passCode) 
	{
		ajn::MsgArg* argPasscode;
		char* tmp;
		aboutDataStore.GetField("Passcode", argPasscode);
		argPasscode->Get("s", &tmp);
		passCode = tmp;
		return;
	}

	QStatus VirtualDevice::InitAllJoyn()
	{
		LOGD(TAG, "");

		QStatus status = ER_OK;
		
		////////// 1. Initialize AllJoyn
		// alljoyn init
		AJInitializer ajInit;
		if (ajInit.Initialize() != ER_OK) {
			VDLOGD(TAG, mId, "AllJoyn initialize failed");
			return ER_FAIL;
		}

		// create AboutApi & AboutDataStore
		mAboutApi = new AboutObjApi();
		if (!mAboutApi) {
			VDLOGD(TAG, mId, "Could not set up the AboutObjApi.");
			return ER_FAIL;
		}
		mAboutDataStore = new AboutDataStore(mOptions->GetFactoryConfigFile().c_str(), mOptions->GetConfigFile().c_str(), mAboutApi);

		// get message bus
		mKeyListener = new SrpKeyXListener(mAboutDataStore);
		mBus = CommonUtil::prepareBusAttachment(mKeyListener);
		if (mBus == NULL) {
			VDLOGD(TAG, mId, "Could not initialize BusAttachment.");
			return ER_FAIL;
		}
    
		// register disconnect callback & set port
		mBusListener = new CommonBusListener(mBus, AllJoynDaemonDisconnectCB);
	    mBusListener->setSessionPort(mServicePort);

		// AboutDataStore init
		mAboutDataStore->Initialize(mOptions->GetDeviceId(), mOptions->GetAppId(), mOptions->GetDeviceName());
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "mAboutDataStore->Initialize fail.");
			return ER_FAIL;
		}

		// create AboutObj
		mAboutObj = new ajn::AboutObj(*(mBus), BusObject::ANNOUNCED);
		// set values for about
		status = CommonUtil::prepareAboutService(mBus, static_cast<AboutData*>(mAboutDataStore), 
				mAboutObj, mAboutApi, mBusListener, mServicePort);
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "Could not set up the AboutService.");
			return ER_FAIL;
		}

		// register icon
		DeviceType type = mOptions->GetDeviceType();
		uint8_t* aboutIconContent = DeviceIconPtr[type];
		size_t aboutIconSize = DeviceIconSize[type];
		qcc::String mimeType("image/png");
		mIcon = new ajn::AboutIcon();
		status = mIcon->SetContent(mimeType.c_str(), aboutIconContent, aboutIconSize);
		LOGD(TAG, "%s icon size %d", DeviceTypeStr[type], aboutIconSize);
		if (ER_OK != status) {
			VDLOGD(TAG, mId, "Failed to setup the AboutIcon content.\n");
		}
		/* uncomment when url is supported
		status = mIcon->SetUrl(mimeType.c_str(), "URL");
		if (ER_OK != status) {
			VDLOGD(TAG, mId, "Failed to setup the AboutIcon url.\n");
		}
		*/

		mAboutIconObj = new AboutIconObj(*(mBus), *(mIcon));
		
		// register ConfigService
		if (mOptions->GetConnectionType() == CONN_BRIDGE) {
			status = InitOnboarding();
			if (status != ER_OK) {
				VDLOGD(TAG, mId, "InitOnboarding failed.");
				return ER_FAIL;
			}
			mConfigServiceListenerImpl = new ConfigServiceListenerImpl(*mAboutDataStore, *mBus, *mBusListener, *mAboutApi, *mOnboardingController);
		}
		mConfigService = new ConfigService(*(mBus), *(mAboutDataStore), *(mConfigServiceListenerImpl));
		mKeyListener->setGetPassCode(readPassword);

		status = mConfigService->Register();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "Could not register the ConfigService.");
			return ER_FAIL;
		}

		status = mBus->RegisterBusObject(*(mConfigService));
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "Could not register the ConfigService BusObject.");
			return ER_FAIL;
		}

		if (mOptions->GetConnectionType() == CONN_BRIDGE)
			mConfigServiceListenerImpl->AddListener(this);

		// Evnets & Actions
		status = InitEvents();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "InitEvents failed.");
			return ER_FAIL;
		}

		status = CreateEvents();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "CreateEvents failed.");
			return ER_FAIL;
		}

		status = InitActions();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "InitActions failed.");
			return ER_FAIL;
		}

		status = CreateActions();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "CreateActions failed.");
			return ER_FAIL;
		}

		status = RegisterEA();
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "RegisterEA failed.");
			return ER_FAIL;
		}

		// Notification
		if (mOptions->GetConnectionType() == CONN_BRIDGE) {
			mNotiService = NotificationService::getInstance();
			QCC_SetDebugLevel(logModules::NOTIFICATION_MODULE_LOG_NAME, logModules::ALL_LOG_LEVELS);
			mNotiSender = mNotiService->initSend(mBus, mAboutDataStore);
			if (!mNotiSender) {
				VDLOGD(TAG, mId, "Could not initialize Sender - exiting application");
				return ER_FAIL;
			}
		}

		// announce
		status = CommonUtil::aboutServiceAnnounce(mAboutApi);
		if (status != ER_OK) {
			VDLOGD(TAG, mId, "Could not announce.");
			return ER_FAIL;
		}

		return status;
	}

	QStatus VirtualDevice::InitController()
	{
		ControllerState state = CTRL_NOT_READY;

		ConnectionType connType = mOptions->GetConnectionType();
		DeviceType devType = mOptions->GetDeviceType();
		if (connType == CONN_BRIDGE) {
			VDLOGD(TAG, mId, "Skip Bridge");
			return ER_OK;
		}
		else if (connType == CONN_ZIGBEE) {
			mController = new ZigbeeController(mId, devType);
		}
		else {
			VDLOGD(TAG, mId, "connType: %d is not supported yet.", connType);
			return ER_FAIL;
		}
		
		if (mController == NULL) {
			VDLOGD(TAG, mId, "mController is null.");
			return ER_FAIL;
		}

		state = mController->Init();
		if (state != CTRL_INITIALIZED) {
			VDLOGD(TAG, mId, "mController->Init() failed.");
			return ER_FAIL;
		}

		mController->AddListener(this);
		
		state = mController->Start();
		if (state != CTRL_READY) {
			VDLOGD(TAG, mId, "mController->Start() failed.");
			return ER_FAIL;
		}

		if (connType == CONN_ZIGBEE) {
			((ZigbeeController*)mController)->SetInterval(60);
		}
		return ER_OK;
	}

	QStatus	VirtualDevice::InitEvents()
	{
		QStatus status = ER_OK;
		
		DeviceType devType = mOptions->GetDeviceType();
		ConnectionType connType = mOptions->GetConnectionType();

		char* eIface = NULL;
		char* aIface = NULL;
		char* servicePath = NULL;
		char (*members)[MEMBER_NAME_LEN] = NULL;
		pCustomEventT events = NULL;
		int eventNum = 0;

		char sendEventIface[128] = {0,};
		char sendActionIface[128] = {0,};

		if (devType == DEV_BRIDGE) {
			eIface = BridgeEventInterface;
			aIface = BridgeActionInterface;
			servicePath = BridgeServicePath;
			members = BridgeEventMembers;
			events = BridgeEvents;
			eventNum = BRIDGE_EVENT_NUM;
		}
		else if (devType == DEV_DOOR) {
			eIface = DoorEventInterface;
			aIface = DoorActionInterface;
			servicePath = DoorServicePath;
			members = DoorEventMembers;
			events = DoorEvents;
			eventNum = DOOR_EVENT_NUM;
		}
		else if (devType == DEV_TEMP_HUMIDITY) {
			eIface = TempHumiEventInterface;
			aIface = TempHumiActionInterface;
			servicePath = TempHumiServicePath;
			members = TempHumiEventMembers;
			events = TempHumiEvents;
			eventNum = TEMPHUMI_EVENT_NUM;
		}
		else if (devType == DEV_PLUG) {
			eIface = PlugEventInterface;
			aIface = PlugActionInterface;
			servicePath = PlugServicePath;
			eventNum = PLUG_EVENT_NUM;
		}
		else {
			VDLOGD(TAG, mId, "Unknown device type %d", devType);
			return ER_NOT_IMPLEMENTED;
		}

		mEventsAndActions = new EventsAndActions(mBus, servicePath, eIface, aIface);
		
		for (int i=0; i < eventNum; i++) {
			mCustomEvents[members[i]] = events[i];
		}
		VDLOGD(TAG, mId, "Has %d event(s).", mCustomEvents.size());

		return status;
	}

	QStatus	VirtualDevice::CreateEvents()
	{
		QStatus status = ER_OK;
		char* desc = BridgeEventDesc;

		CustomEventMap::iterator it;
		for (it = mCustomEvents.begin(); it != mCustomEvents.end(); ++it) {
			char* member = it->first;
			CustomEventT* cet = &(it->second);
			VDLOGD(TAG, mId, "Adding EventName: %s, description: %s, options: %d, data: %d", member, cet->desc, cet->option, cet->data);

			if (mEventsAndActions->AddEvent(member, cet->desc) != ER_OK) {
				VDLOGD(TAG, mId, "Fail to Add EventName: %s", member);
				return ER_FAIL;
			}
		}
		mEventsAndActions->AddEventProperty();
		mEventsAndActions->SetEventsDescription(desc);
		mEventsAndActions->SetEventsComplete();

		return status;
	}

	QStatus	VirtualDevice::InitActions()
	{
		QStatus status = ER_OK;
		
		DeviceType devType = mOptions->GetDeviceType();
		ConnectionType connType = mOptions->GetConnectionType();

		char* desc = BridgeActionDesc;
		char (*members)[MEMBER_NAME_LEN] = NULL;
		pCustomActionT actions;
		int actionNum = 0;

		if (devType == DEV_BRIDGE) {
			members = BridgeActionMembers;
			actions = BridgeActions;
			actionNum = BRIDGE_ACTION_NUM;
		}
		else if (devType == DEV_DOOR) {
			members = DoorActionMembers;
			actions = DoorActions;
			actionNum = DOOR_ACTION_NUM;
		}
		else if (devType == DEV_TEMP_HUMIDITY) {
			members = TempHumiActionMembers;
			actions = TempHumiActions;
			actionNum = TEMPHUMI_ACTION_NUM;
		}
		else if (devType == DEV_PLUG) {
			members = PlugActionMembers;
			actions = PlugActions;
			actionNum = PLUG_ACTION_NUM;
		}
		else {
			VDLOGD(TAG, mId, "Unknown device type %d", devType);
			return ER_NOT_IMPLEMENTED;
		}

		mEventsAndActions->AddAcionListener(this);
		mEventsAndActions->AddActionProperty();
		mEventsAndActions->SetActionsDescription(desc);
		
		for (int i=0; i < actionNum; i++) {
			mCustomActions[members[i]] = actions[i];
		}
		VDLOGD(TAG, mId, "Has %d action(s).", mCustomActions.size());

		return status;
	}

	QStatus	VirtualDevice::CreateActions()
	{
		QStatus status = ER_OK;

		CustomActionMap::iterator it;
		for (it = mCustomActions.begin(); it != mCustomActions.end(); ++it) {
			char* member = it->first;
			CustomActionT* cet = &(it->second);
			VDLOGD(TAG, mId, "Adding ActionName: %s, description: %s, options: %d, data: %d", member, cet->desc, cet->option, cet->data);

			if (mEventsAndActions->AddAction(member, cet->desc, cet->inputSig, cet->outputSig, cet->argNames) != ER_OK) {
				VDLOGD(TAG, mId, "Fail to Add EventName: %s", member);
				return ER_FAIL;
			}
		}
		mEventsAndActions->SetActionsComplete();

		return status;
	}

	QStatus VirtualDevice::RegisterEA()
	{
		return mEventsAndActions->RegisterEABusObject();
	}

	void VirtualDevice::SendEvent(const char* name)
	{
		VDLOGD(TAG, mId, "Send event: %s", name);
		if (mEventsAndActions)
			mEventsAndActions->SendEventByName(name);
	}

	QStatus VirtualDevice::InitOnboarding()
	{
		QStatus status;

		char ssid[32] = {0, };
		char* devName = GetOptions()->GetDeviceName();

		sprintf(ssid, "AJ_%s", devName);

		mOnboardingController = new OnboardingControllerImpl((OBConcurrency)0, *(GetBusAttachment()));
		mOnboardingController->SetSoftApSSID(ssid);

		mOnboardingService = new OnboardingService(*(GetBusAttachment()), *mOnboardingController);
		status = mOnboardingService->Register();
		if (status != ER_OK) {
			LOGD(TAG, "Register OnboardingService error.");
			return status;
		}

		status = GetBusAttachment()->RegisterBusObject(*mOnboardingService);
		if (status != ER_OK) {
			LOGD(TAG, "RegisterBusObject(OnboardingService) error.");
			return status;
		}
		return status;
	}

	ControllerState VirtualDevice::GetNetworkState()
	{
		if (mOnboardingController) {
			//TODO
			// for test
			return mOnboardingController->GetWifiState();
//			return CTRL_STATION;
		}
		return CTRL_NOT_READY;
	}

	void VirtualDevice::AllJoynDaemonDisconnectCB()
	{
		LOGD(TAG, "");
		mDaemonDisconnected = true;
	}

	void VirtualDevice::SendState(VDeviceState state)
	{
		if (mListener)
			mListener->StateChanged(mOptions->GetConnectionType(), mId, state);
	}

	void VirtualDevice::NotiRequstToBridge(const char* msg)
	{
		if (mListener)
			mListener->NotiRequest(mId, mOptions->GetDeviceName(), msg);
	}

	// TEST
	QStatus VirtualDevice::SendTestEvent()
	{
		if (mEventsAndActions)
			mEventsAndActions->SendEventByName("DeviceAdded");
		else
			return ER_FAIL;

		return ER_OK;
	}

	ConfigServiceListenerImpl* VirtualDevice::GetConfigListenerImpl()
	{
		return mConfigServiceListenerImpl;
	}

	NotificationService* VirtualDevice::GetNotiService()
	{
		if (mOptions->GetConnectionType() == CONN_BRIDGE) {
			return mNotiService;
		}
		return NULL;
	}

	NotificationSender* VirtualDevice::GetNotiSender()
	{
		if (mOptions->GetConnectionType() == CONN_BRIDGE) {
			return mNotiSender;
		}
		return NULL;
	}

	OnboardingControllerImpl* VirtualDevice::GetOnbController()
	{
		if (mOptions->GetConnectionType() == CONN_BRIDGE) {
			return mOnboardingController;
		}
		return NULL;
	}

	void VirtualDevice::Restart()
	{
		VDLOGD(TAG, mId, "");
	}

	void VirtualDevice::FactoryReset()
	{
		VDLOGD(TAG, mId, "");
	}

	ControllerIface* VirtualDevice::GetController()
	{
		return mController;
	}
} //namespace services
} //namespace ajn


