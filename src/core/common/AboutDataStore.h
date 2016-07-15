#ifndef ABOUT_DATA_STORE_H_
#define ABOUT_DATA_STORE_H_

#include "AboutObjApi.h"
#include <alljoyn/config/AboutDataStoreInterface.h>

#include <stdio.h>
#include <iostream>

/**
 * class AboutDataStore
 * Property store implementation
 */
namespace ajn {
namespace services {
class AboutDataStore : public AboutDataStoreInterface {

  public:
    /**
     * AboutDataStore - constructor
     * @param factoryConfigFile
     * @param configFile
     */
    AboutDataStore(const char* factoryConfigFile, const char* configFile, AboutObjApi* aboutApi);

    /**
     * SetOBCFG
     */
    void SetOBCFG();

    /**
     * FactoryReset
     */
    void FactoryReset();

    /**
     * GetConfigFileName
     * @return qcc::String&
     */
    const qcc::String& GetConfigFileName();

    /**
     * virtual Destructor
     */
    virtual ~AboutDataStore();

    /**
     * virtual method ReadAll
     * @param languageTag
     * @param filter
     * @param all
     * @return QStatus
     */
    virtual QStatus ReadAll(const char* languageTag, DataPermission::Filter filter, ajn::MsgArg& all);

    /**
     * virtual method Update
     * @param name
     * @param languageTag
     * @param value
     * @return QStatus
     */
    virtual QStatus Update(const char* name, const char* languageTag, const ajn::MsgArg* value);

    /**
     * virtual method Delete
     * @param name
     * @param languageTag
     * @return QStatus
     */
    virtual QStatus Delete(const char* name, const char* languageTag);

    /**
     * Write about data store as an xml config file
     */
    void write();

    /**
     * method Initialize
     */
    void Initialize(qcc::String deviceId = qcc::String(), qcc::String appId = qcc::String(), 
			qcc::String deviceName = qcc::String());

  private:
    bool m_IsInitialized;
	qcc::String m_defaultFileName;
    qcc::String m_configFileName;
    qcc::String m_factoryConfigFileName;

    qcc::String ToXml(AboutData* aboutData);

    QStatus IsLanguageSupported(const char* languageTag);

	AboutObjApi* mAboutApi;
};
}
}
#endif /* ABOUT_DATA_STORE_H_ */

