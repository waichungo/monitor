#include "wmi.h"
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

class COMInitializer
{
public:
    COMInitializer()
    {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to initialize COM library");
        }
    }

    ~COMInitializer()
    {
        CoUninitialize();
    }
};

class WMIInitializer
{
public:
    WMIInitializer()
    {
        HRESULT hr = CoInitializeSecurity(
            nullptr,
            -1,
            nullptr,
            nullptr,
            RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE,
            nullptr);
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to initialize security");
        }
    }
};

class WMIConnection
{
public:
    WMIConnection()
    {
        HRESULT hr = CoCreateInstance(
            CLSID_WbemLocator,
            0,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID *)&pLoc);
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create IWbemLocator object");
        }

        hr = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"),
            nullptr,
            nullptr,
            0,
            NULL,
            0,
            0,
            &pSvc);
        if (FAILED(hr))
        {
            pLoc->Release();
            throw std::runtime_error("Could not connect to WMI namespace");
        }

        hr = CoSetProxyBlanket(
            pSvc,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            nullptr,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            nullptr,
            EOAC_NONE);
        if (FAILED(hr))
        {
            pSvc->Release();
            pLoc->Release();
            throw std::runtime_error("Could not set proxy blanket");
        }
    }

    ~WMIConnection()
    {
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
    }

    IWbemServices *getService() const
    {
        return pSvc;
    }

private:
    IWbemLocator *pLoc = nullptr;
    IWbemServices *pSvc = nullptr;
};

std::vector<std::map<std::string, std::string>> getWmiInfoWithSelector(IWbemServices *pSvc, std::string selector)
{
    std::vector<std::map<std::string, std::string>> processInfoList;
    selector = "SELECT * FROM " + selector;
    IEnumWbemClassObject *pEnumerator = nullptr;
    HRESULT hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(selector.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hr))
    {
        throw std::runtime_error("Query for Win32_Process failed");
    }

    IWbemClassObject *pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        std::map<std::string, std::string> processInfo;

        // Get the property names
        hr = pclsObj->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
        if (FAILED(hr))
        {
            pclsObj->Release();
            throw std::runtime_error("BeginEnumeration failed");
        }

        BSTR strName = nullptr;
        VARIANT vtProp;
        while (pclsObj->Next(0, &strName, &vtProp, 0, 0) == WBEM_S_NO_ERROR)
        {
            _bstr_t bstrName(strName, false);
            if (vtProp.vt == VT_BSTR)
            {
                processInfo[static_cast<const char *>(bstrName)] = _bstr_t(vtProp.bstrVal);
            }
            else if (vtProp.vt == VT_I4)
            {
                processInfo[static_cast<const char *>(bstrName)] = std::to_string(vtProp.lVal);
            }
            else if (vtProp.vt == VT_UI4)
            {
                processInfo[static_cast<const char *>(bstrName)] = std::to_string(vtProp.ulVal);
            }
            else if (vtProp.vt == VT_BOOL)
            {
                processInfo[static_cast<const char *>(bstrName)] = (vtProp.boolVal == VARIANT_TRUE) ? "True" : "False";
            }
            else if (vtProp.vt != VT_NULL)
            {
                processInfo[static_cast<const char *>(bstrName)] = "<unsupported type>";
            }

            VariantClear(&vtProp);
            SysFreeString(strName);
        }

        pclsObj->EndEnumeration();
        pclsObj->Release();
        processInfoList.push_back(processInfo);
    }

    if (pEnumerator)
    {
        pEnumerator->Release();
    }

    return processInfoList;
}

WMIInfo getWmiInformation()
{
    WMIInfo info;
    try
    {
        COMInitializer comInit;
        WMIInitializer wmiInit;
        WMIConnection wmiConn;

        auto os = getWmiInfoWithSelector(wmiConn.getService(), "Win32_OperatingSystem");
        auto cpu = getWmiInfoWithSelector(wmiConn.getService(), "Win32_Processor");
        auto disk = getWmiInfoWithSelector(wmiConn.getService(), "Win32_DiskDrive");
        std::vector<std::map<std::string, std::string>> *curr;

        curr = &os;
        if ((*curr).size() > 0 && (*curr)[0].size() > 0)
        {
            info.osInfo = (*curr)[0];
        }
        curr = &cpu;
        if ((*curr).size() > 0 && (*curr)[0].size() > 0)
        {
            info.cpuInfo = (*curr)[0];
        }
        curr = &disk;
        if ((*curr).size() > 0 && (*curr)[0].size() > 0)
        {
            info.diskInfo = (*curr)[0];
        }

        // for (const auto& processInfo : disk) {
        //     for (const auto& [key, value] : processInfo) {
        //         std::cout << key << ": " << value << std::endl;
        //     }
        //     std::cout << "------------------------" << std::endl;
        // }
    }
    catch (const std::exception &e)
    {
        // std::cerr << "Error: " << e.what() << std::endl;
    }
    return info;
}
