#include "utils/Log.h"
#include "plugins/ZigbeeController.h"

#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <map>

#include <errno.h>

#define TAG "ZigbeeController"

using namespace std;

bool ZigbeeController::isInitialized;

ZigbeeDeviceList* ZigbeeController::singletonDeviceList;

int ZigbeeController::receiveThreadId;

pContInfoT ZigbeeController::mCoordinatorInfo;

ConnectionListener* ZigbeeController::coordinatorConnectionListener;
ControllerListener* ZigbeeController::coordinatorControllerListener;

ZigbeeController::ZigbeeController(uint16_t nodeId, DeviceType devType) :
	mInfo(NULL), mControllerListener(NULL), mConnectionListener(NULL) {

    LOGD(TAG, "Create ZigbeeController by nodeId %d, deviceType %d", nodeId, devType);

	mInfo = (ContInfoT*)malloc(sizeof(ContInfoT));
	if (mInfo == NULL){
		LOGD(TAG, "malloc fail");
		return;
	}
	mInfo->isInitialized = false;
	mInfo->id = nodeId;
	mInfo->connType = CONN_ZIGBEE;
	mInfo->devType = devType;
	mInfo->state = CTRL_NOT_READY;

	deviceList = ZigbeeController::singletonDeviceList;

	// Check intialized before when caller isn't main Bridge
	if (nodeId != 0){
		if(ZigbeeController::isInitialized){
			mInfo->state = CTRL_NOT_READY;
			int index = singletonDeviceList->searchByNodeId(nodeId);
			if (index == -1){
				LOGD(TAG, "cannot find nodeId 0x%04x", nodeId);
				return;
			}

			singletonDeviceList->setZigbeeControllerHandler(index, (void*)this);

			return;
		} else {
			LOGD(TAG,"Zigbee Coordinator is not initialized yet");
			mInfo->state = CTRL_ERROR;
			return;
		}
	}

	ZigbeeController::singletonDeviceList = new ZigbeeDeviceList();

	ZigbeeController::mCoordinatorInfo = mInfo;

	deviceList = ZigbeeController::singletonDeviceList;
	mControllerListener = 0;
	mConnectionListener = 0;

	ZigbeeController::receiveThreadId = 0;

	deviceList->deviceInfoLoad();
	deviceList->printAllConnectedDevice();
}

ZigbeeController::~ZigbeeController() {
	int status;

	if (mInfo->id == 0){
		if (receiveThreadId)
			pthread_cancel(receiveThreadId);

		if (ZigbeeController::deviceList)
			delete ZigbeeController::deviceList;

		// TODO
		// Disconnect from Zigbee daemon or finalize Zigbee library
	}

	if (mInfo)
		delete(mInfo);
}

bool ZigbeeController::sendMessageToDaemon(char* buf, int bufSize) {
	if (buf==NULL){
		LOGD(TAG, "No Data.");
		return false;
	}
	
	if (mCoordinatorInfo->state == CTRL_ERROR){
		LOGD(TAG, "ZIGBEE Daemon socket is not connected.");
		return false;
	}

	LOGD(TAG, "sendMessageToDaemon buf[%s] size[%d]", buf, bufSize);

	// TODO
	// Send Zigbee command

	return true;
}

void* ZigbeeController::initCoordinator(void* arg) {
	pContInfoT mInfo = (pContInfoT) arg;

	// TODO
	// Connect to Zigbee daemon or initialize library

	// thread for receive Zigbee data
	pthread_create((pthread_t*)&ZigbeeController::receiveThreadId, NULL, receiveThread, NULL);
	pthread_detach(ZigbeeController::receiveThreadId);

	mInfo->state = CTRL_INITIALIZED;
	return mInfo;
}

ControllerState ZigbeeController::Init() {
	if (mInfo == NULL) {
		ZBLOGD(TAG, mInfo->id, "mInfo is null.");
		return CTRL_ERROR_DATA_NULL;
	}

	// Already initialized
	if (mInfo->isInitialized) {
		ZBLOGD(TAG, mInfo->id, "Already initialized.");
		return CTRL_INITIALIZED;
	}

	mInfo->isInitialized = true;

	// this device is not a coordinator. no need to comm to daemon
	if (mInfo->id != 0) {
		mInfo->state = CTRL_INITIALIZED;
		ZBLOGD(TAG, mInfo->id, "Initialized.");
	} 
	// coordinator. coordinator initialing start and return this state;
	else {
		return ((pContInfoT)initCoordinator(mInfo))->state;
	}
	return mInfo->state;
}

ControllerState ZigbeeController::Start()
{
	if (mInfo->state != CTRL_INITIALIZED) {
		LOGD(TAG, "Zigbee Daemon is not connected. try Init()");
		return mInfo->state;
	}

	mInfo->state = CTRL_READY;
	if (coordinatorControllerListener)
		coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_READY);

	return mInfo->state;
}

