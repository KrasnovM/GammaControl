#include <iostream>
#include <vector>
#include <algorithm>
#include <windows.h>
#pragma comment(lib, "user32.lib")  

//#include <comdef.h>
//#include <string>

//suffering to find display hDC
//HWND hwnd = GetDesktopWindow();
//HWND hwnd1 = GetTopWindow(hwnd);
//HDC hDC = GetDC(hwnd1);
//std::vector<HDC> dc_list;
//BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data) {
//    dc_list.push_back(hdc);
//    std::cout << hdc << " hdc enum \n";
//    return TRUE;
//}

class Monitor {
public:
    std::string Name;
    HDC hDC;
};

void PrintDevice(const DISPLAY_DEVICE& dd, size_t nSpaceCount)
{
    std::wcout << "Device Name: " << dd.DeviceName << "\n";
    std::wcout << "Device String: " << dd.DeviceString << "\n";
    std::wcout << "State Flags: " << dd.StateFlags << "\n";
    std::wcout << "DeviceID: " << dd.DeviceID << "\n";
    std::wcout << "DeviceKey: " << dd.DeviceKey + 42 << "\n\n";

    std::wstring ws(dd.DeviceID);
    std::string str(ws.begin(), ws.end());
    str = str.substr(str.find('\\') + 1);
    str = str.substr(0, str.find('\\'));
    std::cout << str;
}

std::vector<Monitor> GetDisplayDevices()
{
    std::vector<Monitor> MonitorVector;
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);
    DWORD deviceNum = 0;
    while (EnumDisplayDevices(NULL, deviceNum, &dd, 0)) { //all connections of inegrated and nvidia 
        //PrintDevice(dd, 0);
        DISPLAY_DEVICE newdd = { 0 };
        newdd.cb = sizeof(DISPLAY_DEVICE);
        DWORD monitorNum = 0;
        while (EnumDisplayDevices(dd.DeviceName, monitorNum, &newdd, 0)) //actual monitors
        {
            //PrintDevice(newdd, 4);

            //Monitor hDC
            HDC hDC = CreateDC(NULL, dd.DeviceName, NULL, NULL);

            //Monitor name
            std::wstring ws(dd.DeviceID);
            std::string monName(ws.begin(), ws.end());
            monName = monName.substr(monName.find('\\') + 1);
            monName = monName.substr(0, monName.find('\\'));

            Monitor monitor;
            monitor.Name = monName;
            monitor.hDC = hDC;

            //Add to vector
            MonitorVector.push_back(monitor);
            monitorNum++;
        }
        puts("");
        deviceNum++;
    }
    return MonitorVector;
}

void CheckGamma(HDC display_dc, WORD oldGamma[3][256], WORD newGamma[3][256])
{
    SetDeviceGammaRamp(display_dc, newGamma);
    Sleep(1000);
    SetDeviceGammaRamp(display_dc, oldGamma);
}

