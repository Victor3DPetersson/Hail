#pragma once	

class asIScriptEngine;
struct asSMessageInfo;

namespace Hail
{
	namespace AngelScript
	{
		void MessageCallback(const asSMessageInfo* pMsg, void* param);
		class ErrorHandler
		{
		public:
			void SetScriptEngine(asIScriptEngine* pScriptEngine);

		private:
		};
	}


}