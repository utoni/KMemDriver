#pragma once

#include <Windows.h>
#include <stdarg.h>

#include <string>
#include <map>

struct ISystem;


#define ENTITY_FLAG_LOCAL_PLAYER      0x8000000

#define PENTITYSYSTEM_ISYSTEM_OFFSET 104

template<class _I> class _smart_ptr
{
private:
	_I* p;
public:
	_smart_ptr() : p(NULL) {}
	_smart_ptr(_I* p_)
	{
		p = p_;
		if (p)
			p->AddRef();
	}
	_smart_ptr(const _smart_ptr& p_)
	{
		p = p_.p;
		if (p)
			p->AddRef();
	}
	_smart_ptr(_smart_ptr&& p_) noexcept
	{
		p = p_.p;
		p_.p = nullptr;
	}
	template<typename _Y>
	_smart_ptr(const _smart_ptr<_Y>& p_)
	{
		p = p_.get();
		if (p)
			p->AddRef();
	}
	~_smart_ptr()
	{
		if (p)
			p->Release();
	}
	operator _I*() const { return p; }

	_I&         operator*() const { return *p; }
	_I*         operator->(void) const { return p; }
	_I*         get() const { return p; }
	_smart_ptr& operator=(_I* newp)
	{
		if (newp != this->p)
		{
			_I* oldp = p;
			p = newp;
			if (p)
				p->AddRef();
			if (oldp)
				oldp->Release();
		}
		return *this;
	}
	void reset()
	{
		_smart_ptr<_I>().swap(*this);
	}
	void reset(_I* p)
	{
		if (p != this->p)
		{
			_smart_ptr<_I>(p).swap(*this);
		}
	}
	_smart_ptr& operator=(const _smart_ptr& newp)
	{
		if (newp.p != this->p)
		{
			if (newp.p)
				newp.p->AddRef();
			if (p)
				p->Release();
			p = newp.p;
		}
		return *this;
	}
	_smart_ptr& operator=(_smart_ptr&& p_)
	{
		if (this != &p_)
		{
			if (p)
				p->Release();
			p = p_.p;
			p_.p = nullptr;
		}
		return *this;
	}
	template<typename _Y>
	_smart_ptr& operator=(const _smart_ptr<_Y>& newp)
	{
		_I* const p2 = newp.get();
		if (p2 != this->p)
		{
			if (p2)
				p2->AddRef();
			if (p)
				p->Release();
			p = p2;
		}
		return *this;
	}
	void swap(_smart_ptr<_I>& other)
	{
		std::swap(p, other.p);
	}
	void Assign_NoAddRef(_I* ptr)
	{
		CRY_ASSERT(!p, "Assign_NoAddRef should only be used on a default-constructed, not-yet-assigned smart_ptr instance");
		p = ptr;
	}
	_I* ReleaseOwnership()
	{
		_I* ret = p;
		p = 0;
		return ret;
	}
};

template<typename T, int N>
struct INumberArray
{
};

template<typename T, int N, typename Final>
struct INumberVector : INumberArray<T, N>
{
};

template<typename F> struct Vec3_tpl
	: INumberVector<F, 3, Vec3_tpl<F>>
{
public:
	F x, y, z;
	Vec3_tpl(F vx, F vy, F vz) : x(vx), y(vy), z(vz) {}
};
typedef Vec3_tpl<float> Vec3;

template<typename F> struct Ang3_tpl
	: INumberVector<F, 3, Ang3_tpl<F>>
{
public:
	F x, y, z;
	Ang3_tpl(F vx, F vy, F vz) : x(vx), y(vy), z(vz) {}
};
typedef Ang3_tpl<float> Ang3;

template<typename F> struct Quat_tpl
	: INumberVector<F, 4, Quat_tpl<F>>
{
public:
	Vec3_tpl<F> v;
	F           w;
};
typedef Quat_tpl<float> Quat;

