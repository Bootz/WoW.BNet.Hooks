#include "BNETProxy.h"
#include "BNETModule.h"
#include "BNETHook.h"

class ProxyLibrary : public Library
{
private:
	Library *m_realLibrary;
public:
	ProxyLibrary(Library *impl) : m_realLibrary(impl) {}
	virtual ~ProxyLibrary() {}

	virtual bool GetBuffer( unsigned int Index, unsigned char **Buffer, int *Length)
	{
		bool ret = m_realLibrary->GetBuffer(Index, Buffer, Length);
		if(Length)
			BNETHookLog(L"ProxyLibrary Getbuffer %x %d", Index, *Length);
		return ret;
	}

	virtual bool SetBuffer( unsigned int Index, unsigned char *Buffer, int Length )
	{
		BNETHookLog(L"ProxyLibrary SetBuffer %x %d",Index, Length);
		if(Index == 0xB6E372AE) //set encryption
			BNETHookSetEncryptionKey(Buffer, Length);
		return m_realLibrary->SetBuffer(Index, Buffer, Length);
	}

	virtual void ClearBuffer(unsigned int Index)
	{
		BNETHookLog(L"ProxyLibrary ClearBuffer %x", Index);
		return m_realLibrary->ClearBuffer(Index);
	}

	virtual void InvokeCommand(char* command, InvokeParameterImplementation** ppParams, int ParamCount)
	{
		BNETHookLog(L"ProxyLibrary InvokeCommand %S", command);
		for(int i = 0; i < ParamCount; i ++)
		{
			if(ppParams[i]->FieldType == 2)
				BNETHookLog(L"ProxyLibrary InvokeCommand Param %S", ppParams[i]->StringField);
			else if(ppParams[i]->FieldType == 1)
				BNETHookLog(L"ProxyLibrary InvokeCommand Param %d", ppParams[i]->IntField);
			else if(ppParams[i]->FieldType == 0)
				BNETHookLog(L"ProxyLibrary InvokeCommand Param %d", ppParams[i]->field_8[0]);
		}
		return m_realLibrary->InvokeCommand(command, ppParams, ParamCount);
	}

	virtual void SetResponseData( uint8_t *pbBuffer, int Length )
	{
		BNETHookLog(L"ProxyLibrary SetResponseData %d", Length);
		return m_realLibrary->SetResponseData(pbBuffer, Length);
	}

	virtual void ClearResponseData() 
	{
		BNETHookLog(L"ProxyLibrary ClearResponseData");
		return m_realLibrary->ClearResponseData();
	}

	virtual void Function7()
	{
		BNETHookLog(L"ProxyLibrary Function7");
		return m_realLibrary->Function7();
	}

	virtual void Function8(int errorCode, int a1,int a2) 
	{
		BNETHookLog(L"ProxyLibrary Function8");
		return m_realLibrary->Function8(errorCode, a1, a2);
	}
};

class ProxyModule : public Module
{
private:
	Module *m_realModule;
public:
	ProxyModule(Module *impl) : m_realModule(impl) {}
	virtual ~ProxyModule() {}
	virtual void Destroy()
	{
		m_realModule->Destroy();
	}

	virtual void InvokeCommand( class Library* ChallengeHandlerClass, const char* Command, InvokeParameterImplementation **ppInvokeParameterImplementation, int ParamCount = 1)
	{
		BNETHookLog(L"ProxyModule InvokeCommand %S", Command);
		for(int i = 0; i < ParamCount; i ++)
		{
			if(ppInvokeParameterImplementation[i]->FieldType == 2)
				BNETHookLog(L"ProxyModule InvokeCommand Param %S", ppInvokeParameterImplementation[i]->StringField);
			else if(ppInvokeParameterImplementation[i]->FieldType == 1)
				BNETHookLog(L"ProxyModule InvokeCommand Param %d", ppInvokeParameterImplementation[i]->IntField);
			else if(ppInvokeParameterImplementation[i]->FieldType == 0)
				BNETHookLog(L"ProxyModule InvokeCommand Param %d", ppInvokeParameterImplementation[i]->field_8[0]);
		}
		m_realModule->InvokeCommand(new ProxyLibrary(ChallengeHandlerClass), Command, ppInvokeParameterImplementation, ParamCount);
	}

	virtual void Data( class Library* ChallengeHandlerClass, char* Data, unsigned int Length )
	{
		BNETHookLog(L"ProxyModule Data. length: %d", Length);
		m_realModule->Data(new ProxyLibrary(ChallengeHandlerClass), Data, Length);
	}
};

CREATEMODULE realCreateModule = nullptr;

Module *ProxyCreateModule()
{
	BNETHookLog(L"CreateModule called.");
	return new ProxyModule(realCreateModule());
}
