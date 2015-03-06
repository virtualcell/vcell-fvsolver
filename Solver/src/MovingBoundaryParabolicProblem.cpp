//#pragma warning ( disable: 4996 )
#pragma warning ( disable: 4244 )
#pragma warning ( disable: 4267 )
#include <MPoint.h>
#include <boost/logic/tribool.hpp>
#include <World.h>
#include <VoronoiMesh.h>
#include <sstream>
#include <iomanip>
#include <exception>
#include <forward_list>
#include <chrono>
#include <vcellutil.h>
#include <VCellChrono.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCellFront.h>
#include <algo.h>
#include <MovingBoundaryParabolicProblem.h>
#include <MeshElementNode.h>
#include <VoronoiMesh.h>
#include <Logger.h>
#include <Modulo.h>
#include <boundaryProviders.h>
#include <Physiology.h>

#include <ExplicitSolver.h>

#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <MBridge/MBPatch.h>
#include <MBridge/MBMovie.h>
#include <MBridge/Scatter.h>
#include <MBridge/MatlabDebug.h>

#include <ExpressionException.h>
#include <MapTable.h>
#include <MTExpression.h>
using spatial::cX;
using spatial::cY;

//*********************************************************
// Validation of input to problem functors & ValidationBase 
//*********************************************************
namespace {
	//matlabBridge::Scatter scatterInside('r',2);
	//matlabBridge::Scatter scatterOutside('b',2);

	template<int NUMBER_SYMBOLS>
	struct ConcentrationExpression {
		/**
		* runtime accessible #NUMBER_SYMBOLS
		*/
		static const int numberSymbols = NUMBER_SYMBOLS;

		ConcentrationExpression(VCell::Expression &e) 
			:exp(e) {
				VCELL_KEY_LOG(debug,Key::concentrationExpression, "expression: " << exp.infix( ));
		}

		double conc(double x, double y) {
			double in[NUMBER_SYMBOLS] = {x,y,0};
			const double rval =  exp.evaluateVector(in);
			VCELL_KEY_LOG(debug,Key::concentrationExpression, "f(" << x << ", " << y << ") = " << rval);
			return rval;
		}
		VCell::Expression & exp; 
	};

	//legacy name from early development code
	typedef ConcentrationExpression<3> ConcentrationProvider;


	/**
	* abstract class which validates input
	*/
	struct ValidationBase: public spatial::FronTierLevel, public spatial::FronTierVelocity {

		/**
		* number of user input functions 
		*/
		const static int NUMBER_FUNCTIONS = 3;

		ValidationBase(const moving_boundary::MovingBoundarySetup &mbs)
			:nFunctionPointers(0) { 
				std::stringstream badset;

#define CHECK(X) checkSet(badset,X, #X);  
#define CHECK2(X,Y) checkSet(badset,X, Y,#X);  

				CHECK(mbs.maxTime);
				CHECK(mbs.frontTimeStep);
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
				CHECK2(reinterpret_cast<void *>(mbs.concentrationFunction),mbs.concentrationFunctionStr);
				CHECK2(reinterpret_cast<void*>(mbs.velocityFunction),mbs.advectVelocityFunctionStrX)
					if (mbs.alternateFrontProvider == 0) {
						CHECK2(reinterpret_cast<void *>(mbs.levelFunction),mbs.levelFunctionStr);
					}
#endif
#undef CHECK
#undef CHECK2
					switch (nFunctionPointers) {
					case 0:
					case NUMBER_FUNCTIONS:
						break;
					default:
						badset << "functions must be specified by pointers or strings, not both ";
						break;
					}
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
					if (mbs.velocityFunction == 0 && mbs.advectVelocityFunctionStrY.empty( )) { }
#else
					if (mbs.advectVelocityFunctionStrY.empty( )) {
#endif
						badset << "Unset Y velocity " << std::endl; 
					}

					validateSet(badset);
					if (mbs.frontVelocityFunctionStrX.empty( )) {
						const_cast<std::string &>(mbs.frontVelocityFunctionStrX) = mbs.advectVelocityFunctionStrX;
					}
					if (mbs.frontVelocityFunctionStrY.empty( )) {
						const_cast<std::string &>(mbs.frontVelocityFunctionStrY) = mbs.advectVelocityFunctionStrY;
					}
		}

