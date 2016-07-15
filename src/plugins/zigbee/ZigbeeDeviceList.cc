#include "plugins/ZigbeeDeviceList.h"
#include "plugins/ZigbeeDefine.h"

#include "utils/Log.h"

#define TAG "ZigbeeDeviceList"

ZigbeeDeviceList::ZigbeeDeviceList() {
	connectedDeviceCount = 0;
}

bool ZigbeeDeviceList::removeByIndex(unsigned int index) {
	if (list.size() == 0 )
		return false;
	list.erase(list.begin()+index);
	return true;
}

bool ZigbeeDeviceList::removeByNodeId(unsigned short nodeId) {
	if (list.size() == 0) {
		return false;
	}

	for (unsigned int i = 0; i<list.size();i++) {
		if (list[i].getNodeId() == nodeId){
			list.erase(list.begin()+i);
			return true;
		}
	}
	return false;
}

int ZigbeeDeviceList::searchByEui64(char* cArr) {
	if (list.size() == 0)
		return -1;

	for (unsigned int i=0;i<list.size();i++) {
		if (memcmp(list[i].getEui64(), cArr, 16) == 0) {
			return i;
		}
	}
	return -1;
}

ControllerListener* ZigbeeDeviceList::getControlListenerByIndex(unsigned int index) {
	if (index < 0)
		return 0;
	if (list.size() < index+1)
		return 0;

	return list[index].getControlListener();
}

ConnectionListener* ZigbeeDeviceList::getConnectListenerByIndex(unsigned int index) {
	if (list.size() < index+1)
		return 0;

	return list[index].getConnectListener();
}

bool ZigbeeDeviceList::getDeviceListElemByIndex(unsigned int index,
	   	unsigned short* devId,  unsigned short* IASType, unsigned short* nodeId) {

	if (list.size() < index+1) {
		LOGD(TAG, "list size is %d",list.size());
		return false;
	}
	*devId = list[index].getDeviceTypeId();
	*IASType = list[index].getType();
	*nodeId = list[index].getNodeId();
	return true;
}

unsigned short ZigbeeDeviceList::deviceInfoAdd(char* infoBuf, unsigned short deviceTypeId, 
		unsigned short IASZoneType, int compatibleZigBeeDevInfoTableIdx) {

	deviceNode newNode;

	unsigned short tempNodeId = 0;
	int index = 0;

	// TODO
	// Get node ID from infoBuf
	newNode.setNodeId(tempNodeId);

	// TODO
	// Get EUI64 from infoBuf
	//newNode.setEui64(#EUI64);
	if ((index = searchByEui64(newNode.getEui64())) != -1) {
		LOGD(TAG, "info already exists, change Node ID ");
		int beforeNodeId = list[index].getNodeId();
		list[index].setNodeId(tempNodeId);
		list[index].setDeviceTypeId(deviceTypeId);
		list[index].setType(IASZoneType);
		list[index].setDevTableIdx(compatibleZigBeeDevInfoTableIdx);
		return beforeNodeId;
	}

	newNode.clusterCount = 0;

	switch(deviceTypeId) {
		case DEVICE_TYPE_ONOFF:
			newNode.setClusterId(C_ONOFF);
			break;
		case DEVICE_TYPE_TEMP_HUMIDITY:
			if((strcmp(compatibleZigBeeDevInfoTable[compatibleZigBeeDevInfoTableIdx].name, "3310-S") == 0)
					|| (strcmp(compatibleZigBeeDevInfoTable[compatibleZigBeeDevInfoTableIdx].name, "3310-eu") == 0) ) {

				newNode.setClusterId(C_TEMPERATURE);
				newNode.setClusterId(C_HUMIDITY);
			}
			else {
				newNode.setClusterId(C_TEMPERATURE);
				newNode.setClusterId(C_HUMIDITY_0405);
			}
			break;
		case DEVICE_TYPE_IAS_MOTION:
		case DEVICE_TYPE_IAS_OPEN_CLOSE:
		case DEVICE_TYPE_IAS_MOISTURE:
		case DEVICE_TYPE_IAS_FIRE_SMOKE:
			newNode.setClusterId(C_IAS_ZONE);
			break;

		case DEVICE_TYPE_DIMMABLE_BULB:
			newNode.setClusterId(C_ONOFF);
			newNode.setClusterId(C_LEVEL_CONTROL);

		case DEVICE_TYPE_COLOR_BULB:
			newNode.setClusterId(C_ONOFF);
			newNode.setClusterId(C_LEVEL_CONTROL);

		case DEVICE_TYPE_IAS_FIRE_SMOKE_SIREN:
			newNode.setClusterId(C_IAS_ZONE);
			newNode.setClusterId(C_ONOFF);
	}

	newNode.setDeviceTypeId(deviceTypeId);
	newNode.setDevTableIdx(compatibleZigBeeDevInfoTableIdx);
	newNode.setType(IASZoneType);
	newNode.setStatus(NODE_STATUS_NONE);

	LOGD(TAG, "NodeId[%04x] DeviceId[%04x] DeviceType[%04x]", tempNodeId, deviceTypeId, newNode.getType());

	newNode.setInterval(0);
	list.push_back(newNode);
	return 0;
}

