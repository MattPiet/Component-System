#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>

#include <string>
#include <Core/SceneManager.h>
#include <Core/Debug.h>

#include <Utils/MemoryMonitor.h>

int main(int argc, char* args[]) {
	Debug::DebugInit("GameEngineLog.txt");
	
	SceneManager* gsm = new SceneManager();
	if (gsm->Initialize("Game Engine", 1280, 720) ==  true) {
		gsm->Run();
	} 
	delete gsm;
	_CrtDumpMemoryLeaks();
	ReportLeaks();
	std::cout << "\n--- Program Ended. Press Enter to close console ---" << std::endl;
	std::cin.get();
	exit(0);
}