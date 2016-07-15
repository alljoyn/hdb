#include "Options.h"
#include "GlobalDef.h"
#include "utils/Log.h"

#include <alljoyn/services_common/GuidUtil.h>
#include <alljoyn/AboutData.h>

#include <iostream>

#define TAG "Options"

using namespace ajn;
using namespace services;

char Options::mMacAddr[8] = "000000";
char Options::mHexMac[16] = {0,};

Options::Options(ConnectionType connType, DeviceType devType, int16_t id, const char* addtional) :
	mConnType(connType), mDevType(devType), mId(id)
{
	char conf[256] = {0 ,};
	char factoryConf[256] = {0 ,};
	
	if (addtional != NULL) {
		mAddtional.assign(addtional);
		LOGD(TAG, "Addtional info: %s", mAddtional.c_str());
		
		sprintf(conf, "%s%s-%s.conf", CONF_FILE_DIR, DeviceTypeStr[mDevType], mAddtional.c_str());
		sprintf(factoryConf, "%s%s-%s_factory.conf", CONF_FILE_DIR, DeviceTypeStr[mDevType], mAddtional.c_str());
	}
	else {
		sprintf(conf, "%s%s-%d.conf", CONF_FILE_DIR, DeviceTypeStr[mDevType], mId);
		sprintf(factoryConf, "%s%s-%d_factory.conf", CONF_FILE_DIR, DeviceTypeStr[mDevType], mId);
	}

    mConfigFile.assign(conf);
    mFactoryConfigFile.assign(factoryConf);
	LOGD(TAG, "conf file: %s", mConfigFile.c_str());
	LOGD(TAG, "factory conf file: %s", mFactoryConfigFile.c_str());

}

qcc::String const& Options::GetFactoryConfigFile() const {
    return mFactoryConfigFile;
}

qcc::String const& Options::GetConfigFile() const {
    return mConfigFile;
}

unsigned short Options::GetId() {
	return mId;
}

char* Options::GetAppId() {
	return mAppId;
}

char* Options::GetDeviceId() {
	return mDeviceId;
}

char* Options::GetDeviceName() {
	return mDeviceName;
}

ConnectionType Options::GetConnectionType() {
	return mConnType;
}

DeviceType Options::GetDeviceType() {
	return mDevType;
}

bool Options::IsAllHex(const char* data) {

    for (size_t index = 0; index < strlen(data); ++index) {
        if (!isxdigit(data[index])) {
            return false;
        }
    }
    return true;
}

void Options::String2Hex(char* src, char* res) {
	static unsigned char hex[] = "0123456789abcdef";

	if (src == NULL || strlen(src) != 6) {
		LOGD(TAG, "src(%s) is null or size incorrect.", src);
		return;
	}

	int res_idx = 0;

	for (int i = 0; i < 6; i++) {
		char ch = src[i];

		res[res_idx++] = hex[ch >> 4];
		res[res_idx++] = hex[ch & 0xf];
	}
	res[res_idx] = '\0';
	LOGD(TAG, "String2Hex result : %s", res);
}

void Options::SetWifiMac() {

	if ( strncmp(mMacAddr, "000000", 6) != 0 ) {
		LOGD(TAG, "mMacAddr(%s) already set.", mMacAddr);
		return;
	}

	char addrBuf[24] = {0,};
	int readLen;

	FILE *fp = NULL;

	while(1) {
		LOGD(TAG, "Try to read %s file.", WIFI_ADDR_FILE);
		fp = fopen(WIFI_ADDR_FILE, "r");
		if (fp != NULL) {
			readLen = fread(addrBuf, 1, sizeof(addrBuf), fp);

			int idx_buf, idx_mac;
			for (idx_buf=9, idx_mac=0; idx_buf < readLen; idx_buf++) {
				if (addrBuf[idx_buf] != ':' && addrBuf[idx_buf] != '\n') {
					if (addrBuf[idx_buf] >= 'a' && addrBuf[idx_buf] <= 'f')
						addrBuf[idx_buf] -= 32;
					mMacAddr[idx_mac++] = addrBuf[idx_buf];
				}
			}

			if ( strncmp(mMacAddr, "000000", 6) != 0 ) {
				mMacAddr[6] = '\0';
			}
			fclose(fp);
			break;
		}
		else
			LOGD(TAG, "/sys address file open error!!");

		sleep(2);
	}

	LOGD(TAG, "mMacAddr: %s", mMacAddr);
}

QStatus Options::ParseResult() {
    QStatus status = ER_OK;

	SetWifiMac();
	String2Hex(mMacAddr, mHexMac);

	if (mConnType == CONN_BRIDGE) {
		sprintf(mAppId, "%s%s-%s", APPID_PREFIX, APPID_BRIDGE, mHexMac);
		sprintf(mDeviceId, "%s-%s", DeviceTypeStr[mDevType], mMacAddr);
		sprintf(mDeviceName, "%s-%s", DeviceTypeStr[mDevType], mMacAddr);
	}
	else if (mConnType == CONN_ZIGBEE) {
		sprintf(mAppId, "%s%s-%02x%02x-%s", APPID_PREFIX, APPID_ZIGBEE, mId >> 8, mId & 0xff, mHexMac);
		sprintf(mDeviceId, "%s-%s-%d-%s", ConnectionTypeStr[mConnType], DeviceTypeStr[mDevType], mId, mMacAddr);
		sprintf(mDeviceName, "%s-%d", DeviceTypeStr[mDevType], mId);
	}
	else if (mConnType == CONN_ZWAVE) {
		sprintf(mAppId, "%s%s-%02x%02x-%s", APPID_PREFIX, APPID_ZWAVE, mId >> 8, mId & 0xff, mHexMac);
		sprintf(mDeviceId, "%s-%s-%d-%s", ConnectionTypeStr[mConnType], DeviceTypeStr[mDevType], mId, mMacAddr);
		sprintf(mDeviceName, "%s-%d", DeviceTypeStr[mDevType], mId);
	}
	else {
		LOGD(TAG, "mConnType(%d) is not supported yet.", mConnType);
		status = ER_FAIL;
	}

	LOGD(TAG, "mAppId: %s(%p)", mAppId, mAppId);
	LOGD(TAG, "mDeviceId: %s(%p)", mDeviceId, mDeviceId);
	LOGD(TAG, "mDeviceName: %s(%p)", mDeviceName, mDeviceName);
	LOGD(TAG, "mMacAddr: %s(%p)", mMacAddr, mMacAddr);
	LOGD(TAG, "mHexMac: %s(%p)", mHexMac, mHexMac );

    return status;
}

