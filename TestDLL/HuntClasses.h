#pragma once

#include <Windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


struct SSystemGlobalEnvironment {
	UINT64 pDialogSystem;
	UINT64 p3DEngine;
	UINT64 pNetwork;
	UINT64 pNetContext;
	UINT64 pLobby;
	UINT64 pScriptSystem;
	UINT64 pPhysicalWorld;
	UINT64 pFlowSystem;
	UINT64 pInput;
	UINT64 pStatoscope;
	UINT64 pCryPak;
	UINT64 pFileChangeMonitor;
	UINT64 pParticleManager;
	UINT64 pOpticsManager;
	UINT64 pTimer;
	UINT64 pCryFont;
	UINT64 pGameFramework;
	UINT64 pLocalMemoryUsage;
	UINT64 pEntitySystem;
	UINT64 pConsole;
	UINT64 pAudioSystem;
	UINT64 pSystem;
	UINT64 pCharacterManager;
	UINT64 pAISystem;
	UINT64 pLog;
	UINT64 pCodeCheckpointMgr;
	UINT64 pMovieSystem;
	UINT64 pNameTable;
	UINT64 pRenderer;
	UINT64 pAuxGeomRenderer;
	UINT64 pHardwareMouse;
	UINT64 pMaterialEffects;
	UINT64 pJobManager;
	UINT64 pOverloadSceneManager;
	UINT64 pFlashUI;
	UINT64 pUIFramework;
	UINT64 pServiceNetwork;
	UINT64 pRemoteCommandManager;
	UINT64 pDynamicResponseSystem;
	UINT64 pThreadManager;
	UINT64 pScaleformHelper;
	UINT64 pSchematyc;
	UINT64 pSchematyc2;
	UINT64 pReflection;
	UINT64 pScriptCoreEnv;
	UINT64 pScriptCoreRegistry;
	UINT64 pUDR;
	UINT64 pLiveCreateManager;
	UINT64 pLiveCreateHost;
	UINT64 pMonoRuntime;
	UINT32 mMainThreadId;
	UINT32 nMainFrameID;
	UINT64 szCmdLine;
	CHAR szDebugStatus[128];
	BOOL bServer;
	BOOL bMultiplayer;
	BOOL bHostMigrating;
	UCHAR padding_00;
	UINT64 startProfilingSection;
	UINT64 endProfilingSection;
	UINT64 recordProfilingMarker;
	BOOL bUnattendedMode;
	BOOL bTesting;
	BOOL bNoRandomSeed;
	UCHAR padding_01;
	UINT32 assertSettings;
	UCHAR platformInfo[286];
	UCHAR protectedFunctions[80];
	BOOL bIsOutOfMemory;
	BOOL bIsOutOfVideoMemory;
	BOOL bDedicatedArbitrator;
	BOOL bEditor;
	BOOL bEditorGameMode;
	BOOL bEditorSimulationMode;
	BOOL bDedicated;
	BOOL bClient;
	BOOL m_isFMVPlaying;
	BOOL m_isCutscenePlaying;
};

struct IEntitySystem
{
	enum SinkEventSubscriptions : UINT32
	{
		OnBeforeSpawn = 1,
		OnSpawn = 2,
		OnRemove = 4,
		OnReused = 8,

		Last = OnReused,
		Count = 4,

		AllSinkEvents = ~0u,
	};

