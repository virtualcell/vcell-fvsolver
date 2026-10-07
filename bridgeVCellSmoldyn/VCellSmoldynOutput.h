/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VCELLSMOLDYNOUTPUT_H
#define VCELLSMOLDYNOUTPUT_H

#include "smoldyn.h"
#include "SmoldynDataGenerator.h"
#include <VCELL/DataSet.h>

#include <sstream>
#include <string>
#include <vector>
using std::string;
using std::vector;

class SmoldynHdf5Writer;
class SimTool;

struct SmoldynVariable {
	string name, domain;
	VariableType type;
	compartptr cmpt;
	surfaceptr srf;

	SmoldynVariable() {
		cmpt = 0;
		srf = 0;
	}
	string getFullyQualifiedName() {
		return domain + "::" + name;
	}
};

class VCellSmoldynOutput{
public:
	VCellSmoldynOutput(simptr sim);
	~VCellSmoldynOutput();

	static VCellSmoldynOutput* updateVCellSmoldynOutput(VCellSmoldynOutput* output, simptr sim);

	void write();
	void computeHistogram();
	void parseInput(string& input);	
	void parseDataProcessingInput(string& name, string& input);
	void setSimTool(SimTool* st) {
		this->simTool = st;
	}

	// Input of the vcellWriteOutput and vcellDataProcess command blocks, collected until each block's
	// "end" line. This is per-run state: kept in function-level statics, it survived into the next
	// solve in the same process, whose output was then never set up (vcell-fvsolver#23).
	bool outputInputParsed = false;
	std::stringstream outputInput;
	bool dataProcessInputParsed = false;
	std::stringstream dataProcessInput;
	string dataProcName;
private:
	
	void clearLog();
	void writeSim(char* simFileName, char* zipFileName);
	
	simptr smoldynSim;
	int simFileCount;
	int zipFileCount;
	char baseFileName[256];
	char baseSimName[256];
	int Nx, Ny, Nz;
	int numVolumeElements;
	int numMembraneElements;
	int dimension;	

	double extent[3];
	double origin[3];
	vector<SmoldynVariable*> volVariables;
	vector<SmoldynVariable*> memVariables;
	FileHeader fileHeader;
	DataBlock *dataBlock;
	double **volVarOutputData;
	double **memVarOutputData;
	int* molIdentVarIndexMap;
	SmoldynVariable** variables;

	SmoldynHdf5Writer* hdf5DataWriter;
	vector<SmoldynDataGenerator*> dataGeneratorList;
	SimTool* simTool;
	
	double distance2(double* pos1, double* pos2);

	friend class SmoldynHdf5Writer;
	friend class SmoldynVarStatDataGenerator;
};

#endif
