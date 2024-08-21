#include "Engine_PCH.h"
#include "AngelScriptDebugger.h"
#include "AngelScriptDebuggerMessagePackager.h"
#include <angelscript.h>

#include <fcntl.h>

#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
#endif

#include "MathUtils.h"

namespace
{
#ifndef _WIN32 
    constexpr int InvalidSocket = -1;
#else
    constexpr unsigned int InvalidSocket = INVALID_SOCKET;
#endif

    int SockInit(void)
    {
#ifdef _WIN32
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
        return 0;
#endif
    }

    int SockQuit(void)
    {
#ifdef _WIN32
        return WSACleanup();
#else
        return 0;
#endif
    }

    int SocketClose(Hail::H_Socket socketHandle)
    {

        int status = 0;

#ifdef _WIN32
        status = shutdown(socketHandle, SD_BOTH);
        if (status == 0) 
        { 
            status = closesocket(socketHandle); 
        }
#else
        status = shutdown(socketHandle, SHUT_RDWR);
        if (status == 0) 
        { 
            status = close(socketHandle); 
        }
#endif

        return status;
    }

    Hail::H_Socket CreateSocket()
    {
#define DEFAULT_PORT "27015"
        Hail::H_Socket returnSocket = 0;

#ifdef _WIN32

        struct addrinfo* result = NULL, hints;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the local address and port to be used by the server
        int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            printf("getaddrinfo failed: %d\n", iResult);
            WSACleanup();
            return 1;
        }

        //sockaddr_in serverAddress;
        //serverAddress.sin_family = AF_INET;
        //serverAddress.sin_port = htons(8080);
        //serverAddress.sin_addr.s_addr = INADDR_ANY;

        returnSocket = socket(AF_INET, SOCK_STREAM, 0);
        H_ASSERT(returnSocket != InvalidSocket, "Failed to create server socket.");

        const int bindResult = bind(returnSocket, result->ai_addr, (int)result->ai_addrlen);
        H_ASSERT(bindResult != -1, "Failed to bind server socket");
      
        unsigned long iMode = 1;
        iResult = ioctlsocket(returnSocket, FIONBIO, &iMode);
        H_ASSERT(iResult == NO_ERROR, "ioctlsocket failed with error.");
#else
        // TODO: POSIX sockets

#endif

        return returnSocket;
    }
}


using namespace Hail;
using namespace AngelScript;


DebuggerServer::DebuggerServer()
: m_socketHandle(InvalidSocket)
, m_currentClient(MAX_UINT)
, m_bIsDebugging(false)
, m_pActiveScript(nullptr)
{
    // Initialize socket use
    H_ASSERT(SockInit() == 0, "Failed to initialize the socket.");
    m_socketHandle = CreateSocket();
    // listening to the assigned socket 
    int iListenResult = listen(m_socketHandle, 4096);
    H_ASSERT(iListenResult == NO_ERROR, "Failed with listen.");
}

DebuggerServer::~DebuggerServer()
{
    H_ASSERT(SocketClose(m_socketHandle) == 0, "Failed to close socket properly");
    //Close Winsock / socket use.
    SockQuit();
}

Hail::AngelScript::ScriptDebugger::ScriptDebugger(asIScriptContext* pContext)
    : m_bIsDebugging(false)
    , m_pScriptContext(pContext)
    , m_executionStatus(eScriptExecutionStatus::Normal)
{
}

void Hail::AngelScript::ScriptDebugger::SetScriptContext(asIScriptContext* pContext)
{
    if (pContext && m_pScriptContext != pContext)
    {
        H_ERROR("Removing script context from debugger with a new context.");
    }
    m_pScriptContext = pContext;
}

void ScriptDebugger::LineCallback(asIScriptContext* pContext)
{
    if (!m_bIsDebugging)
        return;

    const char *file = 0;
	int lineNbr = pContext->GetLineNumber(0, 0, &file);
    const FilePath scriptObjectPath = file;
    FileBreakPoints* pFileBreakPoints = nullptr;
    for (uint16 i = 0; i < m_breakPoints.Size(); i++)
    {
        if (StringCompare(m_breakPoints[i].fileName.Data(), scriptObjectPath.Object().Name().CharString()))
        {
            pFileBreakPoints = &m_breakPoints[i];
            break;
        }
    }

    // no active file with breakpoints to check.
    if (!pFileBreakPoints)
        return;

    for (size_t i = 0; i < pFileBreakPoints->breakPoints.Size(); i++)
    {
        if (pFileBreakPoints->breakPoints[i].line == lineNbr)
        {
            H_DEBUGMESSAGE("Hit breakpoint in file");
            m_executionStatus = eScriptExecutionStatus::HitBreakpoint;
            m_messages.Add(CreateHitBreakpointMessage(lineNbr, scriptObjectPath.Object().Name().CharString().Data()));
            break;
        }
    }

	StringL declaration = pContext->GetFunction()->GetDeclaration();

}

