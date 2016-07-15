#include "AJInitializer.h"
#include <alljoyn/Init.h>

QStatus AJInitializer::Initialize()
{
    QStatus status = AllJoynInit();
    if (status != ER_OK) {
        return status;
    }
#ifdef ROUTER
    status = AllJoynRouterInit();
    if (status != ER_OK) {
        AllJoynShutdown();
        return status;
    }
#endif
    return status;
}

AJInitializer::~AJInitializer()
{
#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
}
