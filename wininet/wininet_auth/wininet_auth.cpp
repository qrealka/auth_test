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
#define FLAGS_ERROR_INIT (FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_NO_UI)
enum class auth_status {
    do_auth,
    do_abort,
    do_nothing
};

auth_status need_auth(const CHttpFile& request) {
    DWORD response_code{};
    if (!request.QueryInfoStatusCode(response_code)) {
        wprintf(L"Cannot get response code %d\n", ::GetLastError());
        return auth_status::do_abort;
    }
    
    DWORD query_flags = 0;
    switch (response_code)
        {
        case HTTP_STATUS_DENIED:
            query_flags = HTTP_QUERY_WWW_AUTHENTICATE;
            break;

        case HTTP_STATUS_PROXY_AUTH_REQ:
            query_flags = HTTP_QUERY_PROXY_AUTHENTICATE;
            break;

        default:
            return auth_status::do_nothing;
        }

    // Enumerate the authentication types.
    for (DWORD index = 0, i = 0; i < 10; ++i)
    {
        CString sheme;
        if (request.QueryInfo(query_flags, sheme, &index)) {
            wprintf(L"Found auth scheme: %s\n", sheme.GetString());
            return auth_status::do_auth;
        }
    }
    
    wprintf(L"Auth scheme not found\n");
    return auth_status::do_abort;
}

void dump_reponse(CHttpFile& request) {
     CString content_length;
     if(!request.QueryInfo(HTTP_QUERY_CONTENT_LENGTH, content_length)) {
         wprintf(L"Empty response\n");
         return;
     }

     const UINT size = atoi(CW2A(content_length));
     if (size == 0) {
         wprintf(L"Content-Length is invalid\n");
         return;
     }

     std::string buffer;
     buffer.resize(size + 1, 0);
     const auto readed = request.Read(&buffer[0], size);
     if (readed > 0) {
         wprintf(L"%hs\n", buffer.c_str());
     }
}

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

        const DWORD flags = service_type == AFX_INET_SERVICE_HTTPS 
            ? INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE
            : INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD;
        std::unique_ptr<CHttpFile> request(
            connection->OpenRequest(
                CHttpConnection::HTTP_VERB_GET, query, 
                nullptr, 1, nullptr, nullptr, 
                flags
            )
        );

        if (!request) {
            wprintf(L"Cannot create request %s\n", query);
            return;
        }
        
        DWORD status = ERROR_INTERNET_FORCE_RETRY;
        for(DWORD ui_flags = FLAGS_ERROR_INIT; status == ERROR_INTERNET_FORCE_RETRY;) 
        {
            if (!request->SendRequest()) {
                wprintf(L"Cannot send request %s\n", query);
                return;
            }
            const auto last_error = ::GetLastError();

            switch (need_auth(*request)) {
                case auth_status::do_auth: {
                    status = request->ErrorDlg(nullptr, last_error, ui_flags);
                    if (ui_flags & FLAGS_ERROR_UI_FLAGS_NO_UI) {
                        status = ERROR_INTERNET_FORCE_RETRY;
                    }
                    ui_flags ^= FLAGS_ERROR_UI_FLAGS_NO_UI;
                    break;
                }
                case auth_status::do_nothing: {
                    wprintf(L"It works!\n");
                    dump_reponse(*request);
                    return;
                }
                case auth_status::do_abort: 
                default: 
                    wprintf(L"Does not work!\n");
                    return;
            }
        }
        
        wprintf(L"Possible auth error: %d\n", status);
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
