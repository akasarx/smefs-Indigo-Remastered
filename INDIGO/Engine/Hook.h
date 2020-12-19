#pragma once
#include "Engine.h"

using FrameStageNotify = void(__thiscall*)(IBaseClientDLL*, ClientFrameStage_t);
using CreateMove = void(__thiscall *)(IClientMode*, float flInputSampleTime, CUserCmd* pCmd);
using CHLCreateMove = void(__thiscall*)(IBaseClientDLL*, int, float, bool);
using GameEvent = bool(__thiscall*)(void*, IGameEvent*);
using ResetHook = void(__thiscall *)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
using LockCursor_t = void(__thiscall*)(ISurface*);
using OverrideView_t = void(__thiscall*)(void*, CViewSetup*);
using GetBool_t = bool(__thiscall*)(void*);
using DoPostScreenEffects_t = int(__thiscall*)(IClientMode*, int);

namespace Engine {
	namespace Hook {
		bool Initialize();
		int Shutdown();
	}
}