#include "Hook.h"
#include "intrin.h"

//[enc_string_enable /]
//[junk_enable /]

CAimbot* g_Aim;

namespace Engine {
	namespace Hook {
		/*
			WARNING: This cheat (like pretty much any other pastes out there) uses Virtual Method Table (VMT) hooking, which is now detected by Valve Anti-Cheat.
			We are NOT responsible for ANY bans that may occur during the process of using this cheat. This includes, but not limited to, VAC, Untrusted and
			Overwatch bans.
		*/
		vfunc_hook device; //IDirect3DDevice9Table;
		vfunc_hook surface; //ISurfaceTable
		vfunc_hook client; //IBaseClientTable
		vfunc_hook clientmode; //IClientModeTable;
		vfunc_hook eventmanager; //IGameEventManagerTable
		vfunc_hook sound;
		vfunc_hook modelrender;
		vfunc_hook sv_cheats; //convar lol
		vfunc_hook engine;
		vfunc_hook steam; //steamgamecoordinator
		IDirect3DDevice9* g_pDevice = nullptr;

		typedef HRESULT(WINAPI* Present_t)(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
		Present_t Present_o;

		typedef HRESULT(WINAPI* Reset_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
		Reset_t Reset_o;

		bool ps;
		HRESULT WINAPI Hook_Present(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
#if ENABLE_DEBUG_FILE == 1
			if(!ps) {
				CSX::Log::Add("[Hooked - Present]");
				ps = true;
			}
#endif
			Client::OnRender();
			return Present_o(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
		}

		bool rs;
		HRESULT WINAPI Hook_Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
#if ENABLE_DEBUG_FILE == 1
			if(!rs) {
				CSX::Log::Add("[Hooked - Reset]");
				rs = true;
			}
#endif
			Client::OnLostDevice();
			Client::OnResetDevice();
			return Reset_o(pDevice, pPresentationParameters);
		}

		/* //shouldn't be needed anymore?
		//original iclientmode::createmove
		bool cm;
		void __stdcall Hook_CreateMove(float flInputSampleTime, CUserCmd* pCmd) {
#if ENABLE_DEBUG_FILE == 1
			if (!cm) {
				CSX::Log::Add("[Hooked - CreateMove]");
				cm = true;
			}
#endif
			static auto ofunc = clientmode.get_original<CreateMove>(TABLE::IClientMode::CreateMove);
			ofunc(Interfaces::ClientMode(), flInputSampleTime, pCmd);
			//Client::OnCreateMove(pCmd);
		}*/

		//ibaseclientdll::createmove
		bool chlcm;
		bool pcmdfail;
		void __stdcall Hook_CHLCreateMove(int sequence_number, float input_sample_frametime, bool active, bool &bSendPacket) {
#if ENABLE_DEBUG_FILE == 1
			if(!chlcm) {
				CSX::Log::Add("[Hooked - IClientMode::CreateMove]\n");
				chlcm = true;
			}
#endif
			static auto ofunc = client.get_original<CHLCreateMove>(TABLE::IBaseClientDLL::CreateMove);
			ofunc(Interfaces::Client(), sequence_number, input_sample_frametime, active);

			CUserCmd* pCmd = Interfaces::Input()->GetUserCmd(sequence_number);

			if(!pCmd || pCmd->command_number) {
#if ENABLE_DEBUG_FILE == 1
				if(!chlcm) {
					CSX::Log::Add("\n[Hooked - IClientMode::CreateMove pCmd = null!]\n");
					chlcm = true;
				}
#endif
				return;
			}

			Client::OnCreateMove(pCmd, bSendPacket);
		}

		__declspec(naked) void __fastcall CHLCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active) {
			__asm {
				push ebp
				mov  ebp, esp
				push ebx
				push esp
				push dword ptr[active]
				push dword ptr[input_sample_frametime]
				push dword ptr[sequence_number]
				call Hook_CHLCreateMove
				pop  ebx
				pop  ebp
				retn 0Ch
			}
		}

		bool dpsse; //glow :D (not like it works currently VMT hook or non vmt lool)
		int __stdcall Hook_DoPostScreenSpaceEffects(int a1) {
#if ENABLE_DEBUG_FILE == 1
			if (!dpsse) {
				CSX::Log::Add("[Hooked - DoPostScreenSpaceEffects]");
				dpsse = true;
			}
#endif
			static auto ofunc = clientmode.get_original<DoPostScreenEffects_t>(TABLE::IClientMode::DoPostScreenSpaceEffects);
			if (Interfaces::GlowManager && Interfaces::Engine()->IsConnected() &&
				Settings::Esp::glow && Interfaces::Engine()->GetLocalPlayer()) {
				CMisc::Glow::Get().Run(); //lol
			}
			return ofunc(Interfaces::ClientMode(), a1);
		}

		int ic1;
		bool WINAPI Hook_IsConnected() {
			//19th October 2020
			static void* unk = CSX::Memory::NewPatternScan(GetModuleHandleA(CLIENT_DLL), "75 04 B0 01 5F") - 2;
#if ENABLE_DEBUG_FILE == 1
			if (!ic1) {
				CSX::Log::Add("[FindPattern/Hooked - IsConnected = %X]", unk);
				ic1 = true;
			}
#endif
			//NOODLED DID IT
			if (_ReturnAddress() == unk && Settings::Misc::misc_inventory) {
				return false;
			}  //not called anyway afaik
			return Interfaces::Engine()->IsConnected();
		}

		bool orv; //third person :D
		void __stdcall Hook_OverrideView(CViewSetup* pSetup) {
#if ENABLE_DEBUG_FILE == 1
			if (!orv) {
				CSX::Log::Add("[Hooked - OverrideView]");
				orv = true;
			}
#endif#
			static auto ofunc = clientmode.get_original<OverrideView_t>(TABLE::IClientMode::OverrideView);
			Client::OnOverrideView(pSetup);
			ofunc(Interfaces::Client(), pSetup);
		}

		bool gvm; //gvm
		float WINAPI Hook_GetViewModelFOV() {
#if ENABLE_DEBUG_FILE == 1
			if(!gvm) {
				CSX::Log::Add("[Hooked - GetViewModelFOV]\n");
				gvm = true;
			}
#endif
			clientmode.UnHook();
			float fov = Interfaces::ClientMode()->GetViewModelFOV();
			clientmode.ReHook();
			Client::OnGetViewModelFOV(fov);
			return fov;
		}

		bool rm;
		EGCResults __fastcall Hook_RetrieveMessage(void* ecx, void* edx, uint32_t *punMsgType, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize) {
#if ENABLE_DEBUG_FILE == 1
			if(!rm) {
				CSX::Log::Add("[Hooked - RetrieveMessage]");
				rm = true;
			}
#endif
			steam.UnHook();
			EGCResults status = Interfaces::SteamGameCoordinator()->RetrieveMessage(punMsgType, pubDest, cubDest, pcubMsgSize);
			steam.ReHook();
			if(status != k_EGCResultOK) {
				return status;
			}
			Client::OnRetrieveMessage(ecx, edx, punMsgType, pubDest, cubDest, pcubMsgSize);
			return status;
		}

		bool sm;
		EGCResults __fastcall Hook_SendMessage(void* ecx, void* edx, uint32_t unMsgType, const void* pubData, uint32_t cubData) {
#if ENABLE_DEBUG_FILE == 1
			if(!sm) {
				CSX::Log::Add("\n[Hooked - SendMessage]");
				sm = true;
			}
#endif
			uint32_t messageType = unMsgType & 0x7FFFFFFF;
			void* pubDataMutable = const_cast<void*>(pubData);
			Client::OnSendMessage(ecx, edx, unMsgType, pubData, cubData);
			steam.UnHook();
			EGCResults status = Interfaces::SteamGameCoordinator()->SendMessage(unMsgType, pubData, cubData); //access violitions IF YOU DARE!!!!
			steam.ReHook();
			return status;
		}

		bool fecst;
		bool __fastcall Hook_FireEventClientSideThink(void* ecx, void* edx, IGameEvent* pEvent) {
#if ENABLE_DEBUG_FILE == 1
			if(!fecst) {
				CSX::Log::Add("[Hooked - FireEventClientSideThink]");
				fecst = true;
			}
#endif
			static auto ofunc = eventmanager.get_original<GameEvent>(TABLE::IGameEventManager2::FireEventClientSide);
			if(!pEvent) {
				return ofunc(ecx, pEvent);
			}
			Client::OnFireEventClientSideThink(pEvent);
			return ofunc(ecx, pEvent);
		}

		bool fsn;
		void WINAPI Hook_FrameStageNotify(ClientFrameStage_t Stage) {
#if ENABLE_DEBUG_FILE == 1
			if(!fsn) {
				CSX::Log::Add("[Hooked - FrameStageNotify]");
				fsn = true;
			}
#endif
			Client::OnFrameStageNotify(Stage);
			client.UnHook();
			Interfaces::Client()->FrameStageNotify(Stage); //fuck you
			client.ReHook();
		}
		
	    //?
		bool es1;
		int WINAPI Hook_EmitSound1(IRecipientFilter& filter, int iEntIndex, int iChannel, const char *pSoundEntry, unsigned int nSoundEntryHash, const char *pSample,
			float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
			const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1, int unklown = 0)
		{
#if ENABLE_DEBUG_FILE == 1
			if (!es1) {
				CSX::Log::Add("[Hooked - EmitSound1]\n");
				es1 = true;
			}
#endif
			if (pSample) {
				Client::OnPlaySound(pOrigin, pSample);
			}
			sound.UnHook();
			int ret = Interfaces::Sound()->EmitSound1(filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample,
				flVolume, iSoundlevel, nSeed, iFlags, iPitch,
				pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity);
			sound.ReHook();
			return ret;
		}

		bool es2;
		int WINAPI Hook_EmitSound2(IRecipientFilter& filter, int iEntIndex, int iChannel, const char *pSoundEntry, unsigned int nSoundEntryHash, const char *pSample,
			float flVolume, float flAttenuation, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
			const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1, int unklown = 0) {
#if ENABLE_DEBUG_FILE == 1
			if(!es2) {
				CSX::Log::Add("\n[Hooked - EmitSound2]\n");
				es2 = true;
			}
#endif
			if(pSample) {
				Client::OnPlaySound(pOrigin, pSample);
			}
			sound.UnHook();
			int ret = Interfaces::Sound()->EmitSound2(filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample,
				flVolume, flAttenuation, nSeed, iFlags, iPitch,
				pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity);
			sound.ReHook();
			return ret;
		}

		bool dme;
		void WINAPI Hook_DrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t &state,
			const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld = NULL) {
#if ENABLE_DEBUG_FILE == 1
			if(!dme) {
				CSX::Log::Add("[Hooked - DrawModelExecute]\n");
				dme = true;
			}
#endif
			modelrender.UnHook();
			if(ctx && pCustomBoneToWorld) {
				Client::OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
			}
			Interfaces::ModelRender()->DrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
			if(ctx && pCustomBoneToWorld && Client::g_pEsp && Settings::Esp::esp_Chams) {
				Interfaces::ModelRender()->ForcedMaterialOverride(0);
			}
			modelrender.ReHook();
		}

