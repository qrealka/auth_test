// sharepoint_dir.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#import <winhttp.dll> named_guids

int __cdecl wmain(int argc, WCHAR **argv){
    if (argc < 2)
        return wprintf(L"sharepoint_dir <url>\n");

    CoInitialize(nullptr );

    VARIANT varFalse;
    VariantInit(&varFalse);
    V_VT(&varFalse) = VT_BOOL;
    V_BOOL(&varFalse) = VARIANT_FALSE;

    try {
        WinHttp::IWinHttpRequestPtr request;
        HRESULT last_error = request.CreateInstance(WinHttp::CLSID_WinHttpRequest);
        if (FAILED(last_error)) {
            _com_error error = last_error;
            wprintf(L"Cannot create WinHttpRequest: %s\n", error.ErrorMessage());
            return -1;
        }

        last_error = request->SetAutoLogonPolicy(WinHttp::AutoLogonPolicy_Always);
        if (FAILED(last_error)) {
            _com_error error = last_error;
            wprintf(L"Cannot set auto logon policy: %s\n", error.ErrorMessage());
            return -1;
        }

        last_error = request->Open("GET", argv[1], varFalse);
        if (FAILED(last_error)) {
            _com_error error = last_error;
            wprintf(L"Cannot initialize request: %s\n", error.ErrorMessage());
            return -1;
        }

        last_error = request->Send();
        if (FAILED(last_error)) {
            _com_error error = last_error;
            wprintf(L"Send request error: %s\n", error.ErrorMessage());
            return -1;
        }

        const long response_status = request->GetStatus();
        if (response_status == 200) {
            wprintf(L"It works!\n");
        } else {
            wprintf(L"Response code: %d. '%s'\n", response_status, std::wstring(request->GetStatusText()).c_str());
        }
    } catch (const _com_error& e) {
        wprintf(L"Unexpected error: %s", e.ErrorMessage());
    }

    CoUninitialize();
    return 0;
}