class Matrix34 {
public:
	float m00;
	float m01;
	float m02;
	float m03;
	float m10;
	float m11;
	float m12;
	float m13;
	float m20;
	float m21;
	float m22;
	float m23;
	Vec3 GetTranslation() const { return Vec3(m03, m13, m23); }
};

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

template<class T> struct Color_tpl
{
	T r, g, b, a;
	Color_tpl() {};
	Color_tpl(T _r, T _g, T _b, T _a) : r(_r), g(_g), b(_b), a(_a) {}
};

typedef Color_tpl<UINT8>  ColorB;

struct SAuxGeomRenderFlags
{
	UINT32 m_renderFlags;
};

class IRenderAuxGeom
{
public:
	virtual ~IRenderAuxGeom() {}
	virtual SAuxGeomRenderFlags  SetRenderFlags(const SAuxGeomRenderFlags& renderFlags) = 0;
	virtual SAuxGeomRenderFlags  GetRenderFlags() = 0;
	virtual PVOID GetCamera() const = 0;
};

struct IEntity
{
public:
	virtual ~IEntity() {}
	virtual int GetId() const = 0;
	virtual const PVOID GetGuid() const = 0;
	virtual PVOID GetClass() const = 0;
	virtual void fn_00(void) = 0;
	virtual void fn_01(void) = 0;
	virtual UINT32 GetFlags() const = 0;
	virtual void fn_02(void) = 0;
	virtual void fn_03(void) = 0;
	virtual void fn_04(void) const = 0;
	virtual void fn_05(void) = 0;
	virtual UINT32 GetFlagsExtended() const = 0;
	virtual bool IsInitialized() const = 0;
	virtual bool IsGarbage() const = 0;
	virtual UINT8 GetComponentChangeState() const = 0;
	virtual void fn_06(void) = 0;
	virtual const char* GetName() const = 0;
	virtual std::string GetEntityTextDescription() const = 0;
	virtual void fn_07(void) = 0;
	virtual bool IsLoadedFromLevelFile() const = 0;
	virtual void fn_08(void) = 0;
	virtual void fn_09(void) = 0;
	virtual void fn_10(void) = 0;
	virtual int GetChildCount() const = 0;
	virtual IEntity* GetChild(int nIndex) const = 0;
	virtual IEntity* GetParent() const = 0;
	virtual IEntity* GetLocalSimParent() const = 0;
	virtual Matrix34 GetParentAttachPointWorldTM() const = 0;
	virtual bool IsParentAttachmentValid() const = 0;
	virtual void fn_11(void) = 0;
	virtual void fn_12(void) = 0;
	virtual const Matrix34& GetWorldTM() const = 0;
	virtual Matrix34 GetLocalTM() const = 0;
	virtual void fn_13(void) const = 0;
	virtual void fn_14(void) const = 0;
	virtual void fn_15(void) = 0;
	virtual void fn_16(void) = 0;
	virtual void fn_17(void) = 0;
	virtual const Vec3& GetPos() const = 0;
	virtual void fn_18(void) = 0;
	virtual void fn_19(void) const = 0;
	virtual void fn_20(void) = 0;
	virtual const Vec3& GetScale() const = 0;
	virtual void fn_21(void) = 0;
	virtual Vec3 GetWorldPos() const = 0;
	virtual Ang3 GetWorldAngles() const = 0;
	virtual Quat GetWorldRotation() const = 0;
	virtual Vec3 GetWorldScale() const = 0;
	//virtual IScriptTable* GetScriptTable() const final;
	//GetScriptTable -> VirtualFuncIndex 64
};

struct IEntityIt
{
	virtual ~IEntityIt() {}
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual bool IsEnd() = 0;
	virtual IEntity* Next() = 0;
	virtual IEntity* This() = 0;
	virtual void MoveFirst() = 0;
};

typedef _smart_ptr<IEntityIt> IEntityItPtr;

