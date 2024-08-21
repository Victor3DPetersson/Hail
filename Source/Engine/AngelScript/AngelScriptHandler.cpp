#include "Engine_PCH.h"
#include "AngelScriptHandler.h"
#include "angelscript.h"
#include "AngelScriptScriptstdstring.h"
#include "AngelScriptMath.h"
#include "Input\InputActionList.h"
#include <new>
#include "Input\InputActionMap.h"
#include <string>
#include "Utility\DebugLineHelpers.h"
#include "ThreadSynchronizer.h"

using namespace Hail;
using namespace AngelScript;

namespace
{
	InputActionMap* g_pInputActionMap;
	ThreadSyncronizer* g_pThreadSynchronizer;

	const char* localGetInputActionMapValueName(eInputAction inputAction)
	{
		switch (inputAction)
		{
		case Hail::eInputAction::PlayerMoveUp:
			return "PlayerMoveUp";
		case Hail::eInputAction::PlayerMoveRight:
			return "PlayerMoveRight";
		case Hail::eInputAction::PlayerMoveLeft:
			return "PlayerMoveLeft";
		case Hail::eInputAction::PlayerMoveDown:
			return "PlayerMoveDown";
		case Hail::eInputAction::PlayerMoveJoystickL:
			return "PlayerMoveJoystickL";
		case Hail::eInputAction::PlayerMoveJoystickR:
			return "PlayerMoveJoystickR";
		case Hail::eInputAction::PlayerAction1:
			return "PlayerAction1";
		case Hail::eInputAction::PlayerAction2:
			return "PlayerAction2";
		case Hail::eInputAction::PlayerPause:
			return "PlayerPause";
		case Hail::eInputAction::DebugAction1:
			return "DebugAction1";
		case Hail::eInputAction::DebugAction2:
			return "DebugAction2";
		case Hail::eInputAction::DebugAction3:
			return "DebugAction3";
		case Hail::eInputAction::DebugAction4:
			return "DebugAction4";
		case Hail::eInputAction::InputCount:
		default:
			break;
		}
		return "";
	}

	Vec2 localGetDirectionInput(eInputAction actionToGet, int gamepadToCheck)
	{
		const glm::vec2 direction = g_pInputActionMap->GetDirectionInput(actionToGet, gamepadToCheck);
		return Vec2(direction);
	}
	int localGetButtonInput(eInputAction actionToGet, int gamepadToCheck)
	{
		return (int)g_pInputActionMap->GetButtonInput(actionToGet, gamepadToCheck);
	}
	float localGetGamepadTriggerInput(eInputAction actionToGet, int gamepadToCheck)
	{
		return g_pInputActionMap->GetGamepadTriggerInput(actionToGet, gamepadToCheck);
	}

	//TODO: replace with cusotm string class
	void localPrint(std::string& stringToPrint)
	{
		H_DEBUGMESSAGE(stringToPrint.c_str());
	}
	void localPrintError(std::string& stringToPrint)
	{
		H_ERROR(stringToPrint.c_str());
	}
	void localPrintWarning(std::string& stringToPrint)
	{
		H_WARNING(stringToPrint.c_str());
	}

