#ifndef __MADCHOOK_HPP__
#define __MADCHOOK_HPP__

#include <string>
#include <windows.h>
class MadCodeHook
{
	HINSTANCE hDll;
	template <typename T> inline void SetFuncPtr(T&ptr,LPCSTR func)
	{
		ptr = (T)GetProcAddress(hDll,func);
	}
	public:
		typedef DWORD (WINAPI *PREMOTE_EXECUTE_ROUTINE)( LPVOID pParams );
		typedef VOID (WINAPI *PIPC_CALLBACK_ROUTINE)(LPCSTR pIpc,PVOID pMessageBuf,DWORD dwMessageLen,PVOID pAnswerBuf,DWORD dwAnswerLen);
		enum { USE_EXTERNAL_DRIVER_FILE = 0x1,
			   SECURE_MEMORY_MAPS = 0x2,
			   DISABLE_CHANGED_CODE_CHECK = 0x3,
			   USE_NEW_IPC_LOGIC = 0x4,
			   SET_INTERNAL_IPC_TIMEOUT = 0x5,
			   VMWARE_INJECTION_MODE = 0x6,
			   DONT_COUNT=0x1, //old name - compatibility
			   NO_SAFE_UNHOOKING=0x1, // new name
			   NO_IMPROVED_SAFE_UNHOOKING=0x40,
			   SAFE_HOOKING=0x20,
			   MIXTURE_MODE=0x2,
			   NO_MIXTURE_MODE=0x10,
			   ALLOW_WINSOCK2_MIXTURE_MODE=0x80,
			   SYSTEM_WIDE_9X=0x4,
			   ACCEPT_UNKNOWN_TARGETS_9X=0x8,
			   ALL_SESSIONS = 0xFFFFFFED,  // apps of all sessions
			   CURRENT_SESSION = 0xFFFFFFEC,  // apps of current session
			   CURRENT_USER = 0xFFFFFFEB, // apps of current user
			   SYSTEM_PROCESSES = 0x10,  // include this flag to include system processes + services
			   CURRENT_PROCESS = 0x08 };  // exclude this flag to exclude injection into yourself
		static const HANDLE STOP_VIRUS;
		MadCodeHook();
		~MadCodeHook();
		bool Init(const std::string&strFileName="madCHook.dll");