void Hail::AngelScript::ScriptDebugger::SetLineCallback()
{
    if (m_bIsDebugging)
    {
        H_ERROR("Debugger is already set for debugging, potential code error.");
        return;
    }
    Hail::AngelScript::ScriptDebugger* pDebugger = this;
    // Attaching line-callback for angelscript.
    m_pScriptContext->SetLineCallback(asMETHOD(ScriptDebugger, LineCallback), pDebugger, asCALL_THISCALL);
    m_bIsDebugging = true;
}

void Hail::AngelScript::ScriptDebugger::StopDebugging()
{
    if (!m_bIsDebugging)
    {
        H_ERROR("Debugger is not set for debugging, potential code error.");
    }
    m_bIsDebugging = false;
    m_pScriptContext->ClearLineCallback();
}

void Hail::AngelScript::ScriptDebugger::AddBreakpoints(const FileBreakPoints& breakPoints)
{
    m_breakPoints.Add(breakPoints);
}

void Hail::AngelScript::ScriptDebugger::RemoveBreakpoints(const FileBreakPoints& breakPoints)
{
    for (int i = m_breakPoints.Size(); i > 0; i--)
    {
        if (StringCompareCaseInsensitive(breakPoints.fileName, m_breakPoints[i - 1].fileName))
        {
            m_breakPoints.RemoveCyclicAtIndex(i - 1);
            break;
        }
    }
}

void Hail::AngelScript::ScriptDebugger::RemoveBreakpoints()
{
    m_breakPoints.RemoveAll();
}

DebuggerMessage Hail::AngelScript::ScriptDebugger::CreateHitBreakpointMessage(int line, StringL file)
{
    DebuggerMessage message;
    MessageHeader header;
    message.m_header.messageLength = file.Length();
    message.m_header.type = eDebuggerMessageType::HitBreakpoint;
    memcpy(message.m_message, file.Data(), file.Length());
    return message;
}

void Hail::AngelScript::DebuggerServer::Update()
{
    {
        // Look for new connections
        const H_Socket connectingSocket = accept(m_socketHandle, nullptr, nullptr);
        if (connectingSocket != InvalidSocket)
        {
            Client newClient;
            newClient.m_socket = connectingSocket;
            newClient.m_bConnected = false;
            m_clients.Add(newClient);
        }
    }

    ListenToMessages();
    if (m_bIsDebugging && m_pActiveScript)
    {
        GrowingArray<DebuggerMessage>& debuggerMessages = m_pActiveScript->m_pDebugger->GetMessages();

        for (size_t iDebugMessage = 0; iDebugMessage < debuggerMessages.Size(); iDebugMessage++)
        {
            SendDebuggerMessage(debuggerMessages[iDebugMessage]);
        }
        debuggerMessages.RemoveAll();
        while (m_pActiveScript->m_pDebugger->GetStatus() != eScriptExecutionStatus::Normal)
        {
            ListenToMessages();
        }
    }

}

void Hail::AngelScript::DebuggerServer::StartDebugging()
{
    H_ASSERT(m_currentClient != MAX_UINT, "Must start debugging on a valid client.");
    Client& client = m_clients[m_currentClient];
    client.m_bConnected = true;
    H_DEBUGMESSAGE(StringL::Format("Client nr %d connected for debugging.", m_currentClient + 1));
}

void Hail::AngelScript::DebuggerServer::StopDebugging()
{
    H_ASSERT(m_currentClient != MAX_UINT, "Must stop debugging on a valid client.");
    Client& client = m_clients[m_currentClient];
    client.m_bConnected = false;
}

void Hail::AngelScript::DebuggerServer::AddBreakpoints(const FileBreakPoints& breakpointsToAdd)
{
    if (m_pActiveScript)
    {
        for (uint16 iFileName = 0; iFileName < m_pActiveScript->m_fileNames.Size(); iFileName++)
        {
            if (StringCompareCaseInsensitive(m_pActiveScript->m_fileNames[iFileName], breakpointsToAdd.fileName))
            {
                if (breakpointsToAdd.breakPoints.Empty())
                    m_pActiveScript->m_pDebugger->RemoveBreakpoints(breakpointsToAdd);
                else
                    m_pActiveScript->m_pDebugger->AddBreakpoints(breakpointsToAdd);
            }
        }
    }
}

void Hail::AngelScript::DebuggerServer::ContinueDebugging()
{
    if (m_pActiveScript)
        m_pActiveScript->m_pDebugger->SetExecutionStatus(eScriptExecutionStatus::Normal);
}