	// All non normalized functions work in pixel coordinates.
	//TODO: Add a color type to AngelScript
	void localDrawLineNormalized(const Vec2& startPos, const Vec2& endPos)
	{
		DrawLine2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, endPos.m_vec, true);
	}

	void localDrawLineNormalizedScreenAlligned(const Vec2& startPos, const Vec2& endPos)
	{
		DrawLine2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, endPos.m_vec, false);
	}

	void localDrawLine(const Vec2& startPos, const Vec2& endPos)
	{
		DrawLine2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, endPos.m_vec, true);
	}

	void localDrawLineScreenAlligned(const Vec2& startPos, const Vec2& endPos)
	{
		DrawLine2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, endPos.m_vec, false);
	}

	void localDrawLineNormalizedRotLength(const Vec2& startPos, float rotationRadian, float length)
	{
		DrawLine2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, rotationRadian, length, true);
	}

	void localDrawLineNormalizedRotLengthScreenAlligned(const Vec2& startPos, float rotationRadian, float length)
	{
		DrawLine2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, rotationRadian, length, false);
	}

	void localDrawLineRotLength(const Vec2& startPos, float rotationRadian, float length)
	{
		DrawLine2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, rotationRadian, length, true);
	}

	void localDrawLineRotLengthScreenAlligned(const Vec2& startPos, float rotationRadian, float length)
	{
		DrawLine2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, startPos.m_vec, rotationRadian, length, false);
	}

	void localDrawBoxNormalized(const Vec2& pos, const Vec2& dimensions)
	{
		DrawBox2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, dimensions.m_vec, true);
	}

	void localDrawBoxNormalizedScreenAlligned(const Vec2& pos, const Vec2& dimensions)
	{
		DrawBox2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, dimensions.m_vec, false);
	}

	void localDrawBox(const Vec2& pos, const Vec2& dimensions)
	{
		DrawBox2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, dimensions.m_vec, true);
	}

	void localDrawBoxScreenAlligned(const Vec2& pos, const Vec2& dimensions)
	{
		DrawBox2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, dimensions.m_vec, false);
	}

	void localDrawBoxMinMaxNormalized(const Vec2& min, const Vec2& max)
	{
		DrawBox2DMinMax(*g_pThreadSynchronizer->GetAppFrameData().renderPool, min.m_vec, max.m_vec, true);
	}

	void localDrawBoxMinMaxNormalizedScreenAlligned(const Vec2& min, const Vec2& max)
	{
		DrawBox2DMinMax(*g_pThreadSynchronizer->GetAppFrameData().renderPool, min.m_vec, max.m_vec, false);
	}

	void localDrawBoxMinMax(const Vec2& min, const Vec2& max)
	{
		DrawBox2DMinMaxPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, min.m_vec, max.m_vec, true);
	}

	void localDrawBoxMinMaxScreenAlligned(const Vec2& min, const Vec2& max)
	{
		DrawBox2DMinMaxPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, min.m_vec, max.m_vec, false);
	}

	void localDrawCircleNormalized(const Vec2& pos, float radius)
	{
		DrawCircle2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, radius, true);
	}

	void localDrawCircleNormalizedScreenAlligned(const Vec2& pos, float radius)
	{
		DrawCircle2D(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, radius, false);
	}

	void localDrawCircle(const Vec2& pos, float radius)
	{
		DrawCircle2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, radius, true);
	}

	void localDrawCircleScreenAlligned(const Vec2& pos, float radius)
	{
		DrawCircle2DPixelSpace(*g_pThreadSynchronizer->GetAppFrameData().renderPool, pos.m_vec, radius, false);
	}
}


