#pragma once

#include "ServiceBase.h"


class CSampleService : public CServiceBase
{
public:

    CSampleService(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
    virtual ~CSampleService(void);

protected:

    virtual void OnStart();
    virtual void OnStop();

	void CSampleService::ServiceWorkerThread(void);

private:
	int time;
	int second,minute,hours;
	int day,month,year;
	
	BOOL m_fStopping;
    HANDLE m_hStoppedEvent;
};