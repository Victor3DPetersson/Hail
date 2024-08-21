#include "Engine_PCH.h"
#include "AngelScriptDebuggerMessagePackager.h"

#include "AngelScriptDebugger.h"
#include "Utility\FilePath.hpp"


using namespace Hail;
using namespace AngelScript;


namespace
{

	void ReadAndIncrementReadPoint(void* destination, void* streamToReadFrom, uint32& currentReadPoint, uint32 sizeToRead)
	{
		memcpy(destination, (uint8*)streamToReadFrom + currentReadPoint, sizeToRead);
		currentReadPoint += sizeToRead;
	}

	void ReadAndIncrementString(void* destination, void* streamToReadFrom, uint32& currentReadPoint)
	{
		int32 stringLength;
		ReadAndIncrementReadPoint(&stringLength, streamToReadFrom, currentReadPoint, 4);
		ReadAndIncrementReadPoint(destination, streamToReadFrom, currentReadPoint, stringLength);
	}
	;

	bool IsPathAValidProjectPath(const FilePath& filePathToValidate)
	{
		const int16 commonLowestDirectoryLevel = FilePath::FindCommonLowestDirectoryLevel(filePathToValidate, FilePath::GetAngelscriptDirectory());
		return commonLowestDirectoryLevel == FilePath::GetAngelscriptDirectory().GetDirectoryLevel();
	}

	void GenerateFileBreakPoints(DebuggerServer* pDebugger, MessageHeader header, void* messageStream)
	{
		uint32 currentReadPoint = 0;
		FileBreakPoints breakPoints;

		ReadAndIncrementString(breakPoints.fileName.Data(), messageStream, currentReadPoint);

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
				ReadAndIncrementString(breakPoint.condition.Data(), messageStream, currentReadPoint);
			}
			breakPoints.breakPoints.Add(breakPoint);
		}

		H_DEBUGMESSAGE(StringL::Format("Breakpoints in file: %s", breakPoints.fileName.Data()));
		for (size_t i = 0; i < breakPoints.breakPoints.Size(); i++)
		{
			H_DEBUGMESSAGE(StringL::Format("Breakpoint %i at line: %i", i, breakPoints.breakPoints[i].line));
		}
		pDebugger->AddBreakpoints(breakPoints);
	}


	void HandleDebuggerMessageInternal(DebuggerServer* pDebugger, MessageHeader header, void* messageStream)
	{
		switch (header.type)
		{
		case eDebuggerMessageType::StartDebugSession:
			pDebugger->StartDebugging();
			break;
		case eDebuggerMessageType::Disconnect:
			pDebugger->StopDebugging();
			break;
		case eDebuggerMessageType::Continued:
			pDebugger->ContinueDebugging();
			break;
		case eDebuggerMessageType::CreateBreakpoints:
			GenerateFileBreakPoints(pDebugger, header, messageStream);
			break;

		default:
			break;
		}
	}


}

void Hail::AngelScript::HandleDebuggerMessage(DebuggerServer* pDebugger, uint32 messageLength, void* messageStream)
{
	uint32 currentPosition = 0;
	while (currentPosition < messageLength)
	{
		MessageHeader header;
		ReadAndIncrementReadPoint(&header, messageStream, currentPosition, 4);

		HandleDebuggerMessageInternal(pDebugger, header, (uint8*)messageStream + currentPosition);
		currentPosition += header.messageLength;
	}
}