struct IEntitySystem
{
	virtual ~IEntitySystem() {}
	virtual void fn_00(void) = 0;
	virtual void fn_01(void) = 0;
	virtual void fn_02(void) = 0;
	virtual void fn_03(void) = 0;
	virtual void fn_04(void) = 0;
	virtual void fn_05(void) = 0;
	virtual void fn_06(void) = 0;
	virtual void fn_07(void) = 0;
	virtual void fn_08(void) const = 0;
	virtual void fn_09(void) = 0;
	virtual void fn_10(void) = 0;
	virtual PVOID GetEntity(void) const = 0;
	virtual PVOID FindEntityByName(void) const = 0;
	virtual void fn_11(void) = 0;
	virtual void fn_12(void) = 0;
	virtual void fn_13(void) = 0;
	virtual UINT32 GetNumEntities(void) const = 0;
	virtual IEntityItPtr GetEntityIterator() = 0;
	virtual void fn_14(void) = 0;
	virtual void fn_15(void) = 0;
	virtual void fn_16(void) = 0;
	virtual void fn_17(void) = 0;
	virtual void fn_18(void) = 0;
	virtual void fn_19(void) const = 0;
	virtual void fn_20(void) const = 0;
	virtual void fn_21(void) = 0;
	virtual void fn_22(void) = 0;
	virtual void fn_23(void) = 0;
	virtual void fn_24(void) = 0;
	virtual void fn_25(void) const = 0;
	virtual ISystem* GetSystem() const = 0;
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
	virtual int UnProject(
		float sx, float sy, float sz,
		float* px, float* py, float* pz,
		const float modelMatrix[16],
		const float projMatrix[16],
		const int viewport[4]) = 0;
	virtual int UnProjectFromScreen(
		float sx, float sy, float sz,
		float* px, float* py, float* pz) = 0;
	virtual void fn_43(void) = 0;
	virtual void fn_44(void) = 0;
	virtual void fn_45(void) = 0;
	virtual void fn_46(void) = 0;
	virtual void fn_47(void) = 0;
	virtual void fn_48(void) = 0;
	virtual void fn_49(void) = 0;
	virtual void fn_50(void) = 0;
	virtual int  CurThreadList() = 0;
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
	virtual void fn_79(void) const = 0;
	virtual void fn_80(void) = 0;
	virtual void fn_81(void) = 0;
	virtual void fn_82(void) = 0;
	virtual void fn_83(void) = 0;
	virtual void fn_84(void) = 0;
	virtual void fn_85(void) = 0;
	virtual void fn_86(void) = 0;
	virtual void fn_87(void) = 0;
	virtual void fn_88(void) = 0;
	virtual void fn_89(void) = 0;
	virtual void fn_90(void) = 0;
	virtual void fn_91(void) = 0;
	virtual void fn_92(void) = 0;
	virtual void fn_93(void) = 0;
	virtual void fn_94(void) = 0;
	virtual void fn_95(void) = 0;
	virtual void fn_96(void) = 0;
	virtual void fn_97(void) = 0;
	virtual void fn_98(void) = 0;
	virtual void fn_99(void) = 0;
	virtual void fn_100(void) = 0;
	virtual void fn_101(void) = 0;
	virtual void fn_102(void) = 0;
	virtual void fn_103(void) = 0;
	virtual void fn_104(void) const = 0;
	virtual void fn_105(void) = 0;
	virtual void fn_106(void) = 0;
	virtual void fn_107(void) = 0;
	virtual void fn_108(void) = 0;
	virtual void fn_109(void) = 0;
	virtual void fn_110(void) = 0;
	virtual void fn_111(void) = 0;
	virtual void fn_112(void) = 0;
	virtual void fn_113(void) = 0;
	virtual void fn_114(void) = 0;
	virtual void fn_115(void) = 0;
	virtual void fn_116(void) = 0;
	virtual void fn_117(void) = 0;
	virtual void fn_118(void) = 0;
	virtual void fn_119(void) = 0;
	virtual void fn_120(void) = 0;
	virtual void fn_121(void) = 0;
	virtual int  GetPolyCount() = 0;
	virtual void fn_122(void) = 0;
	virtual void fn_123(void) = 0;
	virtual void fn_124(void) = 0;
	virtual int  GetFrameID(bool bIncludeRecursiveCalls = true) = 0;
	virtual void fn_125(void) = 0;
	virtual float ScaleCoordX(float value) const = 0;
	virtual float ScaleCoordY(float value) const = 0;
	virtual void  ScaleCoord(float& x, float& y) const = 0;
	virtual void fn_126(void) = 0;;
	virtual void fn_127(void) = 0;
	virtual void fn_128(void) = 0;
	virtual void fn_129(void) = 0;
	virtual IRenderAuxGeom* GetIRenderAuxGeom() = 0;
	virtual IRenderAuxGeom* GetOrCreateIRenderAuxGeom(const PVOID pCustomCamera = nullptr) = 0;
	virtual void DeleteAuxGeom(IRenderAuxGeom* pRenderAuxGeom) = 0;
	virtual void SubmitAuxGeom(IRenderAuxGeom* pRenderAuxGeom, bool merge = true) = 0;
};