void ZigbeeDeviceList::printAllConnectedDevice(void) {
	LOGD(TAG, "printAllConnectedDevice");
	LOGD(TAG, "------------------------------");

	if (list.size() > 0) {
		LOGD(TAG, "Total conected device [%d]", list.size());
	}
	else {
		LOGD(TAG, "No connected device.");
		return;
	}

	for (unsigned int i=0; i<list.size();i++) {
		LOGD(TAG, "No.            : [%d]", i);
		LOGD(TAG, "NodeId         : [%04X]", list[i].getNodeId());
		LOGD(TAG, "ClusterId      : [%04X]", list[i].getClusterId());
		LOGD(TAG, "DeviceTypeId   : [%04X]", list[i].getDeviceTypeId());
		LOGD(TAG, "DeviceTableIdx : [%04X]", list[i].getDevTableIdx());
		LOGD(TAG, "Type           : [%02X]", list[i].getType());
		LOGD(TAG, "EUI64          : [%s]", list[i].getEui64());
		LOGD(TAG, "Status         : [%02X]", list[i].getStatus());
		LOGD(TAG, "Interval       : [%d]", list[i].getInterval());
		LOGD(TAG, "------------------------------");
	}
	return;
}

void ZigbeeDeviceList::cleanup(void) {
	FILE *f;
	int ret = -1;

	if (list.size() == 0) {
		LOGD(TAG, "deviceInfoCleanup : No connected data");
	}
	else {
		LOGD(TAG, "deviceInfoCleanup : Initial device Info");
		list.clear();
	}

	ret = remove(DEVICE_INFO_FILE_PATH);
	if (ret == 0) {
		LOGD(TAG, "deviceInfoCleanup : Remove file Success!!!");
	}
	else if (ret == -1) {
		LOGD(TAG, "deviceInfoCleanup : Remove file Fail!!!");
	}
}

void ZigbeeDeviceList::saveFile(void) {
	FILE *f;
	int ret = -1;
	if (list.size() == 0) {
		LOGD(TAG, "deviceInfoSaveFile : No data!!!");

		ret = remove(DEVICE_INFO_FILE_PATH);
		if (ret == 0) {
			LOGD(TAG, "deviceInfoSaveFile : Remove file Success!!!");
		} 
		else if (ret == -1) {
			LOGD(TAG, "deviceInfoSaveFile : Remove file Fail!!!");
		}
		return;
	}

	f = fopen(DEVICE_INFO_FILE_PATH, "w+");

	char buf[128]={0,};

	fprintf(f, "<element_count>\n%d\n</element_count>\n", list.size());

	for (unsigned int i = 0; i < list.size(); i++) {
		fprintf(f, "<devInfo>\n");
		fprintf(f, "<node_id>\n%04x\n</node_id>\n", list[i].getNodeId());
		fprintf(f, "<cluster_id>\n%04x\n</cluster_id>\n", list[i].getClusterId());
		fprintf(f, "<device_type_id>\n%04x\n</device_type_id>\n", list[i].getDeviceTypeId());
		fprintf(f, "<device_table_idx>\n%04x\n</device_table_idx>\n", list[i].getDevTableIdx());
		fprintf(f, "<type>\n%04x\n</type>\n", list[i].getType());
		fprintf(f, "<eui64>\n%s\n</eui64>\n", list[i].getEui64());
		fprintf(f, "<status>\n%d\n</status>\n", list[i].getStatus());
		fprintf(f, "<interval>\n%d\n</interval>\n", list[i].getInterval());
		fprintf(f, "</devInfo>\n");
	}

	if (fclose(f) == 0)
		LOGD(TAG, "Save Complete!!!");

	return;
}


