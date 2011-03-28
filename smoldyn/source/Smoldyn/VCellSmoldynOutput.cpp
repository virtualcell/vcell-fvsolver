/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include "VCellSmoldynOutput.h"

#include <VCELL/SimulationMessaging.h>
#include "DataProcessorRoiTimeSeriesSmoldyn.h"

#define SIM_FILE_EXT "sim"
#define LOG_FILE_EXT "log"
#define ZIP_FILE_EXT "zip"
#define DATAPROCOUTPUT_EXT "dataProcOutput"
#define ZIP_FILE_LIMIT 1E9

typedef int int32;
typedef unsigned int uint32;

int zip32(int filecnt, char* zipfile, ...);
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

static int getZipCount(char* zipFileName) {
	char* p = strstr(zipFileName, ZIP_FILE_EXT);
	if (p == NULL) {
		return -1;
	}

	char str[3];
	strncpy(str, p - 2, 2 * sizeof(char));
	str[2] = 0;
	return atoi(str);
}

VCellSmoldynOutput::VCellSmoldynOutput(simptr sim){
	smoldynSim = sim;

	simFileCount = 0;
	zipFileCount = 0;
	Nx = Ny = Nz = 1;
	numVolumeElements = 0;
	numMembraneElements = 0;
	dimension = 0;
	smoldynDataProcessor = 0;

	dataBlock = 0;
	varTypes = 0;
	volVarOutputData = 0;
	memVarOutputData = 0;
	molIdentVarIndexMap = 0;
}

VCellSmoldynOutput::~VCellSmoldynOutput() {
	for (unsigned int i = 0; i < volVarNames.size(); i ++) {
		delete[] volVarNames[i];
	}
	for (unsigned int i = 0; i < memVarNames.size(); i ++) {
		delete[] memVarNames[i];
	}
	delete[] volVarOutputData;
	delete[] memVarOutputData;
	delete[] varTypes;
	delete[] molIdentVarIndexMap;
	delete[] dataBlock;

	delete smoldynDataProcessor;
}

void VCellSmoldynOutput::parseDataProcessingInput(string& name, string& input) {
	if (name == "RoiTimeSeries") {
		smoldynDataProcessor = new DataProcessorRoiTimeSeriesSmoldyn(this, name, input);
	} else {
		throw "unknown DataProcessor";
	}
}

