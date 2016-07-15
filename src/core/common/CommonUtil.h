#ifndef COMMONUTIL_H_
#define COMMONUTIL_H_

#include <alljoyn/BusAttachment.h>
#include <qcc/String.h>
#include <alljoyn/AboutData.h>
#include <alljoyn/AboutObj.h>
#include "AboutObjApi.h"
#include "CommonBusListener.h"
#include <map>

typedef std::map<qcc::String, qcc::String> DeviceNamesType;

/**
 * Util class that creates a busAttachment, starts it and connects it
 */
namespace ajn {
namespace services {

class CommonUtil {
	public:
		/**
		 * static method that prepares the BusAttachment
		 * @param authListener - authListener if want to run secured
		 * @return the BusAttachment created. NULL if failed
		 */
		static ajn::BusAttachment* prepareBusAttachment(ajn::AuthListener* authListener = 0);

		/**
		 * static method fillAboutData
		 * @param aboutData
		 * @param appIdHex
		 * @param appName
		 * @param deviceId
		 * @param deviceNames
		 * @param defaultLanguage
		 * @return the property store created. NULL if failed
		 */
		static QStatus fillAboutData(ajn::AboutData* aboutData, qcc::String const& appIdHex,
				qcc::String const& appName, qcc::String const& deviceId, DeviceNamesType const& deviceNames,
				qcc::String const& defaultLanguage = "en");

		/**
		 * static method prepareAboutService
		 * @param bus
		 * @param aboutData
		 * @param aboutObj
		 * @param busListener
		 * @param port
		 * @return Qstatus
		 */
		static QStatus prepareAboutService(ajn::BusAttachment* bus, ajn::AboutData* aboutData, 
				ajn::AboutObj* aboutObj, AboutObjApi* aboutApi,
				CommonBusListener* busListener, uint16_t port);

		/**
		 * static method aboutServiceAnnounce
		 * @return Qstatus
		 */
		static QStatus aboutServiceAnnounce(AboutObjApi* aboutApi);

		/**
		 * static method aboutServiceDestory
		 * @param bus
		 * @param busListener
		 */
		static void aboutServiceDestroy(ajn::BusAttachment* bus, CommonBusListener* busListener, AboutObjApi* aboutApi);

	private:

		/**
		 * EnableSecurity
		 * @param bus
		 * @param authListener
		 * @return success/failure
		 */
		static QStatus EnableSecurity(ajn::BusAttachment* bus, ajn::AuthListener* authListener);
};
}
}
#endif /* COMMONUTIL_H_ */
