#pragma once

#include "AngelScriptErrorHandler.h"
class asIScriptEngine;

namespace Hail
{
	class InputActionMap;
	class ThreadSyncronizer;

	namespace AngelScript
	{
		struct Vec2
		{
			Vec2();
			Vec2(glm::vec2);
			Vec2(const Vec2& other);
			Vec2(float val);
			Vec2(float x, float y);

			// Assignment operator
			Vec2& operator=(const Vec2& other);

			// Compound assigment operators
			Vec2& operator+=(const Vec2& other);
			Vec2& operator-=(const Vec2& other);
			Vec2& operator*=(const Vec2& other);
			Vec2& operator/=(const Vec2& other);

			float Length() const;
			float SquaredLength() const;
			Vec2 GetNormalized() const;
			void Normalize();

			// Comparison 
			bool operator==(const Vec2& other) const;
			bool operator!=(const Vec2& other) const;

			// Swizzle operators
			Vec2 YX() const;
			Vec2 XX() const;
			Vec2 YY() const;
			float GetX() const;
			float GetY() const;
			void SetX(float x);
			void SetY(float y);

			// Math operators
			Vec2 operator+(const Vec2& other) const;
			Vec2 operator-(const Vec2& other) const;
			Vec2 operator*(const Vec2& other) const;
			Vec2 operator/(const Vec2& other) const;

			glm::vec2 m_vec;
		};

		class Handler
		{
		public:
			void Init(InputActionMap* pInputActionMap, ThreadSyncronizer* pThreadSyncronizer);
			asIScriptEngine* GetScriptEngine() { return m_pScriptEngine; }
		private:

			void RegisterGlobalMessages();
			void RegisterAngelScriptVectorType();

			asIScriptEngine* m_pScriptEngine;
			ErrorHandler m_errorHandler;
		};
	}
}