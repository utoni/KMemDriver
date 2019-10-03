#pragma once

#include <Windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

struct ISystem;

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
	virtual ISystem* GetSystem() const = 0;
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
typedef Color_tpl<UINT8> ColorB;

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
	virtual void DrawLine(const Vec3& v0, const ColorB& colV0, const Vec3& v1, const ColorB& colV1, float thickness = 1.0f) = 0;
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
	virtual PVOID GetMatrix() = 0;
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

	void Draw2dLabel(float x, float y, float font_size, const ColorF& fColor, bool bCenter, const char* label_text, ...)
	{
		va_list args;
		va_start(args, label_text);
		Draw2dLabel(x, y, font_size, fColor, bCenter, label_text, args);
		va_end(args);
	}
};

struct IRenderer//: public IRendererCallbackServer
{
	virtual ~IRenderer() {}
	virtual void fn_00(void) = 0;
	virtual void fn_01(void) = 0;
	virtual void fn_02(void) = 0;
	virtual void fn_03(void) = 0;
	virtual void fn_04(void) = 0;
	virtual void fn_05(void) = 0;
	virtual void fn_06(void) = 0;
	virtual void fn_07(void) = 0;
	virtual void fn_08(void) = 0;
	virtual void fn_09(void) = 0;
	virtual void fn_10(void) = 0;
	virtual void fn_11(void) = 0;
	virtual int  GetFeatures() = 0;
	virtual void fn_12(void) = 0;
	virtual int  GetNumGeomInstances() = 0;
	virtual int  GetNumGeomInstanceDrawCalls() = 0;
	virtual int  GetCurrentNumberOfDrawCalls() = 0;
	virtual void fn_13(void) = 0;
	virtual void fn_14(void) = 0;
	virtual void fn_15(void) = 0;
	virtual void fn_16(void) = 0;
	virtual void fn_17(void) = 0;
	virtual void fn_18(void) = 0;
	virtual void fn_19(void) = 0;
	virtual void fn_20(void) = 0;
	virtual void fn_21(void) = 0;
	virtual void fn_22(void) = 0;
	virtual void fn_23(void) = 0;
	virtual void fn_24(void) = 0;
	virtual void fn_25(void) = 0;
	virtual void fn_26(void) = 0;
	virtual void fn_27(void) = 0;
	virtual void fn_28(void) = 0;
	virtual void fn_29(void) = 0;
	virtual void fn_30(void) = 0;
	virtual void fn_31(void) = 0;
	virtual void fn_32(void) = 0;
	virtual void fn_33(void) = 0;
	virtual void fn_34(void) = 0;
	virtual void fn_35(void) = 0;
	virtual void fn_36(void) = 0;
	virtual void fn_37(void) = 0;
	virtual int GetWhiteTextureId() const = 0;
	virtual void fn_38(void) = 0;
	virtual void fn_39(void) = 0;
	virtual int GetHeight() const = 0;
	virtual int GetWidth() const = 0;
	virtual float GetPixelAspectRatio() const = 0;
	virtual int GetOverlayHeight() const = 0;
	virtual int GetOverlayWidth() const = 0;
	virtual void GetMemoryUsage(PVOID Sizer) = 0;
	virtual void GetBandwidthStats(float* fBandwidthRequested) = 0;
	virtual void fn_40(void) = 0;
	virtual void fn_41(void) = 0;
	virtual void fn_42(void) = 0;
	virtual int GetColorBpp() = 0;
	virtual int GetDepthBpp() = 0;
	virtual int GetStencilBpp() = 0;
	virtual bool IsStereoEnabled() const = 0;
	virtual float GetNearestRangeMax() const = 0;
	virtual bool ProjectToScreen(
		float ptx, float pty, float ptz,
		float* sx, float* sy, float* sz) = 0;
};

struct IGameFramework
{
	virtual void fn_00(void) = 0;
	virtual void fn_01(void) = 0;
	virtual void fn_02(void) = 0;
	virtual void fn_03(void) = 0;
	virtual void fn_04(void) = 0;
	virtual void fn_05(void) = 0;