	virtual ~IEntitySystem() {}
	virtual void Release() = 0;
	virtual void PrePhysicsUpdate() = 0;
	virtual void Update() = 0;
	virtual void Reset() = 0;
	virtual void Unload() = 0;
	virtual void PurgeHeaps() = 0;
	virtual void DeletePendingEntities() = 0;
	virtual PVOID GetClassRegistry() = 0;
	virtual PVOID GetReflectionRegistry() const = 0;
	virtual PVOID SpawnEntity(void) = 0;
	virtual bool InitEntity(void) = 0;
	virtual PVOID GetEntity(void) const = 0;
	virtual PVOID FindEntityByName(void) const = 0;
	virtual void ReserveEntityId(void) = 0;
	virtual int ReserveNewEntityId() = 0;
	virtual void RemoveEntity(void) = 0;
	virtual unsigned int GetNumEntities(void) const = 0;
	virtual PVOID GetEntityIterator() = 0;
	virtual void SendEventToAll(void) = 0;
	virtual void OnLevelLoaded() = 0;
	virtual void OnLevelGameplayStart() = 0;
	virtual int QueryProximity(void) = 0;
	virtual void ResizeProximityGrid(int, int) = 0;
	virtual int GetPhysicalEntitiesInBox(void) const = 0;
	virtual PVOID GetEntityFromPhysics(PVOID) const = 0;
	virtual void AddSink(PVOID) = 0;
	virtual void RemoveSink(PVOID) = 0;
	virtual void PauseTimers(bool bPause, bool bResume = false) = 0;
	virtual bool IsIDUsed(int) const = 0;
	virtual void GetMemoryStatistics(PVOID) const = 0;
	virtual PVOID GetSystem() const = 0;
	virtual bool ExtractArcheTypeLoadParams(void) const = 0;
	virtual bool ExtractEntityLoadParams(void) const = 0;
	virtual void BeginCreateEntities(int amtToCreate) = 0;
	virtual bool CreateEntity(void) = 0;
	virtual void EndCreateEntities() = 0;
	virtual void LoadEntities(bool bIsLoadingLevelFile) = 0;
	virtual void LoadEntities(bool bIsLoadingLevelFile, const int) = 0;
	virtual void AddEntityLayerListener(PVOID, PVOID, const bool bCaseSensitive = true) = 0;
	virtual void RemoveEntityLayerListener(PVOID, PVOID, const bool bCaseSensitive = true) = 0;
	virtual int FindEntityByGuid(int) const = 0;
	virtual PVOID GetAreaManager() const = 0;
	virtual PVOID GetBreakableManager() const = 0;
	virtual PVOID LoadEntityArchetype(void) = 0;
	virtual PVOID LoadEntityArchetype(const char* sArchetype) = 0;
	virtual void  UnloadEntityArchetype(const char* sArchetype) = 0;
	virtual PVOID CreateEntityArchetype(PVOID, const char* sArchetype) = 0;
	virtual void  RefreshEntityArchetypesInRegistry() = 0;
	virtual void  SetEntityArchetypeManagerExtension(PVOID) = 0;
	virtual PVOID GetEntityArchetypeManagerExtension() const = 0;
	virtual void Serialize(void) = 0;
	virtual void SetNextSpawnId(int id) = 0;
	virtual void ResetAreas() = 0;
	virtual void UnloadAreas() = 0;
	virtual void DumpEntities() = 0;
	virtual void LockSpawning(bool lock) = 0;
	virtual bool OnLoadLevel(const char* szLevelPath) = 0;
	virtual PVOID AddLayer(const char* szName, const char* szParent, UINT16 id, bool bHasPhysics, int specs, bool bDefaultLoaded) = 0;
	virtual void LoadLayers(const char* dataFile) = 0;
	virtual void LinkLayerChildren() = 0;
	virtual void AddEntityToLayer(const char* layer, int id) = 0;
	virtual void RemoveEntityFromLayers(int id) = 0;
	virtual void ClearLayers() = 0;
	virtual void EnableDefaultLayers(bool isSerialized = true) = 0;
	virtual void EnableLayer(const char* layer, bool isEnable, bool isSerialized = true) = 0;
	virtual void EnableLayerSet(const char* const* pLayers, size_t layerCount, bool isSerialized = true, PVOID pListener = nullptr) = 0;
	virtual void EnableScopedLayerSet(const char* const* pLayers, size_t layerCount, const char* const* pScopeLayers, size_t scopeLayerCount, bool isSerialized = true, PVOID pListener = nullptr) = 0;
	virtual PVOID FindLayer(const char* szLayerName, const bool bCaseSensitive = true) const = 0;
	virtual bool IsLayerEnabled(const char* layer, bool bMustBeLoaded, bool bCaseSensitive = true) const = 0;
	virtual bool ShouldSerializedEntity(PVOID pEntity) = 0;
	virtual void RegisterPhysicCallbacks() = 0;
	virtual void UnregisterPhysicCallbacks() = 0;
	virtual void PurgeDeferredCollisionEvents(bool bForce = false) = 0;
	virtual void ResumePhysicsForSuppressedEntities(bool bWakeUp) = 0;
	virtual void SaveInternalState(struct IDataWriteStream& writer) const = 0;
	virtual void LoadInternalState(struct IDataReadStream& reader) = 0;
	virtual int GetLayerId(const char* szLayerName) const = 0;
	virtual const char* GetLayerName(int layerId) const = 0;
	virtual int GetLayerChildCount(const char* szLayerName) const = 0;
	virtual const char* GetLayerChild(const char* szLayerName, int childIdx) const = 0;
	virtual int GetVisibleLayerIDs(UINT8* pLayerMask, const UINT32 maxCount) const = 0;
	virtual void ToggleLayerVisibility(const char* layer, bool isEnabled, bool includeParent = true) = 0;
	virtual void ToggleLayersBySubstring(const char* pSearchSubstring, const char* pExceptionSubstring, bool isEnable) = 0;
	virtual PVOID CreateBSPTree3D(void) = 0;
	virtual void ReleaseBSPTree3D(PVOID) = 0;

	//! Represents a unique identifier for a static entity loaded from disk
	//! Used to quickly identify static entities over the network, instead of needing to send over long GUIDs
	using StaticEntityNetworkIdentifier = UINT16;

	//! Queries an entity identifier from its static entity network id, most likely sent by the server
	virtual int GetEntityIdFromStaticEntityNetworkId(StaticEntityNetworkIdentifier id) const = 0;
};

