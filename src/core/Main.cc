#include "Bridge.h"
#include "common/AJInitializer.h"
#include "utils/Log.h"
#include "utils/FileUtil.h"

#include <signal.h>

using namespace ajn;
using namespace services;

#define TAG "Main"

static volatile sig_atomic_t s_interrupt = false;
static volatile sig_atomic_t s_restart = false;
	
static Bridge* brg = NULL;

FILE* gLogFile = NULL;
qcc::Mutex* gLogFileMtx;

static void CDECL_CALL SigIntHandler(int sig)
{
    QCC_UNUSED(sig);
    s_interrupt = true;
}

static void CleanUp()
{
	if (brg) {
		delete brg;
		brg = NULL;
	}
}

int main(int argc, char**argv, char**envArg) 
{
    QCC_UNUSED(envArg);
	
	gLogFile = fopen(LOG_FILE_PATH, "w");

	gLogFileMtx = new qcc::Mutex();

	// Initialize AllJoyn
    AJInitializer ajInit;
    if (ajInit.Initialize() != ER_OK) {
        return 1;
    }

	QStatus status = ER_OK;
    
	LOGD(TAG, "AllJoyn Library version: %s", ajn::GetVersion());
    LOGD(TAG, "AllJoyn Library build info: %s", ajn::GetBuildInfo());
    //QCC_SetLogLevels("ALLJOYN_ABOUT_SERVICE=7;");
    //QCC_SetLogLevels("ALLJOYN_ABOUT_ICON_SERVICE=7;");

    /* Install SIGINT handler so Ctrl + C deallocates memory properly */
    signal(SIGINT, SigIntHandler);

start:

	LOGD(TAG, "Bridge creating...");
	
	brg = new Bridge();
	do {
		status = brg->Init();
		if (status != ER_OK) {
			LOGD(TAG, "brg create failed");
			sleep(1);
		}
	} while(status != ER_OK && s_interrupt == false);

	if (status == ER_OK) {
		if (brg->GetNetworkState() == CTRL_STATION)
		{
			LOGD(TAG, "Current station mode.");

			status = brg->InitControllers();
			if (status != ER_OK) {
				LOGD(TAG, "brg InitControllers failed");
			}
		}
		else {
			LOGD(TAG, "Current not station mode.");
		}

		LOGD(TAG, "Bridge creating done.");
	}

    while (s_interrupt == false && brg->IsDaemonDisconnected() == false) {
		usleep(100 * 1000);
	}

	s_restart = brg->IsDaemonDisconnected();

	LOGD(TAG, "Bridge exiting~(s_interrupt %d, IsDaemonDisconnected: %d, s_restart: %d", s_interrupt, brg->IsDaemonDisconnected(), s_restart);

	CleanUp();

	// need restart
	if (s_interrupt == false && s_restart == true) {
		s_restart = false;
		sleep(1);
		goto start;
	}

	gLogFileMtx->Lock();

	if (gLogFile) {
		fclose(gLogFile);
		gLogFile = NULL;
	}
	gLogFileMtx->Unlock();

	delete gLogFileMtx;

	return 0;
}