		bool plays;
		void WINAPI Hook_PlaySound(const char* pszSoundName) {
#if ENABLE_DEBUG_FILE == 1
			if(!plays) {
				CSX::Log::Add("\n[Hooked - PlaySound]\n");
				plays = true;
			}
#endif
			surface.UnHook();
			if(pszSoundName) {
				Client::OnPlaySound(pszSoundName);
			}
			Interfaces::Surface()->PlaySound(pszSoundName);
			surface.ReHook();
		}

		bool lc;
		void __stdcall Hook_LockCursor() {
#if ENABLE_DEBUG_FILE == 1
			if (!lc) {
				CSX::Log::Add("[Hooked - LockCursor]");
				lc = true;
			}
#endif
			static auto ofunc = surface.get_original<LockCursor_t>(TABLE::ISurface::LockCursor);
			if (bIsGuiVisible) {
				Interfaces::Surface()->UnlockCursor();
				return; //for some reason this works
			}
			ofunc(Interfaces::Surface());
		}
		
		//log
		bool svinit;
		bool svofunc;
		bool svconvar;
		//bypass sv_cheats
		bool __fastcall Hook_GetBool_SVCheats(PVOID pConvar, int edx) {
			//FOR IDA:
			//xref : "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %16s"
			//This is the return address of GetBool_SVCheats
			//static DWORD CAM_THINK


			/*SCROLL TO THE FUCKING BOTTOM. RETURN ADDR IS THE "HOOK". KILL ME KILL ME KILL ME*/

			//21 dec 2020 - i cant be fucked, i want to fucking die FUCK THIS SHIT AND FUCK THIS LONG ASS FUNCTION
			//VALVE DO YOU KNOW THE PAIN OF HAVING TO SCROLL ALL THE WAY DOWN TO THE ABYSS, THEN MANAGE TO CREATE A SIG????!??!?!?!?!?!?!?!?! FUCK YOU VALVE.

			//JUST TO SPITE ANYONE WHO READS THIS, LOOK AT  THIS BEAUTIFUL MASTERPIECE OF A SIGNATURE. AND NO I WONT BOTHER TO MAKE IT SMALLER. IF YOU ASK I SHALL SIG THE ENTIRE FUCKING DLL FOR THIS ONE FUNCTION JUST TO SPITE THIS SHIT GAME. FUCK THIS. IF THIS DOESN'T WORK I'M ACTUALLY DONE. IM GOING TO SLEEP. FUCK THIS.
			static DWORD CAM_THINK = CSX::Memory::FindPattern(CLIENT_DLL, "F3 0F 10 44 24 ? F3 0F 11 86 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 86 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 86 ? ? ? ? 5F 5E 8B E5 5D C3", 0) + 0x2F;
#if ENABLE_DEBUG_FILE == 1
			if(!svinit) {
				CSX::Log::Add("[FindPattern/Hooked - GetBool_SVCheats = %X]", CAM_THINK);
				svinit = true;
			}
#endif
			//:(
			if(!pConvar) {
				return false;
			}

#if ENABLE_DEBUG_FILE == 1
			if(!svconvar) {
				CSX::Log::Add("[Hooked - GetBool_SVCheats pConvar = %X]", pConvar);
				svconvar = true;
			}
#endif

			//if any features are "untrusted", or require sv_cheats, then add :)
			if(Settings::Untrusted) {
				if((DWORD)_ReturnAddress() == CAM_THINK) {
#if ENABLE_DEBUG_FILE == 1
					CSX::Log::Add("\n[Hooked - GetBool_SVCheats set!]\n");
#endif
					return true;
				}
			}
			/*bool GetBool() {
				using OriginalFn = bool(__thiscall*)(void*);
				return GetMethod<OriginalFn>(this, 13)(this);
			}*/ //this is Convar

			//make sure returning original function return addr (GetBool), not the convar!
			static auto ofunc = sv_cheats.get_original<GetBool_t>(13);
#if ENABLE_DEBUG_FILE == 1
			if(!svofunc) {
				CSX::Log::Add("[Hooked - GetBool_SVCheats ofunc = %X]\n", ofunc);
				svofunc = true;
			}
#endif
			//return ofunc
			return ofunc;
		}

