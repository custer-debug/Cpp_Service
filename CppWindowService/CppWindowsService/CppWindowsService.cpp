#pragma region Includes
#include <stdio.h>
#include <windows.h>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "SampleService.h"
#pragma endregion


// 
// Настройки службы
// 


#define SERVICE_NAME             L"CppWindowsService"					//Имя сервиса

#define SERVICE_DISPLAY_NAME     L"CppWindowsService Sample Service"	//Имя сервиса на дисплее

#define SERVICE_START_TYPE       SERVICE_DEMAND_START					//Оптиция стартового значения севриса

#define SERVICE_DEPENDENCIES     L""

#define SERVICE_ACCOUNT          L"NT AUTHORITY\\LocalService"			//Имя учетной записи, под которой должен работать сервис

#define SERVICE_PASSWORD         NULL									//Пароль сервиса




//Точка входа
int wmain(int argc, wchar_t *argv[])
{
    if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
    {
        if (_wcsicmp(L"install", argv[1] + 1) == 0)
        {
          
            InstallService(
                SERVICE_NAME,               // Name of service
                SERVICE_DISPLAY_NAME,       // Name to display
                SERVICE_START_TYPE,         // Service start type
                SERVICE_DEPENDENCIES,       // Dependencies
                SERVICE_ACCOUNT,            // Service running account
                SERVICE_PASSWORD            // Password of the account
                );
        }
        else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
        {
            // Uninstall the service when the command is 
            // "-remove" or "/remove".
            UninstallService(SERVICE_NAME);
        }
    }
    else
    {
        wprintf(L"Parameters:\n");
        wprintf(L" -install  to install the service.\n");
        wprintf(L" -remove   to remove the service.\n");

        CSampleService service(SERVICE_NAME);
        if (!CServiceBase::Run(service))
        {
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
        }
    }

    return 0;
}