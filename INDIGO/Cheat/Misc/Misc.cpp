/*
	WARNING: This cheat (like pretty much any other pastes out there) uses Virtual Method Table (VMT) hooking, which is now detected by Valve Anti-Cheat.
	We are NOT responsible for ANY bans that may occur during the process of using this cheat. This includes, but not limited to, VAC, Untrusted and
	Overwatch bans.
*/

#include "Misc.h"
#include "../../SDK/SDK.h"
#include "../../Engine/Engine.h" //cba

using namespace Client;
//[junk_enable /]
//[enc_string_enable /]

bool miscor; //misc on render
void CMisc::OnRender() {
	if (Settings::Misc::misc_Punch) {
		CBaseEntity* lp = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(Interfaces::Engine()->GetLocalPlayer());
		try {
			if (Interfaces::Engine()->IsInGame()) {
				//check to see if player has a weapon so no crash XD
				CBaseWeapon* pWeaponEntity = lp->GetBaseWeapon();
				if (lp && pWeaponEntity && !lp->IsDead() && !lp->IsDormant() && Interfaces::Engine()->IsConnected()){
					Vector ViewAngles;
					Interfaces::Engine()->GetViewAngles(ViewAngles);
					ViewAngles += (lp->GetAimPunchAngle()) * 2.f;
					Vector fowardVec;
					AngleVectors(ViewAngles, fowardVec);
					fowardVec *= 10000;
					Vector start = lp->GetEyePosition();
					Vector end = start + fowardVec, endScreen;

					//potential crash
					if (WorldToScreen(end, endScreen) && IsLocalAlive()) {
						g_pRender->DrawFillBox(endScreen.x - 1, endScreen.y - 1, 3, 3, Color::Green());
					}
				}
			}
		}
		catch (...) {
#if ENABLE_DEBUG_FILE == 1
			if (!miscor) {
				CSX::Log::Add("[CMisc:OnRender - error!]");
				miscor = 1;
			}
#endif
		}
	}
} 

void CMisc::OnCreateMove( CUserCmd* pCmd )
{
	CMe* local = Client::g_pPlayers->GetLocal();

	if (Settings::Misc::misc_Bhop)
	{
		if (local->m_pEntity->GetMoveType() == MOVETYPE_LADDER)
			return;

		static bool bLastJumped = false;
		static bool bShouldFake = false;
	
		if (!bLastJumped && bShouldFake) 
		{
			bShouldFake = false;
			pCmd->buttons |= IN_JUMP;
		}
		else if (pCmd->buttons & IN_JUMP) 
		{
			if (local->m_pEntity->GetFlags() & FL_ONGROUND) 
			{
				bLastJumped = true;
				bShouldFake = true;
			}
			else 
			{
				pCmd->buttons &= ~IN_JUMP;
				bLastJumped = false;
			}
		}
		else 
		{
			bLastJumped = false;
			bShouldFake = false;
		}
	}

	if (Settings::Misc::misc_spamregular)
		ChatSpamRegular();

	if (Settings::Misc::misc_spamrandom)
		ChatSpamRandom();

	if (Settings::Misc::misc_Clan > 0)
		ClanTag();

	ConVar* skybox = Interfaces::GetConVar()->FindVar("sv_skyname");

	if (Settings::Misc::misc_SkyName && Settings::Untrusted) { if (skybox) skybox->SetValue(Settings::Misc::misc_SkyName); }
	if (Settings::Misc::misc_NoSky && Settings::Untrusted) { if (skybox) skybox->SetValue("sky_l4d_rural02_ldr"); }


	ConVar* PostProcess = Interfaces::GetConVar()->FindVar("mat_postprocess_enable");
	*(int*)((DWORD)&PostProcess->fnChangeCallback + 0xC) = 0;
	PostProcess->SetValue(!Settings::Misc::misc_Postprocess);

	if (Settings::Misc::misc_namespamidkmemes)
	{
		static bool steal = false;

		ConVar* Name = Interfaces::GetConVar()->FindVar("name");
		*(int*)((DWORD)&Name->fnChangeCallback + 0xC) = 0;

		if (Name)
		{
			if (!steal)
			{
				Name->SetValue(Settings::Misc::First);
				steal = true;
			}
			else
			{
				Name->SetValue(Settings::Misc::Second);
				steal = false;
			}
		}
	}

	if (Settings::Misc::misc_namespamidkmemes_static)
	{
		static bool steal = false;

		ConVar* Name = Interfaces::GetConVar()->FindVar("name");
		*(int*)((DWORD)&Name->fnChangeCallback + 0xC) = 0;

		if (Name)
		{
			if (!steal)
			{
				Name->SetValue(Settings::Misc::First_static);
				steal = true;
			}
			else
			{
				Name->SetValue(Settings::Misc::Second_static);
				steal = false;
			}
		}
	}
}

