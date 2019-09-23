#include "madCHook.hpp"

const HANDLE MadCodeHook::STOP_VIRUS = (HANDLE)0xFFFFFFFA;

MadCodeHook::MadCodeHook()
{
	hDll = NULL;
}
MadCodeHook::~MadCodeHook()
{
	if (hDll) FreeLibrary(hDll);
}

bool MadCodeHook::Init(const std::string&strFileName)
{
	if (hDll) FreeLibrary(hDll);
	hDll = LoadLibrary(strFileName.c_str());
	if (hDll)
	{
		SetFuncPtr(SetMadCHookOption,"SetMadCHookOption");
		SetFuncPtr(InstallMadCHook,"InstallMadCHook");
		SetFuncPtr(UninstallMadCHook,"UninstallMadCHook");
		SetFuncPtr(HookCode,"HookCode");
		SetFuncPtr(HookAPI,"HookAPI");
		SetFuncPtr(RenewHook,"RenewHook");
		SetFuncPtr(IsHookInUse,"IsHookInUse");
		SetFuncPtr(UnhookCode,"UnhookCode");
		SetFuncPtr(UnhookAPI,"UnhookAPI");
		SetFuncPtr(CollectHooks,"CollectHooks");
		SetFuncPtr(FlushHooks,"FlushHooks");
		SetFuncPtr(CreateProcessExA,"CreateProcessExA");
		SetFuncPtr(CreateProcessExW,"CreateProcessExW");
		SetFuncPtr(AllocMemEx,"AllocMemEx");
		SetFuncPtr(FreeMemEx,"FreeMemEx");
		SetFuncPtr(CopyFunction,"CopyFunction");
		SetFuncPtr(CreateRemoteThreadEx,"CreateRemoteThreadEx");
		SetFuncPtr(RemoteExecute,"RemoteExecute");
		SetFuncPtr(InjectLibraryA,"InjectLibraryA");
		SetFuncPtr(InjectLibraryW,"InjectLibraryW");
		SetFuncPtr(InjectLibrarySessionA,"InjectLibrarySessionA");
		SetFuncPtr(InjectLibrarySessionW,"InjectLibrarySessionW");
		SetFuncPtr(UninjectLibraryA,"UninjectLibraryA");
		SetFuncPtr(UninjectLibraryW,"UninjectLibraryW");
		SetFuncPtr(UninjectLibrarySessionA,"UninjectLibrarySessionA");
		SetFuncPtr(UninjectLibrarySessionW,"UninjectLibrarySessionW");
		SetFuncPtr(ProcessHandleToId,"ProcessHandleToId");
		SetFuncPtr(ThreadHandleToId,"ThreadHandleToId");
		SetFuncPtr(ProcessIdToFileName,"ProcessIdToFileName");
		SetFuncPtr(AmSystemProcess,"AmSystemProcess");
		SetFuncPtr(AmUsingInputDesktop,"AmUsingInputDesktop");
		SetFuncPtr(GetCurrentSessionId,"GetCurrentSessionId");
		SetFuncPtr(GetInputSessionId,"GetInputSessionId");
		SetFuncPtr(GetCallingModule,"GetCallingModule");
		SetFuncPtr(CreateGlobalMutex,"CreateGlobalMutex");
		SetFuncPtr(OpenGlobalMutex,"OpenGlobalMutex");
		SetFuncPtr(CreateGlobalEvent,"CreateGlobalEvent");
		SetFuncPtr(OpenGlobalEvent,"OpenGlobalEvent");
		SetFuncPtr(CreateGlobalFileMapping,"CreateGlobalFileMapping");
		SetFuncPtr(OpenGlobalFileMapping,"OpenGlobalFileMapping");
		SetFuncPtr(AnsiToWide,"AnsiToWide");
		SetFuncPtr(WideToAnsi,"WideToAnsi");
		SetFuncPtr(CreateIpcQueueEx,"CreateIpcQueueEx");
		SetFuncPtr(CreateIpcQueue,"CreateIpcQueue");
		SetFuncPtr(SendIpcMessage,"SendIpcMessage");
		SetFuncPtr(DestroyIpcQueue,"DestroyIpcQueue");
		SetFuncPtr(AddAccessForEveryone,"AddAccessForEveryone");
	}
	return (hDll);
}