void Hail::AngelScript::Handler::RegisterGlobalMessages()
{
	// Print functions
	int r = m_pScriptEngine->RegisterGlobalFunction("void Print(const string &in)", asFUNCTION(localPrint), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void PrintError(const string &in)", asFUNCTION(localPrintError), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void PrintWarning(const string &in)", asFUNCTION(localPrintWarning), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	
	// Vector2
	RegisterAngelScriptVectorType();

	// Math
	RegisterScriptMath(m_pScriptEngine);

	// Input handling
	// Input Enum
	r = m_pScriptEngine->RegisterEnum("eInputAction");
	H_ASSERT(r >= 0, "Failed to register input enum");
	for (int i = 0; i < (int)eInputAction::InputCount; i++)
	{
		r = m_pScriptEngine->RegisterEnumValue("eInputAction", localGetInputActionMapValueName((eInputAction)i), i);
	}
	// Input handler
	r = m_pScriptEngine->RegisterGlobalFunction("Vec2 GetDirectionInput(eInputAction, int)", asFUNCTION(localGetDirectionInput), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("int GetButtonInput(eInputAction, int)", asFUNCTION(localGetButtonInput), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("float GetGamepadTriggerInput(eInputAction, int)", asFUNCTION(localGetGamepadTriggerInput), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");

	// Debug lines
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineNormalized(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawLineNormalized), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineNormalizedScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawLineNormalizedScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLine(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawLine), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawLineScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");

	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineNormalized(const Vec2 &in, float, float)", asFUNCTION(localDrawLineNormalizedRotLength), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineNormalizedScreenAlligned(const Vec2 &in, float, float)", asFUNCTION(localDrawLineNormalizedRotLengthScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLine(const Vec2 &in, float, float)", asFUNCTION(localDrawLineRotLength), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawLineScreenAlligned(const Vec2 &in, float, float)", asFUNCTION(localDrawLineRotLengthScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");

	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxNormalized(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxNormalized), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxNormalizedScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxNormalizedScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBox(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBox), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");

	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxMinMaxNormalized(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxMinMaxNormalized), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxMinMaxNormalizedScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxMinMaxNormalizedScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxMinMax(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxMinMax), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawBoxMinMaxScreenAlligned(const Vec2 &in, const Vec2 &in)", asFUNCTION(localDrawBoxMinMaxScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");

	r = m_pScriptEngine->RegisterGlobalFunction("void DrawCircleNormalized(const Vec2 &in, float)", asFUNCTION(localDrawCircleNormalized), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawCircleNormalizedScreenAlligned(const Vec2 &in, float)", asFUNCTION(localDrawCircleNormalizedScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawCircle(const Vec2 &in, float)", asFUNCTION(localDrawCircle), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
	r = m_pScriptEngine->RegisterGlobalFunction("void DrawCircleScreenAlligned(const Vec2 &in, float)", asFUNCTION(localDrawCircleScreenAlligned), asCALL_CDECL); H_ASSERT(r >= 0, "Failed to register global function");
}

//-----------------------
// AngelScript functions
//-----------------------

static void Vec2DefaultConstructor(Vec2* self)
{
	new(self) Vec2();
}

static void Vec2CopyConstructor(const Vec2& other, Vec2* self)
{
	new(self) Vec2(other);
}

static void Vec2ConvConstructor(float val, Vec2* self)
{
	new(self) Vec2(val);
}

static void Vec2InitConstructor(float x, float y, Vec2* self)
{
	new(self) Vec2(x, y);
}

static void Vec2ListConstructor(float* list, Vec2* self)
{
	new(self) Vec2(list[0], list[1]);
}

void Hail::AngelScript::Handler::RegisterAngelScriptVectorType()
{
	int r;
	// Register a primitive type, that doesn't need any special management of the content

	// Register the type
	r = m_pScriptEngine->RegisterObjectType("Vec2", sizeof(Vec2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS); assert(r >= 0);

	// Register the object properties
	r = m_pScriptEngine->RegisterObjectProperty("Vec2", "float x", asOFFSET(Vec2, m_vec.x)); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectProperty("Vec2", "float y", asOFFSET(Vec2, m_vec.y)); H_ASSERT(r >= 0, "Failed to register Vec2 func");

	// Register the constructors
	r = m_pScriptEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vec2DefaultConstructor), asCALL_CDECL_OBJLAST); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(const Vec2 &in)", asFUNCTION(Vec2CopyConstructor), asCALL_CDECL_OBJLAST); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(float)", asFUNCTION(Vec2ConvConstructor), asCALL_CDECL_OBJLAST); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(Vec2InitConstructor), asCALL_CDECL_OBJLAST); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectBehaviour("Vec2", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float}", asFUNCTION(Vec2ListConstructor), asCALL_CDECL_OBJLAST); H_ASSERT(r >= 0, "Failed to register Vec2 func");

	// Register the operator overloads
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 &opAddAssign(const Vec2 &in)", asMETHODPR(Vec2, operator+=, (const Vec2&), Vec2&), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 &opSubAssign(const Vec2 &in)", asMETHODPR(Vec2, operator-=, (const Vec2&), Vec2&), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 &opMulAssign(const Vec2 &in)", asMETHODPR(Vec2, operator*=, (const Vec2&), Vec2&), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 &opDivAssign(const Vec2 &in)", asMETHODPR(Vec2, operator/=, (const Vec2&), Vec2&), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "bool opEquals(const Vec2 &in) const", asMETHODPR(Vec2, operator==, (const Vec2&) const, bool), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 opAdd(const Vec2 &in) const", asMETHODPR(Vec2, operator+, (const Vec2&) const, Vec2), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 opSub(const Vec2 &in) const", asMETHODPR(Vec2, operator-, (const Vec2&) const, Vec2), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 opMul(const Vec2 &in) const", asMETHODPR(Vec2, operator*, (const Vec2&) const, Vec2), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 opDiv(const Vec2 &in) const", asMETHODPR(Vec2, operator/, (const Vec2&) const, Vec2), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");

	// Register the object methods
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "float Length() const", asMETHOD(Vec2, Length), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "float SquaredLength() const", asMETHOD(Vec2, SquaredLength), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 GetNormalized() const", asMETHOD(Vec2, GetNormalized), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "void Normalize()", asMETHOD(Vec2, Normalize), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");

	// Register the swizzle operators
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 get_yx() const property", asMETHOD(Vec2, YX), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 get_xx() const property", asMETHOD(Vec2, XX), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "Vec2 get_yy() const property", asMETHOD(Vec2, YY), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "float get_x() const property", asMETHOD(Vec2, GetX), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "float get_y() const property", asMETHOD(Vec2, GetY), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "void set_x(float) property", asMETHOD(Vec2, SetX), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
	r = m_pScriptEngine->RegisterObjectMethod("Vec2", "void set_y(float) property", asMETHOD(Vec2, SetY), asCALL_THISCALL); H_ASSERT(r >= 0, "Failed to register Vec2 func");
}

Hail::AngelScript::Vec2::Vec2() : m_vec(0) {}

Hail::AngelScript::Vec2::Vec2(glm::vec2 vec) : m_vec(vec) {}

Hail::AngelScript::Vec2::Vec2(const Vec2& other) : m_vec(other.m_vec) {}

Hail::AngelScript::Vec2::Vec2(float val) : m_vec(val, val) {}

Hail::AngelScript::Vec2::Vec2(float x, float y) : m_vec(x, y) {}

Vec2& Hail::AngelScript::Vec2::operator=(const Vec2& other)
{
	m_vec = other.m_vec;
	return *this;
}

Vec2& Hail::AngelScript::Vec2::operator+=(const Vec2& other)
{
	m_vec += other.m_vec;
	return *this;
}

Vec2& Hail::AngelScript::Vec2::operator-=(const Vec2& other)
{
	m_vec -= other.m_vec;
	return *this;
}

Vec2& Hail::AngelScript::Vec2::operator*=(const Vec2& other)
{
	m_vec *= other.m_vec;
	return *this;
}

Vec2& Hail::AngelScript::Vec2::operator/=(const Vec2& other)
{
	m_vec /= other.m_vec;
	return *this;
}

float Hail::AngelScript::Vec2::Length() const
{
	return m_vec.length();
}

float Hail::AngelScript::Vec2::SquaredLength() const
{
	return m_vec.x * m_vec.x + m_vec.y * m_vec.y;
}

Vec2 Hail::AngelScript::Vec2::GetNormalized() const
{
	const glm::vec2 vec2 = glm::normalize(m_vec);
	return Vec2(vec2.x, vec2.y);
}

void Hail::AngelScript::Vec2::Normalize()
{
	m_vec = glm::normalize(m_vec);
}

bool Hail::AngelScript::Vec2::operator==(const Vec2& other) const
{
	return m_vec == other.m_vec;
}

bool Hail::AngelScript::Vec2::operator!=(const Vec2& other) const
{
	return m_vec != other.m_vec;
}

Vec2 Hail::AngelScript::Vec2::YX() const
{
	return Vec2(m_vec.y, m_vec.x);
}

Vec2 Hail::AngelScript::Vec2::XX() const
{
	return Vec2(m_vec.x, m_vec.x);
}

Vec2 Hail::AngelScript::Vec2::YY() const
{
	return Vec2(m_vec.y, m_vec.y);
}

void Hail::AngelScript::Vec2::SetX(float x)
{
	m_vec.x = x;
}

void Hail::AngelScript::Vec2::SetY(float y)
{
	m_vec.y = y;
}

float Hail::AngelScript::Vec2::GetX() const
{
	return m_vec.x;
}

float Hail::AngelScript::Vec2::GetY() const
{
	return m_vec.y;
}

Vec2 Hail::AngelScript::Vec2::operator+(const Vec2& other) const
{
	const glm::vec2 vec = m_vec + other.m_vec;
	return Vec2(vec.x, vec.y);
}

Vec2 Hail::AngelScript::Vec2::operator-(const Vec2& other) const
{
	const glm::vec2 vec = m_vec - other.m_vec;
	return Vec2(vec.x, vec.y);
}

Vec2 Hail::AngelScript::Vec2::operator*(const Vec2& other) const
{
	const glm::vec2 vec = m_vec * other.m_vec;
	return Vec2(vec.x, vec.y);
}

Vec2 Hail::AngelScript::Vec2::operator/(const Vec2& other) const
{
	const glm::vec2 vec = m_vec / other.m_vec;
	return Vec2(vec.x, vec.y);
}

void Hail::AngelScript::Handler::Init(InputActionMap* pInputActionMap, ThreadSyncronizer* pThreadSyncronizer)
{
	H_ASSERT(pInputActionMap, "Must set action input map with a valid pointer.");
	H_ASSERT(pThreadSyncronizer, "Must set thread synchronizer with a valid pointer.");
	g_pInputActionMap = pInputActionMap;
	g_pThreadSynchronizer = pThreadSyncronizer;

	m_pScriptEngine = asCreateScriptEngine();
	H_ASSERT(m_pScriptEngine, "Failed to create angelscript engine.");
	m_errorHandler.SetScriptEngine(m_pScriptEngine);
	RegisterStdString(m_pScriptEngine);
	RegisterGlobalMessages();
}