		void checkSet(std::stringstream & badset,const double value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		void checkSet(std::stringstream & badset,const std::string & value, const char * const description) {
			if (value.size( ) == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		void checkSet(std::stringstream & badset,void * value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param value
		* @param str
		* put error message in badset if not exactly one of value, str, is set
		*/
		void checkSet(std::stringstream & badset, void * value, const std::string & str, const char * const description) {
			int argCount = 0;
			if (value != 0) {
				++argCount;
				++nFunctionPointers;
			}
			if (!str.empty( )) { 
				++argCount;
			}
			switch (argCount) {
			case 0:
				badset << "Unset value " << description << std::endl;  
				break;
			case 1:
				break;
			default:
				badset << description << " set more than once" << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param one 
		* @param other 
		* put error message in badset if not exactly one of one, other is et 
		*/
		template<typename T, typename U>
		void checkSet(std::stringstream & badset, T one, U other, const char * const description) {
			if (one == 0 && other == 0) {
				badset << "Unset value " << description << std::endl;  
			}
			if (one != 0 && other != 0) {
				badset << description << " set twice " << std::endl; 
			}
		}

		void validateSet(std::stringstream & badset ) {
			if (badset.str( ).length() > 0) {
				badset << " in MovingBoundarySetup ";
				std::invalid_argument ia(badset.str( ));
				throw ia; 
			}
		}
		int nFunctionPointers;
	};

}

//**************************************************
// main solver, local definition 
//**************************************************
namespace moving_boundary {
	struct MovingBoundaryParabolicProblemImpl : public ValidationBase {
		typedef MovingBoundaryParabolicProblemImpl Outer;
		typedef VoronoiMesh VrnMesh;
		typedef VrnMesh::MBMeshDef MBMeshDef;
		typedef VrnMesh::MBMesh MBMesh;
		//typedef VrnMesh::FrontType FrontType;
		typedef std::vector<spatial::TPoint<moving_boundary::CoordinateType,2> > FrontType;
		typedef World<moving_boundary::CoordinateType,2> WorldType;
		typedef spatial::TPoint<moving_boundary::CoordinateType,2> WorldPoint;
		typedef spatial::TPoint<double,2> ProblemDomainPoint; 


		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs) 
			:ValidationBase(mbs),
			numSpecies(mbs.speciesSpecs.size( )),
			world(WorldType::get( )),
			setup_(mbs),
			//diffusionConstant(mbs.diffusionConstant),
			generationCount(0),
			currentTime(0),
			maxTime(mbs.maxTime),
			//TDX			frontTimeStep(mbs.frontTimeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

			symTable(buildSymTable( )),
			levelExp(mbs.levelFunctionStr,symTable), 
			advectVelocityExpX(mbs.advectVelocityFunctionStrX,symTable), 
			advectVelocityExpY(mbs.advectVelocityFunctionStrY,symTable), 
			frontVelocityExpX(mbs.frontVelocityFunctionStrX,symTable), 
			frontVelocityExpY(mbs.frontVelocityFunctionStrY,symTable), 
			initialConcentrationExpressions(numSpecies), 
			zeroSourceTerms(boost::logic::indeterminate),

			vcFront(initFront(world, *this,mbs)),
			currentFront( ),
			meshDefinition(createMeshDef(world, mbs, numSpecies)),
			physiology( ),
			meNodeEnvironment(meshDefinition,physiology),
			interiorVolume(calculateInteriorVolume(world, meshDefinition)),
			primaryMesh(meshDefinition,meNodeEnvironment),
			voronoiMesh(primaryMesh),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			statusPercent(0),
			estimateProgress(false),
			isRunning(false),
			timeClients( ),
			elementClients( ),
			percentInfo( )
		{  

			MeshElementNode::setProblemToWorldDistanceScale(world.theScale( ));

			assert(nFunctionPointers == 0); 
			double maxConstantDiffusion = std::numeric_limits<double>::min( );
			for (int i = 0; i < numSpecies; i++) {
				const SpeciesSpecification & ss = mbs.speciesSpecs[i];
				initialConcentrationExpressions[i] = VCell::Expression(ss.initialConcentrationStr,symTable);
				const biology::Species & sp = physiology.createSpecies(ss.name,ss.sourceExpressionStr, ss.diffusionExpressionStr);
				const SExpression & dt = sp.diffusionTerm( );
				if (dt.isConstant( )) {
					maxConstantDiffusion = std::max(maxConstantDiffusion, dt.constantValue( ));
				}
			}
			std::array<std::string,3> syms = {"x","y","t"};
			physiology.buildSymbolTable<3>(syms);
			physiology.lock( );

			setInitialValues( );

			//check time step
			/* TDX
			if (mbs.frontTimeStep <= 0) {
			VCELL_EXCEPTION(domain_error,"invalid time step  " << mbs.frontTimeStep);
			}
			*/
			VCell::MapTable mt;
			mt["D"] = maxConstantDiffusion;
			double hx = mt["hx"] = world.distanceToProblemDomain(meshDefinition.interval(spatial::cX));
			double hy = mt["hy"] = world.distanceToProblemDomain(meshDefinition.interval(spatial::cY));
			mt["hmin"] = minimimMeshInterval = std::min(hx,hy);
			mt["hmax"] = std::max(hx,hy);
			VCell::MTExpression fts(mbs.frontTimeStep,mt);
			frontTimeStep = fts.evaluate( );

			double maxStep = minimimMeshInterval * minimimMeshInterval/(4 * maxConstantDiffusion); 
			if (maxStep < frontTimeStep) {
				if (mbs.hardTime) {
					VCELL_EXCEPTION(logic_error,"hard set input time step " << frontTimeStep << " greater than maximum allowed by problem (" << maxStep << ')');
				}
				frontTimeStep = maxStep;
			}

			using matlabBridge::MatLabDebug;
			if (MatLabDebug::on("tiling")) {
				std::ofstream master("tilingMaster.m");
				matlabBridge::TPolygon<moving_boundary::CoordinateType> pFront("b");
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					MeshElementNode &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						std::stringstream command;
						command << "tilingX" << e.indexOf(spatial::cX) << 'Y' << e.indexOf(spatial::cY) << std::ends;

						std::stringstream filestr;
						filestr <<  command.str( ).c_str( ) << ".m" << std::ends;
						std::ofstream os(filestr.str( ));

						master << command.str( ) << std::endl;
						master << matlabBridge::PauseSeconds(0.5) << std::endl;
						//os << matlabBridge::FigureName(command.str( ).c_str( ));
						os << matlabBridge::clearFigure << pFront;
						e.writeMatlab(os);
					}
				}
			}
			if (MatLabDebug::on("tilingmovie")) {
				std::ofstream master("tilingMovie.m");
				matlabBridge::Movie movie(master);
				master << "% move 'axis' command to beginning of file" << std::endl;
				master << matlabBridge::FigureName("Tiling, the movie");
				matlabBridge::Polygon pFront("b");
				matlabBridge::FigureLimits<moving_boundary::CoordinateType> limits;
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					MeshElementNode &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						master << matlabBridge::clearFigure << pFront;
						e.writeMatlab(master, &limits);
						movie.recordFrame( );
					}
				}
				master << "% move this to top before executing "<< std::endl;
				master << limits; 
				movie.play( );
			}
#ifdef TEST_DUMP 
			std::ofstream bp("bp.txt");
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				iter->listBoundary(bp,primaryMesh);
			}
#endif
			//validate grid 
			CoordinateType hSpace = primaryMesh.interval(cX);
			CoordinateType vSpace = primaryMesh.interval(cY);
			if (hSpace%2 != 0 || vSpace%2 != 0) {
				VCELL_EXCEPTION(logic_error,"spacings horiz " << hSpace << " and vert " << vSpace << " not even");
			}
			const size_t maxI =  primaryMesh.numCells(cX) - 1;
			const size_t maxJ =  primaryMesh.numCells(cY) - 1;
			for (size_t i = 0; i < maxI; i++) 
				for (size_t j = 0; j < maxJ - 1; j++) 
				{
					std::array<size_t,2> p = {i    ,j};
					std::array<size_t,2> r = {i + 1,j};
					std::array<size_t,2> d = {i    ,j + 1};
					MeshElementNode &  e = primaryMesh.get(p);
					MeshElementNode & down = primaryMesh.get(d);
					MeshElementNode & right = primaryMesh.get(r);
					CoordinateType hGap = right(cX) - e(cX); 
					CoordinateType vGap = down(cY) - e(cY); 
					if (hGap != hSpace) {
						VCELL_EXCEPTION(logic_error, "bad h gap between " << e << " and " << right << ", " << hGap << " != expected " << hSpace);
					}
					if (vGap != vSpace) {
						VCELL_EXCEPTION(logic_error, "bad v gap between " << e << " and " << down << ", " << vGap << " != expected " << vSpace);
					}
				}
		}

		~MovingBoundaryParabolicProblemImpl( ) {
			delete vcFront;
		}

		void setInterior(MeshElementNode &e) {
			e.setInteriorVolume(interiorVolume);
		}

		void setConcentrations( vector<ConcentrationProvider> & concExp, MeshElementNode & e) {
			e.allocateSpecies();
			ProblemDomainPoint pdp = world.toProblemDomain(e);
			for (int i = 0; i< numSpecies; i++) {
				double mu = concExp[i].conc(pdp(cX),pdp(cY));
				e.setConcentration(i,mu);
			}
		}