		BOOL WINAPI (*SetMadCHookOption)(DWORD option,LPCWSTR param);
		BOOL WINAPI (*InstallMadCHook)();
		BOOL WINAPI (*UninstallMadCHook)();
		BOOL WINAPI (*HookCode)(PVOID pCode,PVOID pCallbackFunc,PVOID *pNextHook,DWORD dwFlags/* = 0*/);
		BOOL WINAPI (*HookAPI)(LPCSTR pszModule,LPCSTR pszFuncName,PVOID pCallbackFunc,PVOID *pNextHook,DWORD  dwFlags/* = 0*/);
		BOOL WINAPI (*RenewHook)(PVOID *pNextHook);
		DWORD WINAPI (*IsHookInUse)(PVOID *pNextHook);
		BOOL WINAPI (*UnhookCode)(PVOID *pNextHook );
		BOOL WINAPI (*UnhookAPI)(PVOID *pNextHook );
		VOID WINAPI (*CollectHooks)();
		VOID WINAPI (*FlushHooks)();
		BOOL WINAPI (*CreateProcessExA)(LPCSTR lpApplicationName,LPSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,
									 LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,
									 LPVOID lpEnvironment,LPCSTR lpCurrentDirectory,LPSTARTUPINFOA lpStartupInfo,
									 LPPROCESS_INFORMATION lpProcessInformation,LPCSTR lpLoadLibrary);
		BOOL WINAPI (*CreateProcessExW)(LPCWSTR lpApplicationName,LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,
									 LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,
									 LPVOID lpEnvironment,LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo,
									 LPPROCESS_INFORMATION lpProcessInformation,LPCWSTR lpLoadLibrary);
		PVOID WINAPI (*AllocMemEx)(DWORD dwSize,HANDLE hProcess/* = 0*/);
		BOOL WINAPI (*FreeMemEx)(PVOID pMem,HANDLE hProcess/* = 0*/);
		PVOID WINAPI (*CopyFunction)(PVOID pFunction,HANDLE hProcess/* = 0*/,BOOL bAcceptUnknownTargets/* = FALSE*/,PVOID *pBuffer/* = NULL*/);
		HANDLE WINAPI (*CreateRemoteThreadEx)(HANDLE hProcess,LPSECURITY_ATTRIBUTES lpThreadAttributes,DWORD dwStackSize,
										   LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
		BOOL WINAPI (*RemoteExecute)(HANDLE hProcess,PREMOTE_EXECUTE_ROUTINE pFunc,DWORD*dwFuncResult,PVOID pParams/* = NULL*/,DWORD dwSize/* = 0*/);
		BOOL WINAPI (*InjectLibraryA)(DWORD dwProcessHandleOrSpecialFlags,LPCSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*InjectLibraryW)(DWORD dwProcessHandleOrSpecialFlags,LPCWSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*InjectLibrarySessionA)(DWORD dwSession,BOOL bSystemProcesses,LPCSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*InjectLibrarySessionW)(DWORD dwSession,BOOL bSystemProcesses,LPCWSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*UninjectLibraryA)(DWORD dwProcessHandleOrSpecialFlags,LPCSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*UninjectLibraryW)(DWORD dwProcessHandleOrSpecialFlags,LPCWSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*UninjectLibrarySessionA)(DWORD dwSession,BOOL bSystemProcesses,LPCSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		BOOL WINAPI (*UninjectLibrarySessionW)(DWORD dwSession,BOOL bSystemProcesses,LPCWSTR pLibFileName,DWORD dwTimeOut/* = 7000*/);
		DWORD WINAPI (*ProcessHandleToId)(HANDLE dwProcessHandle);
		DWORD WINAPI (*ThreadHandleToId)(HANDLE dwThreadHandle);
		BOOL WINAPI (*ProcessIdToFileName)(DWORD dwProcessId,LPSTR pFileName);
		BOOL WINAPI (*AmSystemProcess)(VOID);
		BOOL WINAPI (*AmUsingInputDesktop)(VOID);
		DWORD WINAPI (*GetCurrentSessionId)(VOID);
		DWORD WINAPI (*GetInputSessionId)(VOID);
		HMODULE WINAPI (*GetCallingModule)(VOID);
		HANDLE WINAPI (*CreateGlobalMutex)(LPCSTR pName);
		HANDLE WINAPI (*OpenGlobalMutex)(LPCSTR pName);
		HANDLE WINAPI (*CreateGlobalEvent)(LPCSTR pName,BOOL bManual,BOOL bInitialState);
		HANDLE WINAPI (*OpenGlobalEvent)(LPCSTR pName);
		HANDLE WINAPI (*CreateGlobalFileMapping)(LPCSTR pName,DWORD dwSize);
		HANDLE WINAPI (*OpenGlobalFileMapping)(LPCSTR  pName,BOOL bWrite);
		VOID WINAPI (*AnsiToWide)(LPCSTR pAnsi,LPWSTR pWide);
		VOID WINAPI (*WideToAnsi)(LPCWSTR pWide,LPSTR pAnsi);
		BOOL WINAPI (*CreateIpcQueueEx)(LPCSTR  pIpc,PIPC_CALLBACK_ROUTINE pCallback,DWORD dwMaxThreadCount/* = 16*/,DWORD dwMaxQueueLen/* = 0x1000*/);
		BOOL WINAPI (*CreateIpcQueue)(LPCSTR pIpc,PIPC_CALLBACK_ROUTINE pCallback);
		BOOL WINAPI (*SendIpcMessage)(LPCSTR pIpc,PVOID pMessageBuf,DWORD dwMessageLen,PVOID pAnswerBuf/* = NULL*/,DWORD dwAnswerLen/* = 0*/,DWORD dwAnswerTimeOut/* = INFINITE*/,BOOL bHandleMessage/* = TRUE*/);
		BOOL WINAPI (*DestroyIpcQueue)(LPCSTR pIpc);
		BOOL WINAPI (*AddAccessForEveryone)(HANDLE hProcessOrService,DWORD dwAccess);
};

#endif