void* ZigbeeController::receiveThread(void* arg) {

	ZIGBEE_TAG_FLAG tag = NO_MSG_TAG;

	unsigned int tempNodeId = 0;

	map<uint16_t, joinInfo> joinRequestedMap;
	map<uint16_t, joinInfo>::iterator FindIter;

	while (1) {

		// TODO
		// Implement receiving data from Zigbee daemon or library

		// Distinguish data by custom tag
		switch (tag) {
			case COMM_RSSI_TAG:
				{
					int tempRSSI = 0;
					int index = -1;

					// TODO
					// Get node ID
					//tempNodeId = Received_Node_ID;

					// Check existing node
					index = singletonDeviceList->searchByNodeId(tempNodeId);
					if (index == -1) {
						LOGD(TAG, "cannot find nodeId 0x%04x", tempNodeId);
						continue;
					}

					// TODO
					// Get RSSI
					//tempRSSI = Received_RSSI;

					void* handler = singletonDeviceList->getZigbeeControllerHandler(index);
					if (handler)
						((ZigbeeController*)handler)->setRSSI(tempRSSI);

					LOGD(TAG, "RSSI Received, node %x, RSSI dB %d", tempNodeId, tempRSSI);
				}
				break;

			case JOIN_SUCCESS_TAG:
				{
					joinInfo tempInfo;

					mCoordinatorInfo->state = CTRL_ADDING;
					if (coordinatorControllerListener)
						coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_ADDING);

					// TODO
					// Store device infomation has node ID, EUI64...
					memset(tempInfo.infoBuf, 0, DEFAULT_BUFFER_LEN);
					//memcpy(tempInfo.infoBuf, Received_Data, strlen(temp));

					// TODO
					// Get joined device info and node ID from received data

					LOGD(TAG, "insert item %x", tempNodeId);
					joinRequestedMap.insert(map< uint16_t, joinInfo >::value_type(tempNodeId, tempInfo));

					// TODO
					// Request device name(DEVICE_ORIG_NAME_TAG) to confirm connection
				}
				break;

			case DEVICE_ORIG_NAME_TAG:
				{
					// TODO
					// Get node ID

					// Find joining device
					FindIter = joinRequestedMap.find(tempNodeId);
					if (FindIter == joinRequestedMap.end()) {
						LOGD(TAG, "cannot find nodeId %04x!!!", tempNodeId);
						continue;
					} 
					else {

						// TODO
						// Find index from compatible device table
						int index = -1;
						//index = findMatchDevInfo("Name_of_Device");
						if (index != -1) {
							LOGD(TAG, "Not compatible device!!");
							continue;
						}

						FindIter->second.compatibleZigBeeDevInfoTableIdx = index;
						FindIter->second.deviceTypeId = strtoul(compatibleZigBeeDevInfoTable[index].type, NULL, 16);

						memset(FindIter->second.realNameBuf, 0, DEFAULT_DEVICE_NAME_LEN);
						strcpy(FindIter->second.realNameBuf, compatibleZigBeeDevInfoTable[index].name);

						LOGD(TAG, "Save device DeviceTableIdx[%d], Type[0x%04X], Name[%s]", 
								index, compatibleZigBeeDevInfoTable[index].type, compatibleZigBeeDevInfoTable[index].name);

						int ret = singletonDeviceList->deviceInfoAdd(FindIter->second.infoBuf, FindIter->second.deviceTypeId, 0xFF, index);
						if (0 != ret) {
							LOGD(TAG, "duplicate device (Node ID %d) find. remove first", ret);
							if (coordinatorConnectionListener)
								coordinatorConnectionListener->DeviceLost(CONN_ZIGBEE, ret);
						}
						
						// TODO
						// Send remove cmd
						//sendMessageToDaemon("Command_of_Device_Remove", Size_of_Command); // +1: NULL

						// Save & Print
						singletonDeviceList->saveFile();
						singletonDeviceList->printAllConnectedDevice();

						// Inform to Bridge devcie is found
						if (coordinatorConnectionListener)
							coordinatorConnectionListener->DeviceFound(CONN_ZIGBEE, findDeviceId(FindIter->second.deviceTypeId, IAS_ZONE_UNKNOWN), tempNodeId);
						mCoordinatorInfo->state = CTRL_READY;
						if (coordinatorControllerListener)
							coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_READY);

						joinRequestedMap.erase(FindIter);
					}
				}
				break;

			case LEAVE_SUCCESS_TAG:
				{
					// TODO
					// Get node ID

					singletonDeviceList->removeByNodeId(tempNodeId);
					singletonDeviceList->saveFile();
					singletonDeviceList->printAllConnectedDevice();

					if (coordinatorConnectionListener)
						coordinatorConnectionListener->DeviceLost(CONN_ZIGBEE, tempNodeId);

					if (mCoordinatorInfo->state == CTRL_ADDING) {
						mCoordinatorInfo->state = CTRL_ADDING;
					} 
					else {
						mCoordinatorInfo->state = CTRL_READY;
						if (coordinatorControllerListener)
							coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_READY);
					}
				}
				break;

			// Sensor data
			case RX_MSG_RECEIVE_TAG:
				//sendRxMsgData(Receiced_Data);
				break;

			// reset
			case NETWORK_LEAVE_SUCCESS_TAG:
				LOGD(TAG, "network leave success!!");
				singletonDeviceList->cleanup();
				break;

			default:
				break;
		}
		usleep(100*1000);
	}
	return NULL;
}

