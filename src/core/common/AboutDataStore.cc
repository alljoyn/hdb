#include "AboutDataStore.h"

#include "utils/FileUtil.h"
#include "utils/Log.h"
#include "GlobalDef.h"

#include <alljoyn/config/AboutDataStoreInterface.h>
#include <alljoyn/AboutData.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <alljoyn/services_common/GuidUtil.h>

#define TAG "AboutDataStore"

using namespace ajn;
using namespace services;

AboutDataStore::AboutDataStore(const char* factoryConfigFile, const char* configFile, AboutObjApi* aboutApi) :
    AboutDataStoreInterface(factoryConfigFile, configFile), m_IsInitialized(false)
{
    LOGD(TAG, "AboutDataStore::AboutDataStore");

	m_defaultFileName.assign(DEFAULT_DEVICE_CONF_FILE);
    m_configFileName.assign(configFile);
    m_factoryConfigFileName.assign(factoryConfigFile);
    SetNewFieldDetails("Passcode", EMPTY_MASK, "s");
    SetNewFieldDetails("Daemonrealm", EMPTY_MASK, "s");
    MsgArg arg("s", "");
    SetField("Daemonrealm", arg);

	mAboutApi = aboutApi;
}

void AboutDataStore::Initialize(qcc::String deviceId, qcc::String appId, qcc::String deviceName)
{
    LOGD(TAG, "AboutDataStore::Initialize");
    QStatus status;

    std::ifstream configFile(m_configFileName.c_str(), std::ios::binary);
    if (configFile) {
        std::string str((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
        //LOGD(TAG, "Contains: %s", str.c_str());
        status = CreateFromXml(qcc::String(str.c_str()));

        if (status != ER_OK) {
            LOGD(TAG, "AboutDataStore::Initialize ERROR");
            return;
        }

        if ((!deviceId.empty()) || (!appId.empty())) {
            AboutData factoryData;
		   	char* saved = NULL;
			char* etcVersion = NULL;
            std::ifstream factoryFile(m_factoryConfigFileName.c_str(), std::ios::binary);
            if (factoryFile) {
                std::string factoryStr((std::istreambuf_iterator<char>(factoryFile)),
                                       std::istreambuf_iterator<char>());
                //LOGD(TAG, "Contains: %s", factoryStr.c_str());
                status = factoryData.CreateFromXml(qcc::String(factoryStr.c_str()));
                if (status != ER_OK) {
                    LOGD(TAG, "AboutDataStore::Initialize ERROR");
                    return;
                }
            }

            if (!deviceId.empty()) {
                SetDeviceId(deviceId.c_str());
                factoryData.SetDeviceId(deviceId.c_str());
            }

            if (!appId.empty()) {
                SetAppId(appId.c_str());
                factoryData.SetAppId(appId.c_str());
            }
          
			GetDeviceName(&saved);
			LOGD(TAG, "saved: %s, mac-applied deviceName.c_str(): %s", saved, deviceName.c_str());
			// If saved device name is default, then set mac-applied.
			if (strcmp(saved, DEFAULT_BRIDGE_DEV_NAME) == 0 || strcmp(saved, DEFAULT_VIRTUAL_DEV_NAME) == 0) {
				SetDeviceName(deviceName.c_str());
				factoryData.SetDeviceName(deviceName.c_str());
			}
			// If saved device name is not equals with mac-applied, then set saved.
			// Because it is user modified.
			else if (strcmp(saved, deviceName.c_str()) != 0) {
				SetDeviceName(saved);
			}
			// Set mac-applied.
			else {
				SetDeviceName(deviceName.c_str());
			}

            //Generate xml
            qcc::String str = ToXml(this);
            //write to config file
            std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
            //write to config file
            iniFileWrite.write(str.c_str(), str.length());
            iniFileWrite.close();

            //Generate xml
            qcc::String writeStr = ToXml(&factoryData);
            //write to config file
            std::ofstream factoryFileWrite(m_factoryConfigFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
            //write to config file
            factoryFileWrite.write(writeStr.c_str(), writeStr.length());
            factoryFileWrite.close();
        }

        size_t numFields = GetFields();
        LOGD(TAG, "AboutDataStore::Initialize() numFields=%d", numFields);
    }
	else {
		if (FU_CopyFile(m_defaultFileName.c_str(), m_configFileName.c_str()) == ER_OK &&
				FU_CopyFile(m_defaultFileName.c_str(), m_factoryConfigFileName.c_str()) == ER_OK) {

			Initialize(deviceId, appId, deviceName);
			return;
		}
		else {
			LOGD(TAG, "FU_CopyFile() error.");
		}
	}

    if (!IsValid()) {
        LOGD(TAG, "AboutDataStore::Initialize FAIL");
    } else {
        m_IsInitialized = true;
        LOGD(TAG, "AboutDataStore::Initialize End");
    }
}

void AboutDataStore::SetOBCFG()
{
    SetNewFieldDetails("scan_file", EMPTY_MASK, "s");
    MsgArg argScanFile("s", "/tmp/wifi_scan_results");
    SetField("scan_file", argScanFile);
    SetNewFieldDetails("error_file", EMPTY_MASK, "s");
    MsgArg argErrorFile("s", "/tmp/state/alljoyn-onboarding-lasterror");
    SetField("error_file", argErrorFile);
    SetNewFieldDetails("state_file", EMPTY_MASK, "s");
    MsgArg argStateFile("s", "/tmp/state/alljoyn-onboarding-lasterror");
    SetField("state_file", argStateFile);
    SetNewFieldDetails("connect_cmd", EMPTY_MASK, "s");
    MsgArg argConnectCmd("s", "/tmp/state/alljoyn-onboarding");
    SetField("connect_cmd", argConnectCmd);
    SetNewFieldDetails("offboard_cmd", EMPTY_MASK, "s");
    MsgArg argOffboardCmd("s", "/tmp/state/alljoyn-onboarding");
    SetField("offboard_cmd", argOffboardCmd);
    SetNewFieldDetails("configure_cmd", EMPTY_MASK, "s");
    MsgArg argConfigureCmd("s", "/tmp/state/alljoyn-onboarding");
    SetField("configure_cmd", argConfigureCmd);
    SetNewFieldDetails("scan_cmd", EMPTY_MASK, "s");
    MsgArg argScanCmd("s", "/tmp/state/alljoyn-onboarding");
    SetField("scan_cmd", argScanCmd);
}

void AboutDataStore::FactoryReset()
{
    LOGD(TAG, "AboutDataStore::FactoryReset");

    m_IsInitialized = false;

    std::ifstream factoryConfigFile(m_factoryConfigFileName.c_str(), std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(factoryConfigFile)),
                    std::istreambuf_iterator<char>());
    factoryConfigFile.close();

    std::ofstream configFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    configFileWrite.write(str.c_str(), str.length());
    configFileWrite.close();

    Initialize();
}

AboutDataStore::~AboutDataStore()
{
    LOGD(TAG, "AboutDataStore::~AboutDataStore");
}

QStatus AboutDataStore::ReadAll(const char* languageTag, DataPermission::Filter filter, ajn::MsgArg& all)
{
    QCC_UNUSED(filter);
    LOGD(TAG, "AboutDataStore::ReadAll");
    QStatus status = GetAboutData(&all, languageTag);
    LOGD(TAG, "GetAboutData status = %s", QCC_StatusText(status));
    return status;
}

QStatus AboutDataStore::Update(const char* name, const char* languageTag, const ajn::MsgArg* value)
{
    LOGD(TAG, "AboutDataStore::Update name: %s, languageTag: %s, value: %s", name, languageTag, value->ToString().c_str());

    QStatus status = ER_INVALID_VALUE;
    if (strcmp(name, AboutData::APP_ID) == 0) {
        uint8_t* appId = NULL;
        size_t* num = NULL;
        status = value->Get("ay", num, &appId);
        if (status == ER_OK) {
            status = SetAppId(appId, *num);
        }
    } else if (strcmp(name, AboutData::DEFAULT_LANGUAGE) == 0) {
        char* defaultLanguage;
        status = value->Get("s", &defaultLanguage);
        if (status == ER_OK) {
            if (0 == strcmp(defaultLanguage, "")) {
                status = ER_LANGUAGE_NOT_SUPPORTED;
            } else {
                status = IsLanguageSupported(defaultLanguage);
                if (status == ER_OK) {
                    status = SetDefaultLanguage(defaultLanguage);
                }
            }
        }
    } else if (strcmp(name, AboutData::DEVICE_NAME) == 0) {
        LOGD(TAG, "Got device name");
        char* deviceName = NULL;
        status = value->Get("s", &deviceName);
        status = IsLanguageSupported(languageTag);
        if (status == ER_OK) {
            status = SetDeviceName(deviceName, languageTag);
        }
    } else if (strcmp(name, AboutData::DEVICE_ID) == 0) {
        char* deviceId = NULL;
        status = value->Get("s", &deviceId);
        if (status == ER_OK) {
            status = SetDeviceId(deviceId);
        }
    } else if (strcmp(name, AboutData::APP_NAME) == 0) {
        char* appName = NULL;
        status = value->Get("s", &appName);
        if (status == ER_OK) {
            status = SetAppName(appName, languageTag);
        }
    } else if (strcmp(name, AboutData::MANUFACTURER) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetManufacturer(chval);
        }
    } else if (strcmp(name, AboutData::MODEL_NUMBER) == 0) {
        char* chval = NULL;
        status = value->Get("s", chval);
        if (status == ER_OK) {
            status = SetModelNumber(chval);
        }
    } else if (strcmp(name, AboutData::SUPPORTED_LANGUAGES) == 0) {
        //Added automatically when adding value
        LOGD(TAG, "AboutDataStore::Update - supported languages will be added automatically when adding value");
    } else if (strcmp(name, AboutData::DESCRIPTION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetDescription(chval);
        }
    } else if (strcmp(name, AboutData::DATE_OF_MANUFACTURE) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetDateOfManufacture(chval);
        }
    } else if (strcmp(name, AboutData::SOFTWARE_VERSION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetSoftwareVersion(chval);
        }
    } else if (strcmp(name, AboutData::HARDWARE_VERSION) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetHardwareVersion(chval);
        }
    } else if (strcmp(name, AboutData::SUPPORT_URL) == 0) {
        char* chval = NULL;
        status = value->Get("s", &chval);
        if (status == ER_OK) {
            status = SetSupportUrl(chval);
        }
    }

    if (status == ER_OK) {
        //Generate xml
        qcc::String str = ToXml(this);
        //write to config file
        std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
        //write to config file
        iniFileWrite.write(str.c_str(), str.length());
        iniFileWrite.close();

        if (mAboutApi) {
            status = mAboutApi->Announce();
            LOGD(TAG, "Announce status %s", QCC_StatusText(status));
        }
    }

    return status;
}

