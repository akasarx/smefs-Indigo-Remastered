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

//new - test
using DrawModelExecute_t = void(__thiscall*)(IMatRenderContext*, const DrawModelState_t, const ModelRenderInfo_t, matrix3x4_t);

//idk let's give it a go
using PlaySound_t = void(__thiscall*)(const char*);
using GetViewModelFOV_t = float(__thiscall*)(void*);

/* From sprthack/CSGOSimple :)
long __stdcall hkEndScene(IDirect3DDevice9* device);
long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
void __fastcall hkEmitSound1(void* _this, int, IRecipientFilter & filter, int iEntIndex, int iChannel, const char * pSoundEntry, unsigned int nSoundEntryHash, const char * pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector * pOrigin, const Vector * pDirection, void * pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
void __fastcall hkOverrideView(void* _this, int, CViewSetup * vsView);
void __fastcall hkLockCursor(void* _this);
int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
bool __fastcall hkSvCheatsGetBool(void* pConVar, void* edx);*/

namespace Engine {
	namespace Hook {
		bool Initialize();
		int	 Shutdown();
	}
}