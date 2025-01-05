#include "Engine_PCH.h"
#include "AngelScriptDebuggerMessagePackager.h"

#include "AngelScriptDebugger.h"
#include "Utility\FilePath.hpp"


using namespace Hail;
using namespace AngelScript;


namespace ReadMessages
{

	void ReadAndIncrementReadPoint(void* destination, void* streamToReadFrom, uint32& currentReadPoint, uint32 sizeToRead)
	{
		memcpy(destination, (uint8*)streamToReadFrom + currentReadPoint, sizeToRead);
		currentReadPoint += sizeToRead;
	}

	void ReadAndIncrementString(StringL& destination, void* streamToReadFrom, uint32& currentReadPoint)
	{
		int32 stringLength;
		ReadAndIncrementReadPoint(&stringLength, streamToReadFrom, currentReadPoint, 4);
		destination.Reserve(stringLength);
		ReadAndIncrementReadPoint(destination, streamToReadFrom, currentReadPoint, stringLength);
	}

	bool IsPathAValidProjectPath(const FilePath& filePathToValidate)
	{
		const int16 commonLowestDirectoryLevel = FilePath::FindCommonLowestDirectoryLevel(filePathToValidate, FilePath::GetAngelscriptDirectory());
		return commonLowestDirectoryLevel == FilePath::GetAngelscriptDirectory().GetDirectoryLevel();
	}

	void GenerateFileBreakPoints(DebuggerServer* pDebugger, MessageHeader header, void* messageStream)
	{
		uint32 currentReadPoint = 0;
		FileBreakPoints breakPoints;

		ReadAndIncrementString(breakPoints.fileName, messageStream, currentReadPoint);

		const FilePath filePath = breakPoints.fileName;


		if (!IsPathAValidProjectPath(filePath))
		{
			H_ERROR("Recieving breakpoints from a non project path.")
			return;
		}
		else
		{
			const String64 fileName = filePath.Object().Name().CharString();
			breakPoints.fileName = fileName.Data();
			// Get filename
		}

		int16 numberOfBreakPoints;
		ReadAndIncrementReadPoint(&numberOfBreakPoints, messageStream, currentReadPoint, 2);
		for (size_t i = 0; i < numberOfBreakPoints; i++)
		{
			BreakPoint breakPoint;
			ReadAndIncrementReadPoint(&breakPoint.line, messageStream, currentReadPoint, 2);
			ReadAndIncrementReadPoint(&breakPoint.bHasConditional, messageStream, currentReadPoint, 1);
			if (breakPoint.bHasConditional)
			{
				ReadAndIncrementString(breakPoint.condition, messageStream, currentReadPoint);
			}
			breakPoints.breakPoints.Add(breakPoint);
		}

		pDebugger->AddBreakpoints(breakPoints);
	}

	void DecodeVariableRequest(DebuggerServer* pDebugger, MessageHeader header, void* messageStream)
	{
		StringL request;
		uint32 readPoint = 0;
		ReadAndIncrementString(request, messageStream, readPoint);

		int32 separatorSymbolIndex = StringUtility::FindFirstOfSymbol(request, ':');
		if (separatorSymbolIndex == -1)
		{
			H_ERROR("Invalid variable request, no separator in message. Check debugger extension.");
			// TODO: Send empty return message
		}
		int32 frameReference = StringUtility::IntFromConstChar(request, 0);
		const char* stackType = request.Data() + separatorSymbolIndex + 1;


		if (StringCompare(stackType, "local"))
		{
			pDebugger->SendVariables(eCallStack::local);
		}
		else if (StringCompare(stackType, "this"))
		{
			pDebugger->SendVariables(eCallStack::self);
		}
		else if (StringCompare(stackType, "global"))
		{
			pDebugger->SendVariables(eCallStack::global);
		}

		// TODO: fix for child fetching
		if (StringContains(stackType, '.'))
		{

		}
	}

	void DecodeEvaluateRequest(DebuggerServer* pDebugger, MessageHeader msgHeader, void* messageStream)
	{
		uint32 readPoint = 0;
		StringL requestExpression;
		ReadAndIncrementString(requestExpression, messageStream, readPoint);
		uint32 scope = 0;
		ReadAndIncrementReadPoint(&scope, messageStream, readPoint, sizeof(uint32));
		pDebugger->FindVariable((eCallStack)scope, requestExpression);
	}

