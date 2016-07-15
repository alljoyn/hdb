#include "CommonUtil.h"

#include "utils/Log.h"

#include <qcc/StringUtil.h>
#include <algorithm>

#define TAG "CommonUtil"

using namespace ajn;
using namespace services;

#define CHECK_RETURN(x) if ((status = x) != ER_OK) { return status; }

BusAttachment* CommonUtil::prepareBusAttachment(ajn::AuthListener* authListener)
{
    BusAttachment* bus = new BusAttachment("InnoPiaBridge", true);

    /* Start the BusAttachment */
    QStatus status = bus->Start();
    if (status != ER_OK) {
		LOGD(TAG, "bus->Start error");
        delete bus;
        return NULL;
    }

    /* Connect to the daemon using address provided*/
    status = bus->Connect();
    if (status != ER_OK) {
		LOGD(TAG, "bus->Connect error");
        delete bus;
        return NULL;
    }

    if (authListener) {
        status = CommonUtil::EnableSecurity(bus, authListener);
        if (status != ER_OK) {
		LOGD(TAG, "EnableSecurity error");
            delete bus;
            return NULL;
        }
    }

    return bus;
}

QStatus CommonUtil::fillAboutData(AboutData* aboutdata, qcc::String const& appIdHex,
                                        qcc::String const& appName, qcc::String const& deviceId, DeviceNamesType const& deviceNames,
                                        qcc::String const& defaultLanguage)
{
    if (!aboutdata) {
        return ER_BAD_ARG_1;
    }

    QStatus status = ER_OK;

    if (!appIdHex.empty()) {
        CHECK_RETURN(aboutdata->SetAppId(appIdHex.c_str()));
    }

    if (deviceId != "") {
        CHECK_RETURN(aboutdata->SetDeviceId(deviceId.c_str()))
    }

    std::vector<qcc::String> languages(3);
    languages[0] = "en";
    languages[1] = "es";
    languages[2] = "fr";

    for (size_t i = 0; i < languages.size(); i++) {
        CHECK_RETURN(aboutdata->SetSupportedLanguage(languages[i].c_str()))
    }

    if (defaultLanguage != "") {
        CHECK_RETURN(aboutdata->SetDefaultLanguage(defaultLanguage.c_str()))
    }

    if (appName != "") {
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[0].c_str()))
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[1].c_str()))
        CHECK_RETURN(aboutdata->SetAppName(appName.c_str(), languages[2].c_str()))
    }

    CHECK_RETURN(aboutdata->SetModelNumber("Wxfy388i"))
    CHECK_RETURN(aboutdata->SetDateOfManufacture("10/1/2199"))
    CHECK_RETURN(aboutdata->SetSoftwareVersion("12.20.44 build 44454"))
    CHECK_RETURN(aboutdata->SetHardwareVersion("355.499. b"))

    DeviceNamesType::const_iterator iter = deviceNames.find(languages[0]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[0].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("My device name", "en"));
    }

    iter = deviceNames.find(languages[1]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[1].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("Mi nombre de dispositivo", "es"));
    }

    iter = deviceNames.find(languages[2]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(aboutdata->SetDeviceName(iter->second.c_str(), languages[2].c_str()));
    } else {
        CHECK_RETURN(aboutdata->SetDeviceName("Mon nom de l'appareil", "fr"));
    }

    CHECK_RETURN(aboutdata->SetDescription("This is an Alljoyn Application", "en"))
    CHECK_RETURN(aboutdata->SetDescription("Esta es una Alljoyn aplicacion", "es"))
    CHECK_RETURN(aboutdata->SetDescription("C'est une Alljoyn application", "fr"))

    CHECK_RETURN(aboutdata->SetManufacturer("Company", "en"))
    CHECK_RETURN(aboutdata->SetManufacturer("Empresa", "es"))
    CHECK_RETURN(aboutdata->SetManufacturer("Entreprise", "fr"))

    CHECK_RETURN(aboutdata->SetSupportUrl("http://www.alljoyn.org"))

    if (!aboutdata->IsValid()) {
        LOGD(TAG, "failed to setup about data.\n");
        return ER_FAIL;
    }
    return status;
}

QStatus CommonUtil::prepareAboutService(BusAttachment* bus, AboutData* aboutData, AboutObj* aboutObj, AboutObjApi* aboutApi,
                                              CommonBusListener* busListener, uint16_t port)
{
    if (!bus) {
        return ER_BAD_ARG_1;
    }

    if (!aboutData) {
        return ER_BAD_ARG_2;
    }

    if (!busListener) {
        return ER_BAD_ARG_3;
    }

    if (!aboutApi) {
        return ER_BUS_NOT_ALLOWED;
    }
    aboutApi->Init(bus, aboutData, aboutObj);

    busListener->setSessionPort(port);
    bus->RegisterBusListener(*busListener);

    TransportMask transportMask = TRANSPORT_ANY;
    SessionPort sp = port;
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, transportMask);

    QStatus status = bus->BindSessionPort(sp, opts, *busListener);
    if (status != ER_OK) {
        return status;
    }

    aboutApi->SetPort(port);

    return ER_OK;
}

QStatus CommonUtil::aboutServiceAnnounce(AboutObjApi* aboutApi)
{
    if (!aboutApi) {
        return ER_BUS_NOT_ALLOWED;
    }

    return (aboutApi->Announce());
}

void CommonUtil::aboutServiceDestroy(BusAttachment* bus, CommonBusListener* busListener, AboutObjApi* aboutApi)
{
    if (bus && busListener) {
        bus->UnregisterBusListener(*busListener);
        bus->UnbindSessionPort(busListener->getSessionPort());
    }
    return;
}

QStatus CommonUtil::EnableSecurity(BusAttachment* bus, AuthListener* authListener)
{
    QStatus status = bus->EnablePeerSecurity("ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", authListener);
    return status;
}
