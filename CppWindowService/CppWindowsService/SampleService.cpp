#pragma region Includes
#include "SampleService.h"
#include "ThreadPool.h"
#include <fstream>
#pragma endregion


#define FILE "C:\\MyService.txt"


//Реализация конструктора
//Создаёт событие дефолтного сервиса
CSampleService::CSampleService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = TRUE;

    
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}

//Деструктор закрывает дескриптор,т.е. 
//число с помощью которого можно идентифицировать ресурс
CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


/*
*	Функция CSampleService::OnStart выполняется при запуске службы
*	и вызывает функцию CServiceBase::WriteEventLogEntry для записи
*	сведений о запуске в журнал. Также она вызывает функцию CThreadPool::QueueUserWorkItem
*	для постановки основной функции службы (CSampleService::ServiceWorkerThread)
*	в очередь на выполнение в рабочем потоке.
*/

void CSampleService::OnStart(DWORD dwArgc, PWSTR* pszArgv)
{
	printf("OnStart \n");

    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);


	Sleep(7000); ///Этот слип для отладки, того чтоб не закончился процесс
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}






/*
*	Метод выполняет основную функцию сервиса.
*	Он работает на рабочем потоке пула потоков.
*/

void CSampleService::ServiceWorkerThread(void)
{
	std::fstream file(FILE, std::ios_base::out);
	if (!file.is_open())
		SetEvent(m_hStoppedEvent);
	file << "Hello World";
	file.close();


	//Эта функция выполняется
    //while (m_fStopping)
    //{
    //    // Основная функция сервиса располагается здесь...
    //    ::Sleep(1000);  // Задержка на 2 секунды
    //}
    // Сигнал на остановку

    SetEvent(m_hStoppedEvent);
	

}


/*	
*	Функция CSampleService::OnStop выполняется при остановке службы и вызывает
*	функцию CServiceBase::WriteEventLogEntry для записи сведений об остановке в журнал. 
*	После этого она присваивает переменной m_fStopping значение TRUE, что свидетельствует
*	о выполнении остановки службы и ожидании завершения основной функции службы, о 
*	котором сигнализирует объект службы m_hStoppedEvent.
*/
void CSampleService::OnStop()
{
	printf("OnStop \n");

    // Добавление в журнал события
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}