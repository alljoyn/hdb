#ifndef _ZIGBEECONTROLLER_H
#define _ZIGBEECONTROLLER_H

#include "ControllerDef.h"
#include "ControllerIface.h"

#include "ZigbeeDeviceList.h"
#include "ZigbeeDefine.h"

#include <pthread.h>

struct joinInfo{
	unsigned short deviceTypeId;
	unsigned short type;
	char realNameBuf[DEFAULT_DEVICE_NAME_LEN];
	int compatibleZigBeeDevInfoTableIdx;
	char infoBuf[DEFAULT_BUFFER_LEN];
};

class ZigbeeController : public ControllerIface {
	public:
		ZigbeeController(uint16_t nodeId, DeviceType devType);
		~ZigbeeController();

		// ControllerIface functions
		ControllerState Init();
		ControllerState Start();
		void Final();

		ControllerState GetState();
		void SetState(ControllerState state);
		void GetList();

		void Reset();

		void AddListener(ControllerListener* listener);
		void RemoveListener(ControllerListener* listener);

		void AddConnListener(ConnectionListener* listener);
		void RemoveConnListener(ConnectionListener* listener);

		// Custom functions
		void RemoveNode();
		void SetInterval(int sec);
		void SetBinary(bool val);
		void SetLevel(int val);

		inline void setRSSI(int val) { lastHopRSSIVal = val; }
		int GetSignalStrength(void) { return lastHopRSSIVal; }

	private:

		static bool 		isInitialized;

		pContInfoT			mInfo;
		pRepDataT			mData;

		int lastHopRSSIVal;

		static pContInfoT mCoordinatorInfo;

		ControllerListener* mControllerListener;
		ConnectionListener* mConnectionListener;

		static ConnectionListener* coordinatorConnectionListener;
		static ControllerListener* coordinatorControllerListener;

		static ZigbeeDeviceList* singletonDeviceList;
		ZigbeeDeviceList* deviceList;

		static void* receiveThread(void* arg);
		static  int receiveThreadId;

		static void sendRxMsgData(char* recvBuffer);

		static void* initCoordinator(void* arg);
		static bool sendMessageToDaemon(char* buf, int bufsize);
		static DeviceType findDeviceId(unsigned short deviceTypeId, unsigned short IASZoneType);

		static int findMatchDevInfo(char* recvBufer);
};

#endif // _ZIGBEECONTROLLER_H