template<typename T, int N>
struct INumberArray
{
};

template<typename T, int N, typename Final>
struct INumberVector : INumberArray<T, N>
{
};

template<typename T> struct Vec2_tpl;
template<class F> struct Vec2_tpl
	: INumberVector<F, 2, Vec2_tpl<F>>
{
	F x, y;
};
typedef Vec2_tpl<float> Vec2;

template<typename F> struct Vec3_tpl;
template<typename F> struct Vec3_tpl
	: INumberVector<F, 3, Vec3_tpl<F>>
{
	F x, y, z;
};
typedef Vec3_tpl<float> Vec3;

struct SDrawTextInfo
{
	//! One of EDrawTextFlags flags.
	int flags;

	//! Text color, (r,g,b,a) all members must be specified.
	float   color[4];
	Vec2    scale;
	PVOID   pFont;

	SDrawTextInfo()
	{
		flags = 0;
		color[0] = color[1] = color[2] = color[3] = 1;
		scale.x = 0.0f;
		scale.y = 0.0f;
		pFont = nullptr;
	}
};

template<class T> struct Color_tpl
{
	T r, g, b, a;
};
typedef Color_tpl<float> ColorF;

enum EDrawTextFlags : UINT32
{
	eDrawText_Default,
	eDrawText_Center = 1,
	eDrawText_Right = 2,
	eDrawText_CenterV = 4,
	eDrawText_Bottom = 8,
	eDrawText_2D = 16,
	eDrawText_FixedSize = 32,
	eDrawText_800x600 = 64,
	eDrawText_Monospace = 128,
	eDrawText_Framed = 256,
	eDrawText_DepthTest = 512,
	eDrawText_IgnoreOverscan = 1024,
	eDrawText_LegacyBehavior = 2048
};

class IRenderAuxGeom
{
public:
	virtual ~IRenderAuxGeom() {}
	virtual void SetRenderFlags(void) = 0;
	virtual void GetRenderFlags() = 0;
	virtual const void GetCamera() const = 0;
	virtual void SetCurrentDisplayContext(void) = 0;
	virtual void DrawPoint(void) = 0;
	virtual void DrawPoints(void) = 0;
	virtual void DrawPoints(int) = 0;
	virtual void DrawLine(void) = 0;
	virtual void DrawLines(void) = 0;
	virtual void DrawLines(int) = 0;
	virtual void DrawLines(int, int) = 0;
	virtual void DrawLines(int, int, int) = 0;
	virtual void DrawLines(int, int, int, int) = 0;
	virtual void DrawLineStrip(void) = 0;
	virtual void DrawPolyline(void) = 0;
	virtual void DrawPolyline(int) = 0;
	virtual void DrawTriangle(void) = 0;
	virtual void DrawTriangles(void) = 0;
	virtual void DrawTriangles(int) = 0;
	virtual void DrawTriangles(int, int) = 0;
	virtual void DrawTriangles(int, int, int) = 0;
	virtual void DrawBuffer(void) = 0;
	virtual void BeginDrawBuffer(void) = 0;
	virtual void EndDrawBuffer(void) = 0;
	virtual void DrawAABB(void) = 0;
	virtual void DrawAABBs(void) = 0;
	virtual void DrawAABB(int) = 0;
	virtual void DrawOBB(void) = 0;
	virtual void DrawOBB(int) = 0;
	virtual void DrawSphere(void) = 0;
	virtual void DrawCone(void) = 0;
	virtual void DrawCylinder(void) = 0;
	virtual void DrawBone(void) = 0;
	virtual void RenderTextQueued(Vec3 pos, const SDrawTextInfo& ti, const char* text) = 0;
	virtual int PushMatrix(void) = 0;
	virtual void PushImage(void) = 0;
	virtual int SetTexture(void) { return -1; }
	virtual void GetMatrix() = 0;
	virtual void SetMatrixIndex(void) = 0;
	virtual void Submit(void) = 0;
	virtual void SetOrthographicProjection(void) = 0;

	void RenderText(Vec3 pos, const SDrawTextInfo& ti, const char* format, va_list args)
	{
		if (format)
		{
			char str[512];

			vsnprintf(str, sizeof str, format, args);

			RenderTextQueued(pos, ti, str);
		}
	}

	void Draw2dLabel(float x, float y, float font_size, const ColorF& fColor, bool bCenter, const char* format, va_list args)
	{
		SDrawTextInfo ti;
		ti.scale.x = font_size;
		ti.scale.y = font_size;
		ti.flags = eDrawText_2D | eDrawText_800x600 | eDrawText_FixedSize | ((bCenter) ? eDrawText_Center : 0);
		ti.color[0] = fColor.r;
		ti.color[1] = fColor.g;
		ti.color[2] = fColor.b;
		ti.color[3] = fColor.a;
		Vec3 pos;
		pos.x = x;
		pos.y = y;
		pos.z = 0.5f;

		RenderText(pos, ti, format, args);
	}
};