	void HandleDebuggerMessageInternal(DebuggerServer* pDebugger, MessageHeader header, void* messageStream)
	{
		switch (header.type)
		{
		case eDebuggerMessageType::StartDebugSession:
			pDebugger->StartDebugging();
			H_DEBUGMESSAGE("Recieved start session.");
			break;
		case eDebuggerMessageType::Paused:
			pDebugger->PauseDebugging();
			H_DEBUGMESSAGE("Recieved pause.");
			break;
		case eDebuggerMessageType::Disconnect:
			pDebugger->StopDebugging();
			H_DEBUGMESSAGE("Recieved disconnect.");
			break;
		case eDebuggerMessageType::Continued:
			pDebugger->ContinueDebugging();
			H_DEBUGMESSAGE("Recieved Continued.");
			break;
		case eDebuggerMessageType::CreateBreakpoints:
			GenerateFileBreakPoints(pDebugger, header, messageStream);
			H_DEBUGMESSAGE("Recieved CreateBreakpoints.");
			break;
		case eDebuggerMessageType::CallStack:
			pDebugger->SendCallstack();
			break;
		case eDebuggerMessageType::VariableRequest:
			DecodeVariableRequest(pDebugger, header, messageStream);
			H_DEBUGMESSAGE("Recieved VariableRequest.");
			break;
		case eDebuggerMessageType::EvaluateRequest:
			DecodeEvaluateRequest(pDebugger, header, messageStream);
			H_DEBUGMESSAGE("Recieved EvaluateRequest.");
			break;
		case eDebuggerMessageType::StepIn:
			pDebugger->StepIn();
			H_DEBUGMESSAGE("Recieved StepIn.");
			break;
		case eDebuggerMessageType::StepOver:
			pDebugger->StepOver();
			H_DEBUGMESSAGE("Recieved StepOver.");
			break;
		case eDebuggerMessageType::StepOut:
			pDebugger->StepOut();
			H_DEBUGMESSAGE("Recieved StepOut.");
			break;
		default:
			break;
		}
	}
}

namespace WriteMessages
{
	MessageHeader WriteHeader(int sizeOfMessage, eDebuggerMessageType type)
	{
		MessageHeader header;
		header.messageLength = sizeOfMessage;
		header.type = type;
		return header;
	}

	uint32 EncodeString(MessageData& memoryToFill, const StringL& stringToEncode)
	{
		uint32 offsetMoved = 0;
		uint16 stringLength = stringToEncode.Length();
		memoryToFill.FillMessageBuffer(&stringLength, sizeof(stringLength));
		offsetMoved += sizeof(stringLength);
		memoryToFill.FillMessageBuffer((void*)stringToEncode.Data(), sizeof(char) * stringLength);
		offsetMoved += sizeof(char) * stringLength;
		return offsetMoved;
	}

	template <typename T>
	void EncodeBaseType(MessageData& memoryToFill, uint32& bufferOffset, T dataToWrite)
	{
		// TODO: Assert if not base type
		memoryToFill.FillMessageBuffer(&dataToWrite, sizeof(T));
		bufferOffset += sizeof(T);
	}
}

void Hail::AngelScript::HandleDebuggerMessage(DebuggerServer* pDebugger, uint32 messageLength, void* messageStream)
{
	uint32 currentPosition = 0;
	while (currentPosition < messageLength)
	{
		MessageHeader header;
		ReadMessages::ReadAndIncrementReadPoint(&header, messageStream, currentPosition, 4);

		ReadMessages::HandleDebuggerMessageInternal(pDebugger, header, (uint8*)messageStream + currentPosition);
		currentPosition += header.messageLength;
	}
}

DebuggerMessage Hail::AngelScript::CreateStopDebugSessionMessage()
{
	DebuggerMessage message;
	message.m_header = WriteMessages::WriteHeader(0, eDebuggerMessageType::Disconnect);
	return message;
}

DebuggerMessage Hail::AngelScript::CreateHitBreakpointMessage(int line, const StringL& file)
{
	DebuggerMessage message;
	message.m_header = WriteMessages::WriteHeader(file.Length() + sizeof(uint16) + sizeof(int), eDebuggerMessageType::HitBreakpoint);

	uint32 offset = WriteMessages::EncodeString(message.m_data, file);
	message.m_data.FillMessageBuffer(&line, sizeof(int));
	offset += sizeof(int);
	H_ASSERT(message.m_header.messageLength == offset, "Message length must match final offset");
	return message;
}

