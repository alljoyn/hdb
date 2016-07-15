#ifndef _ZIGBEE_DEVICELIST_H
#define _ZIGBEE_DEVICELIST_H

#include <vector>
#include "plugins/ControllerIface.h"
#include <string.h>

using namespace std;

typedef struct {
	const char* type;
	const char* name;
	const char* vendor;
	const char* commonName;
} compatibleZigBeeDevInfo;

const compatibleZigBeeDevInfo compatibleZigBeeDevInfoTable[] = {
//SmartThings
/*00*/	{"0015", "3300-S", "CentraLite", "Door/Window Sensor"}, //SmartThings Open/Closed Sensor (Old)
/*01*/	{"0302", "3310-S", "CentraLite", "Temp/Humidity Sensor"}, //SmartThings Temp/Humidity Sensor (Old)
/*02*/	{"0100", "4257050-RZHAC", "CentraLite", "On/Off Power Outlet"}, //SmartThings Power outlet (Old)
//Unknown
/*Max*/	{"FFFF", "Unknown name", "Unknown name", "Unknown name"} //Unknown Device
};

class deviceNode {
public:
	inline void setNodeId(unsigned short i) { nodeId = i; }

	inline void setClusterId(unsigned short i) { clusterId[0] = i; }

	inline void setDeviceTypeId(unsigned short i) { deviceTypeId = i; }
	inline void setType(unsigned short i) { type = i; }
	inline void setEui64(char* arr) { memcpy(eui64, arr, 16); }
	inline void setStatus(unsigned short i) { status = i; }
	inline void setSignalType(int i) { signalType = i; }
	inline void setInterval(int i) { interval = i; }
	inline void setDevTableIdx(int i) { compatibleZigBeeDevInfoTableIdx = i; }

	inline void setControlListener(ControllerListener* i) { contListener = i; }
	inline void setConnectListener(ConnectionListener* i) { connListener = i; }
	inline void setZigbeeControllerHandler(void* handler) { zigbeeControllerHandler = handler; }

	inline unsigned short getNodeId(void) { return nodeId; }
	inline unsigned short getClusterId(void) { return clusterId[0]; }
	inline unsigned short getDeviceTypeId(void) { return deviceTypeId; }
	inline unsigned short getType(void) { return type; }
	inline char* getEui64(void) { return eui64; }
	inline unsigned short getStatus(void) { return status; }
	inline int getSignalType(void) { return signalType; }
	inline int getInterval(void) { return interval; }
	inline int getDevTableIdx(void) { return compatibleZigBeeDevInfoTableIdx; }

	inline ControllerListener* getControlListener(void) { return contListener; }
	inline ConnectionListener* getConnectListener(void) { return connListener; }
	inline void* getZigbeeControllerHandler(void) {return zigbeeControllerHandler; }

	unsigned short  clusterCount;

private:
	unsigned short	nodeId;
	unsigned short 	clusterId[5];
	unsigned short 	deviceTypeId;
	unsigned short 	type;
	char 			eui64[16];
	unsigned short 	status;
	unsigned short 	signalType;
	unsigned int 	interval;
	int compatibleZigBeeDevInfoTableIdx;

	//don't save below.
	unsigned short attrListLength;
	struct clusterAttr* attrList;

	ConnectionListener* connListener;
	ControllerListener* contListener;

	void* zigbeeControllerHandler;
};


typedef enum {
	NODE_STATUS_NONE
} nodeStatus;


class ZigbeeDeviceList{
public:
	ZigbeeDeviceList();
	~ZigbeeDeviceList() {};

	unsigned short deviceInfoAdd(char* infoBuf, unsigned short deviceTypeId, unsigned short IASZoneType, int compatibleZigBeeDevInfoTableIdx);
	bool removeByIndex(unsigned int index);
	bool removeByNodeId(unsigned short nodeId);

	bool getDeviceListElemByIndex(unsigned int index, unsigned short* devId, unsigned short* IASType, unsigned short* nodeId);

	int searchByNodeId(int nodeId);
	int searchByEui64(char* cArr);
	inline unsigned int connectedDeviceCnt(void) { return connectedDeviceCount; };

	void printAllConnectedDevice(void);
	void cleanup(void);
	void saveFile(void);
	int deviceInfoLoad(void);
	ControllerListener* getControlListenerByIndex(unsigned int index);
	ConnectionListener* getConnectListenerByIndex(unsigned int index);

	bool setControlListenerByIndex(unsigned int index, ControllerListener* listener);
	bool setConnectListenerByIndex(unsigned int index, ConnectionListener* listener);

	bool getEui64ByNodeId(unsigned short nodeId, char* output);
	int getDevTableIdxByNodeId(unsigned short nodeId);

	bool setIntervalByNodeId(unsigned short nodeId, int interval);
	unsigned int getIntervalByNodeId(unsigned short nodeId);

	void* getZigbeeControllerHandler(unsigned int index);
	void setZigbeeControllerHandler(unsigned int index, void* handler);

private:
	vector<deviceNode> list;

	unsigned int connectedDeviceCount;
};

#endif // _ZIGBEE_DEVICELIST_H
