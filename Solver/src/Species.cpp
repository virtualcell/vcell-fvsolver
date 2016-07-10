#include <stdexcept>
#include <Species.h>
#include <VCellException.h>
using moving_boundary::biology::Species;

void Species::bindExpressions(const SimpleSymbolTable &symTable) {
	sourceExp.bindExpression(symTable);
	diffusionExp.bindExpression(symTable);
}