void ZigbeeController::sendRxMsgData(char* recvBuffer) {
	int convVal = 0;
	int tempNodeId = 0;

	RepDataT data;

	// TODO
	// Get cluster type & node ID from recvBuffer

	int index = singletonDeviceList->searchByNodeId(tempNodeId);
	LOGD(TAG, "index %d", index);
	if (index < 0) {
		LOGD(TAG, "nodeID %d not found!!");
		return;
	}

	switch (convVal) {
		case C_ONOFF:
			// TODO
			// Get sensor data
			data.bin.flag = false;
			LOGD(TAG, "[ONOFF] reporting %d", data.bin.flag);
			singletonDeviceList->getControlListenerByIndex(index)->Reported(CONN_ZIGBEE, data);
			break;

		case C_IAS_ZONE:
			// TODO
			// Get sensor data
			data.bin.flag = false;
			LOGD(TAG, "[IAS_ZONE] reporting %d", data.bin.flag);
			singletonDeviceList->getControlListenerByIndex(index)->Reported(CONN_ZIGBEE, data);
			break;

		case C_TEMPERATURE:
			// TODO
			// Get sensor data
			data.th.temp = 28;
			singletonDeviceList->getControlListenerByIndex(index)->Reported(CONN_ZIGBEE, data);
			break;

		case C_HUMIDITY:
			// TODO
			// Get sensor data
			data.th.humi = 50;
			singletonDeviceList->getControlListenerByIndex(index)->Reported(CONN_ZIGBEE, data);
			break;

		case C_BATTERY_INFO:
			// TODO
			// Get battery info
			singletonDeviceList->getControlListenerByIndex(index)->BatteryLevel(CONN_ZIGBEE, 80);
			break;
	}
}


DeviceType ZigbeeController::findDeviceId(unsigned short deviceTypeId, unsigned short IASZoneType) {

	switch(deviceTypeId){
		case DEVICE_TYPE_ONOFF:
			return DEV_PLUG;
		case DEVICE_TYPE_IAS_OPEN_CLOSE:
			return DEV_DOOR;
		case DEVICE_TYPE_TEMP_HUMIDITY:
			return DEV_TEMP_HUMIDITY;
		default:
			break;
	}
	return DEV_NONE;
}

int ZigbeeController::findMatchDevInfo(char* recvBufer) {
	// TODO
	// Find device from compatible device table & return index of table
	return -1;
}

void ZigbeeController::Final() {
}

ControllerState ZigbeeController::GetState() {
	return mInfo->state;
}

void ZigbeeController::SetState(ControllerState state) {
	char zigbeeWriteBuf[DEFAULT_BUFFER_LEN] = {0,};
	if ((mInfo->state == CTRL_NOT_READY) || (mInfo->state == CTRL_INIT_FAILED) || (mInfo->state == CTRL_ERROR)) {
		LOGD(TAG, "Cannot change state. Init() first.");
		return;
	}
	if ((mInfo->state == CTRL_ADD_ENABLED) || (mInfo->state == CTRL_ADDING) 
			|| (mInfo->state == CTRL_REMOVE_ENABLED) || (mInfo->state == CTRL_REMOVING)) {
		LOGD(TAG, "Cannot change state. ADDING/REMOVING.....");
		return;
	}
	switch(state) {
		case CTRL_ADD_ENABLED:
			if (mInfo->state != CTRL_READY) {
				LOGD(TAG, "Cannot change state. zigbee is not ready");
			}
			if (mInfo->id == 0) {
				// TODO
				// Send Zigbee command
				/*	Example code for Amber Zigbee daemon
					sprintf(zigbeeWriteBuf, "network pjoin %d\r\n", PJOIN_TIMEOUT);
					sendMessageToDaemon(zigbeeWriteBuf, strlen(zigbeeWriteBuf)+1);// +1: NULL
				 */

				mInfo->state = CTRL_ADD_ENABLED;
				if (coordinatorControllerListener)
					coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_ADD_ENABLED);
			} 
			else {
				LOGD(TAG, "this is not Bridge. cannot change to add mode");
			}
			break;

		default:
			break;
	}
}

