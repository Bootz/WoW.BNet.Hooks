#pragma once

#include <cstdint>

enum InvokeParameterType
{
	PARAM_TYPE_BOOL = 0,
	PARAM_TYPE_INT = 1,
	PARAM_TYPE_STRING = 2,
};
class InvokeParameterImplementation
{
public:
	virtual void Function1( int a1, int a2, int a3 )
	{
	}
	virtual bool GetBoolean( uint8_t* pbBuffer )
	{
		if ( FieldType != PARAM_TYPE_BOOL)
			return false;
		pbBuffer[0] = field_8[0];
		return true;
	}
	virtual bool GetInt( int& Buffer )
	{
		if ( FieldType == PARAM_TYPE_INT )
			Buffer = IntField;
		else
			return false;

		return true;
	}
	virtual bool GetString( char*& Buffer )
	{
		if ( FieldType == PARAM_TYPE_STRING )
			Buffer = StringField;
		else
			return false;

		return true;
	}

	int FieldType; // Type: 0 = Bool, 1 = Int, 2 = String
	uint8_t field_8[4];
	int IntField;
	char* StringField;
};

class Library
{
public:
	virtual bool GetBuffer( unsigned int Index, unsigned char **Buffer, int *Length) = 0;
	virtual bool SetBuffer( unsigned int Index, unsigned char *Buffer, int Length ) = 0;
	virtual void ClearBuffer(unsigned int Index) = 0;
	virtual void InvokeCommand(char* command, InvokeParameterImplementation** ppParams, int ParamCount) = 0;
	virtual void SetResponseData( uint8_t *pbBuffer, int Length ) = 0;
	virtual void ClearResponseData() = 0;
	virtual void Function7() = 0;
	virtual void Function8(int errorCode,int,int) = 0;
};

class Module
{
public:
	virtual void Destroy() = 0;
	virtual void InvokeCommand( class Library* ChallengeHandlerClass, const char* Command,
		InvokeParameterImplementation **ppInvokeParameterImplementation, int ParamCount = 1) = 0;
	virtual void Data( class Library* ChallengeHandlerClass, char* Data, unsigned int Length ) = 0;
};