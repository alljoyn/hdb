#include "utils/Log.h"
#include "EventsAndActions.h"

#include <alljoyn/BusObject.h>
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/AllJoynStd.h>

#include <algorithm>

#define TAG "EventsAndActions"

using namespace ajn;
using namespace qcc;
using namespace std;

namespace ajn {
namespace services {

struct EventInfo {
	char* name;
	char* desc;
	const InterfaceDescription::Member* member;
};

struct ActionInfo {
	char* name;
	char* desc;
};

struct FindEvent {
	FindEvent(const char* name) : name(name) {}
	bool operator()(const EventInfo& ei) { return (strcmp(ei.name, name) == 0); }
	const char* name;
};

struct FindAction {
	FindAction(const char* name) : name(name) {}
	bool operator()(const ActionInfo& ai) { return (strcmp(ai.name, name) == 0); }
	const char* name;
};

static const char* PROPERTY_NAME_VERSION = "Version";
static const uint16_t EVENTS_AND_ACTIONS_VERSION = 1;

EventsAndActions::EventsAndActions(BusAttachment* busAttachment, const char* servicePath, const char* eventIfaceName, const char* actionIfaceName) :
    BusObject(servicePath),
    bus(busAttachment),
	mEventInterfaceDescription(NULL),
	mActionInterfaceDescription(NULL),
	mEventListMtx(new qcc::Mutex()),
	mActionListMtx(new qcc::Mutex()),
	mListener(NULL)
{
    LOGD(TAG, "servicePath: %s, eventIfaceName:%s, actionIfaceName:%s", servicePath, eventIfaceName, actionIfaceName);
    QStatus status = ER_OK;

	IF_NULL_RETURN(servicePath);
	IF_NULL_RETURN(eventIfaceName);
	IF_NULL_RETURN(actionIfaceName);

	mServicePath = qcc::String(servicePath);
	mEventIfaceName = qcc::String(eventIfaceName);
	mActionIfaceName = qcc::String(actionIfaceName);

    InitEventsInterface();
    InitActionsInterface();
}

EventsAndActions::~EventsAndActions()
{
	mEventListMtx->Lock();
	std::list<EventInfo>::iterator eit;
	for (eit = mEvents.begin(); eit != mEvents.end(); ++eit) {
		EventInfo* di = &(*eit);
		delete di->name;
		delete di->desc;
	}
	mEvents.clear();
	mEventListMtx->Unlock();

	mActionListMtx->Lock();
	std::list<ActionInfo>::iterator ait;
	for (ait = mActions.begin(); ait != mActions.end(); ++ait) {
		ActionInfo* di = &(*ait);
		delete di->name;
		delete di->desc;
	}
	mActions.clear();
	mActionListMtx->Unlock();

	if (mEventListMtx != NULL) {
		delete mEventListMtx;
		mEventListMtx = NULL;
	}

	if (mActionListMtx != NULL) {
		delete mActionListMtx;
		mActionListMtx = NULL;
	}
}

void EventsAndActions::InitEventsInterface()
{
    QStatus status = ER_OK;
    mEventInterfaceDescription = const_cast<InterfaceDescription*>(bus->GetInterface(mEventIfaceName.c_str()));
    if (!mEventInterfaceDescription) {
        status = bus->CreateInterface(mEventIfaceName.c_str(), mEventInterfaceDescription, AJ_IFC_SECURITY_OFF);
        if (status != ER_OK) {
            LOGD(TAG, "Failed to create interface %s\n", mEventIfaceName.c_str());
            return;
        }

        if (!mEventInterfaceDescription) {
            LOGD(TAG, "Failed to create interface %s\n", mEventIfaceName.c_str());
            return;
        }
    }
	else {
		LOGD(TAG, "Fail to GetInterface %s", mEventIfaceName.c_str());
	}
}

QStatus EventsAndActions::AddEvent(const char* name, const char* desc)
{
    QStatus status = ER_OK;

	IF_NULL_RETURN_FAIL(mEventInterfaceDescription);
	IF_NULL_RETURN_FAIL(name);
	IF_NULL_RETURN_FAIL(desc);

	status = mEventInterfaceDescription->AddSignal(name, NULL, NULL);
	if (status != ER_OK) {
		LOGD(TAG, "Failed to add signal %s\n", name);
		return ER_FAIL;
	}
	mEventInterfaceDescription->SetMemberDescription(name, desc);

	EventInfo ei;
	memset (&ei, 0x0, sizeof(ei));
	ei.name = strdup(name);
	ei.desc = strdup(desc);

	// check exist and push back
	mEventListMtx->Lock();
	std::list<EventInfo>::iterator eit = find_if(mEvents.begin(), mEvents.end(), FindEvent(name));
	if (eit != mEvents.end()) {
		EventInfo* di = &(*eit);
		delete di->name;
		delete di->desc;

		mEvents.erase(eit);
	}
	mEvents.push_back(ei);	
	mEventListMtx->Unlock();
	return status;
}

QStatus EventsAndActions::AddEventProperty()
{
	LOGD(TAG, "mEventInterfaceDescription: %p", mEventInterfaceDescription);
	QStatus status = mEventInterfaceDescription->AddProperty(PROPERTY_NAME_VERSION, "q", PROP_ACCESS_READ);
	if (status != ER_OK) {
		LOGD(TAG, "Failed to add property %s\n", PROPERTY_NAME_VERSION);
	}
	return status;
}

void EventsAndActions::SetEventsDescription(const char* desc)
{
	mEventInterfaceDescription->SetDescriptionLanguage("en");
	mEventInterfaceDescription->SetDescription(desc);
}

QStatus EventsAndActions::SetEventsComplete()
{
	mEventInterfaceDescription->Activate();

    QStatus status = AddInterface(*mEventInterfaceDescription);
    if (status != ER_OK) {
        LOGD(TAG, "AddInterface failed\n");
		return status;
    }

	mEventListMtx->Lock();
	for (std::list<EventInfo>::iterator eit = mEvents.begin(); eit != mEvents.end(); ++eit) {
		EventInfo* ei = &(*eit);
		ei->member = mEventInterfaceDescription->GetMember(ei->name);
	}
	mEventListMtx->Unlock();

	return status;
}

void EventsAndActions::InitActionsInterface()
{
    QStatus status = ER_OK;
    mActionInterfaceDescription = const_cast<InterfaceDescription*>(bus->GetInterface(mActionIfaceName.c_str()));
    if (!mActionInterfaceDescription) {
        status = bus->CreateInterface(mActionIfaceName.c_str(), mActionInterfaceDescription, AJ_IFC_SECURITY_OFF);
        if (status != ER_OK) {
            LOGD(TAG, "Failed to create interface %s\n", mActionIfaceName.c_str());
            return;
        }
        if (!mActionInterfaceDescription) {
            LOGD(TAG, "Failed to create interface %s\n", mActionIfaceName.c_str());
            return;
        }
    }
}

QStatus EventsAndActions::AddAction(const char* name, const char* desc, const char* inputArg, const char* outputArg, const char* argNames)
{
    QStatus status = ER_OK;

	IF_NULL_RETURN_FAIL(mActionInterfaceDescription);
	IF_NULL_RETURN_FAIL(name);
	IF_NULL_RETURN_FAIL(desc);

	status = mActionInterfaceDescription->AddMethod(name, inputArg, outputArg, argNames);
	if (status != ER_OK) {
		LOGD(TAG, "Failed to add method %s\n", name);
	}
    mActionInterfaceDescription->SetMemberDescription(name, desc);

	ActionInfo ai;
	memset(&ai, 0x0, sizeof(ai));
	ai.name = strdup(name);
	ai.desc = strdup(desc);

	// check exist and push back
	mActionListMtx->Lock();
	std::list<ActionInfo>::iterator ait = find_if(mActions.begin(), mActions.end(), FindAction(name));
	if (ait != mActions.end()) {
		ActionInfo* di = &(*ait);
		delete di->name;
		delete di->desc;

		mActions.erase(ait);
	}
	mActions.push_back(ai);	
	mActionListMtx->Unlock();
	return status;
}

QStatus EventsAndActions::AddActionProperty()
{
	QStatus status = mActionInterfaceDescription->AddProperty(PROPERTY_NAME_VERSION, "q", PROP_ACCESS_READ);
	if (status != ER_OK) {
		LOGD(TAG, "Failed to add property %s\n", PROPERTY_NAME_VERSION);
	}
	return status;
}

void EventsAndActions::SetActionsDescription(const char* desc)
{
    mActionInterfaceDescription->SetDescriptionLanguage("en");
    mActionInterfaceDescription->SetDescription(desc);
}

QStatus EventsAndActions::SetActionsComplete()
{
    mActionInterfaceDescription->Activate();

    QStatus status = AddInterface(*mActionInterfaceDescription);
    if (status != ER_OK) {
        LOGD(TAG, "AddInterface failed\n");
        return status;
    }

	mActionListMtx->Lock();
	for (std::list<ActionInfo>::iterator ait = mActions.begin(); ait != mActions.end(); ++ait) {
		ActionInfo* ai = &(*ait);
    	status = AddMethodHandler(mActionInterfaceDescription->GetMember(ai->name), 
				static_cast<MessageReceiver::MethodHandler>(&EventsAndActions::ActionCalling));
		if (status != ER_OK) {
			LOGD(TAG, "Failed to add method handler for %s\n", ai->name);
		}
	}
	mActionListMtx->Unlock();

	return status;
}

QStatus EventsAndActions::RegisterEABusObject()
{
    SetDescription("en", "Events and Actions");
    QStatus status = bus->RegisterBusObject(*this);
    if (status != ER_OK) {
        LOGD(TAG, "Could not register the Events And Actions BusObject.\n");
    }
	return status;
}

void EventsAndActions::Check_MethodReply(const Message& msg, QStatus status)
{
    //check it the ALLJOYN_FLAG_NO_REPLY_EXPECTED exists if so send response ER_INVALID_DATA
    if (!(msg->GetFlags() & ALLJOYN_FLAG_NO_REPLY_EXPECTED)) {
        status = MethodReply(msg, status);
        if (status != ER_OK) {
            LOGD(TAG, "Method did not execute successfully.\n");
        }
    }
}

void EventsAndActions::ActionCalling(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
    LOGD(TAG, "method %s called.", member->name.c_str());
	std::list<ActionInfo>::iterator ait = find_if(mActions.begin(), mActions.end(), FindAction(member->name.c_str()));
	if (ait != mActions.end()) {
		ActionInfo* ai = &(*ait);
		if (mListener)
			mListener->ActionCallback(member->name.c_str(), msg);
	}
	else {
		LOGD(TAG, "action %s not exist in action list.", member->name.c_str());
	}

    QCC_UNUSED(member);
    Check_MethodReply(msg, ER_OK);
}

QStatus EventsAndActions::Get(const char* ifcName, const char* propName, MsgArg& val)
{
    QCC_UNUSED(ifcName);
    LOGD(TAG, "Get property was called.");

    if (0 != strcmp(PROPERTY_NAME_VERSION, propName)) {
        LOGD(TAG, "Called for property different than version.\n");
        return ER_BUS_NO_SUCH_PROPERTY;
    }

    val.typeId = ALLJOYN_UINT16;
    val.v_uint16 = EVENTS_AND_ACTIONS_VERSION;
    return ER_OK;
}

QStatus EventsAndActions::Set(const char* ifcName, const char* propName, MsgArg& val)
{
    QCC_UNUSED(ifcName);
    QCC_UNUSED(propName);
    QCC_UNUSED(val);
    return ER_ALLJOYN_ACCESS_PERMISSION_ERROR;
}

QStatus EventsAndActions::SendEvent(const InterfaceDescription::Member* actualEvent)
{
    LOGD(TAG, "SendEvent %s", actualEvent->name.c_str());

    if (!actualEvent) {
        LOGD(TAG, "signalMethod not set. Can't send signal.");
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    uint8_t flags =  ALLJOYN_FLAG_SESSIONLESS;
    QStatus status = Signal(NULL, 0, *actualEvent, NULL, 0, 0, flags);
    if (status != ER_OK) {
        LOGD(TAG, "Could not send signal.");
        return status;
    }

    LOGD(TAG, "Sent signal successfully");
    return status;
}

void EventsAndActions::SendEventByName(const char* name)
{
	std::list<EventInfo>::iterator eit = find_if(mEvents.begin(), mEvents.end(), FindEvent(name));
	if (eit != mEvents.end()) {
		EventInfo* ei = &(*eit);
		SendEvent(ei->member);
	}
	else {
		LOGD(TAG, "event %s not exist in event list.", name);
	}
	LOGD(TAG, "evnet %s occured.", name);
}

void EventsAndActions::AddAcionListener(ActionListener* listener)
{
	mListener = listener;
}

void EventsAndActions::RemoveActionListener()
{
	mListener = NULL;
}

} //namespace services
} //namespace ajn