	virtual ~IGameFramework() {}
	virtual void fn_06(void) = 0;
	virtual void fn_07(void) = 0;
	virtual void fn_08(void) = 0;
	virtual void fn_09(void) = 0;
	virtual void fn_10(void) = 0;
	virtual void fn_11(void) = 0;
	virtual void fn_12(void) = 0;
	virtual void fn_13(void) = 0;
	virtual void fn_14(void) = 0;
	virtual void fn_15(void) = 0;
	virtual void fn_16(void) = 0;
	virtual bool IsGamePaused() = 0;
	virtual bool IsGameStarted() = 0;
	virtual ISystem* GetISystem() = 0;
	virtual void fn_17(void) = 0;
	virtual void fn_18(void) = 0;
	virtual void fn_19(void) = 0;
	virtual void fn_20(void) = 0;
	virtual void fn_21(void) = 0;
	virtual void fn_22(void) = 0;
	virtual void fn_23(void) = 0;
	virtual void fn_24(void) = 0;
	virtual void fn_25(void) = 0;
	virtual void fn_26(void) = 0;
	virtual void fn_27(void) = 0;
	virtual void fn_28(void) = 0;
	virtual void fn_29(void) = 0;
	virtual void fn_30(void) = 0;
	virtual void fn_31(void) = 0;
	virtual void fn_32(void) = 0;
	virtual void fn_33(void) = 0;
	virtual void fn_34(void) = 0;
	virtual void fn_35(void) = 0;
	virtual void fn_36(void) = 0;
	virtual void fn_37(void) = 0;
	virtual void fn_38(void) = 0;
	virtual void fn_39(void) = 0;
	virtual void fn_40(void) = 0;
	virtual void fn_41(void) = 0;
	virtual void fn_42(void) = 0;
	virtual void fn_43(void) = 0;
	virtual void fn_44(void) = 0;
	virtual void fn_45(void) = 0;
	virtual void fn_46(void) = 0;
	virtual void fn_47(void) = 0;
	virtual void fn_48(void) = 0;
	virtual void fn_49(void) = 0;
	virtual void fn_50(void) = 0;
	virtual void fn_51(void) = 0;
	virtual void fn_52(void) = 0;
	virtual void fn_53(void) = 0;
	virtual void fn_54(void) = 0;
	virtual void fn_55(void) = 0;
	virtual void fn_56(void) = 0;
	virtual void fn_57(void) = 0;
	virtual void fn_58(void) = 0;
	virtual void fn_59(void) = 0;
	virtual void fn_60(void) = 0;
	virtual void fn_61(void) = 0;
	virtual void fn_62(void) = 0;
	virtual PVOID GetClientActor() const = 0;
	virtual int GetClientActorId() const = 0;
	virtual PVOID GetClientEntity() const = 0;
	virtual int GetClientEntityId() const = 0;
	virtual PVOID GetClientChannel() const = 0;
	virtual int GetServerTime() = 0;
	virtual void fn_63(void) = 0;
	virtual void fn_64(void) = 0;
	virtual void fn_65(void) = 0;
	virtual void fn_66(void) = 0;
	virtual void fn_67(void) = 0;
	virtual void fn_68(void) = 0;
	virtual void fn_69(void) = 0;
	virtual void fn_70(void) = 0;
	virtual void fn_71(void) = 0;
	virtual void fn_72(void) = 0;
	virtual void fn_73(void) = 0;
	virtual void fn_74(void) = 0;
	virtual void fn_75(void) = 0;
	virtual void fn_76(void) = 0;
	virtual void fn_77(void) = 0;
	virtual void fn_78(void) = 0;
	virtual void fn_79(void) = 0;
	virtual void fn_80(void) = 0;
	virtual void fn_81(void) = 0;
	virtual bool IsEditing() = 0;
	virtual bool IsInLevelLoad() = 0;
	virtual bool IsLoadingSaveGame() = 0;
	virtual bool IsInTimeDemo() = 0;
	virtual bool IsTimeDemoRecording() = 0;
	virtual void fn_82(void) = 0;
	virtual void fn_83(void) = 0;
	virtual bool CanSave() = 0;
	virtual bool CanLoad() = 0;
	virtual void fn_84(void) = 0;
	virtual bool CanCheat() = 0;
	virtual const char* GetLevelName() = 0;
	virtual void GetAbsLevelPath(char* pPathBuffer, UINT32 pathBufferSize) = 0;
	virtual PVOID GetIPersistantDebug() = 0;
	virtual void fn_85(void) = 0;
	virtual void fn_86(void) = 0;
	virtual void fn_87(void) = 0;
	virtual void fn_88(void) = 0;
	virtual void fn_89(void) = 0;
	virtual void fn_90(void) = 0;
	virtual void fn_91(void) = 0;
	virtual void fn_92(void) = 0;
	virtual void fn_93(void) = 0;
	virtual const char* GetGameGUID() = 0;
};

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
	UINT64 ukn_00;
	UINT64 pLocalMemoryUsage;
	IGameFramework* pGameFramework;
	UINT64 ukn_01;
	IEntitySystem* pEntitySystem;
	ISystem* pSystem;
	UINT64 pCharacterManager;
	UINT64 pAISystem;
	UINT64 pLog;
	UINT64 pCodeCheckpointMgr;
	UINT64 pMovieSystem;
	UINT64 pNameTable;
	IRenderer* pRenderer;
	IRenderAuxGeom* pAuxGeomRenderer;
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

