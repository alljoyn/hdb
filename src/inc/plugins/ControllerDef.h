#ifndef _CONTROLLERDEF_H
#define _CONTROLLERDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ZIGBEE_LOW_BATTERY_LEVEL	21

// Replace CONN_MAX with new connection support.
enum ConnectionType {
	CONN_NONE	= 0,
	CONN_BRIDGE,
	CONN_ZIGBEE,
	CONN_MAX,
	CONN_ZWAVE,
	CONN_THREAD,
	CONN_BT,
	CONN_WIFI_CAMERA
};

// Add string with new connection support.
const char ConnectionTypeStr[CONN_MAX][16] = {
	"none",
	"bridge",
	"zigbee",
};

enum DeviceType {
	DEV_NONE 	= 0,
	DEV_BRIDGE,					// AllJoyn Bridge
	DEV_DOOR, 					// Door, 			Report: Open(true), Close(false)
	DEV_TEMP_HUMIDITY,			// Temp/Humi, 		Report: values
	DEV_PLUG, 					// Plug, 			Action: On(true), Off(false)
	DEV_MAX,
};

const char DeviceTypeStr[DEV_MAX][16] = {
	"None",
	"Bridge",
	"Door",
	"TempHumi",
	"Plug",
};

enum ControllerState {
	// common state
	CTRL_NOT_READY		= 0,
	CTRL_READY,
	CTRL_INITIALIZED,
	CTRL_INIT_FAILED,

	CTRL_CONNECTED		= 100,
	CTRL_DISCONNECTED,

	CTRL_ERROR			= 200,
	CTRL_ERROR_DATA_NULL,

	// zigbee/zwave
	CTRL_ADD_ENABLED 	= 1000,
	CTRL_ADD_ENABLED_WITH_ID,
	CTRL_ADDING,
	CTRL_REMOVE_ENABLED,
	CTRL_REMOVING,

	// wifi
	CTRL_STATION		= 2000,
	CTRL_SOFTAP,

	// camera
	CTRL_PLAYING		= 3000,
	CTRL_STOPPED,
	CTRL_RECORDING
};

typedef struct _BinaryData {
	bool flag;
} BinDataT, *pBinDataT;

typedef struct _TempHumiData {
	int temp;	// temperature
	int humi;	// humidify
} TempHumiDataT, *pTempHumiDataT;

typedef union _ReportData {
	BinDataT 			bin;
	TempHumiDataT 		th;
} RepDataT, *pRepDataT;

typedef struct _ControllerInfo {
	bool			isInitialized;
	uint16_t		id;
	ConnectionType	connType;
	DeviceType 		devType;
	ControllerState	state;
} ContInfoT, *pContInfoT;

#endif // _CONTROLLERDEF_H