int ZigbeeDeviceList::deviceInfoLoad(void) {
	LOGD(TAG, "deviceInfoLoad");
	FILE *f;

	char buf[1024]={0,};
	char tempTagName[32]={0,};
	char tempUShortBuf[4]={0,};

	f = fopen(DEVICE_INFO_FILE_PATH, "a+");
	LOGD(TAG, "deviceInfoLoad2");

	if (f == NULL) {
		LOGD(TAG, "deviceInfoLoad fopen fail!!!");
		return -1;
	}

	char *token = 0;
	bool isFirst=true;
	int count=0;

	enum {
		WAIT_TAG,
		WAIT_CONTENT,
		WAIT_CLOSE
	};

	enum {
		NODE_INFO_COUNT,
		NODE_INFO_WAIT,
		NODE_INFO_DONE
	};

	int status = WAIT_TAG;
	int nodeInsertStatus = NODE_INFO_COUNT;
	char* pos = 0;
	deviceNode newNode;
	while(!feof(f)) {
		if (NULL == fgets(buf, 1024, f)) {
			LOGD(TAG, "deviceInfoLoad fgets return NULL");
			break;
		}
		token = strtok(buf, "\r\n");
		do {
			LOGD(TAG,"%s", token);

			if (token == NULL)
				continue;

			switch(status) {
				case WAIT_TAG:
					if (token[0] == '<' && token[1] != '/') {
						memset(tempTagName, 0, 32);
						memcpy(tempTagName, token+1, strlen(token)-1);
						if (!strncmp(tempTagName, "devInfo", 7)) {
							nodeInsertStatus = NODE_INFO_WAIT;
							status = WAIT_TAG;
						} 
						else {
							status = WAIT_CONTENT;
						}
					} 
					else {
						LOGD(TAG, "WAIT_TAG ERR input");
						goto err_go;
					}
					break;

				case WAIT_CONTENT:
					if (token[0] != '<') {
						if (nodeInsertStatus == NODE_INFO_COUNT) {
							if (!strncmp(tempTagName, "element_count", 13)) {
								status = WAIT_CLOSE;
							} 
							else {
								LOGD(TAG, "WAIT_CONTENT ERR input");
								goto err_go;
							}
						} 
						else if (nodeInsertStatus == NODE_INFO_WAIT){
							status = WAIT_CLOSE;
							if (!strncmp(tempTagName, "node_id", 7)) {
								newNode.setNodeId((unsigned short)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "cluster_id", 10)) {
								newNode.setClusterId((unsigned short)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "device_type_id", 14)) {
								newNode.setDeviceTypeId((unsigned short)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "device_table_idx", 16)) {
								newNode.setDevTableIdx((unsigned int)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "type", 4)) {
								newNode.setType((unsigned short)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "eui64", 5)) {
								newNode.setEui64(token);
							} else if (!strncmp(tempTagName, "status", 6)) {
								newNode.setStatus((unsigned short)strtoul( token, &pos, 16));
							} else if (!strncmp(tempTagName, "interval", 8)) {
								newNode.setInterval((unsigned char)strtoul( token, &pos, 16));
								list.push_back(newNode);
								status = WAIT_CLOSE;
								nodeInsertStatus = NODE_INFO_DONE;
							} else {
								LOGD(TAG, "WAIT_CONTENT ERR input");
								goto err_go;
							}
						} 
						else {
							LOGD(TAG, "WAIT_CONTENT ERR input");
							goto err_go;
						}
					} 
					else {
						LOGD(TAG, "WAIT_CONTENT ERR input");
						goto err_go;
					}
					break;

				case WAIT_CLOSE:
					if (token[0] == '<' && token[1] == '/' ) {
						if (!strncmp(tempTagName, token+2, strlen(tempTagName))) {
							if (nodeInsertStatus == NODE_INFO_DONE) 
								status = WAIT_CLOSE;
							else
								status = WAIT_TAG;
						} 
						else {
							if (nodeInsertStatus == NODE_INFO_DONE) {
								if (!strncmp("devInfo", token+2, 7))
									status = WAIT_TAG;
								else
									LOGD(TAG, "WAIT_CLOSE ERR input");
									goto err_go;
							} 
							else {
								LOGD(TAG, "WAIT_CLOSE ERR input");
								goto err_go;
							}
						}

					} 
					else{
						LOGD(TAG, "WAIT_CLOSE ERR input");
						goto err_go;
					}
					break;
			}
		} while(NULL!=(token = strtok(NULL, "\r\n")));
	}

	fclose(f);

	return 0;

err_go:
	fclose(f);
	LOGD(TAG, "info File form Err");
	return -1;
}

