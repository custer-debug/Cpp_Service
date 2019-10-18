#pragma region Includes
#include "SampleService.h"
#include "ThreadPool.h"
#include <fstream>
#pragma endregion


#define FILE "C:\\MyService.txt"


//���������� ������������
//������ ������� ���������� �������
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
	printf("OnStart \n");

    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);


	Sleep(7000); ///���� ���� ��� �������, ���� ���� �� ���������� �������
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}






/*
*	����� ��������� �������� ������� �������.
*	�� �������� �� ������� ������ ���� �������.
*/

void CSampleService::ServiceWorkerThread(void)
{
	std::fstream file(FILE, std::ios_base::out);
	if (!file.is_open())
		SetEvent(m_hStoppedEvent);
	file << "Hello World";
	file.close();


	//��� ������� �����������
    //while (m_fStopping)
    //{
    //    // �������� ������� ������� ������������� �����...
    //    ::Sleep(1000);  // �������� �� 2 �������
    //}
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
	printf("OnStop \n");

    // ���������� � ������ �������
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