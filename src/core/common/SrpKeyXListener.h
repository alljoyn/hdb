#ifndef SRPKEYLISTENER_H_
#define SRPKEYLISTENER_H_

#include "AboutDataStore.h"
#include <alljoyn/AuthListener.h>

/**
 * class SrpKeyXListener
 * A listener for Authentication
 */
class SrpKeyXListener : public ajn::AuthListener {
  public:
    /**
     * SrpKeyXListener
     */
    SrpKeyXListener(ajn::services::AboutDataStore* aboutDataStore);

    /**
     * ~SrpKeyXListener
     */
    virtual ~SrpKeyXListener();

    /**
     * setPassCode
     * @param passCode to set
     */
    void setPassCode(qcc::String const& passCode);

    /**
     * setGetPassCode
     * @param getPassCode - callback function to set
     */
    void setGetPassCode(void (*getPassCode)(ajn::services::AboutDataStore&, qcc::String&));

    /**
     * RequestCredentials
     * @param authMechanism
     * @param authPeer
     * @param authCount
     * @param userId
     * @param credMask
     * @param creds
     * @return boolean
     */
    bool RequestCredentials(const char* authMechanism, const char* authPeer, uint16_t authCount, const char* userId,
                            uint16_t credMask, Credentials& creds);

    /**
     * AuthenticationComplete
     * @param authMechanism
     * @param authPeer
     * @param success
     */
    void AuthenticationComplete(const char* authMechanism, const char* authPeer, bool success);

  private:
	ajn::services::AboutDataStore* m_AboutDataStore;
    qcc::String m_PassCode;

    void (*m_GetPassCode)(ajn::services::AboutDataStore&, qcc::String&);
};

#endif /* SRPKEYLISTENER_H_ */


