#include "Bridge.h"
#include "utils/Log.h"

#define TAG "Bridge"

using namespace ajn;
using namespace qcc;
using namespace std;

namespace ajn {
namespace services {

	Bridge::Bridge() : 
		VirtualDevice(CONN_BRIDGE, DEV_BRIDGE, 0),
		mZigbeeController(NULL),
		mVDevicesMutex(new qcc::Mutex())
	{
		VirtualDevice::mDaemonDisconnected = false;
	}

	Bridge::~Bridge()
	{
		LOGD(TAG, "");

		if (mZigbeeController) {
			delete mZigbeeController;
			mZigbeeController = NULL;
		}
		mVDevicesMutex->Lock();
		if (mVDevices.size() > 0) {
			VDMap::iterator it;
			for (it = mVDevices.begin(); it != mVDevices.end(); ++it) {
				if (it->second != NULL)
					delete it->second;
			}
			mVDevices.clear();
		}
		mVDevicesMutex->Unlock();		

		if (mVDevicesMutex) {
			delete mVDevicesMutex;
			mVDevicesMutex = NULL;
		}
		LOGD(TAG, "");
	}
	
	QStatus Bridge::InitControllers()
	{
		// Zigbee
		mZigbeeController = new ZigbeeController(0, DEV_BRIDGE);
		if (!mZigbeeController) {
			LOGD(TAG, "ZigbeeController create failed.");
		}
		else {
			mZigbeeController->AddConnListener(this);
			mZigbeeController->AddListener(this);
			if (mZigbeeController->Init() != CTRL_INITIALIZED) {
				LOGD(TAG, "ZigbeeController init failed.");
			}
			else {
				if (mZigbeeController->Start() != CTRL_READY) {
					LOGD(TAG, "ZigbeeController start failed.");
				}
				else {
					mZigbeeController->GetList();
				}
			}
		}
		
		return ER_OK;
	}

	// ConnectionListener
	void Bridge::DeviceFound(ConnectionType connType, DeviceType devType, uint16_t nodeId, const char* addtional)
	{
		LOGD(TAG, "Found connType: %d, devType: %d, nodeId: %d, addtional: %s", connType, devType, nodeId, addtional);
		uint32_t uid = GetUID(connType, nodeId);
		if (mVDevices[uid]) {
			LOGD(TAG, "Device already exist connType: %d, devType: %d, nodeId: %d", connType, devType, nodeId);
			return;
		}

		VirtualDevice* vd = new VirtualDevice(connType, devType, nodeId, addtional);
		if (vd == NULL) {
			LOGD(TAG, "Fail to create device connType: %d, devType: %d, nodeId: %d", connType, devType, nodeId);
			return;
		}

		if (vd->Init() != ER_OK) {
			LOGD(TAG, "Fail to init device connType: %d, devType: %d, nodeId: %d", connType, devType, nodeId);
			delete vd;
			return;
		}

		vd->AddListener(this);

		mVDevicesMutex->Lock();
		mVDevices[uid] = vd;
		mVDevicesMutex->Unlock();		

		SendEvent("DeviceAdded");
		
		char msg[64] = {0,};
		sprintf(msg, "%s found.", vd->GetOptions()->GetDeviceName());
		SendNoti(msg);
	}

	void Bridge::DeviceLost(ConnectionType connType, uint16_t nodeId)
	{
		VirtualDevice* vd = NULL;
		uint32_t uid = GetUID(connType, nodeId);
		
		LOGD(TAG, "Lost connType: %d, nodeId: %d -> uid: %d", connType, nodeId, uid);

		mVDevicesMutex->Lock();
		if (mVDevices.count(uid) > 0) {
			vd = mVDevices[uid];

			if (vd != NULL) {
				char msg[64] = {0,};
				sprintf(msg, "%s removed.", vd->GetOptions()->GetDeviceName());
				SendNoti(msg);

				SendEvent("DeviceRemoved");
				delete vd;
			}

			LOGD(TAG, "");
			mVDevices.erase(uid);
			LOGD(TAG, "");
		}
		mVDevicesMutex->Unlock();		
	}