//broken
void CMisc::FrameStageNotify(ClientFrameStage_t Stage)
{ /*
	if (Interfaces::Engine()->IsInGame() && Stage == ClientFrameStage_t::FRAME_RENDER_START)
	{
		CBaseEntity* localplayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(Interfaces::Engine()->GetLocalPlayer());
		if (!localplayer)
			return;

		static QAngle vecAngles;
		Interfaces::Engine()->GetViewAngles(vecAngles);
		if (Settings::Misc::misc_ThirdPerson && !localplayer->IsDead() && Settings::Untrusted)
		{
			if (!Interfaces::Input()->m_fCameraInThirdPerson)
				Interfaces::Input()->m_fCameraInThirdPerson = true;

			Interfaces::Input()->m_vecCameraOffset = QAngle(vecAngles.x, vecAngles.y, Settings::Misc::misc_ThirdPersonRange);

			*localplayer->GetVAngles() = Settings::Misc::qLastTickAngle;
		}
		else
		{
			if (Interfaces::Input()->m_fCameraInThirdPerson || localplayer->GetIsScoped())
			{
				Interfaces::Input()->m_fCameraInThirdPerson = false;
				Interfaces::Input()->m_vecCameraOffset = QAngle(vecAngles.x, vecAngles.y, 0);
			}
		}
	}*/
}

std::vector<const char*> smoke_materials = {
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
};