/* generated with: "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\bin\Hostx64\x64\cl.exe" /Zp2 /c /d1reportSingleClassLayoutIActor C:\Users\segfault\Source\Repos\CRYENGINE\Code\CryEngine\CryAction\ActorSystem.cpp /I C:\Users\segfault\Source\Repos\CRYENGINE\Code\CryEngine\CryCommon /I "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023\include" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\ucrt" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\shared" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um" /I "C:\Users\segfault\Source\Repos\CRYENGINE\Code\CryEngine\CryAction" */
struct IActor
{
	virtual void fn_00(void) = 0;
	virtual ~IActor(void) = 0;
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
	virtual void fn_12(void) = 0;
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
	virtual void IActor_SetChannelId(void) = 0;
	virtual void fn_34(void) = 0;
	virtual void fn_35(void) = 0;
	virtual void fn_36(void) = 0;

	virtual void                  SetHealth(float health) = 0;
	virtual float                 GetHealth() const = 0;
	virtual int                   GetHealthAsRoundedPercentage() const = 0;
	virtual void                  SetMaxHealth(float maxHealth) = 0;
	virtual float                 GetMaxHealth() const = 0;
	virtual int                   GetArmor() const = 0;
	virtual int                   GetMaxArmor() const = 0;
	virtual int                   GetTeamId() const = 0;
	virtual bool                  IsFallen() const = 0;
	virtual bool                  IsDead() const = 0;
	virtual int                   IsGod() = 0;
	virtual void                  Fall(Vec3 hitPos = Vec3(0, 0, 0)) = 0;
	virtual bool                  AllowLandingBob() = 0;
	virtual void                  PlayAction(const char* action, const char* extension, bool looping = false) = 0;
	virtual PVOID                 GetAnimationGraphState() = 0;
	virtual void                  ResetAnimationState() = 0;
	virtual void                  CreateScriptEvent(const char* event, float value, const char* str = NULL) = 0;
	virtual bool                  BecomeAggressiveToAgent(int entityID) = 0;
	virtual void                  SetFacialAlertnessLevel(int alertness) = 0;
	virtual void                  RequestFacialExpression(const char* pExpressionName = NULL, float* sequenceLength = NULL) = 0;
	virtual void                  PrecacheFacialExpression(const char* pExpressionName) = 0;
	virtual int                   GetGrabbedEntityId() const = 0;
	virtual void                  HideAllAttachments(bool isHiding) = 0;
	virtual void                  SetIKPos(const char* pLimbName, const Vec3& goalPos, int priority) = 0;
	virtual void                  SetViewInVehicle(PVOID viewRotation) = 0;
	virtual void                  SetViewRotation(PVOID rotation) = 0;
	virtual PVOID                 GetViewRotation() const = 0;
	virtual bool                  IsFriendlyEntity(int entityId, bool bUsingAIIgnorePlayer = true) const = 0;
	virtual Vec3                  GetLocalEyePos() const = 0;
	virtual void                  CameraShake(float angle, float shift, float duration, float frequency, Vec3 pos, int ID, const char* source = "") = 0;
	virtual PVOID                 GetHolsteredItem() const = 0;
	virtual void                  HolsterItem(bool holster, bool playSelect = true, float selectSpeedBias = 1.0f, bool hideLeftHandObject = true) = 0;
	virtual PVOID                 GetCurrentItem(bool includeVehicle = false) const = 0;
	virtual bool                  DropItem(int itemId, float impulseScale = 1.0f, bool selectNext = true, bool byDeath = false) = 0;
	virtual PVOID                 GetInventory() const = 0;
	virtual void                  NotifyCurrentItemChanged(PVOID newItem) = 0;
	virtual PVOID                 GetMovementController() const = 0;
	virtual IEntity*              LinkToVehicle(int vehicleId) = 0;
	virtual IEntity*              GetLinkedEntity() const = 0;
	virtual UINT8                 GetSpectatorMode() const = 0;
	virtual bool                  IsThirdPerson() const = 0;
	virtual void                  ToggleThirdPerson() = 0;
	virtual bool                  IsStillWaitingOnServerUseResponse() const { return false; }
	virtual void                  SetStillWaitingOnServerUseResponse(bool waiting) { UNREFERENCED_PARAMETER(waiting); }
	virtual void                  SetFlyMode(UINT8 flyMode) { UNREFERENCED_PARAMETER(flyMode); };
	virtual UINT8                 GetFlyMode() const { return 0; };
	virtual void                  Release() = 0;
	virtual bool                  IsPlayer() const = 0;
	virtual bool                  IsClient() const = 0;
	virtual bool                  IsMigrating() const = 0;
	virtual void                  SetMigrating(bool isMigrating) = 0;
	virtual void                  InitLocalPlayer() = 0;
	virtual const char*           GetActorClassName() const = 0;
	virtual PVOID                 GetActorClass() const = 0;
	virtual const char*           GetEntityClassName() const = 0;
	virtual void                  SerializeLevelToLevel(PVOID ser) = 0;
	virtual void                  ProcessEvent(const PVOID event) = 0;
	virtual PVOID                 GetAnimatedCharacter() = 0;
	virtual const PVOID           GetAnimatedCharacter() const = 0;
	virtual void                  PlayExactPositioningAnimation(const char* sAnimationName, bool bSignal, const Vec3& vPosition, const Vec3& vDirection, float startWidth, float startArcAngle, float directionTolerance) = 0;
	virtual void                  CancelExactPositioningAnimation() = 0;
	virtual void                  PlayAnimation(const char* sAnimationName, bool bSignal) = 0;
	virtual bool                  Respawn() { return false; }
	virtual void                  ResetToSpawnLocation() {}
	virtual bool                  CanBreakGlass() const { return false; }
	virtual bool                  MustBreakGlass() const { return false; }
	virtual void                  EnableTimeDemo(bool bTimeDemo) = 0;
	void                          SetChannelId(UINT16 id) { UNREFERENCED_PARAMETER(id); }
	virtual void                  SwitchDemoModeSpectator(bool activate) = 0;
	virtual void                  SetCustomHead(const char* customHead) { UNREFERENCED_PARAMETER(customHead); };
	virtual PVOID                 GetLinkedVehicle() const = 0;
	virtual bool                  GetValidPositionNearby(const Vec3& proposedPosition, Vec3& adjustedPosition) const = 0;
	virtual void                  SetExpectedPhysicsPos(const Vec3& expectedPosition) = 0;
	virtual void                  OnAIProxyEnabled(bool enabled) = 0;
	virtual void                  OnReturnedToPool() = 0;
	virtual void                  OnPreparedFromPool() = 0;
	virtual void                  OnShiftWorld() {};
	virtual void                  MountedGunControllerEnabled(bool val) { UNREFERENCED_PARAMETER(val); };
	virtual bool                  MountedGunControllerEnabled() const { return false; }
	virtual bool                  ShouldMuteWeaponSoundStimulus() const = 0;
	virtual int                   GetPhysicalSkipEntities(PVOID pSkipList, const int maxSkipSize) const
	{
		UNREFERENCED_PARAMETER(pSkipList);
		UNREFERENCED_PARAMETER(maxSkipSize);
		return 0;
	}
	virtual void                  OnReused(IEntity* pEntity, PVOID params) = 0;
	virtual bool                  IsInteracting() const = 0;
};