	// ControllerListener
	void Bridge::DeviceRemoved(ConnectionType connType, uint16_t nodeId)
	{
	}

	void Bridge::StateChanged(ConnectionType connType, ControllerState state)
	{
		LOGD(TAG, "connType: %d, state: %d", connType, state);
		if (CONN_ZIGBEE == connType) {
			if (CTRL_ADD_ENABLED == state) {
				SendEvent("ZigbeeAddMode");
				SendNoti("Entered Zigbee add mode");
			}
			else if (CTRL_READY == state) {
				SendEvent("ZigbeeReadyMode");
				SendNoti("Entered Zigbee ready(normal) mode");
			}
		}
	}

	void Bridge::Reported(ConnectionType connType, RepDataT data)
	{
	}

	// VirtualDeviceListener
	void Bridge::StateChanged(ConnectionType connType, uint16_t id, VDeviceState state)
	{
		LOGD(TAG, "connType: %d, id: %d, state: %d", connType, id, state);
		if (VD_REMOVED == state) {
			DeviceLost(connType, id);
		}
	}

	void Bridge::NotiRequest(uint16_t id, char* deviceName, const char* msg)
	{
		char* sendMsg = (char*)malloc(strlen(deviceName) + strlen(msg) + 4);

		LOGD(TAG, "NotiRequest from id: %d, name: %s, msg: %s", id, deviceName, msg);
		sprintf(sendMsg, "(%s) %s", deviceName, msg);
		SendNoti(sendMsg);

		free(sendMsg);
	}

	void Bridge::SendNoti(const char* msg)
	{
		NotificationSender* sender = GetNotiSender();
		if (sender != NULL) {
			NotificationMessageType messageType = NotificationMessageType(INFO);
			std::vector<NotificationText> vecMessages;
			uint16_t ttl = 600;
			NotificationText textToSend("en", msg);

			vecMessages.push_back(textToSend);
			Notification notification(messageType, vecMessages);

			if (sender->send(notification, ttl) != ER_OK) {
				LOGD(TAG, "Notification(%s) send failed", msg);
			}
			else {
				LOGD(TAG, "Notification(%s) send ok.", msg);
			}
		}
		else {
			LOGD(TAG, "Notification sender is null.");
		}
	}

	uint32_t Bridge::GetUID(ConnectionType connType, uint16_t nodeId)
	{
		return connType << 16 | nodeId;
	}

	// ActionListener
	void Bridge::ActionCallback(const char* member, ajn::Message& msg)
	{
		LOGD(TAG, "Action member %s called", member);

		// zigbee add
		if (strcmp("SetZigbeeAddModeOn", member) == 0) {
			mZigbeeController->SetState(CTRL_ADD_ENABLED);
		}
		else if (strcmp("SendTestNoti", member) == 0) {
			SendNoti("Notification send for test");
		}
	}

	void Bridge::Restart()
	{
		// TODO
		// call device restart action
	}

	void Bridge::FactoryReset()
	{
		LOGD(TAG, "");

		mVDevicesMutex->Lock();
		if (mVDevices.size() > 0) {
			VDMap::iterator it;
			for (it = mVDevices.begin(); it != mVDevices.end(); ++it) {
				if (it->second != NULL) {
					VirtualDevice* vd = (VirtualDevice*)it->second;
					ControllerIface* ci = vd->GetController();

					if (ci != NULL) {
						if (CONN_ZIGBEE == vd->GetOptions()->GetConnectionType()) {
							((ZigbeeController*)ci)->RemoveNode();
						}
					}
					usleep(10000);
				}
			}
		}
		mVDevicesMutex->Unlock();		

		if (mZigbeeController)
			mZigbeeController->Reset();

		// TODO
		// call device restart action
	}

	void Bridge::SetSoftAP()
	{
		OnboardingControllerImpl* onbCont = GetOnbController();
		if (onbCont == NULL) {
			LOGD(TAG, "GetOnbController() return null.");
			return;
		}
		onbCont->Offboard();
	}

}
}

