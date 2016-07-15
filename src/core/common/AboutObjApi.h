#ifndef ABOUT_DATA_API_H
#define ABOUT_DATA_API_H

#include <qcc/Log.h>
#include <alljoyn/version.h>
#include <alljoyn/AboutObj.h>
#include <alljoyn/AboutData.h>

namespace ajn {
namespace services {

class AboutObjApi {

  public:
    /**
     * Constructor
     */
    AboutObjApi();
    /**
     * Desctructor
     */
    virtual ~AboutObjApi();

    /**
     * Init with  BusAttachment and AboutData only once.
     * After the first Init you can call getInstance to receive a proper instance of the class
     * @param bus
     * @param aboutData
     * @param aboutObj
     */
    void Init(ajn::BusAttachment* bus, AboutData* aboutData, AboutObj* aboutObj);
    /**
     * SetPort - set the port for the announce
     * @param sessionPort
     */
    void SetPort(SessionPort sessionPort);
    /**
     * This is used to send the Announce signal.
     */
    QStatus Announce();

  private:
    /**
     * pointer to BusAttachment
     */
    BusAttachment* m_BusAttachment;
    /**
     * pointer to AboutData
     */
    AboutData* m_AboutData;
    /**
     * pointer to AboutObj
     */
    AboutObj* m_AboutObj;
    /**
     * sesstion port for the about announcement
     */
    SessionPort m_sessionPort;
};

}
}
#endif /* ABOUT_SERVICE_API_H */
