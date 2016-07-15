#ifndef CONFIGSERVICELISTENERIMPL_H_
#define CONFIGSERVICELISTENERIMPL_H_

#include <alljoyn/config/ConfigService.h>
#include "AboutDataStore.h"
#include "CommonBusListener.h"
#include "OnboardingControllerImpl.h"

/**
 * ConfigServiceListenerImpl
 */

namespace ajn {
namespace services {
	
class ConfigListener {
	public:
		virtual ~ConfigListener() {}
		virtual void Restart() = 0;
		virtual void FactoryReset() = 0;
};

class ConfigServiceListenerImpl : public ajn::services::ConfigService::Listener {
	public:
		ConfigServiceListenerImpl(AboutDataStore& store, BusAttachment& bus, CommonBusListener& busListener, AboutObjApi& aboutApi, OnboardingControllerImpl& onbCont);

		virtual QStatus Restart();
		virtual QStatus FactoryReset();
		virtual QStatus SetPassphrase(const char* daemonRealm, size_t passcodeSize, const char* passcode, ajn::SessionId sessionId);
		virtual ~ConfigServiceListenerImpl();

		void AddListener(ConfigListener* listener);
		void RemoveListener();
	private:

		AboutDataStore* m_AboutDataStore;
		BusAttachment* m_Bus;
		CommonBusListener* m_BusListener;
		AboutObjApi* m_AboutApi;
		OnboardingControllerImpl* m_OnboardingController;

		ConfigListener* mConfigListener;

		void PersistPassword(const char* daemonRealm, const char* passcode);
};

}
}
#endif /* CONFIGSERVICELISTENERIMPL_H_ */

