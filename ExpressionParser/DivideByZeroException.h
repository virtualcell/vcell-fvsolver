#ifndef DIVIDEBYZEROEXCEPTION_H
#define DIVIDEBYZEROEXCEPTION_CPP

#include "ExpressionException.h"

class DivideByZeroException : public ExpressionException
{
public:
	DivideByZeroException(string msg);
	~DivideByZeroException(void);
};
#endif
