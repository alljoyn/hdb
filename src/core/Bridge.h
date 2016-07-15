#ifndef _BRIDGE_H
#define _BRIDGE_H

#include "VirtualDevice.h"

namespace qcc {
class Mutex;
class Thread;
}

namespace ajn {
namespace services {

class Bridge : public VirtualDevice, public VirtualDeviceListener, public ConnectionListener {
	public:
		Bridge();
		~Bridge();

		QStatus InitControllers();

		// ConnectionListener
		void DeviceFound(ConnectionType connType, DeviceType devType, uint16_t nodeId, const char* addtional=NULL);
		void DeviceLost(ConnectionType connType, uint16_t nodeId);
		
		// ControllerListener
		void DeviceRemoved(ConnectionType connType, uint16_t nodeId);
		void StateChanged(ConnectionType connType, ControllerState state);
		void Reported(ConnectionType connType, RepDataT data);

		// VirtualDeviceListener
		void StateChanged(ConnectionType connType, uint16_t id, VDeviceState state);
		void NotiRequest(uint16_t id, char* deviceName, const char* msg);

		// ActionListener
		void ActionCallback(const char* member, ajn::Message&);

		// ConfigListener
		void Restart();
		void FactoryReset();

		void SetSoftAP();

	private:
		void SendNoti(const char* msg);

		uint32_t GetUID(ConnectionType connType, uint16_t nodeId);

	private:
		typedef std::map<uint32_t, VirtualDevice*> VDMap;

		ZigbeeController* 	mZigbeeController;

		qcc::Mutex* mVDevicesMutex;
		VDMap mVDevices;
};

}
}
#endif // _BRIDGE_H

