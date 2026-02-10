#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>

#include <string>
#include <Core/SceneManager.h>
#include <Core/Debug.h>

#include <Utils/MemoryMonitor.h>

int main(int argc, char* args[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	Debug::DebugInit("GameEngineLog.txt");
		
	SceneManager* gsm = new SceneManager();
	if (gsm->Initialize("Game Engine", 1920, 1080) ==  true) {
		gsm->Run();
	} 
	delete gsm;
	ReportLeaks();
	Debug::Shutdown();
	ShutdownMemoryMonitor();
	//std::cout << "\n--- Program Ended. Press Enter to close console ---" << std::endl;
	//std::cin.get();
	return 0;
}