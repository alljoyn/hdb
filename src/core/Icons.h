#ifndef _ICONS_H
#define _ICONS_H

#include "icons/BridgeIcon.h"
#include "icons/DoorIcon.h"
#include "icons/TempHumiIcon.h"
#include "icons/PlugIcon.h"

uint8_t* DeviceIconPtr[DEV_MAX] = {
	NULL,
	bridgeIconContent,
	doorIconContent,
	temphumiIconContent,
	plugIconContent,
};


const int DeviceIconSize[DEV_MAX] = {
	0,
	BRIDGE_ICON_SIZE,
	DOOR_ICON_SIZE,
	TEMPHUMI_ICON_SIZE,
	PLUG_ICON_SIZE,
};

#endif // _ICONS_H