		bool Initialize() {
			if (!CSX::Utils::IsModuleLoad(D3D9_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", D3D9_DLL);
#endif
				return false;
			}
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[Hooks - module %s loaded!]", D3D9_DLL);
#endif
			if (!CSX::Utils::IsModuleLoad(SHADERPIDX9_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", SHADERPIDX9_DLL);
#endif
				return false;
			}
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[Hooks - module %s loaded!]", SHADERPIDX9_DLL);
#endif

			if (!CSX::Utils::IsModuleLoad(GAMEOVERLAYRENDERER_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", GAMEOVERLAYRENDERER_DLL);
#endif
				return false;
			}
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[Hooks - module %s loaded!]", GAMEOVERLAYRENDERER_DLL);
#endif

			//19 October 2020
			DWORD d3d9TablePtrPtr = CSX::Memory::FindPattern(SHADERPIDX9_DLL, D3D9_PATTERN, D3D9_MASK, 1);
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("\n[FindPattern - pD3D9Table = %X]", d3d9TablePtrPtr);
#endif
			//19th October 2020 - GameOverlayRenderer.dll v6.12.87.0
			DWORD_PTR** dwPresent_o = (DWORD_PTR**)CSX::Memory::FindPattern(GAMEOVERLAYRENDERER_DLL, GMOR_PATTERN, GMOR_MASK, 1);
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[FindPattern - dwPresent = %X]\n", dwPresent_o);
#endif
			if (d3d9TablePtrPtr && dwPresent_o) {
				g_pDevice = (IDirect3DDevice9*)(**(PDWORD*)d3d9TablePtrPtr);
				if (device.setup(g_pDevice)) {
					//all undetect and rewrite - DX9 - Hook Present and Reset
					DWORD_PTR* dwAddress = *dwPresent_o;
					Present_o = (Present_t)(*dwAddress);
					*dwAddress = (DWORD_PTR)(&Hook_Present);
					device.hook_index(D3D9::TABLE::Reset, Hook_Reset);
					Reset_o = (Reset_t)device.get_original<ResetHook>(D3D9::TABLE::Reset);

					//surface
					if (!surface.setup(Interfaces::Surface())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Surface - failed to init!]");
#endif
						return false;
					}
					surface.hook_index(TABLE::ISurface::PlaySound, Hook_PlaySound);
					surface.hook_index(TABLE::ISurface::LockCursor, Hook_LockCursor);

					//client
					if (!client.setup(Interfaces::Client())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Client - failed to init!]");
#endif
						return false;
					}
					client.hook_index(TABLE::IBaseClientDLL::FrameStageNotify, Hook_FrameStageNotify); //NO.
					client.hook_index(TABLE::IBaseClientDLL::CreateMove, CHLCreateMove_Proxy);

					//clientmode
					if (!clientmode.setup(Interfaces::ClientMode())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Clientmode - failed to init!]");
#endif
						return false;
					}
					//clientmode.hook_index(TABLE::IClientMode::CreateMove, Hook_CreateMove); //not needed?
					clientmode.hook_index(TABLE::IClientMode::OverrideView, Hook_OverrideView);
					clientmode.hook_index(TABLE::IClientMode::GetViewModelFOV, Hook_GetViewModelFOV);
					clientmode.hook_index(TABLE::IClientMode::DoPostScreenSpaceEffects, Hook_DoPostScreenSpaceEffects);
					
					//eventmanager
					if (!eventmanager.setup(Interfaces::GameEvent())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[EventManager - failed to init!]");
#endif
						return false;
					}
					eventmanager.hook_index(TABLE::IGameEventManager2::FireEventClientSide, Hook_FireEventClientSideThink);
					
					//sound
					if (!sound.setup(Interfaces::Sound())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Model - failed to init!]");
#endif
						return false;
					}
					sound.hook_index(TABLE::IEngineSound::EmitSound2, Hook_EmitSound2);

					//modelrender
					if (!modelrender.setup(Interfaces::ModelRender())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Model - failed to init!]");
#endif
						return false;
					}
					modelrender.hook_index(TABLE::IVModelRender::DrawModelExecute, Hook_DrawModelExecute);
					
					//sv_cheats - test
					if (!sv_cheats.setup(Interfaces::GetConVar()->FindVar("sv_cheats"))) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[GetBool_SVCheats - failed to init!]");
#endif
						return false;
					}
					sv_cheats.hook_index(13, Hook_GetBool_SVCheats);
					