void CMisc::OnDrawModelExecute()
{
	static bool NoSmoke = false;
	static bool NoFlashReset = false;
	IMaterial* flash = Interfaces::MaterialSystem()->FindMaterial("effects\\flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
	IMaterial* flashWhite = Interfaces::MaterialSystem()->FindMaterial("effects\\flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);

	if (flash && flashWhite)
	{
		if (Settings::Misc::misc_NoFlash && !NoFlashReset)
		{
			flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			flashWhite->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

			NoFlashReset = true;
		}
		else if (!Settings::Misc::misc_NoFlash && NoFlashReset)
		{
			flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			flashWhite->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

			NoFlashReset = false;
		}
	}

	if (Settings::Misc::misc_NoSmoke && !NoSmoke)
	{
		for (auto mat : smoke_materials)
			Interfaces::MaterialSystem()->FindMaterial(mat, TEXTURE_GROUP_CLIENT_EFFECTS)->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

		NoSmoke = true;
	}
	else if (!Settings::Misc::misc_NoSmoke && NoSmoke)
	{
		for (auto mat : smoke_materials)
			Interfaces::MaterialSystem()->FindMaterial(mat, TEXTURE_GROUP_CLIENT_EFFECTS)->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);

		NoSmoke = false;
	}
}

//19th October 2020
void CMisc::OnPlaySound(const char* pszSoundName) {
	if (Settings::Misc::misc_AutoAccept && !strcmp(pszSoundName, "UIPanorama.popup_accept_match_beep")) {
		typedef bool(__stdcall* IsReadyCallBack_t)(const char*);
		static auto IsReadyCallBackFn = (IsReadyCallBack_t)CSX::Memory::FindPatternV2(CLIENT_DLL, "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12");
#if ENABLE_DEBUG_FILE == 1
		CSX::Log::Add("[FindPattern - IsReadyCallBack = %X]", IsReadyCallBackFn);
#endif
		if (IsReadyCallBackFn) {
			IsReadyCallBackFn("deferred");
		}
	}
}

//orig third person (Framestagenotify)
/* /*
	if (Interfaces::Engine()->IsInGame() && Stage == ClientFrameStage_t::FRAME_RENDER_START)
	{
		CBaseEntity* localplayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(Interfaces::Engine()->GetLocalPlayer());
		if (!localplayer)
			return;

		static QAngle vecAngles;
		Interfaces::Engine()->GetViewAngles(vecAngles);
		if (Settings::Misc::misc_ThirdPerson && !localplayer->IsDead() && Settings::Untrusted)
		{
			if (!Interfaces::Input()->m_fCameraInThirdPerson)
				Interfaces::Input()->m_fCameraInThirdPerson = true;

			Interfaces::Input()->m_vecCameraOffset = QAngle(vecAngles.x, vecAngles.y, Settings::Misc::misc_ThirdPersonRange);

			*localplayer->GetVAngles() = Settings::Misc::qLastTickAngle;
		}
		else
		{
			if (Interfaces::Input()->m_fCameraInThirdPerson || localplayer->GetIsScoped())
			{
				Interfaces::Input()->m_fCameraInThirdPerson = false;
				Interfaces::Input()->m_vecCameraOffset = QAngle(vecAngles.x, vecAngles.y, 0);
			}
		}
	}*/

void CMisc::OnOverrideView( CViewSetup * pSetup ) {
	//thirdperson :D
	if(Interfaces::Engine()->IsInGame()) {
		CBaseEntity* localplayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(Interfaces::Engine()->GetLocalPlayer());
		if(localplayer && !localplayer->IsDead() && Settings::Misc::misc_ThirdPerson && Settings::Untrusted) {
			if(!Interfaces::Input()->m_fCameraInThirdPerson) {
				Interfaces::Input()->m_fCameraInThirdPerson = true;
			}

			float dist = Settings::Misc::misc_ThirdPersonRange;

			QAngle *view = localplayer->GetVAngles();
			trace_t tr;
			Ray_t ray;

			Vector desiredCamOffset = Vector(cos((view->y)) * dist,
				sin(DEG2RAD2(view->y)) * dist,
				sin(DEG2RAD2(-view->x)) * dist
			);

			//cast a ray from the Current camera Origin to the Desired 3rd person Camera origin
			ray.Init(localplayer->GetEyePosition(), (localplayer->GetEyePosition() - desiredCamOffset));
			CTraceFilter traceFilter;
			traceFilter.pSkip = localplayer;
			Interfaces::EngineTrace()->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

			Vector diff = localplayer->GetEyePosition() - tr.endpos;

			float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));// Pythagorean wtf

			bool horOK = distance2D > (dist - 2.0f);
			bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

			float cameraDistance;

			if(horOK && vertOK) { // If we are clear of obstacles
				cameraDistance = dist; // go ahead and set the distance to the setting
			}
			else {
				if(vertOK) { // if the Vertical Axis is OK
					cameraDistance = distance2D * 0.95f;
				}
				else { // otherwise we need to move closer to not go into the floor/ceiling
					cameraDistance = abs(diff.z) * 0.95f;
				}
			}
			Interfaces::Input()->m_fCameraInThirdPerson = true;
			Interfaces::Input()->m_vecCameraOffset.z = cameraDistance;
		}
		else { //if thirdperson disabled
			if(Interfaces::Input()->m_fCameraInThirdPerson) {
				Interfaces::Input()->m_fCameraInThirdPerson = false;
			}
		}
	}

	if (Settings::Misc::misc_FovChanger && !Interfaces::Engine()->IsTakingScreenshot()) {
		CBaseEntity* pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity( Interfaces::Engine()->GetLocalPlayer() );

		if (!pPlayer)
			return;

		if (pPlayer->GetIsScoped())
			return;

		if (pPlayer->IsDead()) {
			if (pPlayer->GetObserverMode() == ObserverMode_t::OBS_MODE_IN_EYE && pPlayer->GetObserverTarget())
				pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntityFromHandle(pPlayer->GetObserverTarget());

			if (!pPlayer)
				return;
		}
		pSetup->fov = (float)Settings::Misc::misc_FovView;
	}
}

void CMisc::OnGetViewModelFOV(float& fov)
{
	if (Settings::Misc::misc_FovChanger && !Interfaces::Engine()->IsTakingScreenshot())
	{
		CBaseEntity* pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(Interfaces::Engine()->GetLocalPlayer());

		if (!pPlayer)
			return;

		if (pPlayer->IsDead())
		{
			if (pPlayer->GetObserverMode() == ObserverMode_t::OBS_MODE_IN_EYE && pPlayer->GetObserverTarget())
				pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntityFromHandle(pPlayer->GetObserverTarget());

			if (!pPlayer)
				return;
		}

		fov = (float)Settings::Misc::misc_FovModelView;
	}
}

