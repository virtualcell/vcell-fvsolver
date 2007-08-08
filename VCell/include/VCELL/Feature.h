/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef FEATURE_H
#define FEATURE_H

#include <stdio.h>
#include <VCELL/SimTypes.h>
#include <VCELL/Element.h>

class VolumeVarContext;
class VolumeRegionVarContext;
class VolumeParticleContext;
class MembraneParticleContext;
class ContourParticleContext;
class MembraneVarContext;
class MembraneRegionVarContext;
class FastSystem;
class Feature;
class Simulation;
class VolumeVariable;
class MembraneVariable;
class VolumeRegionVariable;
class MembraneRegionVariable;


class Feature
{
	friend class VCellModel;

public:
	Feature(string& Aname, FeatureHandle handle, int priority);
	~Feature();

	bool resolveReferences(Simulation *sim);
	virtual bool initVolumeValues(long volumeIndex);
	virtual bool initMembraneValues(MembraneElement *membraneElement);
	virtual bool initVolumeRegionValues(int volumeRegionIndex);
	virtual bool initMembraneRegionValues(int membraneRegionIndex);

	virtual BoundaryType getXmBoundaryType() { return boundaryType[0]; }
	virtual BoundaryType getXpBoundaryType() { return boundaryType[1]; }
	virtual BoundaryType getYmBoundaryType() { return boundaryType[2]; }
	virtual BoundaryType getYpBoundaryType() { return boundaryType[3]; }
	virtual BoundaryType getZmBoundaryType() { return boundaryType[4]; }
	virtual BoundaryType getZpBoundaryType() { return boundaryType[5]; }
	     
	virtual void setXmBoundaryType(BoundaryType bt) { boundaryType[0] = bt; }
	virtual void setXpBoundaryType(BoundaryType bt) { boundaryType[1] = bt; }
	virtual void setYmBoundaryType(BoundaryType bt) { boundaryType[2] = bt; }
	virtual void setYpBoundaryType(BoundaryType bt) { boundaryType[3] = bt; }
	virtual void setZmBoundaryType(BoundaryType bt) { boundaryType[4] = bt; }
	virtual void setZpBoundaryType(BoundaryType bt) { boundaryType[5] = bt; }

	double          getMaxIterationTime();
	int             getPriority() { return priority; }   // higher priority is inside   
	string getName() { return name; }
	FeatureHandle   getHandle();
	   
	VolumeVarContext   *getVolumeVarContext(string& volumeVarName);
	VolumeVarContext   *getVolumeVarContext(VolumeVariable *var);
	VolumeVarContext   *getNextVolumeVarContext(VolumeVarContext *vc=NULL);

	MembraneVarContext *getMembraneVarContext(string& membraneVarName);
	MembraneVarContext *getMembraneVarContext(MembraneVariable *var);
	MembraneVarContext *getNextMembraneVarContext(MembraneVarContext *vc=NULL);
	   
	VolumeRegionVarContext   *getVolumeRegionVarContext(VolumeRegionVariable *var);
	VolumeRegionVarContext   *getNextVolumeRegionVarContext(VolumeRegionVarContext *vc=NULL);

	MembraneRegionVarContext *getMembraneRegionVarContext(MembraneRegionVariable *var);
	MembraneRegionVarContext *getNextMembraneRegionVarContext(MembraneRegionVarContext *vc=NULL);
	   
	void                addVolumeVarContext(VolumeVarContext *vc);
	void                addMembraneVarContext(MembraneVarContext *vc);
	void                addVolumeRegionVarContext(VolumeRegionVarContext *vc);
	void                addMembraneRegionVarContext(MembraneRegionVarContext *vc);
	   
	VolumeParticleContext     *getVolumeParticleContext(){return vpc;}
	MembraneParticleContext   *getMembraneParticleContext(){return mpc;}
	ContourParticleContext    *getContourParticleContext(){return cpc;}
	VolumeParticleContext     *setVolumeParticleContext(VolumeParticleContext *Avpc);
	MembraneParticleContext   *setMembraneParticleContext(MembraneParticleContext *Ampc);
	ContourParticleContext    *setContourParticleContext(ContourParticleContext *Acpc);

	void setFastSystem(FastSystem* arg_fastSystem)	{fastSystem = arg_fastSystem; }
	void setMembraneFastSystem(FastSystem* arg_fastSystem)	{membraneFastSystem = arg_fastSystem; }
	FastSystem         *getFastSystem(){ return fastSystem; }
	FastSystem         *getMembraneFastSystem(){ return membraneFastSystem; }
   
protected:

	VolumeParticleContext     *vpc;
	MembraneParticleContext   *mpc;
	ContourParticleContext    *cpc;
	VolumeVarContext   *currVolumeVarContext;
	VolumeVarContext   *volumeVarContextList;
	MembraneVarContext *currMembraneVarContext;
	MembraneVarContext *membraneVarContextList;
	VolumeRegionVarContext   *currVolumeRegionVarContext;
	VolumeRegionVarContext   *volumeRegionVarContextList;
	MembraneRegionVarContext *currMembraneRegionVarContext;
	MembraneRegionVarContext *membraneRegionVarContextList;
	FastSystem         *fastSystem;
	FastSystem         *membraneFastSystem;
	   
	int              priority;
	FeatureHandle    handle;
	string          name;	
	Feature  *next; 

private:
	BoundaryType boundaryType[6];

};  

#endif