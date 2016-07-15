#ifndef _CONTROLLERIFACE_H
#define _CONTROLLERIFACE_H

#include "ControllerDef.h"

class ConnectionListener {
	public:
		virtual ~ConnectionListener() {}

		virtual void DeviceFound(ConnectionType connType, DeviceType devType, uint16_t nodeId, const char* addtional=NULL) = 0;
		virtual void DeviceLost(ConnectionType connType, uint16_t nodeId) = 0;
};

class ControllerListener {
	public:
		virtual ~ControllerListener() {}
		
		virtual void DeviceRemoved(ConnectionType connType, uint16_t nodeId) = 0;

		virtual void StateChanged(ConnectionType connType, ControllerState state) = 0;

		virtual void Reported(ConnectionType connType, RepDataT data) = 0;

		virtual void BatteryLevel(ConnectionType connType, int val) = 0;
};

class ControllerIface {
	public:
		virtual ~ControllerIface() {}

		// return CTRL_INITIALIZED when success initializing
		virtual ControllerState Init() = 0;

		// return CTRL_READY when success starting
		virtual ControllerState Start() = 0;

		virtual void Final() = 0;

		virtual ControllerState GetState() = 0;

		virtual void SetState(ControllerState state) = 0;

		virtual void GetList() = 0;

		virtual void Reset() = 0;

		virtual void AddListener(ControllerListener* listener) = 0;

		virtual void RemoveListener(ControllerListener* listener) = 0;
};

#endif // _CONTROLLERIFACE_H