struct IActorIterator
{
	virtual ~IActorIterator() {}
	virtual size_t  Count() = 0;
	virtual IActor* Next() = 0;
	virtual void    AddRef() = 0;
	virtual void    Release() = 0;
};
typedef _smart_ptr<IActorIterator> IActorIteratorPtr;

typedef std::map<int, IActor*> TActorMap;

struct IActorSystem
{
	virtual ~IActorSystem() {}
	virtual void                   Reset() = 0;
	virtual void                   Reload() = 0;
	virtual IActor*                GetActor(int entityId) = 0;
	virtual IActor*                GetActorByChannelId(UINT16 channelId) = 0;
	virtual IActor*                fn_00(void) = 0;
	virtual int                    GetActorCount() const = 0;
	virtual IActorIteratorPtr      CreateActorIterator() = 0;
	virtual void                   SetDemoPlaybackMappedOriginalServerPlayer(int id) = 0;
	virtual int                    GetDemoPlaybackMappedOriginalServerPlayer() const = 0;
	virtual void                   SwitchDemoSpectator(int id = 0) = 0;
	virtual IActor*                GetCurrentDemoSpectator() = 0;
	virtual IActor*                GetOriginalDemoSpectator() = 0;
	virtual void                   AddActor(int entityId, IActor* pActor) = 0;
	virtual void                   RemoveActor(int entityId) = 0;
	virtual void                   Scan(const char* folderName) = 0;
	virtual bool                   fn_01(void) = 0;
	virtual const PVOID            GetActorParams(const char* actorClass) const = 0;
	virtual bool                   IsActorClass(PVOID pClass) const = 0;
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
	virtual IActorSystem* GetIActorSystem(void) = 0;
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
	virtual void fn_62_1(void) = 0;
	virtual void fn_63(void) = 0;
	virtual IActor* GetClientActor() const = 0;
	virtual int GetClientActorId() const = 0;
	virtual IEntity* GetClientEntity() const = 0;
	virtual int GetClientEntityId() const = 0;
	virtual PVOID GetClientChannel() const = 0;
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
	virtual UINT8 IsEditing() = 0;
	virtual UINT8 IsInLevelLoad() = 0;
	virtual UINT8 IsLoadingSaveGame() = 0;
	virtual UINT8 IsInTimeDemo() = 0;
	virtual UINT8 IsTimeDemoRecording() = 0;
	virtual void fn_82(void) = 0;
	virtual void fn_83(void) = 0;
	virtual UINT8 CanSave() = 0;
	virtual UINT8 CanLoad() = 0;
	virtual void fn_84(void) = 0;
	virtual UINT8 CanCheat() = 0;
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
	UINT64 ukn_00;
	UINT64 ukn_01;
	UINT64 ukn_02;
	UINT64 ukn_03;
	UINT64 ukn_04;
	UINT64 ukn_05;
	UINT64 ukn_06;
	UINT64 ukn_07;
	UINT64 ukn_08;
	UINT64 ukn_09;
	UINT64 ukn_10;
	UINT64 ukn_11;
	UINT64 ukn_12;
	UINT64 ukn_13;
	UINT64 ukn_14;
	UINT64 ukn_15;
	UINT64 ukn_16;
	UINT64 ukn_17;
	IGameFramework* pGameFramework;
	UINT64 ukn_18;
	IEntitySystem* pEntitySystem;
	UINT64 ukn_19;
	UINT64 ukn_20;
	ISystem* pSystem;
	UINT64 ukn_21;
	UINT64 ukn_22;
	UINT64 ukn_23;
	UINT64 ukn_24;
	UINT64 ukn_25;
	UINT64 ukn_26;
	IRenderer* pRenderer;
	IRenderAuxGeom* pAuxGeomRenderer; /* NullAuxGeomRenderer */
	UINT64 ukn_27;
	UINT64 ukn_28;
	UINT64 ukn_29;
	UINT64 ukn_30;
	UINT64 ukn_31;
	UINT64 ukn_32;
	UINT64 ukn_33;
	UINT64 ukn_34;
	UINT64 ukn_35;
	UINT64 ukn_36;
	UINT64 ukn_37;
	UINT64 ukn_38;
	UINT64 ukn_39;
	UINT64 ukn_40;
	UINT64 ukn_41;
	UINT64 ukn_42;
	UINT64 ukn_43;
	UINT64 ukn_44;
	UINT64 ukn_45;
	UINT64 ukn_46;
	UINT32 mMainThreadId;
	UINT32 nMainFrameID;
	LPCSTR szCmdLine;
	CHAR szDebugStatus[128];
	BOOL bServer;
	BOOL bMultiplayer;
	BOOL bHostMigrating;
};

