#pragma region Includes
#include "SampleService.h"
#include "ThreadPool.h"
#include <fstream>
#include <string>
#include <vector>
#pragma endregion


#define FILE "C:\\Games\\MyService.txt"




//Реализация конструктора
//Создаёт событие дефолтного сервиса
CSampleService::CSampleService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;
	time =		0;
	second = minute = hours =	0; 
	day = month = year		=	0;
    
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
	WriteEventLogEntry(L"CppWindowsService in OnStart",
		EVENTLOG_INFORMATION_TYPE);

	

	
	CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}






/*
*	Метод выполняет основную функцию сервиса.
*	Он работает на рабочем потоке пула потоков.
*/

void CSampleService::ServiceWorkerThread(void)
{

	//Sleep(10000);

	SYSTEMTIME st;
	GetSystemTime(&st);

	std::fstream file(FILE, std::ios_base::in);

	file >> day;
	file.ignore(1);
	if (day == st.wDay)
	{
		file >> month;
		file.ignore(1);
		file >> year;
		file.ignore(1);
		file >> hours;
		file.ignore(1);
		file >> minute;
		file.ignore(1);
		file >> second;
		time = hours * 60 * 60 + minute * 60 + second;
	}



	file.close();


	//Эта функция выполняется
    while (!m_fStopping)
    {
        // Основная функция сервиса располагается здесь...
		time++;
		::Sleep(1000);  
	}
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
	second = time % 60;
	minute = time / 60;
	hours = minute / 60;

    // Добавление в журнал события
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);
	SYSTEMTIME st;
	GetSystemTime(&st);
	
	if (day == st.wDay)
	{
		std::fstream file1(FILE, std::ios_base::in);
		std::string s;
		std::vector<std::string> vec;
		while (getline(file1,s))
		{
			vec.push_back(s);
		}
		file1.close();
		vec.erase(vec.end() - 2);
		s.clear();
		std::fstream file2(FILE, std::ios_base::out | std::ios_base::trunc);
		for (auto& item : vec)
			file2 << item << std::endl; 

		file2 << st.wDay << ".";
		file2 << st.wMonth << ".";
		file2 << st.wYear << " ";
		file2 << hours << ":";
		file2 << minute << ":";
		file2 << second << std::endl;
		
		file2.close();
	}
	else
	{

		std::fstream file(FILE, std::ios_base::out);
		file << st.wDay << ".";
		file << st.wMonth << ".";
		file << st.wYear << " ";
		file << hours << ":";
		file << minute << ":";
		file << second << std::endl;
		file.close();

	}




    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}