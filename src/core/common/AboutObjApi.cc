#include <alljoyn/AboutObj.h>
#include <alljoyn/AboutData.h>
#include "AboutObjApi.h"

using namespace ajn;
using namespace services;

AboutObjApi::AboutObjApi() {
	m_BusAttachment = NULL;
	m_AboutData = NULL;
	m_AboutObj = NULL;
	m_sessionPort = 0;
}

AboutObjApi::~AboutObjApi() {
}

void AboutObjApi::Init(ajn::BusAttachment* bus, AboutData* aboutData, AboutObj* aboutObj) {
    m_BusAttachment = bus;
    m_AboutData = aboutData;
    m_AboutObj = aboutObj;
}

void AboutObjApi::SetPort(SessionPort sessionPort) {
    m_sessionPort = sessionPort;
}

QStatus AboutObjApi::Announce() {
    if (m_AboutObj == NULL) {
        return ER_FAIL;
    }
    return m_AboutObj->Announce(m_sessionPort, *m_AboutData);
}

