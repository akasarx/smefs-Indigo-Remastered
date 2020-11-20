#pragma once
#include "../../Engine/Engine.h"
#include "../../SDK/Spoofed.h"
#include "../../SDK/CGlowObjManager.h"

//singleton.hpp from csgosimple lol
template<typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

public:
	static T& Get()
	{
		static T inst{};
		return inst;
	}
};

//decided just to add glow under cmisc, change if u want
class CMisc
{
public:
	//[swap_lines]
	void OnRender();
	void OnCreateMove( CUserCmd* pCmd );
	void FrameStageNotify(ClientFrameStage_t Stage);
#define POSTPROCESS_VALUE
	void OnDrawModelExecute();
	void OnPlaySound( const char* pszSoundName );

	void OnOverrideView( CViewSetup* pSetup );
	void OnGetViewModelFOV( float& fov );

	vector<int> GetObservervators( int playerId );
	void FakeLag(bool & bSendPacket);
	void OnRenderSpectatorList();

	class Glow
		: public Singleton<Glow>
	{
		friend class Singleton<Glow>;

		Glow();
		~Glow();
		CGlowObjectManager* g_GlowObjManager = Interfaces::GlowManager(); //idk if this will work lol

	public:
		void Run();
		void Shutdown();
	};
//[/swap_lines]
};