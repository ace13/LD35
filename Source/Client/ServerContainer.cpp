#include "ServerContainer.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Dbghelp.h>
#include <cstdio>
#include <thread>
#define READSYM(sym) GetProcAddress(mod, sym)
#pragma comment(lib,"dbghelp.lib")

namespace
{
	void ParseIAT(HINSTANCE h)
	{
		// Find the IAT size
		DWORD ulsize = 0;
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(h, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulsize);
		if (!pImportDesc)
			return;

		// Loop names
		for (; pImportDesc->Name; pImportDesc++)
		{
			PSTR pszModName = (PSTR)((PBYTE)h + pImportDesc->Name);
			if (!pszModName)
				break;

			HINSTANCE hImportDLL = LoadLibraryA(pszModName);
			if (!hImportDLL)
			{
				// ... (error)
			}

			// Get caller's import address table (IAT) for the callee's functions
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
				((PBYTE)h + pImportDesc->FirstThunk);

			// Replace current function address with new function address
			for (; pThunk->u1.Function; pThunk++)
			{
				FARPROC pfnNew = 0;
				size_t rva = 0;
#ifdef _WIN64
				if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
#else
				if (pThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32)
#endif
				{
					// Ordinal
#ifdef _WIN64
					size_t ord = IMAGE_ORDINAL64(pThunk->u1.Ordinal);
#else
					size_t ord = IMAGE_ORDINAL32(pThunk->u1.Ordinal);
#endif

					PROC* ppfn = (PROC*)&pThunk->u1.Function;
					if (!ppfn)
					{
						// ... (error)
					}
					rva = (size_t)pThunk;

					char fe[100] = { 0 };
					sprintf_s(fe, 100, "#%u", ord);
					pfnNew = GetProcAddress(hImportDLL, (LPCSTR)ord);
					if (!pfnNew)
					{
						// ... (error)
					}
				}
				else
				{
					// Get the address of the function address
					PROC* ppfn = (PROC*)&pThunk->u1.Function;
					if (!ppfn)
					{
						// ... (error)
					}
					rva = (size_t)pThunk;
					PSTR fName = (PSTR)h;
					fName += pThunk->u1.Function;
					fName += 2;
					if (!fName)
						break;
					pfnNew = GetProcAddress(hImportDLL, fName);
					if (!pfnNew)
					{
						// ... (error)
					}
				}

				// Patch it now...
				auto hp = GetCurrentProcess();
				if (!WriteProcessMemory(hp, (LPVOID*)rva, &pfnNew, sizeof(pfnNew), NULL) && (ERROR_NOACCESS == GetLastError()))
				{
					DWORD dwOldProtect;
					if (VirtualProtect((LPVOID)rva, sizeof(pfnNew), PAGE_WRITECOPY, &dwOldProtect))
					{
						if (!WriteProcessMemory(GetCurrentProcess(), (LPVOID*)rva, &pfnNew, sizeof(pfnNew), NULL))
						{
							// ... (error)
						}
						if (!VirtualProtect((LPVOID)rva, sizeof(pfnNew), dwOldProtect, &dwOldProtect))
						{
							// ... (error)
						}
					}
				}
			}
		}
	}
}

#else
#include <dlfcn.h>
#define READSYM(sym) dlsym(mServerModule, sym)
#endif

ServerContainer::ServerContainer()
	: mServerState(State_Invalid)
	, mServerInit(nullptr)
	, mServerRun(nullptr)
{

}
ServerContainer::~ServerContainer()
{
	if (mServerState != State_Invalid && mServerModule)
	{
#ifdef _WIN32
		FreeLibrary(reinterpret_cast<HMODULE>(mServerModule));
#else
		dlclose(mServerModule);
#endif
		mServerModule = nullptr;
	}
}

bool ServerContainer::init()
{
	if (mServerState != State_Invalid)
		return mServerModule != nullptr;

#ifdef _WIN32
	mServerModule = LoadLibrary("Server.exe");

	if (!mServerModule)
		return false;

	HMODULE mod = reinterpret_cast<HMODULE>(mServerModule);

	ParseIAT(mod);

	typedef void(*ServerInitCRT)(HANDLE);

	auto* crtInit = (ServerInitCRT)READSYM("e0");
	if (!crtInit)
	{
		FreeLibrary(mod);
		mServerModule = nullptr;

		return false;
	}

	HANDLE hEv = CreateEvent(0, 0, 0, 0);
	std::thread t([&](HANDLE h)
		{
			crtInit(h);
		}, hEv);
	t.detach();

	WaitForSingleObject(hEv, INFINITE);
#else
	mServerModule = dlopen("Server", RTLD_NOW);

	if (!mServerModule)
		return false;
#endif

	mServerInit     =     (ServerInit_f)READSYM("server_init");
	mServerRun      =      (ServerRun_f)READSYM("server_run");
	mServerStop     =     (ServerStop_f)READSYM("server_stop");
	mServerReset    =    (ServerReset_f)READSYM("server_reset");
	mServerRunCmd   =   (ServerRunCmd_f)READSYM("server_cmd");

	mServerSetBProp = (ServerSetBProp_f)READSYM("server_setBProp");
	mServerGetBProp = (ServerGetBProp_f)READSYM("server_getBProp");
	mServerSetIProp = (ServerSetIProp_f)READSYM("server_setIProp");
	mServerGetIProp = (ServerGetIProp_f)READSYM("server_getIProp");
	mServerSetFProp = (ServerSetFProp_f)READSYM("server_setFProp");
	mServerGetFProp = (ServerGetFProp_f)READSYM("server_getFProp");

	if (mServerInit
		&& mServerRun
		&& mServerRunCmd
		&& mServerStop
		&& mServerReset
		&& mServerSetBProp
		&& mServerGetBProp
		&& mServerSetFProp
		&& mServerGetFProp
		&& mServerGetIProp
		&& mServerSetIProp)
		mServerState = State_Stopped;

	if (mServerState == State_Invalid)
	{
#ifdef _WIN32
		FreeLibrary(reinterpret_cast<HMODULE>(mServerModule));
#else
		dlclose(mServerModule);
#endif
		mServerModule = nullptr;
	}

	mServerInit();

	return mServerState != State_Invalid;
}

void ServerContainer::launch()
{
	if (mServerState == State_Stopped)
	{
		mServerRun();
		mServerState = State_Starting;
	}
}

void ServerContainer::stop()
{
	if (mServerState == State_Running)
	{
		mServerStop();
		mServerState = State_Stopping;
	}
}

void ServerContainer::runCmd(const std::string& cmd)
{
	if (mServerState == State_Running)
		mServerRunCmd(cmd.c_str());
}
