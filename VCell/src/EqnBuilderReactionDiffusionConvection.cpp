/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Simulation.h>
#include <VCELL/SimTypes.h>
#include <VCELL/Solver.h>
#include <VCELL/PdeSolverDiana.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/Feature.h>
#include <VCELL/Element.h>
#include <VCELL/VolumeVarContext.h>
#include <VCELL/EqnBuilderReactionDiffusionConvection.h>
#include <VCELL/Region.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/FVUtils.h>

//---------------------------------------------------------------------
//
//     dC       d^2 C
//    ---- = D ------- + rate
//     dt       dx^2
//
//---------------------------------------------------------------------
EqnBuilderReactionDiffusionConvection::
EqnBuilderReactionDiffusionConvection(VolumeVariable *Aspecies,
                            CartesianMesh *Amesh,
                            PdeSolverDiana *Asolver)
: EqnBuilder(Aspecies, Amesh)
{
	pdeSolverDiana = Asolver;
}

//------------------------------------------------------------------
//
// Initializes Matrix only
//
//------------------------------------------------------------------
bool EqnBuilderReactionDiffusionConvection::initEquation(double deltaTime, 
					int volumeIndexStart, int volumeIndexSize, 
					int membraneIndexStart, int membraneIndexSize)
{    
	double DELTAX, DELTAY, DELTAZ, DIM;
	double AREAX, AREAY, AREAZ, FLUXDELTAX, FLUXDELTAY, FLUXDELTAZ, VOLUME;
	double areaX, areaY, areaZ, fluxDeltaX, fluxDeltaY, fluxDeltaZ, volume;
	long SIZEX, SIZEXY;
	VolumeVarContext *varContext;
	DiscreteEqn nullEqn, eqn;
	int mask;
	double epsilon = 1e-10;    // zero diffusion threshold at 1e-10 micron^2/second

	//printf("EqnBuilderReactionDiffusionConvection::initEquation1D()\n");
	ASSERTION(solver->getVar() == var);

	Simulation *sim = SimTool::getInstance()->getSimulation();

	nullEqn.zero();

	DIM =  mesh->getDimension();
	DELTAX = ((CartesianMesh*)mesh)->getXScale_um();
	DELTAY = ((CartesianMesh*)mesh)->getYScale_um();
	DELTAZ = ((CartesianMesh*)mesh)->getZScale_um();

	AREAX  = ((CartesianMesh*)mesh)->getXArea_squm();
	AREAY  = ((CartesianMesh*)mesh)->getYArea_squm();
	AREAZ  = ((CartesianMesh*)mesh)->getZArea_squm();

	FLUXDELTAX = AREAX/DELTAX;
	FLUXDELTAY = AREAY/DELTAY;
	FLUXDELTAZ = AREAZ/DELTAZ;

	VOLUME = ((CartesianMesh*)mesh)->getVolume_cu();
	SIZEX = ((CartesianMesh*)mesh)->getNumVolumeX();
	SIZEXY = SIZEX*((CartesianMesh*)mesh)->getNumVolumeY();

	ASSERTION((volumeIndexStart>=0) && ((volumeIndexStart+volumeIndexSize)<=mesh->getNumVolumeElements()));

	VolumeElement *pVolumeElement = mesh->getVolumeElements() + volumeIndexStart;
	ASSERTION(pVolumeElement);
	for (long index=volumeIndexStart;index<(volumeIndexStart+volumeIndexSize);index++){
		eqn = nullEqn;
		ASSERTION(pVolumeElement->feature);
		varContext = pVolumeElement->feature->getVolumeVarContext(
													(VolumeVariable*)var);
		// ignore indices that have no varContexts (for region indexing)
		if (varContext != NULL) {
			mask = pVolumeElement->neighborMask;

			double diffConstant = varContext->getDiffusionRate(index);
			double Vx = varContext->getConvectionVelocity_X(index);
			double Vy = varContext->getConvectionVelocity_Y(index);
			double Vz = varContext->getConvectionVelocity_Z(index);

			double D, Dxm, Dxp, Dym, Dyp, Dzm, Dzp;
			double Vxm, Vxp, Vym, Vyp, Vzm, Vzp;
			if (mask & NEIGHBOR_MASK){
				if (mask & BOUNDARY_TYPE_DIRICHLET){   // boundary and dirichlet
					eqn.Ap = 1.0;
				}else if (mask & NEIGHBOR_BOUNDARY_MASK){   // boundary and neumann
					volume = VOLUME/(mask&VOLUME_MASK);
					eqn.Ap0 = volume/deltaTime;

					areaX = AREAX;
					areaY = AREAY;
					areaZ = AREAZ;

					fluxDeltaX = FLUXDELTAX;
					fluxDeltaY = FLUXDELTAY;
					fluxDeltaZ = FLUXDELTAZ;
#ifdef DEBUG
					double deltaX=DELTAX;
					double deltaY=DELTAY;
					double deltaZ=DELTAZ;
#endif
					if (mask & NEIGHBOR_X_BOUNDARY_MASK){
						areaY /= 2.0;
						areaZ /= 2.0;
						fluxDeltaY /= 2.0;
						fluxDeltaZ /= 2.0;
#ifdef DEBUG
						deltaX/=2.0;
#endif
					}
					if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
						areaX /= 2.0;
						areaZ /= 2.0;
						fluxDeltaX /= 2.0;
						fluxDeltaZ /= 2.0;
#ifdef DEBUG
						deltaY/=2.0;
#endif
					}
					if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
						areaX /= 2.0;
						areaY /= 2.0;
						fluxDeltaX /= 2.0;
						fluxDeltaY /= 2.0;
#ifdef DEBUG
						deltaZ/=2.0;
#endif
					}
#ifdef DEBUG
					ASSERTION(volume == deltaX*deltaY*deltaZ);
#endif
	 
					if (!(mask & NEIGHBOR_XM_MASK)){
						Dxm = varContext->getDiffusionRate(index-1);
						D = (diffConstant+Dxm<epsilon)?(0.0):(2 * Dxm * diffConstant/(diffConstant + Dxm));
						Vxm = 0.5*(Vx+varContext->getConvectionVelocity_X(index-1));

						//   eqn.Ax_minus = D * fluxDeltaX + Vxm * (Vxm > 0.0) * areaX;//upwind
						//   eqn.Ap += D * fluxDeltaX - Vxm * (Vxm < 0.0) * areaX;//upwind
					
						eqn.Ax_minus = max(D * fluxDeltaX + 0.5 * Vxm * areaX, max(Vxm * areaX,0));//hybrid
						eqn.Ap += max(D * fluxDeltaX - 0.5 * Vxm * areaX, max(- Vxm * areaX,0));//hybrid
					
						//  eqn.Ap += eqn.Ax_minus = diffConstant * fluxDeltaX;
					}
					if (!(mask & NEIGHBOR_XP_MASK)){
						Dxp = varContext->getDiffusionRate(index+1);
						D = (diffConstant+Dxp<epsilon)?(0.0):(2 * Dxp * diffConstant/(diffConstant + Dxp));
						Vxp = 0.5*(Vx+varContext->getConvectionVelocity_X(index+1));
					
						//   eqn.Ax_plus = D * fluxDeltaX - Vxp * (Vxp < 0.0) * areaX;//upwind
						//   eqn.Ap += D * fluxDeltaX + Vxp * (Vxp > 0.0) * areaX;//upwind
					
						eqn.Ax_plus = max(D * fluxDeltaX - 0.5 * Vxp * areaX,max(- Vxp * areaX,0));//hybrid
						eqn.Ap += max(D * fluxDeltaX + 0.5 * Vxp * areaX,max(Vxp * areaX,0));//hybrid

						//  eqn.Ap += eqn.Ax_plus  = diffConstant * fluxDeltaX;
					}
					if (DIM>1){
						if (!(mask & NEIGHBOR_YM_MASK)){
							Dym = varContext->getDiffusionRate(index-SIZEX);
							D = (diffConstant+Dym<epsilon)?(0.0):(2 * Dym * diffConstant/(diffConstant + Dym));
							Vym = 0.5*(Vy+varContext->getConvectionVelocity_Y(index-SIZEX));
						
							//  eqn.Ay_minus = D * fluxDeltaY + Vym * (Vym > 0.0) * areaY;//upwind
							//  eqn.Ap += D * fluxDeltaY - Vym * (Vym < 0.0) * areaY;//upwind
						
							eqn.Ay_minus = max(D * fluxDeltaY + 0.5 * Vym * areaY,max(Vym * areaY,0));//hybrid
							eqn.Ap += max(D * fluxDeltaY - 0.5 * Vym * areaY,max(- Vym * areaY,0));//hybrid
						
							//  eqn.Ap += eqn.Ay_minus = diffConstant * fluxDeltaY;
						}
						if (!(mask & NEIGHBOR_YP_MASK)){
							Dyp = varContext->getDiffusionRate(index+SIZEX);
							D = (diffConstant+Dyp<epsilon)?(0.0):(2 * Dyp * diffConstant/(diffConstant + Dyp));
							Vyp = 0.5*(Vy+varContext->getConvectionVelocity_Y(index+SIZEX));

							//  eqn.Ay_plus  = D * fluxDeltaY - Vyp * (Vyp < 0.0) * areaY;//upwind
							//  eqn.Ap += D * fluxDeltaY + Vyp * (Vyp > 0.0) * areaY;//upwind
						
							eqn.Ay_plus  = max(D * fluxDeltaY - 0.5 * Vyp * areaY,max(- Vyp * areaY,0));//hybrid
							eqn.Ap += max(D * fluxDeltaY + 0.5 * Vyp * areaY,max(Vyp * areaY,0));//hybrid
						
							//  eqn.Ap += eqn.Ay_plus  = diffConstant * fluxDeltaY;
						}
						if (DIM>2){
							if (!(mask & NEIGHBOR_ZM_MASK)){
								Dzm = varContext->getDiffusionRate(index-SIZEXY);
								D = (diffConstant+Dzm<epsilon)?(0.0):(2 * Dzm * diffConstant/(diffConstant + Dzm));
								Vzm = 0.5*(Vz+varContext->getConvectionVelocity_Z(index-SIZEXY));

								//	eqn.Az_minus = D * fluxDeltaZ + Vzm * (Vzm > 0.0) * areaZ;//upwind
								//  eqn.Ap += D * fluxDeltaZ - Vzm * (Vzm < 0.0) * areaZ;//upwind
						
								eqn.Az_minus = max(D * fluxDeltaZ + 0.5 * Vzm * areaZ,max(Vzm * areaZ,0));//hybrid
								eqn.Ap += max(D * fluxDeltaZ - 0.5 * Vzm * areaZ,max(- Vzm * areaZ,0));//hybrid
							
								//   eqn.Ap += eqn.Az_minus = diffConstant * fluxDeltaZ;
							}
							if (!(mask & NEIGHBOR_ZP_MASK)){
								Dzp = varContext->getDiffusionRate(index+SIZEXY);
								D = (diffConstant+Dzp<epsilon)?(0.0):(2 * Dzp * diffConstant/(diffConstant + Dzp));
								Vzp = 0.5*(Vz+varContext->getConvectionVelocity_Z(index+SIZEXY));
							
								//  eqn.Az_plus  = D * fluxDeltaZ - Vzp * (Vzp <0.0) * areaZ;//upwind
								//  eqn.Ap += D * fluxDeltaZ + Vzp * (Vzp >0.0) * areaZ;//upwind
							
								eqn.Az_plus  = max(D * fluxDeltaZ - 0.5 * Vzp * areaZ,max(- Vzp * areaZ,0));//hybrid
								eqn.Ap += max(D * fluxDeltaZ + 0.5 * Vzp * areaZ,max(Vzp * areaZ,0));//hybrid
							
								//   eqn.Ap += eqn.Az_plus  = diffConstant * fluxDeltaZ;
							}
						}// end DIM>1
					}// end DIM>2
				}else{ // has no boundary patches, but at least one membrane
					if (!(mask & NEIGHBOR_XM_MEMBRANE)){
						Dxm = varContext->getDiffusionRate(index-1);
						D = (diffConstant+Dxm<epsilon)?(0.0):(2 * Dxm * diffConstant/(diffConstant + Dxm));
						Vxm = 0.5*(Vx+varContext->getConvectionVelocity_X(index-1));

						//	eqn.Ax_minus = D * FLUXDELTAX + Vxm * (Vxm > 0.0) * AREAX;//upwind
						//  eqn.Ap += D * FLUXDELTAX - Vxm * (Vxm < 0.0) * AREAX;//upwind
					
						eqn.Ax_minus = max(D * FLUXDELTAX + 0.5 * Vxm * AREAX,max(Vxm * AREAX,0));//hybrid
						eqn.Ap += max(D * FLUXDELTAX - 0.5 * Vxm * AREAX,max(- Vxm * AREAX,0));//hybrid
					
						//  eqn.Ap += eqn.Ax_minus = diffConstant * FLUXDELTAX;
					}
					if (!(mask & NEIGHBOR_XP_MEMBRANE)){
						Dxp = varContext->getDiffusionRate(index+1);
						D = (diffConstant+Dxp<epsilon)?(0.0):(2 * Dxp * diffConstant/(diffConstant + Dxp));
						Vxp = 0.5*(Vx+varContext->getConvectionVelocity_X(index+1));
			            
						// eqn.Ax_plus  = D * FLUXDELTAX - Vxp * (Vxp < 0.0) * AREAX;//upwind
						// eqn.Ap += D * FLUXDELTAX + Vxp * (Vxp > 0.0) * AREAX;//upwind
					
						eqn.Ax_plus  = max(D * FLUXDELTAX - 0.5 * Vxp * AREAX,max(- Vxp * AREAX,0));//hybrid
						eqn.Ap += max(D * FLUXDELTAX + 0.5 * Vxp * AREAX,max(Vxp * AREAX,0));//hybrid
					
						//  eqn.Ap += eqn.Ax_plus  = diffConstant * FLUXDELTAX;
					}
					if (DIM>1){
						if (!(mask & NEIGHBOR_YM_MEMBRANE)){
							Dym = varContext->getDiffusionRate(index-SIZEX);
							D = (diffConstant+Dym<epsilon)?(0.0):(2 * Dym * diffConstant/(diffConstant + Dym));
							Vym = 0.5*(Vy+varContext->getConvectionVelocity_Y(index-SIZEX));
				            
							// eqn.Ay_minus = D * FLUXDELTAY + Vym * (Vym > 0.0) * AREAY;//upwind
							// eqn.Ap += D * FLUXDELTAY - Vym * (Vym < 0.0) * AREAY;//upwind

							eqn.Ay_minus = max(D * FLUXDELTAY + 0.5 * Vym * AREAY,max(Vym * AREAY,0));//hybrid
							eqn.Ap += max(D * FLUXDELTAY - 0.5 * Vym * AREAY,max(- Vym * AREAY,0));//hybrid
						
							//   eqn.Ap += eqn.Ay_minus = diffConstant * FLUXDELTAY;
						}
						if (!(mask & NEIGHBOR_YP_MEMBRANE)){
							Dyp = varContext->getDiffusionRate(index+SIZEX);
							D = (diffConstant+Dyp<epsilon)?(0.0):(2 * Dyp * diffConstant/(diffConstant + Dyp));
							Vyp = 0.5*(Vy+varContext->getConvectionVelocity_Y(index+SIZEX));

							// eqn.Ay_plus  = D * FLUXDELTAY - Vyp * (Vyp < 0.0) * AREAY;//upwind
							// eqn.Ap += D * FLUXDELTAY + Vyp * (Vyp > 0.0) * AREAY;//upwind

							eqn.Ay_plus  = max(D * FLUXDELTAY - 0.5 * Vyp * AREAY,max(- Vyp * AREAY,0));//hybrid
							eqn.Ap += max(D * FLUXDELTAY + 0.5 * Vyp * AREAY,max(Vyp * AREAY,0));//hybrid
						
							//  eqn.Ap += eqn.Ay_plus  = diffConstant * FLUXDELTAY;
						}
						if (DIM>2){
							if (!(mask & NEIGHBOR_ZM_MEMBRANE)){
								Dzm = varContext->getDiffusionRate(index-SIZEXY);
								D = (diffConstant+Dzm<epsilon)?(0.0):(2 * Dzm * diffConstant/(diffConstant + Dzm));
								Vzm = 0.5*(Vz+varContext->getConvectionVelocity_Z(index-SIZEXY));
					            
								// eqn.Az_minus = D * FLUXDELTAZ + Vzm * (Vzm > 0.0) * AREAZ;//upwind
								// eqn.Ap += D * FLUXDELTAZ - Vzm * (Vzm < 0.0) * AREAZ;//upwind

								eqn.Az_minus = max(D * FLUXDELTAZ + 0.5 * Vzm * AREAZ,max(Vzm * AREAZ,0));//hybrid
								eqn.Ap += max(D * FLUXDELTAZ - 0.5 * Vzm * AREAZ,max(- Vzm * AREAZ,0));//hybrid
							
								//  eqn.Ap += eqn.Az_minus = diffConstant * FLUXDELTAZ;
							}
							if (!(mask & NEIGHBOR_ZP_MEMBRANE)){
								Dzp = varContext->getDiffusionRate(index+SIZEXY);
								D = (diffConstant+Dzp<epsilon)?(0.0):(2 * Dzp * diffConstant/(diffConstant + Dzp));
								Vzp = 0.5*(Vz+varContext->getConvectionVelocity_Z(index+SIZEXY));
					            
								//eqn.Az_plus  = D * FLUXDELTAZ - Vzp * (Vzp < 0.0) * AREAZ;//upwind
								//eqn.Ap += D * FLUXDELTAZ + Vzp * (Vzp > 0.0) * AREAZ;//upwind
							
								eqn.Az_plus  = max(D * FLUXDELTAZ - 0.5 * Vzp * AREAZ,max(- Vzp * AREAZ,0));//hybrid
								eqn.Ap += max(D * FLUXDELTAZ + 0.5 * Vzp * AREAZ,max(Vzp * AREAZ,0));//hybrid
							
								//  eqn.Ap += eqn.Az_plus  = diffConstant * FLUXDELTAZ;
							}
						}// end DIM>1
					}// end DIM>2
					eqn.Ap0 = VOLUME/deltaTime;
				}
			}else{   // has no boundaries or membranes
				Dxm = varContext->getDiffusionRate(index-1);
				D = (diffConstant+Dxm<epsilon)?(0.0):(2 * Dxm * diffConstant/(diffConstant + Dxm));
				Vxm = 0.5*(Vx+varContext->getConvectionVelocity_X(index-1));
	            
				//eqn.Ax_minus = D * FLUXDELTAX + Vxm * (Vxm > 0.0) * AREAX;//upwind
				//eqn.Ap += D * FLUXDELTAX - Vxm * (Vxm < 0.0) * AREAX;//upwind
			
				eqn.Ax_minus = max(D * FLUXDELTAX + 0.5 * Vxm * AREAX,max(Vxm * AREAX,0));//hybrid
				eqn.Ap += max(D * FLUXDELTAX - 0.5 * Vxm * AREAX,max(- Vxm * AREAX,0));//hybrid
			
				//eqn.Ap += eqn.Ax_minus = diffConstant * FLUXDELTAX;

				Dxp = varContext->getDiffusionRate(index+1);
				D = (diffConstant+Dxp<epsilon)?(0.0):(2 * Dxp * diffConstant/(diffConstant + Dxp));
				Vxp = 0.5*(Vx+varContext->getConvectionVelocity_X(index+1));
	            
				//eqn.Ax_plus  = D * FLUXDELTAX - Vxp * (Vxp < 0.0) * AREAX;//upwind
				//eqn.Ap += D * FLUXDELTAX + Vxp * (Vxp > 0.0) * AREAX;//upwind
			
				eqn.Ax_plus  = max(D * FLUXDELTAX - 0.5 * Vxp * AREAX,max(- Vxp * AREAX,0));//hybrid
				eqn.Ap += max(D * FLUXDELTAX + 0.5 * Vxp * AREAX,max(Vxp * AREAX,0));//hybrid
			
				//eqn.Ap += eqn.Ax_plus  = diffConstant * FLUXDELTAX;

				if (DIM>1){
					Dym = varContext->getDiffusionRate(index-SIZEX);
					D = (diffConstant+Dym<epsilon)?(0.0):(2 * Dym * diffConstant/(diffConstant + Dym));
					Vym = 0.5*(Vy+varContext->getConvectionVelocity_Y(index-SIZEX));

					//eqn.Ay_minus = D * FLUXDELTAY + Vym * (Vym > 0.0) * AREAY;//upwind
					//eqn.Ap += D * FLUXDELTAY - Vym * (Vym < 0.0) * AREAY;//upwind
				
					eqn.Ay_minus = max(D * FLUXDELTAY + 0.5 * Vym * AREAY,max(Vym * AREAY,0));//hybrid
					eqn.Ap += max(D * FLUXDELTAY - 0.5 * Vym * AREAY,max(- Vym * AREAY,0));//hybrid
				
					//eqn.Ap += eqn.Ay_minus = diffConstant * FLUXDELTAY;

					Dyp = varContext->getDiffusionRate(index+SIZEX);
					D = (diffConstant+Dyp<epsilon)?(0.0):(2 * Dyp * diffConstant/(diffConstant + Dyp));
					Vyp = 0.5*(Vy+varContext->getConvectionVelocity_Y(index+SIZEX));
		            
					//eqn.Ay_plus  = D * FLUXDELTAY - Vyp * (Vyp < 0.0) * AREAY;//upwind
					//eqn.Ap += D * FLUXDELTAY + Vyp * (Vyp > 0.0) * AREAY;//upwind

					eqn.Ay_plus  = max(D * FLUXDELTAY - 0.5 * Vyp * AREAY,max(- Vyp * AREAY,0));//hybrid
					eqn.Ap += max(D * FLUXDELTAY + 0.5 * Vyp * AREAY,max(Vyp * AREAY,0));//hybrid
				
					//eqn.Ap += eqn.Ay_plus  = diffConstant * FLUXDELTAY;

					if (DIM>2){
						Dzm = varContext->getDiffusionRate(index-SIZEXY);
						D = (diffConstant+Dzm<epsilon)?(0.0):(2 * Dzm * diffConstant/(diffConstant + Dzm));
						Vzm = 0.5*(Vz+varContext->getConvectionVelocity_Z(index-SIZEXY));
			            
						//eqn.Az_minus = D * FLUXDELTAZ + Vzm * (Vzm > 0.0) * AREAZ;//upwind
						//eqn.Ap += D * FLUXDELTAZ - Vzm * (Vzm < 0.0) * AREAZ;//upwind
					
						eqn.Az_minus = max(D * FLUXDELTAZ + 0.5 * Vzm * AREAZ,max(Vzm * AREAZ,0));//hybrid
						eqn.Ap += max(D * FLUXDELTAZ - 0.5 * Vzm * AREAZ,max(- Vzm * AREAZ,0));//hybrid
					
						//eqn.Ap += eqn.Az_minus = diffConstant * FLUXDELTAZ;

						Dzp = varContext->getDiffusionRate(index+SIZEXY);
						D = (diffConstant+Dzp<epsilon)?(0.0):(2 * Dzp * diffConstant/(diffConstant + Dzp));
						Vzp = 0.5*(Vz+varContext->getConvectionVelocity_Z(index+SIZEXY));
			            
						//eqn.Az_plus  = D * FLUXDELTAZ - Vzp * (Vzp < 0.0) * AREAZ;//upwind
						//eqn.Ap += D * FLUXDELTAZ + Vzp * (Vzp > 0.0) * AREAZ;//upwind
					
						eqn.Az_plus  = max(D * FLUXDELTAZ - 0.5 * Vzp * AREAZ,max(- Vzp * AREAZ,0));//hybrid
						eqn.Ap += max(D * FLUXDELTAZ + 0.5 * Vzp * AREAZ,max(Vzp * AREAZ,0));//hybrid
			
						//eqn.Ap += eqn.Az_plus  = diffConstant * FLUXDELTAZ;
					}// end DIM>1
				}// end DIM>2

				eqn.Ap0 = VOLUME/deltaTime;
			}
		} // endif (varContext!=NULL)  
		if (!pdeSolverDiana->setEqn(index, eqn)){
			printf("error setting Equation\n");
			return false;
		}
		pVolumeElement++;
	} // end index

	return true;
}

