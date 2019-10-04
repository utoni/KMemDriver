#pragma once

#include <Windows.h>
#include <stdarg.h>

#include <string>

struct ISystem;


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

struct IEntity
{
public:
	virtual ~IEntity() {}
	virtual int GetId() const = 0;
	virtual const PVOID GetGuid() const = 0;
	virtual PVOID GetClass() const = 0;
	virtual PVOID GetArchetype() const = 0;
	virtual void SetFlags(UINT32 flags) = 0;
	virtual UINT32 GetFlags() const = 0;
	virtual void AddFlags(UINT32 flagsToAdd) = 0;
	virtual void ClearFlags(UINT32 flagsToClear) = 0;
	virtual bool CheckFlags(UINT32 flagsToCheck) const = 0;
	virtual void SetFlagsExtended(UINT32 flags) = 0;
	virtual UINT32 GetFlagsExtended() const = 0;
	virtual bool IsInitialized() const = 0;
	virtual bool IsGarbage() const = 0;
	virtual UINT8 GetComponentChangeState() const = 0;
	virtual void SetName(const char* sName) = 0;
	virtual const char* GetName() const = 0;
	virtual std::string GetEntityTextDescription() const = 0;
	virtual void SerializeXML(PVOID entityNode, bool bLoading, bool bIncludeScriptProxy = true, bool bExcludeSchematycProperties = false) = 0;
	virtual bool IsLoadedFromLevelFile() const = 0;
	virtual void AttachChild(IEntity* pChildEntity, const PVOID attachParams) = 0;
	virtual void DetachAll(int attachmentFlags = 0) = 0;
	virtual void DetachThis(int attachmentFlags = 0, int transformReasons = 0) = 0;
	virtual int GetChildCount() const = 0;
	virtual IEntity* GetChild(int nIndex) const = 0;
	virtual IEntity* GetParent() const = 0;
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

/*
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

struct IPersistantDebug
{
	virtual ~IPersistantDebug() {}
	virtual void Begin(const char* szName, bool clear) = 0;
	virtual void AddSphere(const Vec3& pos, float radius, ColorF clr, float timeout) = 0;
	virtual void AddDirection(const Vec3& pos, float radius, const Vec3& dir, ColorF clr, float timeout) = 0;
	virtual void AddLine(const Vec3& pos1, const Vec3& pos2, ColorF clr, float timeout) = 0;
	virtual void AddPlanarDisc(const Vec3& pos, float innerRadius, float outerRadius, ColorF clr, float timeout) = 0;
	virtual void AddCone(const Vec3& pos, const Vec3& dir, float baseRadius, float height, ColorF clr, float timeout) = 0;
	virtual void AddCylinder(const Vec3& pos, const Vec3& dir, float radius, float height, ColorF clr, float timeout) = 0;
	virtual void Add2DText(const char* szText, float size, ColorF clr, float timeout) = 0;
	virtual void AddText(float x, float y, float size, ColorF clr, float timeout, const char* fmt, ...) = 0;
	virtual void AddText3D(const Vec3& pos, float size, ColorF clr, float timeout, const char* fmt, ...) = 0;
	virtual void Add2DLine(float x1, float y1, float x2, float y2, ColorF clr, float timeout) = 0;
	virtual void AddQuat(void) = 0;
	virtual void AddAABB(const Vec3& min, const Vec3& max, ColorF clr, float timeout) = 0;
	virtual void AddEntityTag(void) = 0;
	virtual void ClearEntityTags(void) = 0;
	virtual void ClearStaticTag(void) = 0;
	virtual void ClearTagContext(const char* tagContext) = 0;
	virtual void ClearTagContext(void) = 0;
	virtual void Update(float frameTime) = 0;
	virtual void PostUpdate(float frameTime) = 0;
	virtual void Reset() = 0;
};
*/

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
	PVOID pAuxGeomRenderer; /* NullAuxGeomRenderer */
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
	virtual bool IsQuitting() const = 0;
	virtual void fn_13(void) = 0;
	virtual void fn_14(void) = 0;
	virtual void fn_15(void) = 0;
	virtual bool IsRelaunch() const = 0;
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
	virtual void fn_49(void) = 0;
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
