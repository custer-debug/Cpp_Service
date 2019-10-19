#pragma region Includes
#include "SampleService.h"
#include "ThreadPool.h"
#include <fstream>
#pragma endregion


#define FILE "C:\\MyService.txt"
//#define FILE_DATE "C:\\Date.txt"



//���������� ������������
//������ ������� ���������� �������
CSampleService::CSampleService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;
	time =		0;
	second =	0; 
	minute =	0;
	hours =		0;
	day =		0;
    
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}

//���������� ��������� ����������,�.�. 
//����� � ������� �������� ����� ���������������� ������
CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}






/*
*	������� CSampleService::OnStart ����������� ��� ������� ������
*	� �������� ������� CServiceBase::WriteEventLogEntry ��� ������
*	�������� � ������� � ������. ����� ��� �������� ������� CThreadPool::QueueUserWorkItem
*	��� ���������� �������� ������� ������ (CSampleService::ServiceWorkerThread)
*	� ������� �� ���������� � ������� ������.
*/

void CSampleService::OnStart(DWORD dwArgc, PWSTR* pszArgv)
{
	WriteEventLogEntry(L"CppWindowsService in OnStart",
		EVENTLOG_INFORMATION_TYPE);
	std::fstream file(FILE,std::ios_base::in);
	file >> hours;
	file.ignore(1);
	file >> minute;
	file.ignore(1);
	file >> second;
	file.close();
	time = hours * 360 + minute * 60 + second;
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}






/*
*	����� ��������� �������� ������� �������.
*	�� �������� �� ������� ������ ���� �������.
*/

void CSampleService::ServiceWorkerThread(void)
{
	

	//��� ������� �����������
    while (!m_fStopping)
    {
        // �������� ������� ������� ������������� �����...
		time++;
		::Sleep(1000);  
	}
    // ������ �� ���������

    SetEvent(m_hStoppedEvent);
	

}


/*	
*	������� CSampleService::OnStop ����������� ��� ��������� ������ � ��������
*	������� CServiceBase::WriteEventLogEntry ��� ������ �������� �� ��������� � ������. 
*	����� ����� ��� ����������� ���������� m_fStopping �������� TRUE, ��� ���������������
*	� ���������� ��������� ������ � �������� ���������� �������� ������� ������, � 
*	������� ������������� ������ ������ m_hStoppedEvent.
*/
void CSampleService::OnStop()
{
	second = time % 60;
	minute = time / 60;
	hours = minute / 60;

    // ���������� � ������ �������
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);
	std::fstream file(FILE, std::ios_base::out, std::ios_base::trunc);
	file << hours << ":";
	file << minute << ":";
	file << second << std::endl;
	file.close();

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}