//------------------------------------------------------------------
//
// updates B vector only with reaction rates and boundary conditions
//
//------------------------------------------------------------------
bool EqnBuilderReactionDiffusionConvection::buildEquation(double deltaTime, 
                            int volumeIndexStart, int volumeIndexSize, 
			    int membraneIndexStart, int membraneIndexSize)
{    
	double AREAX, AREAY, AREAZ, VOLUME;
	double areaX, areaY, areaZ, volume;
	VolumeVarContext *varContext;
	Feature *feature;
	int mask;

	//printf("EqnBuilderReactionDiffusionConvection::initEquation1D()\n");
	ASSERTION(solver->getVar() == var);

	Simulation *sim = SimTool::getInstance()->getSimulation();

	AREAX  = ((CartesianMesh*)mesh)->getXArea_squm();
	AREAY  = ((CartesianMesh*)mesh)->getYArea_squm();
	AREAZ  = ((CartesianMesh*)mesh)->getZArea_squm();
	VOLUME = ((CartesianMesh*)mesh)->getVolume_cu();

	long arraySize = pdeSolverDiana->getArraySize();
	ASSERTION(arraySize<=mesh->getNumVolumeElements());

	if(arraySize==0){
		ASSERTION((volumeIndexStart>=0) && ((volumeIndexStart+volumeIndexSize)<=mesh->getNumVolumeElements()));

		VolumeElement *pVolumeElement = mesh->getVolumeElements() + volumeIndexStart;
		ASSERTION(pVolumeElement);
		DiscreteEqn *pEqn = pdeSolverDiana->getEqns() + volumeIndexStart;
		ASSERTION(pEqn);
		for (long index=volumeIndexStart;index<(volumeIndexStart+volumeIndexSize);index++){
			ASSERTION(pVolumeElement->feature);
			feature = pVolumeElement->feature;
			varContext = feature->getVolumeVarContext((VolumeVariable*)var);
			mask = pVolumeElement->neighborMask;
			pEqn->B = 0.0;
			double diffConstant = varContext->getDiffusionRate(index);

			if (mask & NEIGHBOR_BOUNDARY_MASK){
				if (mask & BOUNDARY_TYPE_DIRICHLET){
			
					if ((mask & NEIGHBOR_XM_BOUNDARY) &&
							(feature->getXmBoundaryType() == BOUNDARY_VALUE)){
						sim->advanceTimeOn();
						pEqn->B = varContext->getXmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_XP_BOUNDARY) &&
						(feature->getXpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getXpBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_YM_BOUNDARY) &&
						(feature->getYmBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getYmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_YP_BOUNDARY) &&
						(feature->getYpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getYpBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_ZM_BOUNDARY) &&
						(feature->getZmBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getZmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_ZP_BOUNDARY) &&
						(feature->getZpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getZpBoundaryValue(index);
						sim->advanceTimeOff();

					}else{
						throw "EqnBuilderReactionDiffusionConvection::buildEquation() : invalid boundary type";
					}
			    
				}else{ // no Dirichlet conditions
					volume = VOLUME/(mask&VOLUME_MASK);

					sim->advanceTimeOn();
					pEqn->B = varContext->getReactionRate(index)*volume;
					sim->advanceTimeOff();

					areaX = AREAX;
					areaY = AREAY;
					areaZ = AREAZ;
			    
					if (mask & NEIGHBOR_X_BOUNDARY_MASK){
						areaY /= 2.0;
						areaZ /= 2.0;
					}
					if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
						areaX /= 2.0;
						areaZ /= 2.0;
					}
					if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
						areaX /= 2.0;
						areaY /= 2.0;
					}
			    
					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getXmBoundaryFlux(index)*areaX;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getXpBoundaryFlux(index)*areaX;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getYmBoundaryFlux(index)*areaY;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getYpBoundaryFlux(index)*areaY;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getZmBoundaryFlux(index)*areaZ;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getZpBoundaryFlux(index)*areaZ;
						sim->advanceTimeOff();
					}
				}
			}else{ // no boundary
				//sim->advanceTimeOn();
				pEqn->B = varContext->getReactionRate(index)*VOLUME;
				//sim->advanceTimeOff();
			}

			pVolumeElement++;
			pEqn++;
		} // end index


		ASSERTION((membraneIndexStart>=0) && 
				((membraneIndexStart+membraneIndexSize)<=mesh->getNumMembraneElements()));

		MembraneElement *pMembraneElement = mesh->getMembraneElements() + membraneIndexStart;
		ASSERTION(pMembraneElement);
		double inFlux;
		double outFlux;
		double area;

		pEqn = pdeSolverDiana->getEqns();
		for (long memIndex=membraneIndexStart;memIndex<(membraneIndexStart+membraneIndexSize);memIndex++){
			feature = pMembraneElement->feature;
			ASSERTION(feature);

			varContext = feature->getVolumeVarContext((VolumeVariable*)var);

			sim->advanceTimeOn();
			varContext->getFlux(pMembraneElement,&inFlux,&outFlux);
			sim->advanceTimeOff();

			area = pMembraneElement->area;
 			//
			// check if Dirichlet condition, if yes, then don't update due to membrane flux term
			//
			VolumeElement *pVolumeElement = mesh->getVolumeElements() + pMembraneElement->insideIndexNear;
			if ((pVolumeElement->neighborMask & BOUNDARY_TYPE_DIRICHLET) == 0){
				pEqn[pMembraneElement->insideIndexNear].B += inFlux * area;
			}
			pVolumeElement = mesh->getVolumeElements() + pMembraneElement->outsideIndexNear;
			if ((pVolumeElement->neighborMask & BOUNDARY_TYPE_DIRICHLET) == 0){
				pEqn[pMembraneElement->outsideIndexNear].B += outFlux * area;
			}

			pMembraneElement++;
		} // end memIndex
	}
	else if(arraySize>0){
		VolumeElement *pVolumeElement;
		DiscreteEqn *pEqn = pdeSolverDiana->getEqns() + volumeIndexStart;

		for (long i=0;i<arraySize;i++){
			long index = pdeSolverDiana->getGlobalIndex(i);
			pVolumeElement = mesh->getVolumeElements() + index;
			ASSERTION(pVolumeElement);
			pEqn = pdeSolverDiana->getEqns() + index;
			ASSERTION(pEqn);

			ASSERTION(pVolumeElement->feature);
			feature = pVolumeElement->feature;
			varContext = feature->getVolumeVarContext((VolumeVariable*)var);
			mask = pVolumeElement->neighborMask;
			pEqn->B = 0.0;
			double diffConstant = varContext->getDiffusionRate(index);

			if (mask & NEIGHBOR_BOUNDARY_MASK){
				if (mask & BOUNDARY_TYPE_DIRICHLET){
			   
					if ((mask & NEIGHBOR_XM_BOUNDARY) &&
								(feature->getXmBoundaryType() == BOUNDARY_VALUE)){
						sim->advanceTimeOn();
						pEqn->B = varContext->getXmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_XP_BOUNDARY) &&
							(feature->getXpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getXpBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_YM_BOUNDARY) &&
							(feature->getYmBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getYmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_YP_BOUNDARY) &&
							(feature->getYpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getYpBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_ZM_BOUNDARY) &&
							(feature->getZmBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getZmBoundaryValue(index);
						sim->advanceTimeOff();

					}else if ((mask & NEIGHBOR_ZP_BOUNDARY) &&
						(feature->getZpBoundaryType() == BOUNDARY_VALUE)){

						sim->advanceTimeOn();
						pEqn->B = varContext->getZpBoundaryValue(index);
						sim->advanceTimeOff();

					}else{
						throw "EqnBuilderReactionDiffusionConvection::buildEquation() : invalid boundary type";
					}
		      
				}else{ // no Dirichlet conditions
					volume = VOLUME/(mask&VOLUME_MASK);

					sim->advanceTimeOn();
					pEqn->B = varContext->getReactionRate(index)*volume;
					sim->advanceTimeOff();

					areaX = AREAX;
					areaY = AREAY;
					areaZ = AREAZ;
			    
					if (mask & NEIGHBOR_X_BOUNDARY_MASK){
						areaY /= 2.0;
						areaZ /= 2.0;
					}
					if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
						areaX /= 2.0;
						areaZ /= 2.0;
					}
					if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
						areaX /= 2.0;
						areaY /= 2.0;
					}
			    
					if (mask & NEIGHBOR_XM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getXmBoundaryFlux(index)*areaX;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_XP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getXpBoundaryFlux(index)*areaX;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getYmBoundaryFlux(index)*areaY;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_YP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getYpBoundaryFlux(index)*areaY;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZM_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += varContext->getZmBoundaryFlux(index)*areaZ;
						sim->advanceTimeOff();
					}
					if (mask & NEIGHBOR_ZP_BOUNDARY){
						sim->advanceTimeOn();
						pEqn->B += - varContext->getZpBoundaryFlux(index)*areaZ;
						sim->advanceTimeOff();
					}
				}
			}else{ // no boundary
				//sim->advanceTimeOn();
				pEqn->B = varContext->getReactionRate(index)*VOLUME;
				//sim->advanceTimeOff();
			}
		} // end index


		ASSERTION((membraneIndexStart>=0) && 
					((membraneIndexStart+membraneIndexSize)<=mesh->getNumMembraneElements()));

		MembraneElement *pMembraneElement = mesh->getMembraneElements() + membraneIndexStart;
		ASSERTION(pMembraneElement);
		double inFlux;
		double outFlux;
		double area;

		pEqn = pdeSolverDiana->getEqns();
		for (long memIndex=membraneIndexStart;memIndex<(membraneIndexStart+membraneIndexSize);memIndex++){
			feature = pMembraneElement->feature;
			ASSERTION(feature);

			varContext = feature->getVolumeVarContext((VolumeVariable*)var);

			sim->advanceTimeOn();
			varContext->getFlux(pMembraneElement,&inFlux,&outFlux);
			sim->advanceTimeOff();

			area = pMembraneElement->area;
 			//
			// check if Dirichlet condition, if yes, then don't update due to membrane flux term
			//
			VolumeElement *pVolumeElement = mesh->getVolumeElements() + pMembraneElement->insideIndexNear;
			if ((pVolumeElement->neighborMask & BOUNDARY_TYPE_DIRICHLET) == 0){
				pEqn[pMembraneElement->insideIndexNear].B += inFlux * area;
			}
			pVolumeElement = mesh->getVolumeElements() + pMembraneElement->outsideIndexNear;
			if ((pVolumeElement->neighborMask & BOUNDARY_TYPE_DIRICHLET) == 0){
				pEqn[pMembraneElement->outsideIndexNear].B += outFlux * area;
			}

			pMembraneElement++;
		} // end memIndex
	}
	return true;
}
