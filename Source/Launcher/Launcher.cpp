#include "Launcher_PCH.h"
#include "HailEngine.h"
#include "StartUpAttributes.h"
#include "Game.h"
#include "Utility/FilePath.hpp"
#include "Utility/InOutStream.h"

#include <Windows.h>

#include <string>
#include <stringapiset.h>

#include "Threading.h"
#include <signal.h>

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <strsafe.h>
#include <minidumpapiset.h>
#include <fstream>
// Comment out below define to disable command line
#ifndef NDEBUG
#define USE_CONSOLE_COMMAND
#endif

Hail::ErrorManager* g_pErrorManager;

void InitConsole()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#pragma warning( pop )
}

void CloseConsole()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
#pragma warning( pop )
}

void PrintStackTrace(Hail::StringL& log, CONTEXT* context)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    // Initialize symbol handler
    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 stack = {};
    DWORD machineType;

#ifdef _M_IX86
    log += "Unsupported platform _M_IX86. \n";
    return;
#elif _M_X64
    machineType = IMAGE_FILE_MACHINE_AMD64;
    stack.AddrPC.Offset = context->Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context->Rsp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context->Rsp;
    stack.AddrStack.Mode = AddrModeFlat;
#else
    log += "Unsupported platform. \n";
    return;
#endif
    log += "Stacktrace: \n";
    const int maxFrames = 64;
    for (int frame = 0; frame < maxFrames; ++frame)
    {
        if (!StackWalk64(
            machineType,
            process,
            thread,
            &stack,
            context,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL))
        {
            break;
        }

        if (stack.AddrPC.Offset == 0)
            break;

        // Get symbol
        BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolBuffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(process, stack.AddrPC.Offset, &displacement, symbol))
        {
            // TODO: Format for hex:
            log += Hail::StringL::Format("%i: %s + 0x %ul", frame, symbol->Name, displacement);
        }
        else
        {
            log += Hail::StringL::Format("%i: Unknown function", frame);
        }

        // Get file/line info
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;

        if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &lineDisplacement, &line))
        {
            log += Hail::StringL::Format(" in %s : %u ", line.FileName, line.LineNumber);
        }

        log += "\n";
    }
    SymCleanup(process);
}

LONG WINAPI HailWindowsUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
    using namespace Hail;
    StringL crashDumpInfo;
    if (pExceptionInfo)
    {
        // TODO: Format ul to uint64
        crashDumpInfo += StringL::Format("Exception Code: %ul \n", pExceptionInfo->ExceptionRecord->ExceptionCode);
        crashDumpInfo += StringL::Format("Exception Address: %ul \n", pExceptionInfo->ExceptionRecord->ExceptionAddress);
    }

    WCHAR* szAppName = L"CloudBuild";
    WCHAR* szVersion = L"v1.0";
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);

    const FilePath& userDirectory = FilePath::GetUserProjectDirectory();
    const StringLW dmpFileName = StringLW::Format(L"%s%s%s-%04d%02d%02d-%02d%02d%02d-%d-%d.dmp", userDirectory.Data(), szAppName, szVersion,
        stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
        GetCurrentProcessId(), GetCurrentThreadId());

    StringL dmpFilePath;
    dmpFilePath.Reserve(dmpFileName.Length());
    FromWCharToConstChar(dmpFileName.Data(), dmpFilePath.Data(), dmpFileName.Length());
    HANDLE hFile = CreateFileA(dmpFilePath.Data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pExceptionInfo;
        mdei.ClientPointers = FALSE;

        MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpNormal,
            &mdei,
            nullptr,
            nullptr
        );

        crashDumpInfo += "Minidump created:";
        crashDumpInfo += dmpFilePath;
        crashDumpInfo += "\n\n............\n\n";
        CloseHandle(hFile);
    }
    else
    {
        crashDumpInfo += "Failed to create minidump:";
        crashDumpInfo += dmpFilePath;
    }

    const StringLW crashLogFileName = StringLW::Format(L"%s%s%s-%04d%02d%02d-%02d%02d%02d-%d-%d.txt", userDirectory.Data(), "CrashDumpInfo", szVersion,
        stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
        GetCurrentProcessId(), GetCurrentThreadId());

    if (pExceptionInfo)
        PrintStackTrace(crashDumpInfo, pExceptionInfo->ContextRecord);

    Hail::InOutStream inOutStream;
    inOutStream.OpenFile(crashLogFileName.Data(), Hail::FILE_OPEN_TYPE::WRITE, false);
    inOutStream.Write(crashDumpInfo.Data(), sizeof(char), crashDumpInfo.Length());
    inOutStream.CloseFile();

    g_pErrorManager->DumpErrorLog();

    return EXCEPTION_EXECUTE_HANDLER;
}

static Hail::BinarySemaphore unhandledExceptionSemaphore;
static LONG WINAPI unhandledException(EXCEPTION_POINTERS* excpInfo = NULL)
{
    unhandledExceptionSemaphore.Signal();
    if (!excpInfo == NULL)
    {
        __try // Generate exception to get proper context in dump
        {
            RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
        }
        __except (HailWindowsUnhandledExceptionFilter(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
    else
    {
        HailWindowsUnhandledExceptionFilter(excpInfo);
    }
    unhandledExceptionSemaphore.Wait();
    return 0;
}

static void invalidParameter(const wchar_t* expr, const wchar_t* func,
    const wchar_t* file, unsigned int line, uintptr_t reserved)
{
    unhandledException();
}

static void pureVirtualCall()
{
    unhandledException();
}

static void sigAbortHandler(int sig)
{
    // this is required, otherwise if there is another thread
    // simultaneously tries to abort process will be terminated
    signal(SIGABRT, sigAbortHandler);
    unhandledException();
}


static void SetExceptionHandlers()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(unhandledException);
    _set_invalid_parameter_handler(invalidParameter);
    _set_purecall_handler(pureVirtualCall);
    signal(SIGABRT, sigAbortHandler);
    _set_abort_behavior(0, 0);
}

namespace Hail
{
	struct ApplicationFrameData;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, char*, int /*nShowCmd*/)
{
#ifdef USE_CONSOLE_COMMAND
	InitConsole();
#endif

    SetExceptionHandlers();
    g_pErrorManager = new Hail::ErrorManager();
	Hail::GameApplication* game = new Hail::GameApplication();
    {
        Hail::StartupAttributes startData;
        startData.initFunctionToCall = [game](void* initData) { game->Init(initData); };
        startData.postInitFunctionToCall = [game]() { game->PostInit(); };
        startData.shutdownFunctionToCall = [game]() { game->Shutdown(); };
        startData.updateFunctionToCall = [game](double totalTime, float dt, Hail::ApplicationFrameData& frameData) { game->Update(totalTime, dt, frameData); };
        startData.m_pErrorManager = g_pErrorManager;;
        if (Hail::InitEngine(startData))
        {
            Hail::StartEngine();
        }
        else
        {
            startData.m_pErrorManager->DumpErrorLog();
        }
    }
    SAFEDELETE(game);
    SAFEDELETE(g_pErrorManager);
    Hail::CleanupEngineSystems();
#ifdef USE_CONSOLE_COMMAND
	CloseConsole();
#endif
	return 0;
}



