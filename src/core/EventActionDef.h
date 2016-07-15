#ifndef _EVENTACTIONDEF_H
#define _EVENTACTIONDEF_H

#include <map>

#define EA_NONE 		0		// 0
#define	EA_BINARY	 	1		// 1
#define	EA_INT_VAL	 	2		// 10
#define	EA_TEXT		 	4		// 100
#define	EA_STREAM	 	8		// 1000
#define EA_HAS_PARAM	16		// 10000
#define	EA_ALWAYS	 	128		// 10000000 	// trigger/act always
#define	EA_EQUAL_VAL	256		// 100000000 	// trigger when value is reached with custom value(currently only int value supported)
#define EA_SEND_NOTI	512		// 1000000000	// send noti after event/action occured

#define ACTION_MEMBER_REMOVE	"Remove"

typedef struct _CustomEventAction {
	char 		desc[128];		// description
	uint32_t	option;			// event type (EA_BINARY | EVNET_ALWAYS)
	int		 	data;			// data for EA_EQUAL_VAL
	const char*		inputSig;
	const char*		outputSig;
	const char*		argNames;
} CustomEventT, *pCustomEventT, CustomActionT, *pCustomActionT;

typedef std::map<char*, CustomEventT> CustomEventMap;
typedef std::map<char*, CustomActionT> CustomActionMap;

#define MEMBER_NAME_LEN 32

/*==============================================================================
 				 Bridge's Events & Action define
==============================================================================*/
#define BRIDGE_EVENT_NUM 5
static char BridgeEventInterface[] = "com.innopia.bridge.events";
static char BridgeServicePath[] = "/BridgeEventsAndActions";
static char BridgeEventDesc[] = "Events of the alljoyn bridge";
static char BridgeEventMembers[BRIDGE_EVENT_NUM][MEMBER_NAME_LEN] = {
	"DongleButtonPressed",
	"DeviceAdded",
	"DeviceRemoved",
	"ZigbeeAddMode",
	"ZigbeeReadyMode"
};
static CustomEventT BridgeEvents[BRIDGE_EVENT_NUM] = {
	{"Triggerred when Dongle button pressed", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when new device is added", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when new device is Removed", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when Zigbee controller add mode is enabled", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when Zigbee controller ready mode is enabled", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL}
};

#define BRIDGE_ACTION_NUM 2
static char BridgeActionInterface[] = "com.innopia.bridge.actions";
static char BridgeActionDesc[] = "Actions of the alljoyn bridge";
static char BridgeActionMembers[BRIDGE_ACTION_NUM][MEMBER_NAME_LEN] = {
	"SetZigbeeAddModeOn",
	"SendTestNoti"
};
static CustomActionT BridgeActions[BRIDGE_EVENT_NUM] = {
	{"Action set Zigbee add mode to on", EA_ALWAYS, 0, NULL, NULL, NULL},
	{"Action send test notification", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL}
};

/*==============================================================================
 				 Door's Events & Action define
==============================================================================*/
#define DOOR_EVENT_NUM 2
static char DoorEventInterface[] = "com.innopia.door.events";
static char DoorServicePath[] = "/DoorEventsAndActions";
static char DoorEventDesc[] = "Events of the door sensor";
static char DoorEventMembers[DOOR_EVENT_NUM][MEMBER_NAME_LEN] = {
	"Opened",
	"Closed"
};
static CustomEventT DoorEvents[DOOR_EVENT_NUM] = {
	{"Triggerred when door is opened", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when door is closed", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL}
};

#define DOOR_ACTION_NUM 1
static char DoorActionInterface[] = "com.innopia.door.actions";
static char DoorActionDesc[] = "Actions of the door sensor";
static char DoorActionMembers[DOOR_ACTION_NUM][MEMBER_NAME_LEN] = {
	ACTION_MEMBER_REMOVE
};
static CustomActionT DoorActions[DOOR_ACTION_NUM] = {
	{"Remove(Disconnect) this device", EA_ALWAYS, 0, NULL, NULL, NULL}
};

/*==============================================================================
 				 TempHumi's Events & Action define
==============================================================================*/
#define TEMPHUMI_EVENT_NUM 6
static char TempHumiEventInterface[] = "com.innopia.temphumi.events";
static char TempHumiServicePath[] = "/TempHumiEventsAndActions";
static char TempHumiEventDesc[] = "Events of the temperature/humidity sensor";
static char TempHumiEventMembers[TEMPHUMI_EVENT_NUM][MEMBER_NAME_LEN] = {
	"TemperatureChanged",
	"HumidityChanged",
	"TemperatureOver30",
	"TemperatureUnder20",
	"HumidityOver60",
	"HumidityUnder45"
};
static CustomEventT TempHumiEvents[TEMPHUMI_EVENT_NUM] = {
	{"Triggerred when temperture is changed", EA_ALWAYS, 0, NULL, NULL, NULL},
	{"Triggerred when humidity is changed", EA_ALWAYS, 0, NULL, NULL, NULL},
	{"Triggerred when temperture is over 30", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when temperture is under 20", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when humidity is over 60", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when humidity is under 45", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL}
};

#define TEMPHUMI_ACTION_NUM 1
static char TempHumiActionInterface[] = "com.innopia.temphumi.actions";
static char TempHumiActionDesc[] = "Actions of the temphumi sensor";
static char TempHumiActionMembers[TEMPHUMI_ACTION_NUM][MEMBER_NAME_LEN] = {
	ACTION_MEMBER_REMOVE
};
static CustomActionT TempHumiActions[TEMPHUMI_ACTION_NUM] = {
	{"Remove(Disconnect) this device", EA_ALWAYS, 0, NULL, NULL, NULL}
};

/*==============================================================================
 				 Plug's Events & Action define
==============================================================================*/
#define PLUG_EVENT_NUM 0
static char PlugEventInterface[] = "com.innopia.plug.events";
static char PlugServicePath[] = "/PlugEventsAndActions";
static char PlugEventDesc[] = "Events of the plug sensor";
/*static char PlugEventMembers[PLUG_EVENT_NUM][MEMBER_NAME_LEN] = {
	"TurnedOn",
	"TurnedOff"
};
static CustomEventT PlugEvents[PLUG_EVENT_NUM] = {
	{"Triggerred when Plug is turned on", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL},
	{"Triggerred when Plug is turned off", EA_ALWAYS | EA_SEND_NOTI, 0, NULL, NULL, NULL}
};*/

#define PLUG_ACTION_NUM 3
static char PlugActionInterface[] = "com.innopia.plug.actions";
static char PlugActionDesc[] = "Actions of the plug sensor";
static char PlugActionMembers[PLUG_ACTION_NUM][MEMBER_NAME_LEN] = {
	"TurnOn",
	"TurnOff",
	ACTION_MEMBER_REMOVE
};
static CustomActionT PlugActions[PLUG_ACTION_NUM] = {
	{"Turn on this plug", EA_ALWAYS, 0, NULL, NULL, NULL},
	{"Turn off this plug", EA_ALWAYS, 0, NULL, NULL, NULL},
	{"Remove(Disconnect) this device", EA_ALWAYS, 0, NULL, NULL, NULL}
};

#endif // _EVENTACTIONDEF_H
