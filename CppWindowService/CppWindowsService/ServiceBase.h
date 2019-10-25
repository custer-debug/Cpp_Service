#pragma once
#include <windows.h>


class CServiceBase
{
public:

	/*
	*	После вызова Run (ServiceBase) SCM выдает команду Start, 
	*	что приводит к вызову метода OnStart в службе.
	*	Этот метод блокируется, пока служба не остановится.
	*/
    static BOOL Run(CServiceBase &service);

    CServiceBase(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);

    // Service object destructor. 
    virtual ~CServiceBase(void) ;

    // Stop the service.
    void Stop();

protected:

    virtual void OnStart();

   /*
   *	При реализации в производном классе выполняется,
   *	когда SCM отправляет команду Stop службе. 
   *	Определяет действия, которые необходимо предпринять, 
   *	когда служба перестает работать.
   */
    virtual void OnStop();

    // When implemented in a derived class, executes when a Pause command is 
    // sent to the service by the SCM. Specifies actions to take when a 
    // service pauses.
    virtual void OnPause();

    // When implemented in a derived class, OnContinue runs when a Continue 
    // command is sent to the service by the SCM. Specifies actions to take 
    // when a service resumes normal functioning after being paused.
    virtual void OnContinue();

    // When implemented in a derived class, executes when the system is 
    // shutting down. Specifies what should occur immediately prior to the 
    // system shutting down.
    virtual void OnShutdown();

    // Добавляет событие в SCM
    void SetServiceStatus(DWORD dwCurrentState, 
        DWORD dwWin32ExitCode = NO_ERROR, 
        DWORD dwWaitHint = 0);

    // Сообщает о событии
    void WriteEventLogEntry(PWSTR pszMessage, WORD wType);

    // Сообщает об ошибки
    void WriteErrorLogEntry(PWSTR pszFunction, 
        DWORD dwError = GetLastError());

private:

    // Entry point for the service. It registers the handler function for the 
    // service and starts the service.
    static void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);

    // The function is called by the SCM whenever a control code is sent to 
    // the service.
    static void WINAPI ServiceCtrlHandler(DWORD dwCtrl);

    // Start the service.
    void Start(DWORD dwArgc, PWSTR *pszArgv);
    
    // Pause the service.
    void Pause();

    // Resume the service after being paused.
    void Continue();

    // Execute when the system is shutting down.
    void Shutdown();

    // The singleton service instance.
    static CServiceBase *s_service;

    // The name of the service
    PWSTR m_name;

    // The status of the service
    SERVICE_STATUS m_status;

    // The service status handle
    SERVICE_STATUS_HANDLE m_statusHandle;
};