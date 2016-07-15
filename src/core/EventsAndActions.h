#ifndef EVENTSANDACTIONS_H_
#define EVENTSANDACTIONS_H_

#include <qcc/platform.h>
#include <qcc/String.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/MsgArg.h>
#include <alljoyn/Status.h>

#include <list>

namespace qcc {
class Mutex;
}

namespace ajn {
namespace services {

struct EventInfo;
struct ActionInfo;

class ActionListener {
	public:
	   	virtual ~ActionListener() {}
		virtual void ActionCallback(const char* member, ajn::Message& msg) = 0;
};

class EventsAndActions : public ajn::BusObject {
  public:
    EventsAndActions(ajn::BusAttachment* bus, const char* eaPath, const char* eventIfaceName, const char* actionIfaceName);
    ~EventsAndActions();

    QStatus Get(const char* ifcName, const char* propName, MsgArg& val);
    QStatus Set(const char* ifcName, const char* propName, MsgArg& val);

    QStatus SendEvent(const InterfaceDescription::Member* actualEvent);
    void SendEventByName(const char* name);

	QStatus AddEvent(const char* name, const char* desc);
	QStatus AddEventProperty();
	void SetEventsDescription(const char* desc);
	QStatus SetEventsComplete();

	QStatus AddAction(const char* name, const char* desc, const char* inputArg = NULL, const char* outputArg = NULL, const char* argNames = NULL);
	QStatus AddActionProperty();
	void SetActionsDescription(const char* desc);
	QStatus SetActionsComplete();

	QStatus RegisterEABusObject();

	void AddAcionListener(ActionListener* listener);
	void RemoveActionListener();

  private:
    BusAttachment* bus;
	qcc::String mServicePath;
	qcc::String mEventIfaceName;
	qcc::String mActionIfaceName;

	InterfaceDescription* mEventInterfaceDescription;
	InterfaceDescription* mActionInterfaceDescription;

	std::list<EventInfo> mEvents;
	qcc::Mutex* mEventListMtx;
	std::list<ActionInfo> mActions;
	qcc::Mutex* mActionListMtx;

	ActionListener* mListener;

    void ActionCalling(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);
    void Check_MethodReply(const Message& msg, QStatus status);
    
	void InitEventsInterface();
    void InitActionsInterface();

};

} //namespace services
} //namespace ajn

#endif /* EVENTSANDACTIONS_H_ */