QStatus AboutDataStore::Delete(const char* name, const char* languageTag)
{
    LOGD(TAG, "AboutDataStore::Delete(%s, %s)", name, languageTag);
    QStatus status = ER_INVALID_VALUE;

    ajn::AboutData factorySettings("en");
    std::ifstream configFile(m_factoryConfigFileName.c_str(), std::ios::binary);
    if (configFile) {
        std::string str((std::istreambuf_iterator<char>(configFile)),
                        std::istreambuf_iterator<char>());
        LOGD(TAG, "Contains: %s", str.c_str());
        QStatus status;
        status = factorySettings.CreateFromXml(qcc::String(str.c_str()));

        if (status != ER_OK) {
            LOGD(TAG, "AboutDataStore::Initialize ERROR");
            return status;
        }
    }

    if (strcmp(name, AboutData::APP_ID) == 0) {
        uint8_t* appId;
        size_t num;
        status = factorySettings.GetAppId(&appId, &num);
        if (status == ER_OK) {
            status = SetAppId(appId, num);
        }
    } else if (strcmp(name, AboutData::DEFAULT_LANGUAGE) == 0) {
        char* defaultLanguage;
        status = factorySettings.GetDefaultLanguage(&defaultLanguage);
        if (status == ER_OK) {
            status = SetDefaultLanguage(defaultLanguage);
        }
    } else if (strcmp(name, AboutData::DEVICE_NAME) == 0) {
        status = IsLanguageSupported(languageTag);
        if (status == ER_OK) {
            char* deviceName = NULL;
            status = factorySettings.GetDeviceName(&deviceName, languageTag);
            LOGD(TAG, "GetDeviceName status %s", QCC_StatusText(status));
            if (status == ER_OK) {
                status = SetDeviceName(deviceName, languageTag);
                LOGD(TAG, "SetDeviceName status %s", QCC_StatusText(status));
            }
        }
    } else if (strcmp(name, AboutData::DEVICE_ID) == 0) {
        char* deviceId = NULL;
        status = factorySettings.GetDeviceId(&deviceId);
        if (status == ER_OK) {
            status = SetDeviceId(deviceId);
        }
    } else if (strcmp(name, AboutData::APP_NAME) == 0) {
        char* appName;
        status = factorySettings.GetAppName(&appName);
        if (status == ER_OK) {
            status = SetAppName(appName, languageTag);
        }
    } else if (strcmp(name, AboutData::MANUFACTURER) == 0) {
        char* manufacturer = NULL;
        status = factorySettings.GetManufacturer(&manufacturer, languageTag);
        if (status == ER_OK) {
            status = SetManufacturer(manufacturer, languageTag);
        }
    } else if (strcmp(name, AboutData::MODEL_NUMBER) == 0) {
        char* modelNumber = NULL;
        status = factorySettings.GetModelNumber(&modelNumber);
        if (status == ER_OK) {
            status = SetModelNumber(modelNumber);
        }
    } else if (strcmp(name, AboutData::SUPPORTED_LANGUAGES) == 0) {
        size_t langNum;
        langNum = factorySettings.GetSupportedLanguages();
        LOGD(TAG, "Number of supported languages: %d", langNum);
        if (langNum > 0) {
            const char** langs = new const char*[langNum];
            factorySettings.GetSupportedLanguages(langs, langNum);
            for (size_t i = 0; i < langNum; ++i) {
                SetSupportedLanguage(langs[i]);
            }
        }
    } else if (strcmp(name, AboutData::DESCRIPTION) == 0) {
        char* description = NULL;
        status = factorySettings.GetDescription(&description, languageTag);
        if (status == ER_OK) {
            status = SetDescription(description, languageTag);
        }
    } else if (strcmp(name, AboutData::DATE_OF_MANUFACTURE) == 0) {
        char* date = NULL;
        status = factorySettings.GetDateOfManufacture(&date);
        if (status == ER_OK) {
            status = SetDateOfManufacture(date);
        }
    } else if (strcmp(name, AboutData::SOFTWARE_VERSION) == 0) {
        char* version = NULL;
        status = factorySettings.GetSoftwareVersion(&version);
        if (status == ER_OK) {
            status = SetSoftwareVersion(version);
        }
    } else if (strcmp(name, AboutData::HARDWARE_VERSION) == 0) {
        char* version = NULL;
        status = factorySettings.GetHardwareVersion(&version);
        if (status == ER_OK) {
            status = SetHardwareVersion(version);
        }
    } else if (strcmp(name, AboutData::SUPPORT_URL) == 0) {
        char* url = NULL;
        status = factorySettings.GetSupportUrl(&url);
        if (status == ER_OK) {
            status = SetSupportUrl(url);
        }
    }

    if (status == ER_OK) {
        //Generate xml
        qcc::String str = ToXml(this);
        //write to config file
        std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
        //write to config file
        iniFileWrite.write(str.c_str(), str.length());
        iniFileWrite.close();

        if (mAboutApi) {
            status = mAboutApi->Announce();
            LOGD(TAG, "Announce status %s", QCC_StatusText(status));
        }
    }

    return status;
}