		/**
		* set initial mesh sizes
		*/
		void setInitialValues( ) {
			using std::vector;
			assert(initialConcentrationExpressions.size( ) == numSpecies);
			assert(physiology.numberSpecies( ) == numSpecies);
			vector<ConcentrationProvider> concExp;
			for (vector<VCell::Expression>::iterator iter = initialConcentrationExpressions.begin( ); iter != initialConcentrationExpressions.end( ); ++iter) {
				concExp.push_back(ConcentrationProvider(*iter));
			}
			assert(concExp.size( ) == numSpecies);

			for (MeshElementNode & e: primaryMesh) {
				e.setInteriorVolume(interiorVolume);
			}
			currentFront = vcFront->retrieveFront( );
			std::cout << "front size " << currentFront.size( ) << std::endl;
			if (currentFront.empty( )) {
				throw std::invalid_argument("empty front");
			}
			VCELL_LOG(warn,"initial front has " << currentFront.size( ) << " points");
			assert(currentFront.front( ) == currentFront.back( ));  //verify closed 
			voronoiMesh.setFront(currentFront);

			//spatial::Positions<MeshElementNode> positions = spatial::classify2(voronoiMesh,currentFront);
			moving_boundary::Positions<MeshElementNode> positions = voronoiMesh.classify2(currentFront);
			for (vector<MeshElementNode *>::iterator iter = positions.inside.begin( ); iter != positions.inside.end( ); ++iter) {
				MeshElementNode  & insideElement = **iter;
				setConcentrations(concExp,insideElement);
				insideElement.getControlVolume();
			}
			for (vector<MeshElementNode *>::iterator iter = positions.boundary.begin( ); iter != positions.boundary.end( ); ++iter) {
				MeshElementNode  & boundaryElement = **iter;
				setConcentrations(concExp,boundaryElement);
				boundaryElements.push_back(&boundaryElement);
				boundaryElement.findNeighborEdges();
			}
			/*
			for (vector<MeshElementNode *>::iterator iter = positions.outside.begin( ); iter != positions.outside.end( ); ++iter) {
			}
			*/
		}

		void boundaryElementSetup(MeshElementNode & bElem) {
			assert (bElem.mPos( ) == spatial::boundarySurface);
		}

		/**
		* constructor support; front initialization
		*/
		static spatial::FrontProvider<moving_boundary::CoordinateType> *initFront(const WorldType & world, const ValidationBase & base,const moving_boundary::MovingBoundarySetup &mbs)  {
			if (mbs.alternateFrontProvider == nullptr) {
				typedef spatial::TGeoLimit<moving_boundary::CoordinateType> LimitType;
				const std::array<LimitType,2> & worldLimits = world.limits( );
				std::vector<spatial::GeoLimit> limits(worldLimits.size( ));
				std::transform(worldLimits.begin( ),worldLimits.end( ),limits.begin( ),spatial::GeoLimitConvert<moving_boundary::CoordinateType,double>( ) );
				const std::array<Universe<2>::CountType,2> & nodes = world.universe( ).numNodes( ); 
				int mnode = *std::max_element(nodes.begin( ),nodes.end( ));
				int numFrontRegions = mnode * mbs.frontToNodeRatio;
				if (base.nFunctionPointers == 0) {
					spatial::FrontProvider<moving_boundary::CoordinateType> *prv = 
						new spatial::VCellFront<moving_boundary::CoordinateType>(limits,numFrontRegions,mbs.maxTime,base,base);
					//std::ofstream lc("levelcalls.m");
					//lc << scatterInside << scatterOutside;
					return prv;
				}  
				/* else */
				throw std::domain_error("function pointers no longer supported");
				/*
				return new VCellFront(limits,numFrontRegions,mbs.maxTime,
				mbs.levelFunction,mbs.velocityFunction);
				*/
			}
			return mbs.alternateFrontProvider;
		}

		/**
		* constructor support; mesh initialization
		*/
		static MBMeshDef createMeshDef(const WorldType & world, const moving_boundary::MovingBoundarySetup &mbs, int numSpecies)  {


			typedef spatial::TGeoLimit<moving_boundary::CoordinateType> LimitType;
			using vcell_util::arrayInit;
			const std::array<LimitType,2> & worldLimits = world.limits( );
			std::array<moving_boundary::CoordinateType,2> origin= arrayInit<moving_boundary::CoordinateType>(worldLimits[0].low( ),worldLimits[1].low( ) );
			std::array<moving_boundary::CoordinateType,2> c = arrayInit<moving_boundary::CoordinateType>(worldLimits[0].span( ),worldLimits[1].span( ) );
			const Universe<2> & universe = world.universe( );
			std::array<size_t,2> p = { universe.numNodes( )[0], universe.numNodes( )[1] };
			return MBMeshDef(origin,c,p, numSpecies);
		}

		/**
		* constructor support; calculate interior volume 
		*/
		static moving_boundary::VolumeType calculateInteriorVolume(const WorldType & world, const MBMeshDef & meshDefinition) {
			//direct multiplication may not work due to type overflow -- assign to VolumeType for type conversion
			moving_boundary::VolumeType rval = meshDefinition.interval(spatial::cX);
			rval *= meshDefinition.interval(spatial::cY);
			rval /= world.theScale( ) * world.theScale( );
			return rval;
		}

		/**
		* constructor support; build SymbolTable 
		*/
		static SimpleSymbolTable buildSymTable( ) {
			std::string syms[] = { "x","y","t"};
			return SimpleSymbolTable(syms, sizeof(syms)/sizeof(syms[0]));
		}

		virtual double level(double *in) const {
			double problemDomainValues[2];
			world.toProblemDomain(in,problemDomainValues);
			double r = levelExp.evaluateVector(problemDomainValues);
			/*
			if (r > 0) {
			scatterInside.add(problemDomainValues[0],problemDomainValues[1]);
			}
			else {
			scatterOutside.add(problemDomainValues[0],problemDomainValues[1]);
			}
			*/
			return r;
		}

		/**
		* front velocity returned in problem domain units
		* @param x world coordinate
		* @param y world coordinate
		*/
		spatial::SVector<double,2> frontVelocityProbDomain(double x, double y) const {
			double worldValues[2] = {x,y};
			enum {ex = 0, ey = 1, et = 2};
			double syms[3];
			world.toProblemDomain(worldValues,syms);
			syms[et] = currentTime; 
			double vX = frontVelocityExpX.evaluateVector(syms);
			double vY = frontVelocityExpY.evaluateVector(syms);
			return spatial::SVector<double,2>(vX,vY);
		}

		/**
		* front velocity returned  in world units 
		* @param x world coordinate
		* @param y world coordinate
		*/
		spatial::SVector<moving_boundary::VelocityType,2> frontVelocity(double x, double y) const {
			//fspy << x << ',' << y << ',' << currentTime << std::endl;
			auto rval =  world.toWorld<moving_boundary::VelocityType>(frontVelocityProbDomain(x,y));
			return rval; 
		}

		spatial::SVector<moving_boundary::VelocityType,2> advectionVelocity(double x, double y) const {

			double worldValues[2] = {x,y};
			enum {ex = 0, ey = 1, et = 2};
			double syms[3];
			world.toProblemDomain(worldValues,syms);
			syms[et] = currentTime; 
			double vX = advectVelocityExpX.evaluateVector(syms);
			double vY = advectVelocityExpY.evaluateVector(syms);
			return world.toWorld<moving_boundary::VelocityType>(spatial::SVector<double,2>(vX,vY)); 
		}

		virtual int velocity(Frontier::Front*,Frontier::POINT* fpoint,HYPER_SURF_ELEMENT*, HYPER_SURF*,double* out) const {
			const spatial::SVector<moving_boundary::VelocityType,2> & v = frontVelocity(fpoint->_coords[cX],fpoint->_coords[cY]);
			out[cX] = v(cX); 
			out[cY] = v(cY); 

			return 0; //return value not used by frontier
		}
		/**
		* @return <timeNow, timeIncrement>
		*/
		std::pair<double,double> times(int generationCount) { 
			double timeNow = baselineTime + (generationCount - baselineGeneration) * frontTimeStep;
			if (timeNow > maxTime) {
				timeNow = maxTime;
			}
			double incr = timeNow - currentTime;
			return std::pair<double,double>(timeNow,incr);
		}