void VCellSmoldynOutput::parseInput(char* input) {
	if (dimension > 0) {
		return;
	}

	char* rootdir = smoldynSim->cmds->root;
	char* fname = smoldynSim->cmds->fname[0];
	strcpy(baseFileName, rootdir);
	strcat(baseFileName, fname);
	char* p = strrchr(baseFileName, '.');
	*p = '\0';

	strcpy(baseSimName, fname);
	p = strrchr(baseSimName, '.');
	*p = '\0';

	clearLog();

	if (input == NULL || strlen(input) == 0) {
		throw "writeOutput : no dimension specified.";
	}
	dimension = sscanf(input, "%d %d %d %d %d", &numMembraneElements, &Nx, &Ny, &Nz);
	dimension --;
	if (dimension == 0) {
		char errMsg[256];
		sprintf(errMsg, "writeOutput : no dimension specified. %d %d %d", Nx, Ny, Nz);
		throw errMsg;
	}
	numVolumeElements = Nx * Ny * Nz;

	origin[0] = smoldynSim->wlist[0]->pos;
	extent[0] = smoldynSim->wlist[1]->pos - origin[0];
	if (dimension > 1) {
		origin[1] = smoldynSim->wlist[2]->pos;
		extent[1] = smoldynSim->wlist[3]->pos - origin[1];
		if (dimension > 2) {
			origin[2] = smoldynSim->wlist[4]->pos;
			extent[2] = smoldynSim->wlist[5]->pos - origin[2];
		}
	}

	molssptr mols = smoldynSim->mols;
	int numVars = mols->nspecies - 1;
	int volCount = 0;
	int memCount = 0;
	molIdentVarIndexMap = new int[numVars];
	varTypes = new VariableType[numVars];
	for (int i = 0; i < numVars; i ++) {
		char* varName = new char[128];	
		int molIdent = i + 1;
		strcpy(varName, mols->spname[molIdent]);		
		if(!mols->exist[molIdent][MSsoln]) { // membrane variable
			varTypes[i] = VAR_MEMBRANE;
			memVarNames.push_back(varName);
			molIdentVarIndexMap[i] = memCount;
			memCount ++;
		} else {
			varTypes[i] = VAR_VOLUME;
			volVarNames.push_back(varName);
			molIdentVarIndexMap[i] = volCount;
			volCount ++;
		}		
	}
	strcpy(fileHeader.magicString, MAGIC_STRING);
	strcpy(fileHeader.versionString, VERSION_STRING);

	fileHeader.sizeX = Nx;
	fileHeader.sizeY = Ny;
	fileHeader.sizeZ = Nz;
	fileHeader.numBlocks = numVars;
	fileHeader.firstBlockOffset = sizeof(FileHeader);

	dataBlock = new DataBlock[numVars];
	//
	// compute data blocks (describing data)
	//
	int dataOffset = fileHeader.firstBlockOffset + numVars * sizeof(DataBlock);
	int blockIndex = 0;
	for (unsigned int v = 0; v < volVarNames.size(); v ++) {
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, volVarNames[v]);

		dataBlock[blockIndex].varType = VAR_VOLUME;
		dataBlock[blockIndex].size = numVolumeElements;
		dataBlock[blockIndex].dataOffset = dataOffset;
		dataOffset += dataBlock[blockIndex].size * sizeof(double);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVarNames.size(); v ++) {
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, memVarNames[v]);

		dataBlock[blockIndex].varType = VAR_MEMBRANE;
		dataBlock[blockIndex].size = numMembraneElements;
		dataBlock[blockIndex].dataOffset = dataOffset;
		dataOffset += dataBlock[blockIndex].size * sizeof(double);
		blockIndex ++;
	}
	volVarOutputData = new double[volVarNames.size() * numVolumeElements];
	memVarOutputData = new double[memVarNames.size() * numMembraneElements];
}

void VCellSmoldynOutput::write() {	
	computeOutputData();
	if (smoldynDataProcessor == 0 || smoldynDataProcessor->isStoreEnabled()) {
		// write sim file
		char simFileName[256];
		char zipFileName[256];
		sprintf(simFileName, "%s%.4d.%s", baseSimName, simFileCount, SIM_FILE_EXT);
		sprintf(zipFileName, "%s%.2d.%s", baseFileName, zipFileCount, ZIP_FILE_EXT);

		writeSim(simFileName, zipFileName);	

		// write log file
		char logfilename[256];
		sprintf(logfilename, "%s.%s", baseFileName, LOG_FILE_EXT);
		FILE* logfp = NULL;
		if (simFileCount == 0) {
			logfp = fopen(logfilename, "w");
		} else {
			logfp = fopen(logfilename, "a");
		}
		if (logfp == NULL) {
			throw "can't open logfile for write";
		}
		int iteration = (int)(smoldynSim->time/smoldynSim->dt + 0.5);
		fprintf(logfp,"%4d %s %s %.15lg\n", iteration, simFileName, zipFileName, smoldynSim->time);
		fclose(logfp);

		// print message
		simFileCount ++;

		struct stat buf;
		if (stat(zipFileName, &buf) == 0) { // if exists
			if (buf.st_size > ZIP_FILE_LIMIT) {
				zipFileCount ++;
			}
		}
	}
	if (smoldynDataProcessor != 0) {
		if (smoldynSim->time == 0) {
			smoldynDataProcessor->onStart();
		}
		smoldynDataProcessor->onWrite();
		if (fabs(smoldynSim->time + smoldynSim->dt - smoldynSim->tmax) > 1e-12) {
			char fileName[256];
			sprintf(fileName, "%s.%s", baseFileName, DATAPROCOUTPUT_EXT);
			smoldynDataProcessor->onComplete(fileName);
		}
	}

	double progress = (smoldynSim->time - smoldynSim->tmin) / (smoldynSim->tmax - smoldynSim->tmin);
	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_DATA, progress, smoldynSim->time));
}

