#include "stdafx.h"

#include "CKinectMonitor.h"

int main()
{
    std::cout << "[ Kinect calibrator ]" << std::endl;

    CKinectMonitor *l_monitor = new CKinectMonitor();
    if(l_monitor->Initialize())
    {
        while(l_monitor->DoPulse());
        l_monitor->Terminate();
    }
    delete l_monitor;

    return EXIT_SUCCESS;
}
