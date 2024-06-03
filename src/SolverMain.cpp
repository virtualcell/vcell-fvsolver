//
// Created by Jim Schaff on 6/1/24.
//
#if (defined(WIN32) || defined(WIN64) )
#define _HAS_STD_BYTE 0
#include <windows.h> 
#endif

#include "SolverMain.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
using namespace std;

#undef USE_MESSAGING

#include <VCELL/FVSolver.h>
#include <sys/stat.h>
#include <VCELL/SimTool.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/GitDescribe.h>
#include <Exception.h>
#include <vcellhybrid.h>



void vcellExit(int returnCode, string& errorMsg) {
	if (SimulationMessaging::getInstVar() == 0) {
		if (returnCode != 0) {
			cerr << errorMsg << endl;
		}
	} else if (!SimulationMessaging::getInstVar()->isStopRequested()) {
		if (returnCode != 0) {
			SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_FAILURE, errorMsg.c_str()));
		}
	}
}

std::string version()
{
	return "Finite Volume version " + std::string(g_GIT_DESCRIBE) + " with smoldyn version " + std::string(VERSION);
}

int solve(const std::string& inputFilename, const std::string& outputDir) {
	// Check if output directory exists, if not create it
	std::filesystem::path dirPath(outputDir);
	if (!std::filesystem::exists(dirPath)) {
		std::filesystem::create_directories(dirPath);
	}

	// Open the input file
	std::ifstream inputFile(inputFilename);
	if (!inputFile.is_open()) {
		throw std::runtime_error("Could not open input file: " + inputFilename);
	}

	vcellhybrid::setHybrid(); //get smoldyn library in correct state
  	int returnCode = 0;
	string errorMsg = "Exception : ";

	bool bSimZip = true;
	int taskID = 0;

	SimulationMessaging::create();

	FVSolver* fvSolver = nullptr;

	try {
		fvSolver = new FVSolver(inputFile, taskID, outputDir.c_str(), bSimZip);

		inputFile.close();

		if(fvSolver->getNumVariables() == 0){
			//sims with no reactions and no diffusing species cause exit logic to 'wait' forever
			//never sending a job failed or job finished message and never cleaning up threads
			throw invalid_argument("FiniteVolume error: Must have at least 1 variable or reaction to solve");
		}
		fvSolver->solve();

	} catch (const char *exStr){
		errorMsg += exStr;
		returnCode = 1;
	} catch (string& exStr){
		errorMsg += exStr;
		returnCode = 1;
	} catch (VCell::Exception& ex){
		errorMsg += ex.getMessage();
		returnCode = 1;
	} catch (std::exception & e) {
		errorMsg += e.what();
		returnCode = 1;
	} catch (...){
		errorMsg += "unknown error";
		returnCode = 1;
	}

	if (inputFile.is_open()) {
		inputFile.close();
	}
	vcellExit(returnCode, errorMsg);
	delete fvSolver;
	return returnCode;
}