					//steamgamecoordinator
					if (!steam.setup(Interfaces::SteamGameCoordinator())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[SteamGameCoordinator - failed to init!]");
#endif
						return false;
					}
					steam.hook_index(0, Hook_SendMessage);
					steam.hook_index(2, Hook_RetrieveMessage);

					//all done
					if (Client::Initialize(g_pDevice)) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Client - initialized!]");
#endif
						return true;
					}
				}
			}
			return false;
		}

		int Shutdown() {
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("\n[Hooks - shutting down!]");
			CSX::Log::Add("[Hooks - device = %X", device);
			CSX::Log::Add("[Hooks - surface = %X", surface);
			CSX::Log::Add("[Hooks - client = %X", client);
			CSX::Log::Add("[Hooks - clientmode = %X", clientmode);
			CSX::Log::Add("[Hooks - eventmanager = %X", eventmanager);
			CSX::Log::Add("[Hooks - sound = %X", sound);
			CSX::Log::Add("[Hooks - modelrender = %X", modelrender);
			CSX::Log::Add("[Hooks - sv_cheats = %X", sv_cheats);
			CSX::Log::Add("[Hooks - engine = %X", engine);
			CSX::Log::Add("[Hooks - steamgamecoordinator = %X", steam);
#endif
			device.unhook_all(); //IDirect3DDevice9Table
			surface.unhook_all(); //ISurfaceTable
			client.unhook_all(); //IBaseClientTable
			clientmode.unhook_all(); //IClientModeTable
			eventmanager.unhook_all(); //IGameEventManagerTable
			sound.unhook_all(); //SoundTable
			modelrender.unhook_all(); //ModelRenderTable
			sv_cheats.unhook_all(); //convar lol
			engine.unhook_all(); //EngineTable
			steam.unhook_all(); //SteamGameCoordinatorTable

			//need to undo glow
			CMisc::Glow* misc;
			misc->Shutdown();

#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[Hooks - unhooked!]\n");
#endif
			return true;
		}
	}
}