void VCellSmoldynOutput::clearLog() {

	char logFileName[256];
	sprintf(logFileName,"%s.%s",baseFileName, LOG_FILE_EXT);
	ifstream logfs(logFileName);
	if (!logfs.is_open()){
		cout << "error opening log file <" << logFileName << ">" << endl;
		return;
	}

	char simFileName[128];
	char zipFileName[128];
	int iteration, oldCount=-1, count;
	double time;

	while (!logfs.eof()) {
		logfs >> iteration >> simFileName >> zipFileName >> time;
		count = getZipCount(zipFileName);
		if (oldCount != count && count >= 0) {
			cout << endl << "clearLog(), removing zip file " << zipFileName << endl;
			remove(zipFileName);
			oldCount = count;
		}
	}
	logfs.close();

	cout << "clearLog(), removing log file " << logFileName << endl;
	remove(logFileName);

	char dataProcOutput[128];
	sprintf(dataProcOutput,"%s.%s",baseFileName, DATAPROCOUTPUT_EXT);
	remove(dataProcOutput);
}

bool VCellSmoldynOutput::isInSameCompartment(double *pos1, double* pos2) {

	for(int cl=0;cl<smoldynSim->cmptss->ncmpt;cl++) {
		int in1=posincompart(smoldynSim,pos1,smoldynSim->cmptss->cmptlist[cl]);
		int in2=posincompart(smoldynSim,pos2,smoldynSim->cmptss->cmptlist[cl]);
		if (in1 == 1 && in2 == 1) {
			return true;
		}
		if (in1 == 1 || in2 == 1) {
			return false;
		}
	}
	throw "shouldn't happend";
	return false;
}

double VCellSmoldynOutput::distance2(double* pos1, double* pos2) {
	if (dimension == 1) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0]);
	}
	if (dimension == 2) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0]) 
			+ (pos1[1] - pos2[1]) * (pos1[1] - pos2[1]);
	}
	if (dimension == 3) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0]) 
			+ (pos1[1] - pos2[1]) * (pos1[1] - pos2[1])
			+ (pos1[2] - pos2[2]) * (pos1[2] - pos2[2]);
	}
}

void VCellSmoldynOutput::computeOutputData() {
	molssptr mols = smoldynSim->mols;

	memset(volVarOutputData, 0, numVolumeElements * volVarNames.size() * sizeof(double));
	memset(memVarOutputData, 0, numMembraneElements * memVarNames.size() * sizeof(double));

	double dx = extent[0]/(Nx-1);
	double dy = (dimension > 1) ? extent[1]/(Ny-1) : 0;
	double dz = (dimension > 2) ? extent[2]/(Nz-1) : 0;
	double center[3];
	for(int ll=0;ll<mols->nlist;ll++) {
		for(int m=0;m<mols->nl[ll];m++) {
			moleculeptr mptr=mols->live[ll][m];
			int molIdent = mptr->ident - 1;
			if (molIdent < 0 ) {
				continue;
			}
			int varIndex = molIdentVarIndexMap[molIdent];
			if (varTypes[molIdent] == VAR_MEMBRANE) {
				char* panelName = mptr->pnl->pname;
				int memIndex = 0;
				char* p = strrchr(panelName, '_');
				sscanf(p+1, "%d", &memIndex);
				memVarOutputData[varIndex * numMembraneElements + memIndex] ++;
			} else {
				double* coord = mptr->pos;
				int i = 0, j = 0, k = 0;
				i = (int)((coord[0] - origin[0])/dx + 0.5);
				center[0] = i * dx;
				if (dimension > 1) {				
					j = (int)((coord[1] - origin[1])/dy + 0.5);
					center[1] = j * dy;
					if (dimension > 2) {
						k = (int)((coord[2] - origin[2])/dz + 0.5);
						center[2] = k * dy;
					}
				}

				int volIndex = k * Nx * Ny + j * Nx + i;
				// not in the same compartment, try to find the nearest neighbor
				// in the same compartment, if not found, keep it in the wrong 
				// compartment
				if (!isInSameCompartment(coord, center)) {
					bool bFound = false;
					double distance = 1e9;
					if (i > 0) {
						double center0[3] = {center[0]-dx, center[1], center[2]};
						if (isInSameCompartment(coord, center0)) {
							double dl = distance2(center0, coord);
							if (distance > dl) {
								distance = dl;
								volIndex = k * Nx * Ny + j * Nx + (i-1);
							}
						}
					}
					if (i < Nx - 1) {
						double center0[3] = {center[0]+dx, center[1], center[2]};
						if (isInSameCompartment(coord, center0)) {
							double dl = distance2(center0, coord);
							if (distance > dl) {
								distance = dl;
								volIndex = k * Nx * Ny + j * Nx + (i+1);
							}
						}
					}
					if (dimension > 1) {
						if (j > 0) {
							double center0[3] = {center[0], center[1]-dy, center[2]};
							if (isInSameCompartment(coord, center0)) {
								double dl = distance2(center0, coord);
								if (distance > dl) {
									distance = dl;
									volIndex = k * Nx * Ny + (j-1) * Nx + i;
								}
							}
						}
						if (j < Ny - 1) {
							double center0[3] = {center[0], center[1]+dy, center[2]};
							if (isInSameCompartment(coord, center0)) {
								double dl = distance2(center0, coord);
								if (distance > dl) {
									distance = dl;
									volIndex = k * Nx * Ny + (j+1) * Nx + i;
								}
							}
						}
						if (dimension > 2) {
							if (k > 0) {
								double center0[3] = {center[0], center[1], center[2]-dz};
								if (isInSameCompartment(coord, center0)) {
									double dl = distance2(center0, coord);
									if (distance > dl) {
										distance = dl;
										volIndex = (k-1) * Nx * Ny + j * Nx + i;
									}
								}
							}
							if (k < Nz - 1) {
								double center0[3] = {center[0], center[1], center[2]+dz};
								if (isInSameCompartment(coord, center0)) {
									double dl = distance2(center0, coord);
									if (distance > dl) {
										distance = dl;
										volIndex = (k+1) * Nx * Ny + j * Nx + i;
									}
								}
							}
						}
					}

				}
				volVarOutputData[varIndex * numVolumeElements + volIndex] ++;
			}
		}
	}
}

