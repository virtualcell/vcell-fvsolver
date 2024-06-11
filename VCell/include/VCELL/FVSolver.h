#ifndef FVSOLVER_H
#define FVSOLVER_H

#include <string>
using std::string;
using std::istream;

class SimTool;
class VCellModel;
namespace VCell {
	class Expression;
}
class VarContext;
class CartesianMesh;
class Variable;
class Feature;
class FastSystemExpression;
class SimulationExpression;
class Structure;
class Membrane;
//class PdeResultSet;

class FVSolver {
public:
	FVSolver(istream& fvinput, istream& vcgInput, int taskID=-1, const char* outdir=0, bool bSimZip=true);
	virtual ~FVSolver();

	void createSimTool(istream& ifsInput, istream& vcgInput, int taskID);
	void solve(bool bLoadFinal=true, double* paramValues=nullptr);

	void init(double* paramValues=0);
	void step(double* paramValues=0);
	
	double getCurrentTime();
	void setEndTime(double endTime);

	int getNumVariables();
	string getVariableName(int index);
	int getVariableLength(string& varName);
	double* getValue(string& varName, int arrayID);  // arrayID=0 for "old" and 1 for "current"
	void setInitialCondition(string& varName, int dataLength, const double* data);

	//void reinit(double *paramValues);
	//PdeResultSet* getPdeResultSet();
private:
	void loadJMSInfo(istream& ifsInput, int taskID);
	void loadModel(istream& ifsInput);
	void loadSimulation(istream& ifsInput);
	void loadSmoldyn(istream& ifsInput);
	VCell::Expression* readExpression(istream& ifsInput, string& var_name, string prefix="");
	VarContext* loadEquation(istream& ifsInput, Structure* structure, Variable* var);
	void loadJumpCondition(istream& ifsInput, Membrane*, string& var_name);
	void loadPseudoConstants(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastRates(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastDependencies(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadJacobians(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFastSystem(istream& ifsInput, FastSystemExpression* fastSystem);
	void loadFeature(istream& ifsInput, Feature* feature);
	void loadMembrane(istream& ifsInput, Membrane*);
	void loadSimulationParameters(istream& ifsInput);
	void loadMeshFromVcg(istream& vcgInput);
	void loadFieldData(istream& ifsInput);
	void loadParameters(istream& ifsInput, int numParams);
	void loadSerialScanParameters(istream& ifsInput, int numSerialScanParameters);
	void loadSerialScanParameterValues(istream& ifsInput, int numSerialScanParamValues);

	const char* outputPath;
	SimTool* simTool;
	SimulationExpression *simulation;
	VCellModel *model;
	CartesianMesh *mesh;
	
	int loadSolveRegions(istream& instream, int*& solveRegions);

	void loadPostProcessingBlock(istream& ifsInput);
};

#endif