void ZigbeeController::GetList() {
	unsigned short tempDevId, tempIASType, tempNodeId;
	for (int i=0; singletonDeviceList->getDeviceListElemByIndex(i, &tempDevId, &tempIASType, &tempNodeId) ; i++) {
		ZBLOGD(TAG, mInfo->id, "zigbee DeviceList update %d : %04x", i, tempNodeId);
		coordinatorConnectionListener->DeviceFound(CONN_ZIGBEE, findDeviceId(tempDevId, tempIASType), tempNodeId);
	}
}

void ZigbeeController::Reset() {
	// TODO
	// Send reset command

	/* Example code for Amber Zigbee daemon
	char leave[] = "network leave\r\n";
	sendMessageToDaemon(leave, strlen(leave)+1);		// +1: NULL
	*/
}

void ZigbeeController::AddListener(ControllerListener* listener) {
	mControllerListener = listener;
	if (mInfo->id !=0){
		deviceList->setControlListenerByIndex(deviceList->searchByNodeId((int)mInfo->id), listener);
	} else {
		coordinatorControllerListener = listener;
	}
}

void ZigbeeController::RemoveListener(ControllerListener* listener) {
	mControllerListener = NULL;
	if (mInfo->id !=0){
		deviceList->setControlListenerByIndex(deviceList->searchByNodeId((int)mInfo->id), NULL);
	} else {
		coordinatorControllerListener = 0;
	}
}

void ZigbeeController::AddConnListener(ConnectionListener* listener) {
	mConnectionListener = listener;
	if (mInfo->id !=0){
		deviceList->setConnectListenerByIndex(deviceList->searchByNodeId((int)mInfo->id), NULL);
	} else {
		coordinatorConnectionListener = listener;
	}
}

void ZigbeeController::RemoveConnListener(ConnectionListener* listener) {
	mConnectionListener = NULL;
	if (mInfo->id !=0){
		deviceList->setConnectListenerByIndex(deviceList->searchByNodeId((int)mInfo->id), NULL);
	} else {
		coordinatorConnectionListener = 0;
	}
}

void ZigbeeController::RemoveNode() {
	mInfo->state = CTRL_REMOVE_ENABLED;
	if (coordinatorControllerListener)
		coordinatorControllerListener->StateChanged(CONN_ZIGBEE, CTRL_REMOVE_ENABLED);

	char zigbeeWriteBuf[DEFAULT_BUFFER_LEN] = {0,};
	if (mInfo->id == 0) {
		ZBLOGD(TAG, mInfo->id, "Leave Req but it is coordinator.");
	} 
	else {
		if (singletonDeviceList->searchByNodeId(mInfo->id) == -1) {
			ZBLOGD(TAG, mInfo->id, "Leave Req but ZB plug has no info..");
			return;
		}
		// TODO
		// Send remove command
		/* Example code for Amber Zigbee daemon
		sprintf(zigbeeWriteBuf, "zdo leave 0x%04X 1 0\r\n", mInfo->id);
		sendMessageToDaemon(zigbeeWriteBuf, strlen(zigbeeWriteBuf)+1);// +1: NULL
		*/

		ZBLOGD(TAG, mInfo->id, "Leave Req. Str [%s]", zigbeeWriteBuf);
	}
}

void ZigbeeController::SetInterval(int sec) {
	// TODO
	// Send setting sensor reporting interval command

	deviceList->setIntervalByNodeId(mInfo->id, sec);
}

void ZigbeeController::SetBinary(bool val)
{
	// TODO
	// Send on/off command
	/* Example code for Amber Zigbee daemon

	if (val){
		sprintf(zigbeeWriteBuf, "zcl on-off on\r\n");
	} else {
		sprintf(zigbeeWriteBuf, "zcl on-off off\r\n");
	}

	sendMessageToDaemon(zigbeeWriteBuf, strlen(zigbeeWriteBuf)+1);// +1: NULL

	usleep(100000);

	memset(zigbeeWriteBuf, 0, DEFAULT_BUFFER_LEN);
	sprintf(zigbeeWriteBuf, "send 0x%04x 1 1\r\n", mInfo->id);
	ZBLOGD(TAG, mInfo->id, "setBinary Str [%s]", zigbeeWriteBuf);

	sendMessageToDaemon(zigbeeWriteBuf, strlen(zigbeeWriteBuf)+1);// +1: NULL
	*/
	ZBLOGD(TAG, mInfo->id, "val: %d", val);
}

void ZigbeeController::SetLevel(int val){
	ZBLOGD(TAG, mInfo->id, "val: %d", val);
}

