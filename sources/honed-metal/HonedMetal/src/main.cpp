#include "SKSE_resolver.h"
#include "CraftingUtils.h"
#include "common/IDebugLog.h"
#include RESOLVE_PATH(SKSE_FOLDER, PluginAPI.h)
#include RESOLVE_PATH(SKSE_FOLDER, ScaleformCallbacks.h)

SKSEPapyrusInterface*	g_papyrus_interface = nullptr;
SKSEScaleformInterface*	g_scaleform = nullptr;

extern "C"
{
	static bool validate_plugin(const SKSEInterface *skse)
	{
		_MESSAGE("Query");
		if ((*skse).isEditor)
		{	_MESSAGE("loaded in editor, marking as incompatible");
			return (false);
		}
		if ((*skse).runtimeVersion != TARGET_RUNTIME)
		{	_MESSAGE("ERROR: unsupported runtime version %08X", (*skse).runtimeVersion);
			return (false);
		}
		if (!(g_scaleform = (SKSEScaleformInterface*)(*skse).QueryInterface(kInterface_Scaleform) ) )
		{	_MESSAGE("ERROR: couldn't get scaleform interface");
			return (false);
		}
		if ((*g_scaleform).interfaceVersion < SKSEScaleformInterface::kInterfaceVersion)
		{	_MESSAGE("ERROR: scaleform interface too old (%i expected %i)", (*g_scaleform).interfaceVersion, SKSEScaleformInterface::kInterfaceVersion);
			return (false);
		}
		if (!(g_papyrus_interface = (SKSEPapyrusInterface*)(*skse).QueryInterface(kInterface_Papyrus) ) )
		{	_MESSAGE("ERROR: couldn't get papyrus interface");
			return (false);
		}
		if ((*g_papyrus_interface).interfaceVersion < SKSEPapyrusInterface::kInterfaceVersion)
		{	_MESSAGE("ERROR: papyrus interface too old (%i expected %i)", (*g_papyrus_interface).interfaceVersion, SKSEPapyrusInterface::kInterfaceVersion);
			return (false);
		}
		return (true);
	}

	#if CASH_GRAB_EDITION
	/*WTF c++! whatever happend to the c-style '.' member initializer?*/
	__declspec(dllexport) SKSEPluginVersionData SKSEPlugin_Version = 
	{
		SKSEPluginVersionData::kVersion,	//dataVersion
		8,									//pluginVersion (Honed Metal 1.26.1)
		"HonedMetal",						//name
		"a_retarded_monkey",				//author
		"404@nowhere.com",					//email
		0,									//versionIndependenceEX
		0,									//versionIndependence
		{TARGET_RUNTIME, 0},				//compatibleVersions
		0,									//setVersionRequired
	};

	# else
	__declspec(dllexport) extern bool SKSEPlugin_Query(const SKSEInterface *skse, PluginInfo *info)
	{	
		(*info).infoVersion = PluginInfo::kInfoVersion;
		(*info).name = "HonedMetal";
		(*info).version = 8;
		return (validate_plugin(skse) );
	}
	#endif

	__declspec(dllexport) extern bool SKSEPlugin_Load(const SKSEInterface *skse)
	{
		#define INIPATH "\\Documents\\My Games\\Skyrim Special Edition\\SKSE\\HonedMetal.log"
		const char path[MAX_PATH + !NULL] = {0};

		static_assert(MAX_PATH > sizeof(INIPATH), "");
		_memccpy((void*)path, getenv("USERPROFILE"), 0, MAX_PATH - sizeof(INIPATH) );
		strcat((char*)path, INIPATH);
		gLog.Open(path);
		#if defined CASH_GRAB_EDITION
		if (!validate_plugin(skse) )
			return (false);
		#endif
		_MESSAGE("Load");
		(*g_scaleform).RegisterForInventory(CraftingUtils::onUIRefresh);
		(*g_papyrus_interface).Register(CraftingUtils::registerPapyrusFunctions);
		return (true);
		(void)skse;
	}
};