void VCellSmoldynOutput::writeSim(char* simFileName, char* zipFileName) {

	FILE* simfp = fopen(simFileName, "wb");
	if (simfp == NULL){
		throw "Cannot open .sim file to write";
	}

	DataSet::writeHeader(simfp, &fileHeader);
	long ftell_pos = ftell(simfp);
	if (ftell_pos != fileHeader.firstBlockOffset){
		char errMsg[256];
		sprintf(errMsg, "DataSet::write() - file offset for first block is incorrect, ftell() says %ld, should be %d", ftell_pos, fileHeader.firstBlockOffset);
		throw errMsg;
	}

	//
	// write data blocks (describing data)
	//
	int blockIndex = 0;	
	for (unsigned int v = 0; v < volVarNames.size(); v ++) {
		DataSet::writeDataBlock(simfp, dataBlock + blockIndex);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVarNames.size(); v ++) {
		DataSet::writeDataBlock(simfp, dataBlock + blockIndex);
		blockIndex ++;
	}

	//
	// write data
	//
	blockIndex = 0;	
	int dataOffset = fileHeader.firstBlockOffset + (volVarNames.size() + memVarNames.size()) * sizeof(DataBlock);
	for (unsigned int v = 0; v < volVarNames.size(); v ++) {
		ftell_pos = ftell(simfp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errMsg[256];
			sprintf(errMsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d", blockIndex, dataBlock[blockIndex].varName,
				ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errMsg;
		}
		DataSet::writeDoubles(simfp, volVarOutputData + v * numVolumeElements, numVolumeElements);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVarNames.size(); v ++) {
		ftell_pos = ftell(simfp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errMsg[256];
			sprintf(errMsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d", blockIndex, dataBlock[blockIndex].varName,
				ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errMsg;
		}
		DataSet::writeDoubles(simfp, memVarOutputData + v * numMembraneElements, numMembraneElements);
		blockIndex ++;
	}
	fclose(simfp);

	int retcode = zip32(1, zipFileName, simFileName);
	remove(simFileName);
	if (retcode != 0) {
		char errMsg[256];
		sprintf(errMsg, "Writing zip file <%s> failed, return code is %d", zipFileName, retcode);
		throw errMsg;
	}
}