const qcc::String& AboutDataStore::GetConfigFileName()
{
    LOGD(TAG, "AboutDataStore::GetConfigFileName");
    return m_configFileName;
}

void AboutDataStore::write()
{
    //Generate xml
    qcc::String str = ToXml(this);
    //write to config file
    std::ofstream iniFileWrite(m_configFileName.c_str(), std::ofstream::out | std::ofstream::trunc);
    //write to config file
    iniFileWrite.write(str.c_str(), str.length());
    iniFileWrite.close();

    if (mAboutApi) {
        QStatus status = mAboutApi->Announce();
        LOGD(TAG, "Announce status %s", QCC_StatusText(status));
    }
}

qcc::String AboutDataStore::ToXml(AboutData* aboutData)
{
    LOGD(TAG, "AboutDataStore::ToXml");
    QStatus status = ER_OK;
    size_t numFields = aboutData->GetFields();
    if (0 == numFields) {
        LOGD(TAG, "numFields is 0");
        return "";
    }
    const char* fieldNames[512];
    aboutData->GetFields(fieldNames, numFields);
    char* defaultLanguage;
    status = aboutData->GetDefaultLanguage(&defaultLanguage);
    if (ER_OK != status) {
        LOGD(TAG, "GetDefaultLanguage failed");
        return "";
    }
    size_t numLangs = aboutData->GetSupportedLanguages();
    const char** langs = new const char*[numLangs];
    aboutData->GetSupportedLanguages(langs, numLangs);
    qcc::String res;
    res += "<AboutData>\n";
    for (size_t i = 0; i < numFields; i++) {
        ajn::MsgArg* arg;
        char* val;
        aboutData->GetField(fieldNames[i], arg);
        if (!strcmp(fieldNames[i], "AppId")) {
            res += "  <" + qcc::String(fieldNames[i]) + ">";
            size_t lay;
            uint8_t* pay;
            arg->Get("ay", &lay, &pay);
            std::stringstream ss;
            for (size_t j = 0; j < lay; ++j) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pay[j]);
            }
            res += ss.str().c_str();
            res += "</" + qcc::String(fieldNames[i]) + ">\n";
            continue;
        }

        if (arg->Signature() != "s") {
            continue;
        }

        arg->Get("s", &val);
        res += "  <" + qcc::String(fieldNames[i]) + ">";
        res += val;
        res += "</" + qcc::String(fieldNames[i]) + ">\n";
        if (!aboutData->IsFieldLocalized(fieldNames[i])) {
            continue;
        }

        for (size_t j = 0; j < numLangs; j++) {
            if (langs[j] == defaultLanguage) {
                continue;
            }

            res += "  <" + qcc::String(fieldNames[i]) + " lang=\"" + langs[j] + "\">";
            aboutData->GetField(fieldNames[i], arg, langs[j]);
            arg->Get("s", &val);
            res += val;
            res += "</" + qcc::String(fieldNames[i]) + ">\n";
        }
    }
    res += "</AboutData>";

    delete [] langs;
    return res;
}

QStatus AboutDataStore::IsLanguageSupported(const char* languageTag)
{
    QStatus status = ((QStatus)0x911a);
    LOGD(TAG, "AboutDataStore::IsLanguageSupported languageTag = %s", languageTag);
    size_t langNum;
    langNum = GetSupportedLanguages();
    LOGD(TAG, "Number of supported languages: %d", langNum);
    if (langNum > 0) {
        const char** langs = new const char*[langNum];
        GetSupportedLanguages(langs, langNum);
        for (size_t i = 0; i < langNum; ++i) {
            if (0 == strcmp(languageTag, langs[i])) {
                status = ER_OK;
                break;
            }
        }
        delete [] langs;
    }

    LOGD(TAG, "Returning %s", QCC_StatusText(status));
    return status;
}