void Hail::AngelScript::DebuggerServer::SendDebuggerMessage(DebuggerMessage& messageToSend)
{
    for (uint32 i = 0; i < m_clients.Size(); i++)
    {
        if (m_clients[i].m_bConnected)
        {
            int iSendResult = send(m_clients[i].m_socket, (const char*)(&messageToSend.m_header), sizeof(MessageHeader), 0);
            iSendResult = send(m_clients[i].m_socket, (messageToSend.m_message), messageToSend.m_header.messageLength, 0);
            if (iSendResult == SOCKET_ERROR) {
                H_ERROR(StringL::Format("send failed: %d\n", WSAGetLastError()));
            }
        }
    }
}

void Hail::AngelScript::DebuggerServer::ListenToMessages()
{
    char buffer[4096] = { 0 };

    VectorOnStack<uint32, MAX_ATTACHED_DEBUGGERS> disconnectingSockets;
    const bool wasDebugging = m_bIsDebugging;
    m_bIsDebugging = false;

    for (uint32 i = 0; i < m_clients.Size(); i++)
    {
        buffer[0] = 0;
        const int iResult = recv(m_clients[i].m_socket, buffer, sizeof(buffer), 0);
        m_currentClient = i;
        if (iResult > 0)
        {
            //char singleMessageBuffer[128];
            //memcpy(singleMessageBuffer, buffer, Math::Min(iResult, 128));
            //H_DEBUGMESSAGE(String256::Format("Server recieved this many bytes: %d Message: %s", Math::Min(iResult, 128), singleMessageBuffer));
            HandleDebuggerMessage(this, iResult, buffer);
            //iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            //if (iSendResult == SOCKET_ERROR) {
            //    printf("send failed: %d\n", WSAGetLastError());
            //    closesocket(ClientSocket);
            //    WSACleanup();
            //}
        }
        else if (iResult == 0)
        {
            H_DEBUGMESSAGE("Client connection closing...");
            disconnectingSockets.Add(i);
        }
        else
        {
            // No messages, no one loves you :(
            //H_ERROR("Socket error encountered when recieving message.");
        }
        if (m_clients[i].m_bConnected)
        {
            m_bIsDebugging = true;
            if (!wasDebugging && m_pActiveScript)
            {
                m_pActiveScript->m_pDebugger->SetLineCallback();
            }
            else
            {
                // Gather messages to send back
            }
        }
    }
    m_currentClient = MAX_UINT;
    // Removing highest indices first to keep the order. 
    for (int i = disconnectingSockets.Size(); i > 0; --i)
    {
        m_clients.RemoveCyclicAtIndex(disconnectingSockets[i - 1]);
    }
    if (wasDebugging && !m_bIsDebugging && m_pActiveScript)
    {
        m_pActiveScript->m_pDebugger->StopDebugging();
    }
}

// -- Old reference debugger v --

