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

	std::fstream file3(FILE, std::ios_base::in);

	file3 >> day;
	file3.ignore(1);
	if (day == st.wDay)
	{
		file3 >> month;
		file3.ignore(1);
		file3 >> year;
		file3.ignore(1);
		file3 >> hours;
		file3.ignore(1);
		file3 >> minute;
		file3.ignore(1);
		file3 >> second;
		time = hours * 60 * 60 + minute * 60 + second;
	}



	file3.close();


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



void Change_Element(std::vector<std::string>& Vec) 
{
	std::string s = *(Vec.end() - 1);
	Vec[0] = s;
	Vec.pop_back();
	std::fstream stream(FILE,std::ios_base::out | std::ios_base::trunc);
	for (const auto& item : Vec)
		stream << item << std::endl;
	stream.close();
}



void Paste_Element(std::vector<std::string>& Vec)
{
	std::string str = *(Vec.end() - 1);
	std::vector<std::string> New_Vector = { str };
	auto it = Vec.begin();
	for (; it != Vec.end() - 1; it++)
		New_Vector.push_back(*it);
	std::fstream stream(FILE, std::ios_base::out | std::ios_base::trunc);
	for (const auto& item : New_Vector)
		stream << item << std::endl;
	stream.close();
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
	minute = (time / 60) % 60;
	hours = minute / 60;

    // Добавление в журнал события
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);
	SYSTEMTIME st;
	GetSystemTime(&st);
	
	std::fstream fout(FILE,std::ios_base::out | std::ios_base::app); //Add new timer's value
	fout << st.wDay << "." << st.wMonth << "." << st.wYear << " ";
	fout << hours << ":" << minute << ":" << second;
	fout.close();

	std::string s;
	std::vector<std::string> Vector;

	std::fstream _file(FILE, std::ios_base::in);		
	while (std::getline(_file, s))					//Read FILE
		Vector.push_back(s);
	_file.close();

	if (day == st.wDay) 
		Change_Element(Vector); //Put last item first
	else
		Paste_Element(Vector); 





    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}