struct ISystem
{
	virtual ~ISystem() {}
	virtual void fn_00(void) const = 0;
	virtual SSystemGlobalEnvironment* GetGlobalEnvironment() = 0;
	virtual void fn_01(void) const = 0;
	virtual const char* GetRootFolder() const = 0;
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
	virtual UINT32 GetUsedMemory() = 0;
	virtual const char* GetUserName() = 0;
	virtual UINT32 GetCPUFlags() = 0;
	virtual int GetLogicalCPUCount() = 0;
	virtual void fn_12(void) = 0;
	virtual void Quit() = 0;
	virtual void Relaunch(bool bRelaunch) = 0;
	virtual UINT32 IsQuitting() const = 0;
	virtual void fn_13(void) = 0;
	virtual void fn_14(void) = 0;
	virtual void fn_15(void) = 0;
	virtual UINT32 IsRelaunch() const = 0;
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
	virtual IEntitySystem* GetIEntitySystem() = 0;
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
	virtual void fn_63(void) = 0;
	virtual void fn_64(void) = 0;
	virtual void fn_65(void) = 0;
	virtual PVOID GetHWND() = 0;
	virtual PVOID GetActiveHWND() = 0;
	virtual void fn_66(void) = 0;
	virtual IRenderer* GetIRenderer() = 0;
};

static inline bool HProjectToScreen(IRenderer* pIRenderer, float sx, float sy, float sz, float* ox, float* oy, float* oz)
{
	return  pIRenderer->ProjectToScreen(sx, sy, sz, ox, oy, oz);
}

static inline bool WorldToScreen(SSystemGlobalEnvironment* globalEnv, Vec3 vEntPos, Vec3 &vOut)
{
	IRenderer* Renderer = globalEnv->pRenderer;
	HProjectToScreen(Renderer, vEntPos.x, vEntPos.y, vEntPos.z,
		&vOut.x, &vOut.y, &vOut.z);

	vOut.x *= (Renderer->GetWidth() / 100.0f);
	vOut.y *= (Renderer->GetHeight() / 100.0f);
	vOut.z *= 1.0f;

	return ((vOut.z < 1.0f) && (vOut.x > 0) && (vOut.x < (float)Renderer->GetWidth()) && (vOut.y > 0) && (vOut.y < (float)Renderer->GetHeight()));
}


struct HuntCtx {
	IEntitySystem ** ppEntSys;
	SSystemGlobalEnvironment ** ppGlobalEnv;
	IGameFramework ** ppCCryAction;
};