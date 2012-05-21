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
		return m_realLibrary->GetBuffer(Index, Buffer, Length);
	}

	virtual bool SetBuffer( unsigned int Index, unsigned char *Buffer, int Length )
	{
		if(Index == 0xB6E372AE) //set encryption
			BNETHookSetEncryptionKey(Buffer, Length);
		return m_realLibrary->SetBuffer(Index, Buffer, Length);
	}

	virtual void ClearBuffer(unsigned int Index)
	{
		return m_realLibrary->ClearBuffer(Index);
	}

	virtual void InvokeCommand(char* command, InvokeParameterImplementation** ppParams, int ParamCount)
	{
		return m_realLibrary->InvokeCommand(command, ppParams, ParamCount);
	}

	virtual void SetResponseData( uint8_t *pbBuffer, int Length )
	{
		return m_realLibrary->SetResponseData(pbBuffer, Length);
	}

	virtual void ClearResponseData() 
	{
		return m_realLibrary->ClearResponseData();
	}

	virtual void Function7()
	{
		return m_realLibrary->Function7();
	}

	virtual void Function8(int errorCode, int a1,int a2) 
	{
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
		m_realModule->InvokeCommand(new ProxyLibrary(ChallengeHandlerClass), Command, ppInvokeParameterImplementation, ParamCount);
	}

	virtual void Data( class Library* ChallengeHandlerClass, char* Data, unsigned int Length )
	{
		m_realModule->Data(new ProxyLibrary(ChallengeHandlerClass), Data, Length);
	}
};

CREATEMODULE realCreateModule = nullptr;

Module *ProxyCreateModule()
{
	return new ProxyModule(realCreateModule());
}
