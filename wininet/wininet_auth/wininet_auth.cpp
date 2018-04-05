// wininet_auth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "wininet_auth.h"

#include <afxinet.h>
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

void download_it(const wchar_t* url) {
    DWORD service_type;
    CString server, query;
    INTERNET_PORT port;

    try {
        if (!AfxParseURL(url, service_type, server, query, port)) {
            wprintf(L"Cannot parse url: %s\n", url);
            return;
        }
    
        CInternetSession internetSession;
        std::unique_ptr<CHttpConnection> connection(internetSession.GetHttpConnection(server.GetString(), port));
        if (!connection) {
            wprintf(L"Cannot open connection to %s:%d\n", server.GetString(), port);
            return;
        }

        std::unique_ptr<CHttpFile> request(connection->OpenRequest(CHttpConnection::HTTP_VERB_GET, query, nullptr, 1, nullptr, nullptr, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_EXISTING_CONNECT));
        if (!request) {
            wprintf(L"Cannot create request %s\n", query);
            return;
        }

        if (!request->SendRequest()) {
            wprintf(L"Cannot send request %s\n", query);
            return;
        }

        DWORD response_code{};
        if (!request->QueryInfoStatusCode(response_code)) {
            wprintf(L"Cannot get response code %d\n", ::GetLastError());
            return;
        }

        if (response_code == 200) {
            wprintf(L"It works!\n");
        } else {
            wprintf(L"Response code: %d\n", response_code);
        }

    } catch (CInternetException& ex) {
        wprintf(L"Internet error(%d)\n", ex.m_dwError);
	} catch(...) {
        wprintf(L"unexpected error\n");
    }
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
            int iNumOfArgs;
            LPWSTR* pArgs = CommandLineToArgvW(GetCommandLine(),&iNumOfArgs);
            if (pArgs == nullptr || iNumOfArgs < 2) {
                wprintf(L"Missing command line parameter. wininet_auth <url>\n");
                return 1;
            }
            download_it(pArgs[1]);
            LocalFree(pArgs);
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