struct ISystem
{
	virtual ~ISystem() {}
	virtual PVOID GetCVarsWhiteListConfigSink() const = 0;
	virtual SSystemGlobalEnvironment* GetGlobalEnvironment() = 0;
	virtual PVOID GetUserCallback() const = 0;
	virtual const char* GetRootFolder() const = 0;
	virtual bool DoFrame(void) = 0;
	virtual void RenderBegin(void) = 0;
	virtual void RenderEnd(bool bRenderStats = true) = 0;
	virtual bool Update(int updateFlags, int nPauseMode = 0) = 0;
	virtual void RenderPhysicsHelpers() = 0;
	virtual PVOID GetManualFrameStepController() const = 0;
	virtual bool UpdateLoadtime() = 0;
	virtual void SynchronousLoadingTick(const char* pFunc, int line) = 0;
	virtual void RenderStatistics() = 0;
	virtual void RenderPhysicsStatistics(PVOID pWorld) = 0;
	virtual UINT32 GetUsedMemory() = 0;
	virtual const char* GetUserName() = 0;
	virtual UINT32 GetCPUFlags() = 0;
	virtual int GetLogicalCPUCount() = 0;
	virtual void DumpMemoryUsageStatistics(bool bUseKB = false) = 0;
	virtual void Quit() = 0;
	virtual void Relaunch(bool bRelaunch) = 0;
	virtual bool IsQuitting() const = 0;
	virtual bool IsShaderCacheGenMode() const = 0;
	virtual void SerializingFile(int mode) = 0;
	virtual int  IsSerializingFile() const = 0;
	virtual bool IsRelaunch() const = 0;
	virtual void DisplayErrorMessage(const char* acMessage, float fTime, const float* pfColor = 0, bool bHardError = true) = 0;
	virtual void FatalError(const char* sFormat, ...) = 0;
	virtual void ReportBug(const char* sFormat, ...) = 0;
	virtual void WarningV(int module, int severity, int flags, const char* file, const char* format, va_list args) = 0;
	virtual void Warning(int module, int severity, int flags, const char* file, const char* format, ...) = 0;
	virtual void WarningOnce(int module, int severity, int flags, const char* file, const char* format, ...) = 0;
	virtual bool CheckLogVerbosity(int verbosity) = 0;
	virtual bool IsUIFrameworkMode() { return false; }
	virtual void FillRandomMT(UINT32* pOutWords, UINT32 numWords) = 0;
	virtual PVOID GetRandomGenerator() = 0;
	virtual PVOID GetIZLibCompressor() = 0;
	virtual PVOID GetIZLibDecompressor() = 0;
	virtual PVOID GetLZ4Decompressor() = 0;
	virtual PVOID GetPerfHUD() = 0;
	virtual PVOID GetMiniGUI() = 0;
	virtual PVOID GetPlatformOS() = 0;
	virtual PVOID GetINotificationNetwork() = 0;
	virtual PVOID GetIHardwareMouse() = 0;
	virtual PVOID GetIDialogSystem() = 0;
	virtual PVOID GetIFlowSystem() = 0;
	virtual PVOID GetIBudgetingSystem() = 0;
	virtual PVOID GetINameTable() = 0;
	virtual PVOID GetIDiskProfiler() = 0;
	virtual PVOID GetProfilingSystem() = 0;
	virtual PVOID GetLegacyProfilerInterface() = 0;
	virtual PVOID GetIValidator() = 0;
	virtual PVOID GetIPhysicsDebugRenderer() = 0;
	virtual PVOID GetIPhysRenderer() = 0;
	virtual PVOID GetIAnimationSystem() = 0;
	virtual PVOID GetStreamEngine() = 0;
	virtual PVOID GetICmdLine() = 0;
	virtual PVOID GetILog() = 0;
	virtual PVOID GetIPak() = 0;
	virtual PVOID GetICryFont() = 0;
	virtual IEntitySystem* GetIEntitySystem() = 0;
	virtual PVOID GetIMemoryManager() = 0;
	virtual PVOID GetAISystem() = 0;
	virtual PVOID GetIMovieSystem() = 0;
	virtual PVOID GetIPhysicalWorld() = 0;
	virtual PVOID GetIAudioSystem() = 0;
	virtual PVOID GetI3DEngine() = 0;
	virtual PVOID GetIScriptSystem() = 0;
	virtual PVOID GetIConsole() = 0;
	virtual PVOID GetIRemoteConsole() = 0;
	virtual PVOID GetIUserAnalyticsSystem() = 0;
	virtual PVOID GetIPluginManager() = 0;
	virtual PVOID GetIProjectManager() = 0;
	virtual PVOID GetIUDR() = 0;
	virtual PVOID GetIResourceManager() = 0;
	virtual PVOID GetISystemEventDispatcher() = 0;
	virtual PVOID GetIFileChangeMonitor() = 0;
	virtual PVOID GetHWND() = 0;
	virtual PVOID GetActiveHWND() = 0;
	virtual PVOID GetINetwork() = 0;
	virtual IRenderer* GetIRenderer() = 0;
	virtual PVOID GetIInput() = 0;
	virtual PVOID GetITimer() = 0;
	virtual PVOID GetIThreadManager() = 0;
	virtual PVOID GetIMonoEngineModule() = 0;

	/* some more virtual functions */
};