//#include <iostream>  // cout
//#include <sstream>   // stringstream
//#include <stdlib.h>  // atoi
//#include <assert.h>  // assert
//#include <cstring>   // strlen
//
//using namespace std;
//
//BEGIN_AS_NAMESPACE
//
//CDebugger::CDebugger()
//{
//	m_action = CONTINUE;
//	m_lastFunction = 0;
//	m_engine = 0;
//}
//
//CDebugger::~CDebugger()
//{
//	SetEngine(0);
//}
//
//string CDebugger::ToString(void *value, asUINT typeId, int expandMembers, asIScriptEngine *engine)
//{
//	if( value == 0 )
//		return "<null>";
//
//	// If no engine pointer was provided use the default
//	if( engine == 0 )
//		engine = m_engine;
//
//	stringstream s;
//	if( typeId == asTYPEID_VOID )
//		return "<void>";
//	else if( typeId == asTYPEID_BOOL )
//		return *(bool*)value ? "true" : "false";
//	else if( typeId == asTYPEID_INT8 )
//		s << (int)*(signed char*)value;
//	else if( typeId == asTYPEID_INT16 )
//		s << (int)*(signed short*)value;
//	else if( typeId == asTYPEID_INT32 )
//		s << *(signed int*)value;
//	else if( typeId == asTYPEID_INT64 )
//#if defined(_MSC_VER) && _MSC_VER <= 1200
//		s << "{...}"; // MSVC6 doesn't like the << operator for 64bit integer
//#else
//		s << *(asINT64*)value;
//#endif
//	else if( typeId == asTYPEID_UINT8 )
//		s << (unsigned int)*(unsigned char*)value;
//	else if( typeId == asTYPEID_UINT16 )
//		s << (unsigned int)*(unsigned short*)value;
//	else if( typeId == asTYPEID_UINT32 )
//		s << *(unsigned int*)value;
//	else if( typeId == asTYPEID_UINT64 )
//#if defined(_MSC_VER) && _MSC_VER <= 1200
//		s << "{...}"; // MSVC6 doesn't like the << operator for 64bit integer
//#else
//		s << *(asQWORD*)value;
//#endif
//	else if( typeId == asTYPEID_FLOAT )
//		s << *(float*)value;
//	else if( typeId == asTYPEID_DOUBLE )
//		s << *(double*)value;
//	else if( (typeId & asTYPEID_MASK_OBJECT) == 0 )
//	{
//		// The type is an enum
//		s << *(asUINT*)value;
//
//		// Check if the value matches one of the defined enums
//		if( engine )
//		{
//			asITypeInfo *t = engine->GetTypeInfoById(typeId);
//			for( int n = t->GetEnumValueCount(); n-- > 0; )
//			{
//				int enumVal;
//				const char *enumName = t->GetEnumValueByIndex(n, &enumVal);
//				if( enumVal == *(int*)value )
//				{
//					s << ", " << enumName;
//					break;
//				}
//			}
//		}
//	}
//	else if( typeId & asTYPEID_SCRIPTOBJECT )
//	{
//		// Dereference handles, so we can see what it points to
//		if( typeId & asTYPEID_OBJHANDLE )
//			value = *(void**)value;
//
//		asIScriptObject *obj = (asIScriptObject *)value;
//		
//		// Print the address of the object
//		s << "{" << obj << "}";
//
//		// Print the members
//		if( obj && expandMembers > 0 )
//		{
//			asITypeInfo *type = obj->GetObjectType();
//			for( asUINT n = 0; n < obj->GetPropertyCount(); n++ )
//			{
//				if( n == 0 )
//					s << " ";
//				else
//					s << ", ";
//
//				s << type->GetPropertyDeclaration(n) << " = " << ToString(obj->GetAddressOfProperty(n), obj->GetPropertyTypeId(n), expandMembers - 1, type->GetEngine());
//			}
//		}
//	}
//	else
//	{
//		// Dereference handles, so we can see what it points to
//		if( typeId & asTYPEID_OBJHANDLE )
//			value = *(void**)value;
//
//		// Print the address for reference types so it will be
//		// possible to see when handles point to the same object
//		if( engine )
//		{
//			asITypeInfo *type = engine->GetTypeInfoById(typeId);
//			if( type->GetFlags() & asOBJ_REF )
//				s << "{" << value << "}";
//
//			if( value )
//			{
//				// Check if there is a registered to-string callback
//				map<const asITypeInfo*, ToStringCallback>::iterator it = m_toStringCallbacks.find(type);
//				if( it == m_toStringCallbacks.end() )
//				{
//					// If the type is a template instance, there might be a
//					// to-string callback for the generic template type
//					if( type->GetFlags() & asOBJ_TEMPLATE )
//					{
//						asITypeInfo *tmplType = engine->GetTypeInfoByName(type->GetName());
//						it = m_toStringCallbacks.find(tmplType);
//					}
//				}
//
//				if( it != m_toStringCallbacks.end() )
//				{
//					if( type->GetFlags() & asOBJ_REF )
//						s << " ";
//
//					// Invoke the callback to get the string representation of this type
//					string str = it->second(value, expandMembers, this);
//					s << str;
//				}
//			}
//		}
//		else
//			s << "{no engine}";
//	}
//
//	return s.str();
//}
//
//void CDebugger::RegisterToStringCallback(const asITypeInfo *ot, ToStringCallback callback)
//{
//	if( m_toStringCallbacks.find(ot) == m_toStringCallbacks.end() )
//		m_toStringCallbacks.insert(map<const asITypeInfo*, ToStringCallback>::value_type(ot, callback));
//}
//
//void CDebugger::LineCallback(asIScriptContext *ctx)
//{
//	assert( ctx );
//
//	// This should never happen, but it doesn't hurt to validate it
//	if( ctx == 0 )
//		return;
//
//	// By default we ignore callbacks when the context is not active.
//	// An application might override this to for example disconnect the
//	// debugger as the execution finished.
//	if( ctx->GetState() != asEXECUTION_ACTIVE )
//		return;
//
//	if( m_action == CONTINUE )
//	{
//		if( !CheckBreakPoint(ctx) )
//			return;
//	}
//	else if( m_action == STEP_OVER )
//	{
//		if( ctx->GetCallstackSize() > m_lastCommandAtStackLevel )
//		{
//			if( !CheckBreakPoint(ctx) )
//				return;
//		}
//	}
//	else if( m_action == STEP_OUT )
//	{
//		if( ctx->GetCallstackSize() >= m_lastCommandAtStackLevel )
//		{
//			if( !CheckBreakPoint(ctx) )
//				return;
//		}
//	}
//	else if( m_action == STEP_INTO )
//	{
//		CheckBreakPoint(ctx);
//
//		// Always break, but we call the check break point anyway 
//		// to tell user when break point has been reached
//	}
//
//	stringstream s;
//	const char *file = 0;
//	int lineNbr = ctx->GetLineNumber(0, 0, &file);
//	s << (file ? file : "{unnamed}") << ":" << lineNbr << "; " << ctx->GetFunction()->GetDeclaration() << endl;
//	Output(s.str());
//
//	TakeCommands(ctx);
//}
//
//bool CDebugger::CheckBreakPoint(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//		return false;
//
//	// TODO: Should cache the break points in a function by checking which possible break points
//	//       can be hit when entering a function. If there are no break points in the current function
//	//       then there is no need to check every line.
//
//	const char *tmp = 0;
//	int lineNbr = ctx->GetLineNumber(0, 0, &tmp);
//
//	// Consider just filename, not the full path
//	string file = tmp ? tmp : "";
//	size_t r = file.find_last_of("\\/");
//	if( r != string::npos )
//		file = file.substr(r+1);
//
//	// Did we move into a new function?
//	asIScriptFunction *func = ctx->GetFunction();
//	if( m_lastFunction != func )
//	{
//		// Check if any breakpoints need adjusting
//		for( size_t n = 0; n < m_breakPoints.size(); n++ )
//		{
//			// We need to check for a breakpoint at entering the function
//			if( m_breakPoints[n].func )
//			{
//				if( m_breakPoints[n].name == func->GetName() )
//				{
//					stringstream s;
//					s << "Entering function '" << m_breakPoints[n].name << "'. Transforming it into break point" << endl;
//					Output(s.str());
//
//					// Transform the function breakpoint into a file breakpoint
//					m_breakPoints[n].name           = file;
//					m_breakPoints[n].lineNbr        = lineNbr;
//					m_breakPoints[n].func           = false;
//					m_breakPoints[n].needsAdjusting = false;
//				}
//			}
//			// Check if a given breakpoint fall on a line with code or else adjust it to the next line
//			else if( m_breakPoints[n].needsAdjusting &&
//					 m_breakPoints[n].name == file )
//			{
//				int line = func->FindNextLineWithCode(m_breakPoints[n].lineNbr);
//				if( line >= 0 )
//				{
//					m_breakPoints[n].needsAdjusting = false;
//					if( line != m_breakPoints[n].lineNbr )
//					{
//						stringstream s;
//						s << "Moving break point " << n << " in file '" << file << "' to next line with code at line " << line << endl;
//						Output(s.str());
//
//						// Move the breakpoint to the next line
//						m_breakPoints[n].lineNbr = line;
//					}
//				}
//			}
//		}
//	}
//	m_lastFunction = func;
//
//	// Determine if there is a breakpoint at the current line
//	for( size_t n = 0; n < m_breakPoints.size(); n++ )
//	{
//		// TODO: do case-less comparison for file name
//
//		// Should we break?
//		if( !m_breakPoints[n].func &&
//			m_breakPoints[n].lineNbr == lineNbr &&
//			m_breakPoints[n].name == file )
//		{
//			stringstream s;
//			s << "Reached break point " << n << " in file '" << file << "' at line " << lineNbr << endl;
//			Output(s.str());
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void CDebugger::TakeCommands(asIScriptContext *ctx)
//{
//	for(;;)
//	{
//		char buf[512];
//
//		Output("[dbg]> ");
//		cin.getline(buf, 512);
//
//		if( InterpretCommand(string(buf), ctx) )
//			break;
//	}
//}
//
//bool CDebugger::InterpretCommand(const string &cmd, asIScriptContext *ctx)
//{
//	if( cmd.length() == 0 ) return true;
//
//	switch( cmd[0] )
//	{
//	case 'c':
//		m_action = CONTINUE;
//		break;
//
//	case 's':
//		m_action = STEP_INTO;
//		break;
//
//	case 'n':
//		m_action = STEP_OVER;
//		m_lastCommandAtStackLevel = ctx ? ctx->GetCallstackSize() : 1;
//		break;
//
//	case 'o':
//		m_action = STEP_OUT;
//		m_lastCommandAtStackLevel = ctx ? ctx->GetCallstackSize() : 0;
//		break;
//
//	case 'b':
//		{
//			// Set break point
//			size_t p = cmd.find_first_not_of(" \t", 1);
//			size_t div = cmd.find(':'); 
//			if( div != string::npos && div > 2 && p > 1 )
//			{
//				string file = cmd.substr(2, div-2);
//				string line = cmd.substr(div+1);
//
//				int nbr = atoi(line.c_str());
//
//				AddFileBreakPoint(file, nbr);
//			}
//			else if( div == string::npos && p != string::npos && p > 1 )
//			{
//				string func = cmd.substr(p);
//
//				AddFuncBreakPoint(func);
//			}
//			else
//			{
//				Output("Incorrect format for setting break point, expected one of:\n"
//				       " b <file name>:<line number>\n"
//				       " b <function name>\n");
//			}
//		}
//		// take more commands
//		return false;
//
//	case 'r':
//		{
//			// Remove break point
//			size_t p = cmd.find_first_not_of(" \t", 1);
//			if( cmd.length() > 2 && p != string::npos && p > 1 )
//			{
//				string br = cmd.substr(2);
//				if( br == "all" )
//				{
//					m_breakPoints.clear();
//					Output("All break points have been removed\n");
//				}
//				else
//				{
//					int nbr = atoi(br.c_str());
//					if( nbr >= 0 && nbr < (int)m_breakPoints.size() )
//						m_breakPoints.erase(m_breakPoints.begin()+nbr);
//					ListBreakPoints();
//				}
//			}
//			else
//			{
//				Output("Incorrect format for removing break points, expected:\n"
//				       " r <all|number of break point>\n");
//			}
//		}
//		// take more commands
//		return false;
//
//	case 'l':
//		{
//			// List something
//			bool printHelp = false;
//			size_t p = cmd.find_first_not_of(" \t", 1);
//			if( p != string::npos && p > 1 )
//			{
//				if( cmd[p] == 'b' )
//				{
//					ListBreakPoints();
//				}
//				else if( cmd[p] == 'v' )
//				{
//					ListLocalVariables(ctx);
//				}
//				else if( cmd[p] == 'g' )
//				{
//					ListGlobalVariables(ctx);
//				}
//				else if( cmd[p] == 'm' )
//				{
//					ListMemberProperties(ctx);
//				}
//				else if( cmd[p] == 's' )
//				{
//					ListStatistics(ctx);
//				}
//				else
//				{
//					Output("Unknown list option.\n");
//					printHelp = true;
//				}
//			}
//			else
//			{
//				Output("Incorrect format for list command.\n");
//				printHelp = true;
//			}
//
//			if( printHelp )
//			{
//				Output("Expected format: \n"
//					   " l <list option>\n"
//					   "Available options: \n"
//					   " b - breakpoints\n"
//					   " v - local variables\n"
//					   " m - member properties\n"
//					   " g - global variables\n"
//					   " s - statistics\n");
//			}
//		}
//		// take more commands
//		return false;
//
//	case 'h':
//		PrintHelp();
//		// take more commands
//		return false;
//
//	case 'p':
//		{
//			// Print a value 
//			size_t p = cmd.find_first_not_of(" \t", 1);
//			if( p != string::npos && p > 1 )
//			{
//				PrintValue(cmd.substr(p), ctx);
//			}
//			else
//			{
//				Output("Incorrect format for print, expected:\n"
//					   " p <expression>\n");
//			}
//		}
//		// take more commands
//		return false;
//
//	case 'w':
//		// Where am I?
//		PrintCallstack(ctx);
//		// take more commands
//		return false;
//
//	case 'a':
//		// abort the execution
//		if( ctx == 0 )
//		{
//			Output("No script is running\n");
//			return false;
//		}
//		ctx->Abort();
//		break;
//
//	default:
//		Output("Unknown command\n");
//		// take more commands
//		return false;
//	}
//
//	// Continue execution
//	return true;
//}
//
//void CDebugger::PrintValue(const std::string &expr, asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	asIScriptEngine *engine = ctx->GetEngine();
//
//	// Tokenize the input string to get the variable scope and name
//	asUINT len = 0;
//	string scope;
//	string name;
//	string str = expr;
//	asETokenClass t = engine->ParseToken(str.c_str(), 0, &len);
//	while( t == asTC_IDENTIFIER || (t == asTC_KEYWORD && len == 2 && str.compare(0, 2, "::") == 0) )
//	{
//		if( t == asTC_KEYWORD )
//		{
//			if( scope == "" && name == "" )
//				scope = "::";			// global scope
//			else if( scope == "::" || scope == "" )
//				scope = name;			// namespace
//			else
//				scope += "::" + name;	// nested namespace
//			name = "";
//		}
//		else if( t == asTC_IDENTIFIER )
//			name.assign(str.c_str(), len);
//
//		// Skip the parsed token and get the next one
//		str = str.substr(len);
//		t = engine->ParseToken(str.c_str(), 0, &len);
//	}
//
//	if( name.size() )
//	{
//		// Find the variable
//		void *ptr = 0;
//		int typeId = 0;
//
//		asIScriptFunction *func = ctx->GetFunction();
//		if( !func ) return;
//
//		// skip local variables if a scope was informed
//		if( scope == "" )
//		{
//			// We start from the end, in case the same name is reused in different scopes
//			for( asUINT n = func->GetVarCount(); n-- > 0; )
//			{
//				const char* varName = 0;
//				ctx->GetVar(n, 0, &varName, &typeId);
//				if( ctx->IsVarInScope(n) && varName != 0 && name == varName )
//				{
//					ptr = ctx->GetAddressOfVar(n);
//					break;
//				}
//			}
//
//			// Look for class members, if we're in a class method
//			if( !ptr && func->GetObjectType() )
//			{
//				if( name == "this" )
//				{
//					ptr = ctx->GetThisPointer();
//					typeId = ctx->GetThisTypeId();
//				}
//				else
//				{
//					asITypeInfo *type = engine->GetTypeInfoById(ctx->GetThisTypeId());
//					for( asUINT n = 0; n < type->GetPropertyCount(); n++ )
//					{
//						const char *propName = 0;
//						int offset = 0;
//						bool isReference = 0;
//						int compositeOffset = 0;
//						bool isCompositeIndirect = false;
//						type->GetProperty(n, &propName, &typeId, 0, 0, &offset, &isReference, 0, &compositeOffset, &isCompositeIndirect);
//						if( name == propName )
//						{
//							ptr = (void*)(((asBYTE*)ctx->GetThisPointer())+compositeOffset);
//							if (isCompositeIndirect) ptr = *(void**)ptr;
//							ptr = (void*)(((asBYTE*)ptr) + offset);
//							if( isReference ) ptr = *(void**)ptr;
//							break;
//						}
//					}
//				}
//			}
//		}
//
//		// Look for global variables
//		if( !ptr )
//		{
//			if( scope == "" )
//			{
//				// If no explicit scope was informed then use the namespace of the current function by default
//				scope = func->GetNamespace();
//			}
//			else if( scope == "::" )
//			{
//				// The global namespace will be empty
//				scope = "";
//			}
//
//			asIScriptModule *mod = func->GetModule();
//			if( mod )
//			{
//				for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
//				{
//					const char *varName = 0, *nameSpace = 0;
//					mod->GetGlobalVar(n, &varName, &nameSpace, &typeId);
//
//					// Check if both name and namespace match
//					if( name == varName && scope == nameSpace )
//					{
//						ptr = mod->GetAddressOfGlobalVar(n);
//						break;
//					}
//				}
//			}
//		}
//
//		if( ptr )
//		{
//			// TODO: If there is a . after the identifier, check for members
//			// TODO: If there is a [ after the identifier try to call the 'opIndex(expr) const' method 
//			if( str != "" )
//			{
//				Output("Invalid expression. Expression doesn't end after symbol\n");
//			}
//			else
//			{
//				stringstream s;
//				// TODO: Allow user to set if members should be expanded
//				// Expand members by default to 3 recursive levels only
//				s << ToString(ptr, typeId, 3, engine) << endl;
//				Output(s.str());
//			}
//		}
//		else
//		{
//			Output("Invalid expression. No matching symbol\n");
//		}
//	}
//	else
//	{
//		Output("Invalid expression. Expected identifier\n");
//	}
//}
//
//void CDebugger::ListBreakPoints()
//{
//	// List all break points
//	stringstream s;
//	for( size_t b = 0; b < m_breakPoints.size(); b++ )
//		if( m_breakPoints[b].func )
//			s << b << " - " << m_breakPoints[b].name << endl;
//		else
//			s << b << " - " << m_breakPoints[b].name << ":" << m_breakPoints[b].lineNbr << endl;
//	Output(s.str());
//}
//
//void CDebugger::ListMemberProperties(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	void *ptr = ctx->GetThisPointer();
//	if( ptr )
//	{
//		stringstream s;
//		// TODO: Allow user to define if members should be expanded or not
//		// Expand members by default to 3 recursive levels only
//		s << "this = " << ToString(ptr, ctx->GetThisTypeId(), 3, ctx->GetEngine()) << endl;
//		Output(s.str());
//	}
//}
//
//void CDebugger::ListLocalVariables(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	asIScriptFunction *func = ctx->GetFunction();
//	if( !func ) return;
//
//	stringstream s;
//	for( asUINT n = 0; n < func->GetVarCount(); n++ )
//	{
//		// Skip temporary variables
//		// TODO: Should there be an option to view temporary variables too?
//		const char* name;
//		func->GetVar(n, &name);
//		if (name == 0 || strlen(name) == 0)
//			continue;
//
//		if( ctx->IsVarInScope(n) )
//		{
//			// TODO: Allow user to set if members should be expanded or not
//			// Expand members by default to 3 recursive levels only
//			int typeId;
//			ctx->GetVar(n, 0, 0, &typeId);
//			s << func->GetVarDecl(n) << " = " << ToString(ctx->GetAddressOfVar(n), typeId, 3, ctx->GetEngine()) << endl;
//		}
//	}
//	Output(s.str());
//}
//
//void CDebugger::ListGlobalVariables(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	// Determine the current module from the function
//	asIScriptFunction *func = ctx->GetFunction();
//	if( !func ) return;
//
//	asIScriptModule *mod = func->GetModule();
//	if( !mod ) return;
//
//	stringstream s;
//	for( asUINT n = 0; n < mod->GetGlobalVarCount(); n++ )
//	{
//		int typeId = 0;
//		mod->GetGlobalVar(n, 0, 0, &typeId);
//		// TODO: Allow user to set how many recursive expansions should be done
//		// Expand members by default to 3 recursive levels only
//		s << mod->GetGlobalVarDeclaration(n) << " = " << ToString(mod->GetAddressOfGlobalVar(n), typeId, 3, ctx->GetEngine()) << endl;
//	}
//	Output(s.str());
//}
//
//void CDebugger::ListStatistics(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	asIScriptEngine *engine = ctx->GetEngine();
//	
//	asUINT gcCurrSize, gcTotalDestr, gcTotalDet, gcNewObjects, gcTotalNewDestr;
//	engine->GetGCStatistics(&gcCurrSize, &gcTotalDestr, &gcTotalDet, &gcNewObjects, &gcTotalNewDestr);
//
//	stringstream s;
//	s << "Garbage collector:" << endl;
//	s << " current size:          " << gcCurrSize << endl;
//	s << " total destroyed:       " << gcTotalDestr << endl;
//	s << " total detected:        " << gcTotalDet << endl;
//	s << " new objects:           " << gcNewObjects << endl;
//	s << " new objects destroyed: " << gcTotalNewDestr << endl;
//
//	Output(s.str());
//}
//
//void CDebugger::PrintCallstack(asIScriptContext *ctx)
//{
//	if( ctx == 0 )
//	{
//		Output("No script is running\n");
//		return;
//	}
//
//	stringstream s;
//	const char *file = 0;
//	int lineNbr = 0;
//	for( asUINT n = 0; n < ctx->GetCallstackSize(); n++ )
//	{
//		lineNbr = ctx->GetLineNumber(n, 0, &file);
//		s << (file ? file : "{unnamed}") << ":" << lineNbr << "; " << ctx->GetFunction(n)->GetDeclaration() << endl;
//	}
//	Output(s.str());
//}
//
//void CDebugger::AddFuncBreakPoint(const string &func)
//{
//	// Trim the function name
//	size_t b = func.find_first_not_of(" \t");
//	size_t e = func.find_last_not_of(" \t");
//	string actual = func.substr(b, e != string::npos ? e-b+1 : string::npos);
//
//	stringstream s;
//	s << "Adding deferred break point for function '" << actual << "'" << endl;
//	Output(s.str());
//
//	BreakPoint bp(actual, 0, true);
//	m_breakPoints.push_back(bp);
//}
//
//void CDebugger::AddFileBreakPoint(const string &file, int lineNbr)
//{
//	// Store just file name, not entire path
//	size_t r = file.find_last_of("\\/");
//	string actual;
//	if( r != string::npos )
//		actual = file.substr(r+1);
//	else
//		actual = file;
//
//	// Trim the file name
//	size_t b = actual.find_first_not_of(" \t");
//	size_t e = actual.find_last_not_of(" \t");
//	actual = actual.substr(b, e != string::npos ? e-b+1 : string::npos);
//
//	stringstream s;
//	s << "Setting break point in file '" << actual << "' at line " << lineNbr << endl;
//	Output(s.str());
//
//	BreakPoint bp(actual, lineNbr, false);
//	m_breakPoints.push_back(bp);
//}
//
//void CDebugger::PrintHelp()
//{
//	Output(" c - Continue\n"
//	       " s - Step into\n"
//	       " n - Next step\n"
//	       " o - Step out\n"
//	       " b - Set break point\n"
//	       " l - List various things\n"
//	       " r - Remove break point\n"
//	       " p - Print value\n"
//	       " w - Where am I?\n"
//	       " a - Abort execution\n"
//	       " h - Print this help text\n");
//}
//
//void CDebugger::Output(const string &str)
//{
//	// By default we just output to stdout
//	cout << str;
//}
//
//void CDebugger::SetEngine(asIScriptEngine *engine)
//{
//	if( m_engine != engine )
//	{
//		if( m_engine )
//			m_engine->Release();
//		m_engine = engine;
//		if( m_engine )
//			m_engine->AddRef();
//	}
//}
//
//asIScriptEngine *CDebugger::GetEngine()
//{
//	return m_engine;
//}
//
//END_AS_NAMESPACE