vector<int> CMisc::GetObservervators( int playerId )
{
	vector<int> SpectatorList;

	CBaseEntity* pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity( playerId );

	if ( !pPlayer )
		return SpectatorList;

	if ( pPlayer->IsDead() )
	{
		CBaseEntity* pObserverTarget = (CBaseEntity*)Interfaces::EntityList()->GetClientEntityFromHandle( pPlayer->GetObserverTarget() );

		if ( !pObserverTarget )
			return SpectatorList;

		pPlayer = pObserverTarget;
	}

	for ( int PlayerIndex = 0; PlayerIndex < g_pPlayers->GetSize(); PlayerIndex++ )
	{
		CBaseEntity* pCheckPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity( PlayerIndex );

		if ( !pCheckPlayer )
			continue;

		if ( pCheckPlayer->IsDormant() || !pCheckPlayer->IsDead() )
			continue;

		CBaseEntity* pObserverTarget = (CBaseEntity*)Interfaces::EntityList()->GetClientEntityFromHandle( pCheckPlayer->GetObserverTarget() );

		if ( !pObserverTarget )
			continue;

		if ( pPlayer != pObserverTarget )
			continue;

		SpectatorList.push_back( PlayerIndex );
	}

	return SpectatorList;
}

void CMisc::FakeLag(bool &bSendPacket)
{
	int amount = Settings::Misc::misc_fakelag_amount;

	static int choke = -1;
	choke++;

	if (choke <= amount && choke > -1)
	{
		bSendPacket = false;
	}
	else
	{
		bSendPacket = true;
		choke = -1;
	}
}

void CMisc::OnRenderSpectatorList()
{
	if (Settings::Misc::misc_Spectators)
	{
		int DrawIndex = 1;

		for (int playerId : GetObservervators(Interfaces::Engine()->GetLocalPlayer()))
		{
			if (playerId == Interfaces::Engine()->GetLocalPlayer())
				continue;

			CBaseEntity* pPlayer = (CBaseEntity*)Interfaces::EntityList()->GetClientEntity(playerId);

			if (!pPlayer)
				continue;

			PlayerInfo Pinfo{};
			Interfaces::Engine()->GetPlayerInfo(playerId, &Pinfo);

			if (Pinfo.m_bIsFakePlayer)
				continue;

			if (g_pRender)
			{
				string Name = Pinfo.m_szPlayerName;

				if (Name != "")
				{
					Color PlayerObsColor;
					//[junk_disable /]
					switch (pPlayer->GetObserverMode())
					{
					case ObserverMode_t::OBS_MODE_IN_EYE:
						Name.append(" ");
						PlayerObsColor = Color::White();
						break;
					default:
						break;
					}
					//[junk_enable /]
					g_pRender->Text(15, 500 + (DrawIndex * 13), false, true, PlayerObsColor, "%s", Name.c_str());
					DrawIndex++;
				}
			}
		}
	}
}

//thx csgosimple :)
CMisc::Glow::Glow() {}

CMisc::Glow::~Glow() {
	// We cannot call shutdown here unfortunately.
	// Reason is not very straightforward but anyways:
	// - This destructor will be called when the dll unloads
	//   but it cannot distinguish between manual unload 
	//   (pressing the Unload button or calling FreeLibrary)
	//   or unload due to game exit.
	//   What that means is that this destructor will be called
	//   when the game exits.
	// - When the game is exiting, other dlls might already 
	//   have been unloaded before us, so it is not safe to 
	//   access intermodular variables or functions.
	//   
	//   Trying to call Shutdown here will crash CSGO when it is
	//   exiting (because we try to access g_GlowObjManager).
	//
}

void CMisc::Glow::Shutdown() {
	// Remove glow from all entities
	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
		auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<CBaseEntity*>(glowObject.m_pEntity);

		if (glowObject.IsUnused()) {
			continue;
		}

		if (!entity || entity->IsDormant()) {
			continue;
		}

		glowObject.m_flAlpha = 0.0f;
	}
}

