#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <wchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")
using namespace std;

char SN[] = { "MyService" };

SERVICE_STATUS Svstatus;
SERVICE_STATUS_HANDLE hStatus;
HANDLE handle = NULL;



void InstallService(void);							//������� ���������
void ServiceMain(DWORD, LPSTR*);					//������ ������� ����������� � ��������� �������� 
void SvcCtrlHandle(DWORD);							//������������ ����������� ��� ������� �������� � ������

void ServiceReportEvent(LPTSTR);					//������� ��� ������ ������� � ������ 
void ReportServiceStatus(DWORD, DWORD, DWORD);		//������ ������� ��������� ������ � �������� � ��� � SCM.
void ServiceInit();									//������ ��� ������
void DoDeleteService();

void __cdecl  _tmain(int argc, TCHAR* argv[])
{

	cout << "Start programm" << endl;
	if (lstrcmp(argv[1], "install") == 0)
	{
		InstallService();
		return;
	}
	else if (lstrcmp(argv[1], "delete") == 0)
	{
		DoDeleteService();
		return;
	}


	cout << "Dispatch" << endl;
	SERVICE_TABLE_ENTRY dispatch[] =
	{
		{SN,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{ NULL , NULL }
	};

	if (!StartServiceCtrlDispatcher(dispatch))
	{

		ServiceReportEvent((LPTSTR)"Cannot start service! ");
	}





}



void DoDeleteService()
{
	cout << "Start delete service" << endl;
	SC_HANDLE scSCManager;
	SC_HANDLE scService;


	scSCManager = OpenSCManager
	(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS
	);

	if (scSCManager == NULL)
	{
		cout << "Cannot delete service: " << GetLastError() << endl;
		return;
	}
	scService = OpenService
	(
		scSCManager,
		SN,
		DELETE
	);

	if (scService == NULL)
	{
		cout << "Cannot delete service: " << GetLastError();
		CloseServiceHandle(scSCManager);
		return;
	}


	if (!DeleteService(scService) == NULL)
	{
		cout << "Cannot delete service: " << GetLastError();

		return;
	}
	else
	{
		cout << "Service delete succesful" << endl;
	}


	CloseServiceHandle(scService);
	CloseServiceHandle(scSCManager);


}




//������������� ������ � ���� ������
void InstallService(void)
{
	cout << "Start install service" << endl;
	SC_HANDLE scCSManager;
	SC_HANDLE sshService;
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileNameA(NULL, szPath, MAX_PATH))
	{
		cout << "Cannot install service: " << GetLastError() << endl;

		return;
	}





	//�������� ����������
	cout << "Get the descriptor" << endl;
	scCSManager = OpenSCManager
	(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS
	);


	//��������, ��� �� ������� ���������
	if (scCSManager == NULL)
	{
		cout << "Cannot get descriptor: " << GetLastError() << endl;
		return;
	}

	//��������� ���������� � ���� ������ � ������
	cout << "Add descriptor in database" << endl;
	sshService = CreateService
	(
		scCSManager,
		SN,
		SN,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);
	if (sshService == NULL)
	{
		cout << "Cannot create service :" << GetLastError();
		CloseServiceHandle(scCSManager);
		return;

	}
	else
		cout << "Service: " << SN << " succesful created" << endl;



	CloseServiceHandle(scCSManager);
	CloseServiceHandle(sshService);



}







//����������� �������-����������� ��� ������
//����� ����� � ������ 
void ServiceMain(DWORD argc, LPTSTR* argv)
{
	//argc - ���������� ����������
	//argv - ������ �����. ������ ������-��� ���
	// ������ � ����������� ������, ������� ���������� ���������
	// ������� ������ ������� StartService ��� ������� ������.

	hStatus = RegisterServiceCtrlHandler
	(
		SN,
		(LPHANDLER_FUNCTION)SvcCtrlHandle);

	if (!hStatus)
	{

		cout << "Cannot register service! " << endl;
		ServiceReportEvent((LPTSTR)"Register Service Handler");
		return;
	}

	Svstatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	Svstatus.dwServiceSpecificExitCode = 0;


	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);


	ServiceInit();

}


void ReportServiceStatus(DWORD CurrentState, DWORD ExitCodeWin32, DWORD WaitHint)
{
	static DWORD CheckPoint = 1;				//��� ���������� ��������� � ������ �� ����� ���������� ���������


	Svstatus.dwCurrentState = CurrentState;		//������� ���������
	Svstatus.dwWin32ExitCode = ExitCodeWin32;	//��� ��������� ������
	Svstatus.dwWaitHint = WaitHint;				// ����� �������� ���������� ��������


	if (CurrentState == SERVICE_START_PENDING)
		Svstatus.dwControlsAccepted = 0;
	else
		Svstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if (CurrentState == SERVICE_RUNNING || CurrentState == SERVICE_STOPPED)
		Svstatus.dwCheckPoint = 0;
	else
		Svstatus.dwCheckPoint = CheckPoint++;

	//������ � ��������� ������
	SetServiceStatus(hStatus, &Svstatus);


}












//Func - ��� ��� ������� �� ������� �� ����������
void ServiceReportEvent(LPTSTR Func)
{
	HANDLE hSource;
	LPCTSTR  str[2];
	TCHAR buffer[80];

	hSource = RegisterEventSource(NULL, SN);

	if (hSource != NULL)
	{
		StringCchPrintf(buffer, 80, "%s Failed with %d", Func, GetLastError());

		str[0] = SN;
		str[1] = buffer;

		ReportEvent(
			hSource,
			EVENTLOG_ERROR_TYPE,
			0,
			(DWORD)"Error",
			NULL,
			2,
			0,
			str,
			NULL
		);

		DeregisterEventSource(hSource);

	}


}











void SvcCtrlHandle(DWORD Control)
{
	switch (Control)
	{
	case SERVICE_CONTROL_STOP:
		ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		SetEvent(handle);
		ReportServiceStatus(Svstatus.dwCurrentState, NO_ERROR, 0);
		return;

	case SERVICE_CONTROL_INTERROGATE:
		break;


	default:
		break;
	}



}

void ServiceInit()
{
	handle = CreateEvent			//������� �������
	(
		NULL,
		TRUE,
		FALSE,
		NULL
	);

	if (handle == NULL)
	{
		ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);

	}

	ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);

	while (1)
	{





		WaitForSingleObject(handle, INFINITE);
		ReportServiceStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;

	}









}