		/**
		* update front time step to ensure last step(s) do not proceed past maxTime
		* for the simulation, and try make steps as even as possible
		* @param desiredStep 
		* @param generationCount for comparing to baseline 
		*/

		void updateTimeStep(double desiredStep, int generationCount) {
			double intervalTilEnd = maxTime - baselineTime;
			int n = ceil(intervalTilEnd / desiredStep);
			frontTimeStep = intervalTilEnd / n;
			baselineGeneration = generationCount;
			baselineTime = currentTime;
		}

		//This implementation is optimized for the case 10/2014 where this is but a single "element" client
		//If there are multiple clients it could be better to loop over them inside this function -- probably
		//more efficient to iterate over std::vector than the mesh
		/**
		* @param changed nodes have changed since last report
		*/
		void giveElementsToClient(MovingBoundaryElementClient & client, size_t generationCount, bool changed) {
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				//std::cout << iter->ident( ) << std::endl;
				if (iter->isInside( )) {
					client.element(*iter);
				}
			}
			client.iterationComplete();
		}

		/**
		* notify all clients that time has changed, and give the elements to those  that want them
		*/
		void notifyClients(size_t generationCount, bool changed) {
			for (MovingBoundaryTimeClient *tclient: timeClients) {
				GeometryInfo<moving_boundary::CoordinateType> gi(currentFront,changed);
				tclient->time(currentTime, generationCount, currentTime == maxTime, gi);
			}
			for (MovingBoundaryElementClient *eclient: elementClients) {
				giveElementsToClient(*eclient,generationCount,changed);
			}
		}

		void debugDump(size_t gc, char key) {
			static bool fullDump = matlabBridge::MatLabDebug::on("frontmovefull");
			VCELL_LOG(trace,"dd " << gc << key << " time " << currentTime); 
			std::ostringstream oss;
			oss << "frontmove"  << std::setfill('0') << std::setw(3) << gc << key << ".m" << std::ends;
			std::ofstream dump(oss.str( ));
			{
				std::ostringstream mbuf;
				mbuf << "Time " << currentTime;
				dump << matlabBridge::ConsoleMessage(mbuf.str());
				dump << matlabBridge::FigureName(oss.str( ).c_str( ));
			}
			matlabBridge::TScatter<CoordinateType> deepInside('b',3,false,0);
			matlabBridge::TScatter<CoordinateType> inside('b',2,true,1);
			matlabBridge::TScatter<CoordinateType> boundary('g',2,true,2);
			matlabBridge::TScatter<CoordinateType> outside('r',2,true,3);
			matlabBridge::TScatter<CoordinateType> deepOutside('r',3,false,4);
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				switch(iter->mPos( )) {
				case spatial::interiorSurface:
					if (iter->boundaryOffset( ) > 1) {
						frontTierAdapt::copyPointInto(deepInside,*iter);
					}
					else {
						frontTierAdapt::copyPointInto(inside,*iter);
					}
					break;
				case spatial::boundarySurface:
					frontTierAdapt::copyPointInto(boundary,*iter);
					break;
				case spatial::outsideSurface:
					if (iter->boundaryOffset( ) > 1) {
						frontTierAdapt::copyPointInto(deepOutside,*iter);
					}
					else {
						frontTierAdapt::copyPointInto(outside,*iter);
					}
					break;
				default:
					VCELL_EXCEPTION(domain_error,"bad state " << static_cast<int>(iter->mPos( )) << iter->ident( )) ;
				}
				VCELL_LOG(trace,"dd " << iter->ident( ));
				double  x = iter->get(cX);
				double  y = iter->get(cY);

				std::stringstream ss;
				ss << iter->indexOf(cX) << ',' << iter->indexOf(cY) << '-' << static_cast<unsigned int>(iter->boundaryOffset( )) << std::ends;
				dump << matlabBridge::Text(x,y, ss.str( ).c_str( ));
				if (fullDump) {
					matlabBridge::Polygons pgons("k");
					const Volume2DClass & vol = iter->getControlVolume( );
					frontTierAdapt::copyVectorsInto(pgons,vol.points( ));
					dump << pgons;
				}
			}
			dump << deepInside << inside << boundary << outside << deepOutside; 
			matlabBridge::TPolygon<CoordinateType> pPoly("k",1);
			frontTierAdapt::copyVectorInto(pPoly,currentFront);
			dump << pPoly; 
		}

		static void commenceSimulation(MeshElementNode &e) {
			e.beginSimulation( );
		}

		/**
		* Functor
		* base class for functors which need access to #Outer
		*/
		struct FunctorBase {
			FunctorBase(const Outer &o)
				:outer(o) {}
			const Outer & outer;
		};
		/**
		* Functor
		* move front
		*/
		struct MoveFront : public FunctorBase {
			MoveFront(Outer &o)
				:FunctorBase(o){}
			void operator( )(MeshElementNode * p) {
				assert(p != nullptr);
				VCELL_LOG(trace,p->indexInfo( ) << " front move")
					if (p->mPos() == spatial::boundarySurface) {
						p->moveFront(this->outer.currentFront);
					}
					else {
						throw std::domain_error("MoveFront");
					}
			}
		};
		/**
		* Functor
		* FindNeighborEdges 
		*/
		struct FindNeighborEdges  : public FunctorBase {
			FindNeighborEdges (Outer &o)
				:FunctorBase(o) {}

			void operator( )(MeshElementNode * p) {
				if (p->isInside( ) ) {
					p->findNeighborEdges( );
				}
			}
		};
#ifdef HOLD
#error should use front velocity, not elements
		/**
		* Functor
		* apply velocity, record max
		*/
		struct MaxVel : public FunctorBase {
			MaxVel (Outer &o)
				:FunctorBase(o),
				maxVel(0) {}
			void operator( )(MeshElementNode & e) {
				VCELL_LOG(trace,e.indexInfo( ) << " max velocity")
					SVector<double,2> v = this->outer.velocity(e(cX),e(cY));
				maxVel  = std::max(maxVel, v(cX));
				maxVel  = std::max(maxVel, v(cY));
				e.setVelocity(v);
			}

			REAL maxVel;
		};