//original
/*auto glow_target = [](GlowObjectDefinition_t& glowObject, Color color) -> void {
				glowObject.m_flRed = color.r() / 255.f;
				glowObject.m_flGreen = color.g() / 255.f;
				glowObject.m_flBlue = color.b() / 255.f;
				glowObject.m_flAlpha = color.a() / 255.f;
				glowObject.m_bRenderWhenOccluded = true;
				glowObject.m_bRenderWhenUnoccluded = false;
			};
			if (Interfaces::GlowManager && Interfaces::Engine()->IsConnected()) {
				if (Settings::Esp::glow) {
					for (auto i = 0; i < Interfaces::GlowManager()->m_GlowObjectDefinitions.Count(); i++) {
						auto& glowObject = Interfaces::GlowManager()->m_GlowObjectDefinitions[i];
						auto entity = reinterpret_cast<CBaseEntity*>(glowObject.m_pEntity);
						if (!entity || glowObject.IsUnused()) {
							continue;
						}
						switch (entity->GetClientClass()->m_ClassID) {
							case (int)CLIENT_CLASS_ID::CCSPlayer: {
								if (entity->GetTeam() != Client::g_pPlayers->GetLocal()->m_pEntity->GetTeam()) {
									glow_target(glowObject, Color(255, 255, 255, 255));
								}
							}
							break;
							default:
								break;
						}
					}
				}
			}*/

void CMisc::Glow::Run() {
	auto glow_target = [](GlowObjectDefinition_t& glowObject, Color color) -> void {
		glowObject.m_flRed = color.r() / 255.f;
		glowObject.m_flGreen = color.g() / 255.f;
		glowObject.m_flBlue = color.b() / 255.f;
		glowObject.m_flAlpha = color.a() / 255.f;
		glowObject.m_bRenderWhenOccluded = true;
		glowObject.m_bRenderWhenUnoccluded = false;
	};

	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
		auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<CBaseEntity*>(glowObject.m_pEntity);

		if (glowObject.IsUnused()) {
			continue;
		}

		if (!entity || entity->IsDormant()) {
			continue;
		}

		//auto color = Color{};

		switch (entity->GetClientClass()->m_ClassID) {
		case (int)CLIENT_CLASS_ID::CCSPlayer: {
			//auto is_enemy = (entity->GetTeam() != Client::g_pPlayers->GetLocal()->m_pEntity->GetTeam());
			if (entity->GetTeam() != Client::g_pPlayers->GetLocal()->m_pEntity->GetTeam()) {
				glow_target(glowObject, Color(255, 255, 255, 255));
			}
			/*if (entity->HasC4() && is_enemy && g_Options.glow_c4_carrier) {
				color = g_Options.color_glow_c4_carrier;
				break;
			}*/

			/*if (/*!g_Options.glow_players || entity->IsDead())
				continue;

			/*if (!is_enemy && Settings::Esp::esp_Enemy)
				continue;*/

			//color = is_enemy ? g_Options.color_glow_enemy : g_Options.color_glow_ally;

			
		}break;
		//remove all below this if you want the default asis indigo glow
		/*case (int)CLIENT_CLASS_ID::CChicken: {
			if(!g_Options.glow_chickens) { //settings -> glow_chickens
				continue;
			}
			entity->m_bShouldGlow() = true;
			color = g_Options.color_glow_chickens; //dont have a setting for this
			break;
		}
		case (int)CLIENT_CLASS_ID::CBaseAnimating: {
			if(!g_Options.glow_defuse_kits) { //im guessing it's talking about defusing?
				continue;
			}
			color = g_Options.color_glow_defuse; //dont have a setting for this
			break;
		}
		case (int)CLIENT_CLASS_ID::CPlantedC4: { //im gonna guess this means -> BOMB PLANTED
			if (!g_Options.glow_planted_c4) { //glow_planted_c4
				continue;
			}
			color = g_Options.color_glow_planted_c4; //dont have a setting for this
			break;
		}
		/*default: {
			if (entity->IsWeapon()) {
				if (!Settings::Esp::esp_Weapon) { //settings -> glow_weapons
					continue;
				}
				color = g_Options.color_glow_weapons; //dont have a setting for this
			}
		}
				 glowObject.m_flRed = color.r() / 255.0f;
				 glowObject.m_flGreen = color.g() / 255.0f;
				 glowObject.m_flBlue = color.b() / 255.0f;
				 glowObject.m_flAlpha = color.a() / 255.0f;
				 glowObject.m_bRenderWhenOccluded = true;
				 glowObject.m_bRenderWhenUnoccluded = false;*/
		}
	}
}
