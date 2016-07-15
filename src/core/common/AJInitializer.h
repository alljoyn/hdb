#ifndef AJINITIALIZER_H_
#define AJINITIALIZER_H_

#include <alljoyn/Status.h>

/**
 * class AJInitializer
 * Utility class for handling AllJoyn lifecycle methods
 */
class AJInitializer {
  public:
    /**
     * Calls AllJoynInit(). If bundled router is enabled it also calls AllJoynRouterInit()
     * @return ER_OK if initialization succeeded
     */
    QStatus Initialize();

    /**
     * ~AJInitializer
     * Calls AllJoynRouterShutdown() if bundled router is enabled and then calls AllJoynShutdown()
     */
    ~AJInitializer();
};

#endif /* AJINITIALIZER_H_ */