void CreateGammaRamp(float rGamma, float gGamma, float bGamma, float rContrast, float gContrast, float bContrast, float rBright, float gBright, float bBright, HDC display_dc)
{
    //Gamma check
    const float MaxGamma = 4.4f;
    const float MinGamma = 0.3f;
    rGamma = std::clamp(rGamma, MinGamma, MaxGamma);
    gGamma = std::clamp(gGamma, MinGamma, MaxGamma);
    bGamma = std::clamp(bGamma, MinGamma, MaxGamma);

    //Contrast check
    const float MaxContrast = 10.0f;
    const float MinContrast = 0.1f;
    rContrast = std::clamp(rContrast, MinContrast, MaxContrast);
    gContrast = std::clamp(gContrast, MinContrast, MaxContrast);
    bContrast = std::clamp(bContrast, MinContrast, MaxContrast);

    //Brightness check
    const float MaxBright = 1.5f;
    const float MinBright = -1.5f;
    rBright = std::clamp(rBright, MinBright, MaxBright);
    gBright = std::clamp(gBright, MinBright, MaxBright);
    bBright = std::clamp(bBright, MinBright, MaxBright);

    //Auxiliary parameters
    double rInvgamma = 1 / rGamma;
    double gInvgamma = 1 / gGamma;
    double bInvgamma = 1 / bGamma;
    double rNorm = std::pow(255.0, rInvgamma - 1);
    double gNorm = std::pow(255.0, gInvgamma - 1);
    double bNorm = std::pow(255.0, bInvgamma - 1);

    //Store previous settings 
    WORD oldGamma[3][256];
    GetDeviceGammaRamp(display_dc, oldGamma);

    //Create new gamma ramp
    WORD newGamma[3][256];
    for (int i = 0; i < 256; i++)
    {
        double rVal = i * rContrast - (rContrast - 1) * 127;
        double gVal = i * gContrast - (gContrast - 1) * 127;
        double bVal = i * bContrast - (bContrast - 1) * 127;

        if (rGamma != 1) rVal = std::pow(rVal, rInvgamma) / rNorm;
        if (gGamma != 1) gVal = std::pow(gVal, gInvgamma) / gNorm;
        if (bGamma != 1) bVal = std::pow(bVal, bInvgamma) / bNorm;

        rVal += rBright * 128;
        gVal += gBright * 128;
        bVal += bBright * 128;

        newGamma[0][i] = (WORD)std::clamp((int)(rVal * 256), 0, 65535); // r
        newGamma[1][i] = (WORD)std::clamp((int)(gVal * 256), 0, 65535); // g
        newGamma[2][i] = (WORD)std::clamp((int)(bVal * 256), 0, 65535); // b
    }

    CheckGamma(display_dc, oldGamma, newGamma);

    //SetDeviceGammaRamp(display_dc, newGamma);
}


int main()
{
    //fear the old_data

    ////my method to print displays
    //DISPLAY_DEVICE display = DISPLAY_DEVICE();
    //display.cb = sizeof(display.DeviceName);
    //DISPLAY_DEVICE* pdisplay = &display;
    //for (int i = 0; i < 10; i++) {
    //    std::wcout << i << "\n";
    //    EnumDisplayDevices(NULL, i, pdisplay, DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
    //    std::wcout << display.DeviceName << "\n";
    //}
    ////old method to find display hDc
    //std::cout << display.DeviceName << "\n";
    //std::wcout << display.DeviceName << "\n";
    //display.DeviceName == L"\\\\.\\DISPLAY1";
    //EnumDisplayDevices(NULL, 0, pdisplay, DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
    //HDC display_dc = CreateDC(NULL, display.DeviceName, NULL, NULL);
    //// old and flawed method to create gamma array 
    //WORD newGamma[3][256];
    //for (int i = 0; i < 256; i++)
    //{
    //    int val = i * (wBrightness + 128); // wBrightness 0-256
    //    if (val > 65535)
    //        val = 65535;
    //    newGamma[0][i] = (WORD)val; // r
    //    newGamma[1][i] = (WORD)val; // g
    //    newGamma[2][i] = (WORD)val; // b
    //}
    ////more competent method to create gamma array
    //USHORT gammaTable[768];
    //gamma = std::clamp(gamma, MinValue, MaxValue); //gamma = val
    //double invgamma = 1 / gamma;
    //double norm = std::pow(255.0, invgamma - 1);
    //for (int i = 0; i < 256; i++)
    //{
    //    double val = i * contrast - (contrast - 1) * 127;
    //    if (gamma != 1) val = std::pow(val, invgamma) / norm;
    //    val += bright * 128;
    //    gammaTable[i] = (USHORT)std::clamp((int)(val * 256), 0, 65535); // r
    //    gammaTable[i + 256] = (USHORT)std::clamp((int)(val * 256), 0, 65535); // g
    //    gammaTable[i + 512] = (USHORT)std::clamp((int)(val * 256), 0, 65535); // b
    //}

    //handling displays
    std::vector<Monitor> MonitorVector = GetDisplayDevices();

    //find display hDc
    HDC display_dc = MonitorVector.front().hDC;

    //create gamma ramp
    float rGamma = 1.0f;
    float gGamma = 1.0f;
    float bGamma = 1.0f;

    float rContrast = 1.0f;
    float gContrast = 1.0f;
    float bContrast = 1.0f;

    float rBright = 0.0f;
    float gBright = 0.0f;
    float bBright = 0.0f;

    CreateGammaRamp(rGamma, gGamma, bGamma, rContrast, gContrast, bContrast, rBright, gBright, bBright, display_dc);
}