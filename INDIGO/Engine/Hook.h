#pragma once
#include "Engine.h"

//not needed but just in case
using GetBool_t = bool(__thiscall*)(void*);

namespace Engine {
	namespace Hook {
		bool Initialize();
		int Shutdown();
	}
}