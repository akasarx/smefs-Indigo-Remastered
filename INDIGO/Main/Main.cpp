/*
	WARNING: This cheat (like pretty much any other pastes out there) uses Virtual Method Table (VMT) hooking, which is now detected by Valve Anti-Cheat.
	We are NOT responsible for ANY bans that may occur during the process of using this cheat. This includes, but not limited to, VAC, Untrusted and
	Overwatch bans.
*/

#include "Main.h"
//#include "../RL/ReflectiveLoader.h"

//[enc_string_enable /]
//[junk_enable /]
DWORD WINAPI CheatEntry(LPVOID lpThreadParameter) {
	HMODULE hModule = HMODULE(lpThreadParameter);
	if (Engine::Initialize()) {
#if ENABLE_DEBUG_FILE == 1
		CSX::Log::Add("[Engine - initialized!]");
#endif
		return 0;
	}
	else {
#if ENABLE_DEBUG_FILE == 1
		CSX::Log::Add("[Engine - failed to initialize!]\n");
#endif
	}
	return 0;
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
#if ENABLE_DEBUG_CONS == 1
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		DeleteFileA(Client::LogFile.c_str());
		CSX::Log::LogFile = Client::LogFile;
		printf("Client::BaseDir = %s\nClient::LogFile = %s\n", Client::BaseDir.c_str(), Client::LogFile.c_str());
#endif
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CheatEntry, hinstDLL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		if (lpvReserved == nullptr) {
			return Engine::Shutdown();
		}
		FreeLibraryAndExitThread(hinstDLL, true); //Free DLL and exit the thread.
	}
	return TRUE;
}