typedef enum
{
	BY_NODE_ID = 0,
	BY_CLUSTER_ID = 1,
	BY_TYPE = 2,
	BY_EUI64 = 3
} DEV_INFO_SEARCH_TYPE;


int ZigbeeDeviceList::searchByNodeId(int nodeId) {
	LOGD(TAG, "searchByNodeId %d %d", nodeId, list.size());
	if (list.size() == 0 ) {
		return -1;
	}
	for (unsigned int i=0;i<list.size();i++) {
		if (list[i].getNodeId() == nodeId) {
			return i;
		}
	}
	return -1;
}

bool ZigbeeDeviceList::setControlListenerByIndex(unsigned int index, ControllerListener* listener) {
	if (list.size() < index+1)
		return false;
	list[index].setControlListener(listener);

	return true;
}

bool ZigbeeDeviceList::setConnectListenerByIndex(unsigned int index, ConnectionListener* listener) {
	if (list.size() < index+1)
		return false;
	list[index].setConnectListener(listener);

	return true;
}

bool ZigbeeDeviceList::getEui64ByNodeId(unsigned short nodeId, char* output) {
	if (list.size() == 0)
		return false;

	for (unsigned int i = 0; i<list.size();	i++) {
		if (list[i].getNodeId() == nodeId) {
			memcpy(output, list[i].getEui64(), 16);
			return true;
		}
	}
	return false;
}

int ZigbeeDeviceList::getDevTableIdxByNodeId(unsigned short nodeId) {
	if (list.size() == 0)
		return 0;

	for (unsigned int i = 0; i<list.size();	i++) {
		if (list[i].getNodeId() == nodeId) {
			return list[i].getDevTableIdx();
		}
	}
	return 0;
}

bool ZigbeeDeviceList::setIntervalByNodeId(unsigned short nodeId, int interval) {
	if (list.size() == 0)
		return false;

	for (unsigned int i = 0; i<list.size();	i++) {
		if (list[i].getNodeId() == nodeId) {
			list[i].setInterval(interval);
			return true;
		}
	}
	return true;
}

unsigned int ZigbeeDeviceList::getIntervalByNodeId(unsigned short nodeId) {
	if (list.size() == 0)
		return 0;

	for (unsigned int i = 0; i<list.size();	i++) {
		if (list[i].getNodeId() == nodeId) {
			return list[i].getInterval();
		}
	}
	return 0;
}

void* ZigbeeDeviceList::getZigbeeControllerHandler(unsigned int index) {
	if (list.size() < index+1)
		return 0;
	return list[index].getZigbeeControllerHandler();
}

void ZigbeeDeviceList::setZigbeeControllerHandler(unsigned int index, void* handler) {
	if (list.size() < index+1)
		return;
	list[index].setZigbeeControllerHandler(handler);
}

