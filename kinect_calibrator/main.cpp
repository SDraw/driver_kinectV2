#include "stdafx.h"

#include "CKinectMonitor.h"

#ifdef _DEBUG
int main()
{
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
#endif
    CKinectMonitor *l_monitor = new CKinectMonitor();
    if(l_monitor->Initialize())
    {
        while(l_monitor->DoPulse());
        l_monitor->Terminate();
    }
    delete l_monitor;

    return EXIT_SUCCESS;
}
