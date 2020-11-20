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
		CSX::Hook::VTable device; //IDirect3DDevice9Table;
		CSX::Hook::VTable surface; //ISurfaceTable
		CSX::Hook::VTable client; //IBaseClientTable
		CSX::Hook::VTable clientmode; //IClientModeTable;
		CSX::Hook::VTable eventmanager; //IGameEventManagerTable
		CSX::Hook::VTable sound;
		CSX::Hook::VTable modelrender;
		CSX::Hook::VTable sv_cheats; //convar lol
		CSX::Hook::VTable engine;
		CSX::Hook::VTable steam; //steamgamecoordinator
		IDirect3DDevice9* g_pDevice = nullptr;
		bool svcheats_init = false;

		typedef HRESULT(WINAPI* Present_t)(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
		Present_t Present_o;

		typedef HRESULT(WINAPI* Reset_t)(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
		Reset_t Reset_o;

		bool ps;
		HRESULT WINAPI Hook_Present(IDirect3DDevice9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
#if ENABLE_DEBUG_FILE == 1
			if (!ps) {
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
			if (!rs) {
				CSX::Log::Add("[Hooked - Reset]");
				rs = true;
			}
#endif
			Client::OnLostDevice();
			HRESULT hRes = Reset_o(pDevice, pPresentationParameters);
			Client::OnResetDevice();
			return hRes;
		}

		bool cm;
		bool WINAPI Hook_CreateMove(float flInputSampleTime, CUserCmd* pCmd) {
#if ENABLE_DEBUG_FILE == 1
			if (!cm) {
				CSX::Log::Add("[Hooked - CreateMove]");
				cm = true;
			}
#endif
			if (!pCmd->command_number) {
				return true;
			}
			clientmode.UnHook();
			auto ebp = (uintptr_t*)(uintptr_t(_AddressOfReturnAddress()) - sizeof(void*));
			bool& bSendPacket = *reinterpret_cast<bool*>( *ebp - 0x1C );
			Client::OnCreateMove(pCmd, bSendPacket);
			bool ret = Interfaces::ClientMode()->CreateMove(flInputSampleTime, pCmd);
			clientmode.ReHook();
			return false;
		}

		bool dpsse; //glow :D
		int WINAPI Hook_DoPostScreenSpaceEffects(int callback) {
#if ENABLE_DEBUG_FILE == 1
			if (!dpsse) {
				CSX::Log::Add("[Hooked - DoPostScreenSpaceEffects]");
				dpsse = true;
			}
#endif
			clientmode.UnHook();
			int ret = Interfaces::ClientMode()->DoPostScreenSpaceEffects(callback);
			clientmode.ReHook();
			if (Interfaces::GlowManager && Interfaces::Engine()->IsConnected() &&
				Settings::Esp::glow && Interfaces::Engine()->GetLocalPlayer()) {
				CMisc::Glow::Get().Run(); //lol
			}
			return ret;
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
			}  //not called anyway
			engine.UnHook();
			bool ret = Interfaces::Engine()->IsConnected();
			engine.ReHook();
			return ret;
		}

		bool orv; //third person :D
		bool WINAPI Hook_OverrideView(CViewSetup* pSetup) {
#if ENABLE_DEBUG_FILE == 1
			if (!orv) {
				CSX::Log::Add("[Hooked - OverrideView]");
				orv = true;
			}
#endif
			Client::OnOverrideView(pSetup);
			clientmode.UnHook();
			bool ret = Interfaces::ClientMode()->OverrideView(pSetup);
			clientmode.ReHook();
			return ret;
		}

		bool gvm;
		float WINAPI Hook_GetViewModelFOV() {
#if ENABLE_DEBUG_FILE == 1
			if (!gvm) {
				CSX::Log::Add("[Hooked - GetViewModelFOV]");
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
			if (!rm) {
				CSX::Log::Add("\n[Hooked - RetrieveMessage]");
				rm = true;
			}
#endif
			steam.UnHook();
			EGCResults status = Interfaces::SteamGameCoordinator()->RetrieveMessage(punMsgType, pubDest, cubDest, pcubMsgSize);
			steam.ReHook();
			if (status != k_EGCResultOK) {
				return status;
			}
			Client::OnRetrieveMessage(ecx, edx, punMsgType, pubDest, cubDest, pcubMsgSize);
			return status;
		}

		bool sm;
		EGCResults __fastcall Hook_SendMessage(void* ecx, void* edx, uint32_t unMsgType, const void* pubData, uint32_t cubData) {
#if ENABLE_DEBUG_FILE == 1
			if (!sm) {
				CSX::Log::Add("\n[Hooked - SendMessage]");
				sm = true;
			}
#endif
			uint32_t messageType = unMsgType & 0x7FFFFFFF;
			void* pubDataMutable = const_cast<void*>(pubData);
			Client::OnSendMessage(ecx, edx, unMsgType, pubData, cubData);
			steam.UnHook();
			EGCResults status = Interfaces::SteamGameCoordinator()->SendMessage(unMsgType, pubData, cubData);
			steam.ReHook();
			return status;
		}

		bool fecst;
		bool WINAPI Hook_FireEventClientSideThink(IGameEvent* pEvent) {
#if ENABLE_DEBUG_FILE == 1
			if (!fecst) {
				CSX::Log::Add("[Hooked - FireEventClientSideThink]");
				fecst = true;
			}
#endif
			bool ret = false;
			if (!pEvent) {
				eventmanager.UnHook();
				ret = Interfaces::GameEvent()->FireEventClientSide(pEvent);
				eventmanager.ReHook();
				return ret;
			}
			Client::OnFireEventClientSideThink(pEvent);
			eventmanager.UnHook();
			ret = Interfaces::GameEvent()->FireEventClientSide(pEvent);
			eventmanager.ReHook();
			return ret;
		}

		bool fsn;
		void WINAPI Hook_FrameStageNotify(ClientFrameStage_t Stage) {
#if ENABLE_DEBUG_FILE == 1
			if (!fsn) {
				CSX::Log::Add("[Hooked - FrameStageNotify]");
				fsn = true;
			}
#endif
			Client::OnFrameStageNotify(Stage); //no need!
			client.UnHook();
			Interfaces::Client()->FrameStageNotify(Stage);
			client.ReHook();
		}
		
		//broken
		bool es1;
		int WINAPI Hook_EmitSound1(IRecipientFilter& filter, int iEntIndex, int iChannel, const char *pSoundEntry, unsigned int nSoundEntryHash, const char *pSample,
			float flVolume, soundlevel_t iSoundlevel, int nSeed, int iFlags = 0, int iPitch = PITCH_NORM,
			const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1, int unklown = 0)
		{
#if ENABLE_DEBUG_FILE == 1
			if (!es1) {
				CSX::Log::Add("\n[Hooked - EmitSound1]\n");
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
			const Vector *pOrigin = NULL, const Vector *pDirection = NULL, CUtlVector< Vector >* pUtlVecOrigins = NULL, bool bUpdatePositions = true, float soundtime = 0.0f, int speakerentity = -1, int unklown = 0)
		{
#if ENABLE_DEBUG_FILE == 1
			if (!es2) {
				CSX::Log::Add("\n[Hooked - EmitSound2]\n");
				es2 = true;
			}
#endif
			if (pSample) {
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
			const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld = NULL)
		{
#if ENABLE_DEBUG_FILE == 1
			if (!dme) {
				CSX::Log::Add("[Hooked - DrawModelExecute]\n");
				dme = true;
			}
#endif
			modelrender.UnHook();
			if (ctx && pCustomBoneToWorld) {
				Client::OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
			}
			Interfaces::ModelRender()->DrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
			if (ctx && pCustomBoneToWorld && Client::g_pEsp && Settings::Esp::esp_Chams) {
				Interfaces::ModelRender()->ForcedMaterialOverride(0);
			}
			modelrender.ReHook();
		}

		bool plays;
		void WINAPI Hook_PlaySound(const char* pszSoundName) {
#if ENABLE_DEBUG_FILE == 1
			if (!plays) {
				CSX::Log::Add("\n[Hooked - PlaySound]\n");
				plays = true;
			}
#endif
			surface.UnHook();
			if (pszSoundName) {
				Client::OnPlaySound(pszSoundName);
			}
			Interfaces::Surface()->PlaySound(pszSoundName);
			surface.ReHook();
		}

		bool lc;
		void Hook_LockCursor() {
#if ENABLE_DEBUG_FILE == 1
			if (!lc) {
				CSX::Log::Add("\n[Hooked - LockCursor]");
				lc = true;
			}
#endif
			surface.UnHook();
			Interfaces::Surface()->LockCursor();
			surface.ReHook();
			if (bIsGuiVisible) {
				Interfaces::Surface()->UnlockCursor();
			}
		}
		
		//bypass sv_cheats
		bool __fastcall Hook_GetBool_SVCheats(PVOID pConvar, int edx) {
			try {
				//11th March 2019
				//xref : "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %16s"
				//This is the return address of GetBool_SVCheats
				//static DWORD CAM_THINK = CSX::Memory::FindCodePattern(CLIENT_DLL, (BYTE*)"\xC3\x55\x8B\xEC\x8B\x55\x0C\x8B\x45\x08", "xxxxxxxxxx");

				//19th october 2020
				static DWORD CAM_THINK = CSX::Memory::FindPattern(CLIENT_DLL, "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89", 0) + 0x1;
#if ENABLE_DEBUG_FILE == 1
				if (!svcheats_init) {
					CSX::Log::Add("[FindPattern/Hooked - GetBool_SVCheats = %X]\n", CAM_THINK);
					svcheats_init = true;
				}
#endif
				if (!pConvar) {
					return false;
				}

				//if any features are "untrusted", or require sv_cheats, then add :)
				if (Settings::Misc::misc_ThirdPerson || Settings::Untrusted) {
					if ((DWORD)_ReturnAddress() == CAM_THINK) {
						return true;
					}
				}

				//i think u unhook then call ofunc then rehook, right?
				sv_cheats.UnHook();
				//idk vmt sooo this is my best try hehe
				static auto ofunc = Interfaces::GetConVar()->FindVar("sv_cheats");
				sv_cheats.ReHook();
				return ofunc;
			}
			catch (...) {
				//fail
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - GetBool_SVCheats failed!]");
#endif
				return false;
			}
			//:'(
			/*static auto ofunc = convar.get_original<GetBool_t>(13);
			return ofunc(pConvar);*/
		}

		bool Initialize() {
			if (!CSX::Utils::IsModuleLoad(D3D9_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", D3D9_DLL);
#endif
				return false;
			}

			if (!CSX::Utils::IsModuleLoad(SHADERPIDX9_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", SHADERPIDX9_DLL);
#endif
				return false;
			}

			if (!CSX::Utils::IsModuleLoad(GAMEOVERLAYRENDERER_DLL)) {
#if ENABLE_DEBUG_FILE == 1
				CSX::Log::Add("[Hooks - module %s not loaded!]", GAMEOVERLAYRENDERER_DLL);
#endif
				return false;
			}

			//19 October 2020
			DWORD d3d9TablePtrPtr = CSX::Memory::FindPattern(SHADERPIDX9_DLL, D3D9_PATTERN, D3D9_MASK, 1);
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[FindPattern - pD3D9Table = %X]", d3d9TablePtrPtr);
#endif
			//19th October 2020 - GameOverlayRenderer.dll v6.12.87.0
			DWORD_PTR** dwPresent_o = (DWORD_PTR**)CSX::Memory::FindPattern(GAMEOVERLAYRENDERER_DLL, GMOR_PATTERN, GMOR_MASK, 1);
#if ENABLE_DEBUG_FILE == 1
			CSX::Log::Add("[FindPattern - dwPresent = %X]", dwPresent_o);
#endif
			if (d3d9TablePtrPtr && dwPresent_o) {
				g_pDevice = (IDirect3DDevice9*)(**(PDWORD*)d3d9TablePtrPtr);
				if (device.InitTable(g_pDevice)) {
					//DX9 - Hook Present and Reset
					DWORD_PTR* dwAddress = *dwPresent_o;
					Present_o = (Present_t)(*dwAddress);
					*dwAddress = (DWORD_PTR)(&Hook_Present);
					Reset_o = (Reset_t)device.GetHookIndex(D3D9::TABLE::Reset, Hook_Reset);

					//surface
					if (!surface.InitTable(Interfaces::Surface())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Surface - failed to init!]");
#endif
						return false;
					}
					surface.HookIndex(TABLE::ISurface::PlaySound, Hook_PlaySound);
					surface.HookIndex(TABLE::ISurface::LockCursor, Hook_LockCursor);

					//client
					if (!client.InitTable(Interfaces::Client())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Client - failed to init!]");
#endif
						return false;
					}
					client.HookIndex(TABLE::IBaseClientDLL::FrameStageNotify, Hook_FrameStageNotify);

					//clientmode
					if (!clientmode.InitTable(Interfaces::ClientMode())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Clientmode - failed to init!]");
#endif
						return false;
					}
					clientmode.HookIndex(TABLE::IClientMode::CreateMove, Hook_CreateMove);
					clientmode.HookIndex(TABLE::IClientMode::OverrideView, Hook_OverrideView);
					clientmode.HookIndex(TABLE::IClientMode::GetViewModelFOV, Hook_GetViewModelFOV);
					clientmode.HookIndex(TABLE::IClientMode::DoPostScreenSpaceEffects, Hook_DoPostScreenSpaceEffects);
					
					//eventmanager
					if (!eventmanager.InitTable(Interfaces::GameEvent())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[EventManager - failed to init!]");
#endif
						return false;
					}
					eventmanager.HookIndex(TABLE::IGameEventManager2::FireEventClientSide, Hook_FireEventClientSideThink);
					
					//sound
					if (!sound.InitTable(Interfaces::Sound())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Model - failed to init!]");
#endif
						return false;
					}
					//sound.HookIndex(TABLE::IEngineSound::EmitSound1, Hook_EmitSound1); //idk if needed
					sound.HookIndex(TABLE::IEngineSound::EmitSound2, Hook_EmitSound2);

					//modelrender
					if (!modelrender.InitTable(Interfaces::ModelRender())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[Model - failed to init!]");
#endif
						return false;
					}
					modelrender.HookIndex(TABLE::IVModelRender::DrawModelExecute, Hook_DrawModelExecute);
					
					//sv_cheats
					if (!sv_cheats.InitTable(Interfaces::GetConVar()->FindVar("sv_cheats"))) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[GetBool_SVCheats - failed to init!]");
#endif
						return false;
					}
					sv_cheats.HookIndex(TABLE::convar::getbool, Hook_GetBool_SVCheats);
					
					//steamgamecoordinator
					if (!steam.InitTable(Interfaces::SteamGameCoordinator())) {
#if ENABLE_DEBUG_FILE == 1
						CSX::Log::Add("\n[SteamGameCoordinator - failed to init!]");
#endif
						return false;
					}
					steam.HookIndex(0, Hook_SendMessage);
					steam.HookIndex(2, Hook_RetrieveMessage);

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
			device.UnHook(); //IDirect3DDevice9Table
			surface.UnHook(); //ISurfaceTable
			client.UnHook(); //IBaseClientTable
			clientmode.UnHook(); //IClientModeTable
			eventmanager.UnHook(); //IGameEventManagerTable
			sound.UnHook(); //SoundTable
			modelrender.UnHook(); //ModelRenderTable
			sv_cheats.UnHook(); //convar lol
			engine.UnHook(); //EngineTable
			steam.UnHook(); //SteamGameCoordinatorTable

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