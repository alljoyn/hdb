#ifndef _ZIGBEE_DEFINE_H
#define _ZIGBEE_DEFINE_H

// TODO
// Change the path, because this will deleted by reboot
#define DEVICE_INFO_FILE_PATH "/tmp/zigbeeDeviceInfo.dat"

#define	DEFAULT_BUFFER_LEN 1024
#define	DEFAULT_DEVICE_NAME_LEN 100

typedef enum {
	JOIN_SUCCESS_TAG,
	LEAVE_SUCCESS_TAG,
	RX_MSG_RECEIVE_TAG,
	RX_IAS_RECEIVE_TAG,
	NETWORK_LEAVE_SUCCESS_TAG,
	CHECK_CLUSTER_TAG,
	DEVICE_ID_TAG,
	IAS_TYPE_TAG,
	DAEMON_CLOSE_TAG,
	JOIN_TIMEOUT_TAG,
	COMM_RSSI_TAG,
	DEVICE_ORIG_NAME_TAG,
	NO_MSG_TAG,
} ZIGBEE_TAG_FLAG;

enum {
	DEVICE_TYPE_IAS_MOTION				= 0xD,
	DEVICE_TYPE_IAS_OPEN_CLOSE			= 0x15,
	DEVICE_TYPE_IAS_MOISTURE			= 0x2A,
	DEVICE_TYPE_IAS_FIRE_SMOKE 			= 0x28,
	DEVICE_TYPE_IAS_FIRE_SMOKE_SIREN	= 0x28A,

	DEVICE_TYPE_ONOFF 					= 0x100,
	DEVICE_TYPE_DIMMABLE_BULB			= 0x101,
	DEVICE_TYPE_COLOR_BULB 				= 0x300,

	DEVICE_TYPE_TEMP_HUMIDITY 			= 0x302,
	DEVICE_TYPE_IAS_ZONE 				= 0x402,
	DEVICE_UNKNOWN						= 0xFFFF
};

typedef enum
{
	C_NONE 				= 0,
	C_BATTERY_INFO 		= 0x0001,
	C_ONOFF 			= 0x0006,
	C_LEVEL_CONTROL 	= 0x0008,
	C_COLOR_CHANGE 		= 0x0300,
	C_TEMPERATURE 		= 0x0402,
	C_HUMIDITY 			= 0xFC45, 	// 0x0405,
	C_HUMIDITY_0405 	= 0x0405, 	// For Heiman Temp/Humidity Sensor
	C_IAS_ZONE 			= 0x0500,
	C_UNKNOWN 			= 0xFFFF
} CLUSTER_TYPE;

typedef enum
{
	IAS_ZONE_STANDARD_CIE 	= 0,
	IAS_ZONE_OPEN_CLOSE 	= 0x15,
	IAS_ZONE_SMOKE 			= 0x28,
	IAS_ZONE_MOISTURE 		= 0x2A,
	IAS_ZONE_MOTION 		= 0x0D,
	IAS_ZONE_UNKNOWN 		= 0xFF
} IAS_ZONE_TYPE;

#define PJOIN_TIMEOUT 60 //seconds
#define LEAVE_TIMEOUT 10 //seconds

#endif // _ZIGBEE_DEFINE_H
