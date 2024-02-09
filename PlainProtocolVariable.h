// PlainProtocolVariable.h

#ifndef _PLAINPROTOCOLVARIABLE_h
#define _PLAINPROTOCOLVARIABLE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "ArduinoDevice.h"

#define MAX_PP_STRING_LENGTH			32
#define MAX_PP_REGISTERED_VARIABLES		32

/// <summary>
/// The abstract-, wrapper-, base-class for a variable to be sent via PlainProtocol
/// </summary>
class PlainProtocolVariableBase : public IArduinoDevice
{

#pragma region STATIC

	static PlainProtocolVariableBase* registeredVariables[MAX_PP_REGISTERED_VARIABLES];
	static uint8_t registeredVariableCount;

protected:
	static PlainProtocol* plainProtocol;
	
public:
	static void SetPlainProtcolPointer(PlainProtocol& pp)
	{
		plainProtocol = &pp;
	}

	static PlainProtocolVariableBase* GetRegisteredVariableByIndex(uint8_t index)
	{
		if (index >= registeredVariableCount)
		{
			Serial.println("INVALID VARIABLE INDEX!!");
			return NULL;
		}

		return registeredVariables[index];
	}

	static void PrintAllRegisteredVariables() //debug-ish
	{
		Serial.println("***PRINTING ALL REGISTERED VARIABLES***");

		for (int i = 0; i < registeredVariableCount; i++)
		{
			Serial.print("Variable Name:");
			Serial.print(registeredVariables[i]->GetCommand());
			
			Serial.println();
		}
	}

	static String GetAllFrames()
	{
		String message = "";

		for (int i = 0; i < registeredVariableCount; i++)
		{
			PlainProtocolVariableBase* pCurrent = registeredVariables[i];
			if (pCurrent->enabled)
				message += pCurrent->GetFrame();
		}

		return message;
	}

#pragma endregion

protected:
	/// <summary>
	/// The unique name or identifier of this particular variable
	/// </summary>
	char ppCommand[MAX_PP_STRING_LENGTH];

	//char ppContent

 public:
	PlainProtocolVariableBase(String _name, bool _enabled = true)
	{
		enabled = _enabled;

		strncpy(ppCommand, _name.c_str(), MAX_PP_STRING_LENGTH);
		registeredVariables[registeredVariableCount++] = this;
	}

	virtual uint8_t Initialize()
	{
		return 0;
	}

	virtual uint8_t Update()
	{
		return 0;
	}

	virtual String GetCommand() const
	{
		String cmd = "";// "<";
		cmd += ppCommand;
		//cmd += ">";
		return cmd;
	}

	virtual String GetFrame() const = 0;

	virtual void Write() = 0;
};

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/// <summary>
/// The templated version of the PlainProtocolBase class
/// </summary>
/// <typeparam name="_Type">The type of the internal variable</typeparam>
template <typename _Type>
class PlainProtocolVariable : public PlainProtocolVariableBase
{
	//PlainProtocolVariable(PlainProtocolVariable&& ctrArg);
	//PlainProtocolVariable(const PlainProtocolVariable& ctrArg);

protected:
	bool isRefType;

	union
	{
		_Type variable;
		_Type* varRef;
	};

public:
	PlainProtocolVariable(String _name, bool _enabled = true)
		: PlainProtocolVariableBase(_name, _enabled)
	{
	}

	PlainProtocolVariable(String _name, _Type& attachedVar, bool _enabled = true)
		: PlainProtocolVariableBase(_name, _enabled),
		varRef(&attachedVar), isRefType(true)
	{
	}

	PlainProtocolVariable(String _name, _Type initialValue, bool _enabled = true)
		: PlainProtocolVariableBase(_name, _enabled), variable(initialValue)
	{
	}

	virtual uint8_t Initialize()
	{
		return PlainProtocolVariableBase::Initialize();
	}

	virtual uint8_t Update()
	{
		return PlainProtocolVariableBase::Update();
	}

	virtual String GetFrame() const
	{
		String cmd = GetCommand();
		cmd += String(variable);
		cmd += ";";
		return cmd;
	}

	virtual void Write()
	{
		//PlainProtocol
		plainProtocol->write(GetCommand(), isRefType ? *varRef : variable);
	}

	_Type operator = (const _Type& rhs)
	{
		if (isRefType)
			*varRef = rhs;
		else
			variable = rhs;
	}

	bool operator == (const _Type& rhs) const
	{
		if (isRefType)
			return *varRef == rhs;
		else
			return variable == rhs;
	}

	bool operator < (const _Type& rhs) const
	{
		if (isRefType)
			return *varRef < rhs;
		else
			return variable < rhs;
	}

	bool operator > (const _Type& rhs) const
	{
		if (isRefType)
			return *varRef > rhs;
		else
			return variable > rhs;
	}

	operator _Type() const
	{
		if (isRefType)
			return *varRef;
		else
			return variable;
	}
	
	virtual void ProcessInput(PlainProtocol& input)
	{
		Serial.println("Shouldn't be here!!!");
	}
};

//template <>
//class PlainProtocolVariable<uint8_t[]

typedef PlainProtocolVariable<uint8_t>		PPV_UInt8;
typedef PlainProtocolVariable<int8_t>		PPV_Int8;
typedef PlainProtocolVariable<uint16_t>		PPV_UInt16;
typedef PlainProtocolVariable<int16_t>		PPV_Int16;
typedef PlainProtocolVariable<uint32_t>		PPV_UInt32;
typedef PlainProtocolVariable<int32_t>		PPV_Int32;
typedef PlainProtocolVariable<float>		PPV_Float;
typedef PlainProtocolVariable<double>		PPV_Double;
typedef PlainProtocolVariable<byte>			PPV_Byte;
typedef PlainProtocolVariable<char>			PPV_Char;
typedef PlainProtocolVariable<String>		PPV_String;

#endif