DebuggerMessage Hail::AngelScript::CreateStopExecutionMessage()
{
	DebuggerMessage message;
	message.m_header = WriteMessages::WriteHeader(0, eDebuggerMessageType::StopExecution);
	return message;
}

DebuggerMessage Hail::AngelScript::CreateCallstackMessage(const GrowingArray<StackFrame>& callstackToSend)
{
	DebuggerMessage message;

	uint32 bufferOffset = 0;
	WriteMessages::EncodeBaseType(message.m_data, bufferOffset, (uint32)callstackToSend.Size());
	for (size_t i = 0; i < callstackToSend.Size(); i++)
	{
		WriteMessages::EncodeBaseType(message.m_data, bufferOffset, callstackToSend[i].m_line);
		bufferOffset += WriteMessages::EncodeString(message.m_data, callstackToSend[i].m_functionName);
		bufferOffset += WriteMessages::EncodeString(message.m_data, callstackToSend[i].m_sourceFile);
	}

	message.m_header = WriteMessages::WriteHeader(bufferOffset, eDebuggerMessageType::CallStack);

	H_ASSERT(message.m_header.messageLength == bufferOffset, "Message length must match final offset");
	return message;
}

void EncodeVariable(const Variable& variableToEncode, uint32& currentOffset, MessageData& messageToFill)
{
	currentOffset += WriteMessages::EncodeString(messageToFill, variableToEncode.m_name);
	currentOffset += WriteMessages::EncodeString(messageToFill, variableToEncode.m_type);
	currentOffset += WriteMessages::EncodeString(messageToFill, variableToEncode.m_value);
	uint32 numberOfMembers = variableToEncode.m_members.Size();
	WriteMessages::EncodeBaseType(messageToFill, currentOffset, numberOfMembers);
	for (size_t i = 0; i < numberOfMembers; i++)
	{
		EncodeVariable(variableToEncode.m_members[i], currentOffset, messageToFill);
	}
}

DebuggerMessage Hail::AngelScript::CreateVariablesMessage(const GrowingArray<Variable>* pVariableScopeToSend)
{
	DebuggerMessage message;
	uint32 bufferOffset = 0;
	const uint32 numberOfVariables = pVariableScopeToSend ? pVariableScopeToSend->Size() : 0;
	WriteMessages::EncodeBaseType(message.m_data, bufferOffset, numberOfVariables);
	uint32 numberOfVariablesEncoded = 0;
	for (size_t i = 0; i < numberOfVariables; i++)
	{
		const Variable& variableToWrite = (*pVariableScopeToSend)[i];
		EncodeVariable(variableToWrite, bufferOffset, message.m_data);
	}
	message.m_header = WriteMessages::WriteHeader(bufferOffset, eDebuggerMessageType::VariableRequest);
	return message;
}

DebuggerMessage Hail::AngelScript::CreateVariableMessage(const Variable* pVariableScopeToSend)
{
	DebuggerMessage message;
	uint32 bufferOffset = 0;
	if (pVariableScopeToSend)
	{
		EncodeVariable(*pVariableScopeToSend, bufferOffset, message.m_data);
	}
	message.m_header = WriteMessages::WriteHeader(bufferOffset, eDebuggerMessageType::EvaluateRequest);
	return message;
}

char* Hail::AngelScript::MessageData::GetMessageData()
{
	return m_extendableMessage.Empty() ? m_message : m_extendableMessage.Data();
}

void Hail::AngelScript::MessageData::FillMessageBuffer(void* dataToFillWith, uint32 numberOfBytesToFill)
{
	if (numberOfBytesToFill + m_dataAmount > 1024)
	{
		if (m_extendableMessage.Empty())
		{
			m_extendableMessage.PrepareAndFill((numberOfBytesToFill + m_dataAmount) * 2);
			memcpy(m_extendableMessage.Data(), m_message, m_dataAmount);
		}
		else if (numberOfBytesToFill + m_dataAmount > m_extendableMessage.Size())
		{
			m_extendableMessage.AddN(numberOfBytesToFill);
		}
	}

	if (m_extendableMessage.Empty())
	{
		memcpy(m_message + m_dataAmount, dataToFillWith, numberOfBytesToFill);
	}
	else
	{
		memcpy(m_extendableMessage.Data() + m_dataAmount, dataToFillWith, numberOfBytesToFill);
	}
	m_dataAmount += numberOfBytesToFill;
}