#endif
		/**
		* Functor
		* find maximum velocity of front points (squared) 
		*/
		struct FrontVelocity : public FunctorBase {
			FrontVelocity (Outer &o)
				:FunctorBase(o),
				maxSquaredVel(0){}
			void operator( )(const spatial::TPoint<CoordinateType,2> & point) {
				spatial::SVector<double,2> velVector = this->outer.frontVelocityProbDomain(point(cX),point(cY));
				maxSquaredVel = std::max(maxSquaredVel,velVector.magnitudeSquared( ));
			}
			moving_boundary::VelocityType maxSquaredVel;
		};

		/**
		* Functor
		* apply velocity, record max
		*/
		struct ApplyVelocity : public FunctorBase {
			ApplyVelocity (Outer &o)
				:FunctorBase(o),
				maxVel(0){}
			void operator( )(MeshElementNode & e) {
				VCELL_LOG(trace,e.indexInfo( ) <<  " set velocity");
				spatial::SVector<moving_boundary::VelocityType,2> v = this->outer.advectionVelocity(e(cX),e(cY));
				e.setVelocity(v);
				maxVel = std::max(maxVel, std::max(v(cX),v(cY)));
			}
			moving_boundary::VelocityType maxVel;
		};

		/**
		* Functor diffuseAdvect
		*/
		template <class SOLVER>
		struct DiffuseAdvect {
			DiffuseAdvect(SOLVER &s)
				:speciesIdx(0),
				solver(s) {}

			void operator( )(MeshElementNode & e) {
				VCELL_LOG(trace,e.ident( ) << " diffuseAdvect");
				e.diffuseAdvect(solver,speciesIdx);
			}
			unsigned int speciesIdx;
			SOLVER &solver;

		};

		/**
		* Functor advectComplete
		*/
		struct AdvectComplete {
			void operator( )(MeshElementNode & e) {
				if (e.isInside( )) {
					VCELL_LOG(trace,e.ident( ) << " advectComplete");
					//e.advectComplete( );
				}
			}
		};

		/**
		* Functor react 
		*/
		struct React {
			React(double time_, double timeStep_)
				:time(time_),
				timeStep(timeStep_){}

			void operator( )(MeshElementNode & e) {
				if (e.isInside( )) {
					VCELL_LOG(trace,e.ident( ) << " react")
						e.react(time, timeStep);
				}
			}

			const double time;
			const double timeStep;
		};

#if 0
		/**
		* Functor ClearDiffuseAdvect
		*/
		struct ClearDiffuseAdvect {
			void operator( )(MeshElementNode & e) {
				if (e.mPos( ) != outsideSurface) {
					VCELL_LOG(trace,e.ident( ) << " clearDiffuseAdvect")
						e.clearDiffuseAdvectAmounts( );
				}
			}

		};
