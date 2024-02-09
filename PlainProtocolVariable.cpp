// 
// 
// 

#include "PlainProtocolVariable.h"

PlainProtocolVariableBase* PlainProtocolVariableBase::registeredVariables[MAX_PP_REGISTERED_VARIABLES];
uint8_t PlainProtocolVariableBase::registeredVariableCount = 0;
PlainProtocol* PlainProtocolVariableBase::plainProtocol = NULL;