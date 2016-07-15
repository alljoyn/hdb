#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "plugins/ControllerDef.h"

#include <Status.h>
#include <qcc/String.h>

#include <map>

#define APPID_PREFIX	"696e6e6f-736d-74"	// "innosmt"
#define APPID_BRIDGE	"62-7267"	// "brg"
#define APPID_ZIGBEE    "62"        // "b"
#define APPID_ZWAVE     "77"        // "w"
#define APPID_TURK_CAM	"74"		// "t"

#define DEFAULT_MODELNAME   "IMT-"

class Options {
	public:
		Options(ConnectionType connType, DeviceType devType, int16_t id, const char* addtional=NULL);

		QStatus ParseResult();

		qcc::String const& GetFactoryConfigFile() const;
		qcc::String const& GetConfigFile() const;

		unsigned short 	GetId();
		char* 			GetAppId();
		char* 			GetDeviceId();
		char*			GetDeviceName();

		ConnectionType 	GetConnectionType();
		DeviceType 		GetDeviceType();

		void String2Hex(char* src, char* res);
		void SetWifiMac();

	private:
		int argc;
		char** argv;

		bool IsAllHex(const char* data);

		qcc::String mFactoryConfigFile;
		qcc::String mConfigFile;
		qcc::String mAppGUID;
		qcc::String mAddtional;

		ConnectionType 	mConnType;
		DeviceType 		mDevType;
		unsigned short 	mId;

		char mAppId[40];
		char mDeviceId[36];
		char mDeviceName[24];

		static char mMacAddr[8];
		static char mHexMac[16];
};

#endif /* _OPTIONS_H_ */