#endif

		/**
		* Functor
		* collect mass 
		*/
		struct CollectMass : public FunctorBase {
			CollectMass (Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode *p) {
				VCELL_LOG(trace,p->ident( ) << " collectMass")
					p->collectMass(this->outer.currentFront);
			}
		};

		/**
		* Functor
		* apply front
		*/
		struct ApplyFront : public FunctorBase {
			ApplyFront(Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode *p) {
				VCELL_LOG(trace,p->ident( ) << " applyFront")
					p->applyFront(this->outer.currentFront, this->outer.interiorVolume);
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct DistributeLost { //OPTIMIZE -- keep collection of objects in state 
			void operator( )(MeshElementNode &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " distributeLost")
					e.distributeMassToNeighbors();
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct EndCycle : public FunctorBase {
			EndCycle(Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " end of cycle");
				e.endOfCycle( );
			}
		};

		void add(MovingBoundaryElementClient & client) {
			VCELL_LOG(info,"added element client " << typeid(client).name( ) );
			if (std::find(elementClients.begin( ), elementClients.end( ),&client) == elementClients.end( ) ) {
				elementClients.push_back(&client);
			}
			add(static_cast<MovingBoundaryTimeClient &>(client) );
		}

		void add(MovingBoundaryTimeClient & client) {
			VCELL_LOG(info,"added time client " << typeid(client).name( ) );
			if (std::find(timeClients.begin( ), timeClients.end( ),&client) == timeClients.end( ) ) {
				timeClients.push_back(&client);
			}
		}

		//reset a flag to false during stack unwinding
		struct Resetter{
			bool & flag;
			Resetter(bool & flag_)
				:flag(flag_) {}
			~Resetter( ) {
				flag = false;
			}
		};

		/**
		* package of percent progress variables for convenience / clarity
		*/
		struct ProgressPercentInfo {
			double simStartTime;
			double lastPercentTime; //PDEL
			double progressDelta; //statusPercent in time 
			size_t nextProgressGeneration; 
			std::chrono::steady_clock::time_point runStartTime;

			/**
			* initialize to unused state
			*/
			ProgressPercentInfo( )
				:simStartTime(0),
				lastPercentTime(0),
				progressDelta(0),
				nextProgressGeneration(inactiveValue( )),
				runStartTime( ) {}

			/**
			* estimate next generation a percent should be output
			* assumes progressData et. al. have been set externally
			*/
			void calculateNextProgress(size_t currentGeneration, double frontTimeStep) {
				//double nextTime = lastPercentTime + progressDelta;
				nextProgressGeneration = static_cast<size_t>(progressDelta/frontTimeStep) + currentGeneration;
				VCELL_KEY_LOG(debug,Key::progressEstimate, "PE lpt " << lastPercentTime << " pd " << progressDelta 
					//	<< " nextTime " << nextTime 
					<< " frontTimeStep " << frontTimeStep << " current gen " << currentGeneration 
					<< " next Gen " << nextProgressGeneration);
			}

			bool isActive( ) const {
				return nextProgressGeneration != inactiveValue( );
			}

			static size_t inactiveValue( ) {
				return std::numeric_limits<size_t>::max( );
			}
		};

		void run( ) {
			VCELL_LOG(info,"commence simulation");
			const AdvectComplete advectComplete;
			ExplicitSolver solver(primaryMesh);
			DiffuseAdvect<ExplicitSolver> diffuseAdvect(solver);
			Resetter r(isRunning);
			isRunning = true;


			if (statusPercent > 0)  {
				percentInfo.simStartTime = currentTime;
				percentInfo.progressDelta = statusPercent / 100.0 * maxTime;
				percentInfo.calculateNextProgress(generationCount,frontTimeStep);
				if (estimateProgress) {
					percentInfo.runStartTime = std::chrono::steady_clock::now( );
				}
			}

			const bool frontMoveTrace = matlabBridge::MatLabDebug::on("frontmove") || matlabBridge::MatLabDebug::on("frontmovefull");
			FrontType oldFront; //for debugging

			try { 
				if (generationCount == 0) {
					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),commenceSimulation);
				}
				notifyClients(generationCount,true);
				if (frontMoveTrace) {
					debugDump(generationCount, 's');
				}
				generationCount++;

				while (currentTime < maxTime) {
					std::pair<double,double> nowAndStep = times(generationCount); 
					//TODO -- we're approximating front velocity for time step with velocity at beginning of time step
					FrontVelocity fv = std::for_each(currentFront.begin( ),currentFront.end( ),FrontVelocity(*this));
					double maxVel = sqrt(fv.maxSquaredVel);
					double maxTimeStep_ = minimimMeshInterval / (2 * maxVel);
					if (nowAndStep.second > maxTimeStep_) {
						updateTimeStep(maxTimeStep_,generationCount - 1);
						nowAndStep = times(generationCount); 
					}
					//currentTime = nowAndStep.first;
					const double endOfStepTime = nowAndStep.first;
					double timeIncr = nowAndStep.second;

					ApplyVelocity mv(*this);
					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),mv);

					VCELL_LOG(debug, "moved to time " << endOfStepTime << " generation " << generationCount);
					vcFront->propagateTo(endOfStepTime); 
					/*
					VCellFront *vcp = dynamic_cast<VCellFront *>(vcFront);
					if (vcp != nullptr) {
					Front * checkpoint = copy_front(vcp->c_ptr( ));
					FT_Save(checkpoint, "testsave.txt");
					}
					*/


					if (frontMoveTrace) {
						oldFront = currentFront; //for debugging
					}
					currentFront = vcFront->retrieveFront( );
					voronoiMesh.setFront(currentFront);

					{
						//					std::ofstream f("fullvdump.m");
						//					voronoiMesh.matlabPlot(f, &currentFront);
					}
					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),MoveFront(*this));



					try {
						std::for_each(primaryMesh.begin( ),primaryMesh.end( ), React(currentTime,timeIncr) );
					} catch (ReverseLengthException &rle) {
						std::ofstream s("rle.m");
						rle.aElement.writeMatlab(s, nullptr,false, 20);
						rle.bElement.writeMatlab(s, nullptr,false, 20);
						throw;
					}
					primaryMesh.diffuseAdvectCache( ).start( );
					for (unsigned s = 0; s < physiology.numberSpecies( ) ; s++) { 
						solver.setStepAndSpecies(timeIncr,s);
						diffuseAdvect.speciesIdx = s;
						std::for_each(primaryMesh.begin( ),primaryMesh.end( ), diffuseAdvect);
						solver.solve( );
					}
					primaryMesh.diffuseAdvectCache( ).finish( );
					//std::for_each(primaryMesh.begin( ),primaryMesh.end( ), advectComplete);

					currentTime = endOfStepTime;
					if (frontMoveTrace) {
						debugDump(generationCount,'b');
					}


					bool changed;
					//adjustNodes calls MeshElementNode.updateBoundaryNeighbors
					try {
						changed = voronoiMesh.adjustNodes(boundaryElements);
					} catch (SkipsBoundary & skips) {
						std::ofstream sb("sb.m");
						matlabBridge::TPolygon<long long> oldPoly("k",1);
						frontTierAdapt::copyVectorInto(oldPoly,oldFront);
						matlabBridge::TPolygon<long long> newPoly("r",1);
						frontTierAdapt::copyVectorInto(newPoly,currentFront);
						matlabBridge::TScatter<long> scatter('b',2,true);
						scatter.add(skips.mes(cX),skips.mes(cY));
						sb << '%' << skips.mes.ident( ) << std::endl;
						sb << oldPoly << newPoly << scatter;
						std::cerr << "rethrowing skips" << std::endl;
						debugDump(generationCount,'e');
						throw;
					}
					//TODO: nochange

					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),CollectMass(*this));

					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),ApplyFront(*this));

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),DistributeLost());

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),EndCycle(*this));

					boundaryElements.front( )->setBoundaryOffsetValues( );

					if (frontMoveTrace) {
						debugDump(generationCount,'a');
					}

					//tell the clients about it
					notifyClients(++generationCount, changed);
					if (generationCount >= percentInfo.nextProgressGeneration) {
						unsigned int percent = static_cast<unsigned int>(100 * currentTime / maxTime + 0.5);
						if (percent < 100) { //looks silly to report 100% when still running
							std::cout << std::setw(2) << percent << "% complete";
							if (estimateProgress) {
								namespace chrono = std::chrono;
								using chrono::steady_clock;
								steady_clock::time_point timeNow = steady_clock::now( );
								steady_clock::duration elapsed = timeNow - percentInfo.runStartTime;
								chrono::seconds eseconds = chrono::duration_cast<chrono::seconds>(elapsed); 
								VCELL_KEY_LOG(debug,Key::progressEstimate, "PE percent " << percent << " elapsed " << eseconds.count( ));

								//simulation may not have started at time 0 if this run was restored from a persisted problem
								const double simTimeThisRun = maxTime - percentInfo.simStartTime;
								const double simTimeThusFar = currentTime - percentInfo.simStartTime;
								if (eseconds.count( ) > 0) { //can't estimate at beginning
									const double t = eseconds.count( )  * simTimeThisRun  / simTimeThusFar;
									chrono::seconds total(static_cast<int>(t));
									chrono::seconds remaining = total - eseconds; 
									namespace HMSf = vcell_util::HoursMinutesSecondsFormat;
									typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL> HMS;
									typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL|HMSf::LONG_UNITS> HMSu;
									std::cout << ", elasped time " << HMS(eseconds) << ", estimated time remaining " << HMSu(remaining);
									VCELL_KEY_LOG(debug,Key::progressEstimate, "PE thisRun " <<  simTimeThisRun 
										<< " thusFar " << simTimeThusFar << " t calc " << t
										<< " est total seconds " << total.count( ) << ' ' << remaining.count( ) << " sec");

								}
							}
							std::cout << std::endl;
							percentInfo.lastPercentTime = currentTime;
							percentInfo.calculateNextProgress(generationCount,frontTimeStep);
						}
					}
				}
				if (percentInfo.isActive( )) {
					std::cout << "100 % complete";
					if (estimateProgress) {
						namespace chrono = std::chrono;
						using chrono::steady_clock;
						steady_clock::time_point timeNow = steady_clock::now( );
						steady_clock::duration elapsed = timeNow - percentInfo.runStartTime;
						chrono::seconds eseconds = chrono::duration_cast<chrono::seconds>(elapsed); 
						namespace HMSf = vcell_util::HoursMinutesSecondsFormat;
						typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL> HMS;
						std::cout << ", elasped time " << HMS(eseconds); 
					}
					std::cout << std::endl;
				}
				for (MovingBoundaryTimeClient *client : timeClients) {
					client->simulationComplete( );
				}
			} catch (std::exception &e) {
				VCELL_LOG(fatal,"run( ) caught " << e.what( )  << " generation " << generationCount << " time" << currentTime);
				throw;
			}
		}

		const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef( ) const {
			return meshDefinition;
		}

		unsigned int numberTimeSteps( ) const {
			return static_cast<unsigned int>(maxTime / frontTimeStep);
		}
		void reportProgress(unsigned char percent, bool estimateTime) {
			if (!isRunning) {
				statusPercent = percent;
				estimateProgress = estimateTime;
				return;
			}
			throw std::logic_error("Call to reportProgress( ) while simulation is running");
		}

		bool noReaction( ) const {
			if (zeroSourceTerms.value == boost::logic::tribool::indeterminate_value) {
				zeroSourceTerms = true; //assume true, test for falseness
				for (auto iter = physiology.beginSpecies( ); iter != physiology.endSpecies( ); ++iter) {
					SExpression sourceTerm = iter->sourceTerm( );
					if (!sourceTerm.isConstant( ) || sourceTerm.constantValue( ) != 0) {
						zeroSourceTerms = false; 
						break;
					}
				}
			}
			return zeroSourceTerms;
		}

		const MovingBoundarySetup & setup( ) const {
			return setup_;
		}

		static void registerType( ) {
			MBMesh::registerType( );
			vcell_persist::Registrar::reg<MovingBoundaryParabolicProblemImpl>("MovingBoundaryParabolicProblemImpl");
		}

		void registerInstanceType( ) const {
			vcFront->registerType(  );
		}

		/**
		* Functor -- convert Element * to index 
		*/
		typedef unsigned short ElementStorageType;

		struct ElementToIndex : public FunctorBase {
			ElementToIndex(const Outer &o)
				:FunctorBase(o) {}

			ElementStorageType operator( )(const MeshElementNode *in) {
				spatial::MeshPosition mp = outer.primaryMesh.indexOf(in->indexes( ));
				return mp.to<ElementStorageType>( ); 
			}
		};

		/**
		* Functor -- convert index to Element *  
		*/
		struct IndexToElement : public FunctorBase {
			IndexToElement(const Outer &o)
				:FunctorBase(o) {}

			MeshElementNode * operator( )(ElementStorageType i) {
				spatial::MeshPosition mp(i);
				return & outer.primaryMesh.get(mp);
			}
		};

		/**
		* persist given vector of elements to os
		* converts Element * to ElementStorageType
		* @tparam E const Element or Element
		*/
		template <class E>
		void persistElementsVector(std::ostream &os, const vector<E *>  &vec) const {
			std::vector<ElementStorageType> writeBuffer(vec.size( ));
			ElementToIndex eToIndex(*this);
			std::transform(vec.begin( ),vec.end( ),writeBuffer.begin( ),eToIndex);
			vcell_persist::save(os,writeBuffer);
		}

		/**
		* restore given vector of elements from is 
		* converts ElementStorageType to (const) Element * 
		* @tparam E const Element or Element
		*/
		template <class E>
		void restoreElementsVector(std::istream &is, vector<E *>  &vec) const {
			std::vector<ElementStorageType> readBuffer; 
			vcell_persist::restore(is,readBuffer);
			IndexToElement iToElement(*this);
			vec.resize( readBuffer.size( ) );
			std::transform(readBuffer.begin( ),readBuffer.end( ),vec.begin( ),iToElement);
		}

		/**
		* save this. Note #MovingBoundarySetup must be saved separately by client
		*/
		void persist(std::ostream &os) const {
			//setup_.persist(os);
			vcell_persist::Token::insert<MovingBoundaryParabolicProblemImpl>(os);
			//vcell_persist::binaryWrite(os,diffusionConstant);
			vcell_persist::binaryWrite(os,generationCount);
			vcell_persist::binaryWrite(os,currentTime);
			//vcell_persist::binaryWrite(os,maxTime);
			vcell_persist::binaryWrite(os,frontTimeStep);
			vcell_persist::binaryWrite(os,baselineTime);
			vcell_persist::binaryWrite(os,baselineGeneration);
			vcell_persist::binaryWrite(os,minimimMeshInterval);
			vcFront->persist(os);
			vcell_persist::save(os,currentFront);
			meshDefinition.persist(os);
			primaryMesh.persist(os);
			vcell_persist::binaryWrite(os,interiorVolume);

			persistElementsVector(os, boundaryElements);
			persistElementsVector(os, lostElements);
			persistElementsVector(os, gainedElements);
			vcell_persist::binaryWrite(os,statusPercent);
			if (statusPercent > 0) {
				vcell_persist::binaryWrite(os,estimateProgress);
			}
		}

		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs, std::istream &is) 
			:ValidationBase(mbs),
			numSpecies(mbs.speciesSpecs.size( )),
			world(WorldType::get( )),
			setup_(mbs),
			//diffusionConstant(mbs.diffusionConstant),
			generationCount(0),
			currentTime(0),
			maxTime(mbs.maxTime),
			//TDX			frontTimeStep(mbs.frontTimeStep),
			//TDX			solverTimeStep(mbs.solverTimeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

			symTable(buildSymTable( )),
			levelExp(mbs.levelFunctionStr,symTable), 
			advectVelocityExpX(mbs.advectVelocityFunctionStrX,symTable), 
			advectVelocityExpY(mbs.advectVelocityFunctionStrY,symTable), 
			frontVelocityExpX(mbs.frontVelocityFunctionStrX,symTable), 
			frontVelocityExpY(mbs.frontVelocityFunctionStrY,symTable), 
			initialConcentrationExpressions( ), //PERSIST TODO
			zeroSourceTerms( ), //PERSIST TODO

			vcFront(nullptr),
			currentFront( ),
			meshDefinition( ),
			physiology( ),
			meNodeEnvironment(meshDefinition,physiology),
			interiorVolume( ),
			primaryMesh(),
			voronoiMesh(),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			statusPercent(0),
			estimateProgress(false),
			isRunning(false)
		{
			MeshElementNode::setProblemToWorldDistanceScale(world.theScale( ));
			vcell_persist::Token::check<MovingBoundaryParabolicProblemImpl>(is);
			//vcell_persist::binaryRead(is,diffusionConstant);
			vcell_persist::binaryRead(is,generationCount);
			vcell_persist::binaryRead(is,currentTime);
			//vcell_persist::binaryRead(is,maxTime);
			vcell_persist::binaryRead(is,frontTimeStep);
			vcell_persist::binaryRead(is,baselineTime);
			vcell_persist::binaryRead(is,baselineGeneration);
			vcell_persist::binaryRead(is,minimimMeshInterval);
			vcFront = moving_boundary::restoreFrontProvider(is);
			vcell_persist::restore(is,currentFront);
			meshDefinition = MBMeshDef(is);
			//MBMesh temp(is);
			primaryMesh.restore(is, meNodeEnvironment);
			vcell_persist::binaryRead(is,interiorVolume);

			restoreElementsVector(is, boundaryElements);
			restoreElementsVector(is, lostElements);
			restoreElementsVector(is, gainedElements);
			vcell_persist::binaryRead(is,statusPercent);
			if (statusPercent > 0) {
				vcell_persist::binaryRead(is,estimateProgress);
			}
			voronoiMesh.setMesh(primaryMesh);
		}

		const int numSpecies;
		WorldType & world;
		const MovingBoundarySetup setup_;
		//const double diffusionConstant;

		size_t generationCount;
		/**
		* currentTime must be set before #vcFront initialized
		*/
		double currentTime;
		const double maxTime;
		double frontTimeStep; 
		//double solverTimeStep; 
		/**
		* time last frontTimeStep was calculated
		*/
		double baselineTime;
		/**
		* generation last time step was calculated
		*/
		unsigned int baselineGeneration;
		/**
		* in problem domain units
		*/
		double minimimMeshInterval;

		/**
		* symbol table for expressions, must be initialized prior to xxExp
		*/
		SimpleSymbolTable symTable;
		/**
		* level expression, must be created before front
		*/
		mutable VCell::Expression levelExp; //Expression doesn't currently support "const"
		mutable VCell::Expression advectVelocityExpX;
		mutable VCell::Expression advectVelocityExpY;
		mutable VCell::Expression frontVelocityExpX;
		mutable VCell::Expression frontVelocityExpY;
		std::vector<VCell::Expression> initialConcentrationExpressions;
		mutable boost::logic::tribool zeroSourceTerms; //logically const, but lazily evaluated
		/**
		* FronTier integration
		*/
		spatial::FrontProvider<moving_boundary::CoordinateType> * vcFront;
		FrontType currentFront;
		MBMeshDef meshDefinition;
		biology::Physiology physiology;
		MeshElementNode::Environment meNodeEnvironment;

		/**
		* standard volume of inside point
		*/
		double interiorVolume;
		MBMesh primaryMesh;
		VrnMesh voronoiMesh;
		//		vcell_util::ChunkAllocator<VoronoiResult,60> alloc;
		/**
		* current boundary elements
		*/
		//typedef std::forward_list<MeshElementNode *> ElementList; 
		std::vector<MeshElementNode *> boundaryElements;
		/**
		* elements "lost" (moved outside of boundary) during transition between generations
		* point to previous generation 
		*/
		std::vector<const MeshElementNode *> lostElements;
		/**
		* elements "gained" (moved inside of boundary) during transition between generations
		* point to next generation 
		*/
		std::vector<MeshElementNode *> gainedElements;
		unsigned char statusPercent;
		bool estimateProgress;

		/**
		* runtime flag (not-persistent)
		*/
		bool isRunning;

		/**
		* client storage; not persistent
		*/
		std::vector<MovingBoundaryTimeClient *> timeClients;
		std::vector<MovingBoundaryElementClient *> elementClients;
		ProgressPercentInfo percentInfo;

	};

	//**************************************************
	// local definitions, functors / functions 
	//**************************************************
	namespace {
		struct Evaluator {
			virtual ~Evaluator( ) {}
			virtual double evaluate(MeshElementNode & mes) const = 0;
			virtual const char * const label( ) const = 0;
			virtual const char * const checkFunction( ) const = 0;
			bool supportsCheck( ) const {
				return checkFunction( ) != 0; 
			}
		};

		struct Concentration : public Evaluator {
			virtual double evaluate(MeshElementNode & mes) const { 
				return mes.concentration(0);
			}
			virtual const char * const label( ) const {
				return "concentration";
			}
			virtual const char * const checkFunction( ) const {
				return 0;
			}
		};

		struct VolumeEval : public Evaluator {
			virtual double evaluate(MeshElementNode & mes) const { 
				double v = mes.volumePD( );
				return v;
			}
			virtual const char * const label( ) const {
				return "volume";
			}
			virtual const char * const checkFunction( ) const {
				return  "polyarea";
			}
		};

		void meshElementPlot(const Evaluator & ev, MovingBoundaryParabolicProblemImpl & impl, std::ostream &os) {
			char colors[] = {'y','m','c','g','b','k'};	
			vcell_util::Modulo<int,int> cIndex(0,sizeof(colors)/sizeof(colors[0]));

			MovingBoundaryParabolicProblemImpl::MBMesh::iterator iter = impl.primaryMesh.begin( );
			matlabBridge::Patch patch(ev.label( ), ev.checkFunction( ), "1e-3");
			matlabBridge::Scatter scatter('g',2,true);
			matlabBridge::Scatter empty('r',2,true);
			int seqNo = 1; //used to cause matlab variables to change
			for (;iter != impl.primaryMesh.end( ); ++iter, ++seqNo) {
				MeshElementNode & mes = *iter;
				/*
				if (mes.indexOf(cX) != 5) 
				continue;
				if (mes.indexOf(cY) != 11 && mes.indexOf(cY) != 11 ) {
				continue;
				}
				*/

				std::stringstream ss; ss << '[' << mes.indexOf(cX) << ',' << mes.indexOf(cY) << ']';
				std::string spec("-");
				spec += colors[cIndex];
				++cIndex;
				matlabBridge::Polygon boundingPoly("k",2, seqNo);
				const Volume2DClass & vol = mes.getControlVolume();
				if (vol.empty( )) {
					empty.add(mes(cX),mes(cY));
					ss << "empty";
					os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
					continue;
				}
				double c =  ev.evaluate(mes);
				Volume2DClass::VectorOfVectors vOfV = vol.points( );
				const bool checkVal = vOfV.size( ) == 1;
				patch.checkControl( ) = ev.supportsCheck( ) && checkVal;
				for (Volume2DClass::VectorOfVectors::const_iterator vvIter = vOfV.begin( ); vvIter != vOfV.end( );++vvIter) {
					matlabBridge::Polygon pPoly("k",3);
					patch.specifyValueAndClear(c, seqNo);
					frontTierAdapt::copyVectorInto(pPoly,*vvIter);
					frontTierAdapt::copyVectorInto(patch,*vvIter);
					os << patch << pPoly;
				}
				scatter.add(mes(cX),mes(cY));
				os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
			}
			patch.setScale(os);
			os << scatter << empty;
		}
	}

	//**************************************************
	// MovingBoundary proper
	//**************************************************

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs)
		:sImpl(std::make_shared<MovingBoundaryParabolicProblemImpl>(mbs) ) {}
	//:impl(new MovingBoundaryParabolicProblemImpl(mbs)) { }

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs, std::istream &is) 
		:sImpl(std::make_shared<MovingBoundaryParabolicProblemImpl>(mbs,is) ) {}
	//:impl(new MovingBoundaryParabolicProblemImpl(mbs,is)) { }

	const spatial::MeshDef<moving_boundary::CoordinateType,2> & MovingBoundaryParabolicProblem::meshDef( ) const {
		return sImpl->meshDef( );
	}
	const biology::Physiology  & MovingBoundaryParabolicProblem::physiology( ) const {
		return sImpl->physiology;
	}
	double MovingBoundaryParabolicProblem::frontTimeStep( ) const {
		return sImpl->frontTimeStep;
	}
	double MovingBoundaryParabolicProblem::solverTimeStep( ) const {
		return -1; 
	}
	unsigned int MovingBoundaryParabolicProblem::numberTimeSteps( ) const {
		return sImpl->numberTimeSteps( );
	}

	MovingBoundaryParabolicProblem::~MovingBoundaryParabolicProblem( ) {
	}

	void MovingBoundaryParabolicProblem::reportProgress(unsigned char percent, bool estimateTime) {
		if (percent == 0 || percent > 99) {
			unsigned int p = percent;
			VCELL_EXCEPTION(domain_error, "percent value " << p << " must be between 1 and 99");
		}
		sImpl->reportProgress(percent,estimateTime);
	}

	void MovingBoundaryParabolicProblem::add(moving_boundary::MovingBoundaryTimeClient & client) {
		sImpl->add(client);
	}
	void MovingBoundaryParabolicProblem::add(moving_boundary::MovingBoundaryElementClient & client) {
		sImpl->add(client);
	}
	void MovingBoundaryParabolicProblem::run( ) {
		sImpl->run( );
	}
	bool MovingBoundaryParabolicProblem::noReaction( ) const {
		return sImpl->noReaction( );
	}

	void MovingBoundaryParabolicProblem::plotPolygons(std::ostream &os)  const {
		meshElementPlot(Concentration( ),*sImpl,os);
	}
	void MovingBoundaryParabolicProblem::plotAreas(std::ostream &os) const {
		meshElementPlot(VolumeEval( ),*sImpl,os);
	}
	double MovingBoundaryParabolicProblem::endTime( ) const {
		return sImpl->maxTime;
	}
	const MovingBoundarySetup &MovingBoundaryParabolicProblem::setup( ) const {
		return sImpl->setup( );
	}
	std::string MovingBoundaryParabolicProblem::frontDescription( ) const {
		return sImpl->vcFront->describe( );
	}

	void MovingBoundaryParabolicProblem::persist(std::ostream &os) const {
		sImpl->persist(os);
	}

	void MovingBoundaryParabolicProblem::registerType( ) {
		MovingBoundaryParabolicProblemImpl::registerType( );
	}

	void MovingBoundaryParabolicProblem::registerInstanceType( ) const {
		sImpl->registerInstanceType( );
	}

}
