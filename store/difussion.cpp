/* This file is part of the Palabos library.
 * Copyright (C) 2009 Jonas Latt
 * E-mail contact: jonas@lbmethod.org
 * Copyright (C) 2010 Arne Babenhauserheide
 * Contact: [draketo.de]
 * The most recent release of Palabos can be downloaded at 
 * [www.lbmethod.org]
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see [www.gnu.org].
*/

/** \file
 * Simulation of simple diffusion. 
 * It is adapted from the RayleighTaylor simulation. 
 **/

#include "palabos3D.h"
#include "palabos3D.hh"
#include <cstdlib>
#include <iostream>

using namespace plb;
using namespace std;

// Use double-precision arithmetics
typedef double T;
// Use a grid which additionally to the f's stores two variables for
//   the external force term.
#define DESCRIPTOR descriptors::ForcedShanChenD3Q19Descriptor

/// Initial condition: one fluid in center, the other one around it.
/** This functional is going to be used as an argument to the function "applyIndexed",
 *  to setup the initial condition. For efficiency reasons, this approach should
 *  always be preferred over explicit space loops in end-user codes.
 */
template<typename T, template<typename U> class Descriptor>
class TwoFluidInitializer : public OneCellIndexedFunctional3D<T,Descriptor> {
public:
  TwoFluidInitializer(plint nx_, plint ny_, plint nz_, bool topLayer_, T force_)
    : nx(nx_), 
      ny(ny_),
      nz(nz_), 
      topLayer(topLayer_),
      force(force_)
    { }
    TwoFluidInitializer<T,Descriptor>* clone() const {
        return new TwoFluidInitializer<T,Descriptor>(*this);
    }
    virtual void execute(plint iX, plint iY, plint iZ, 
                         Cell<T,Descriptor>& cell) const {
        T densityFluctuations = 1.e-2;
        T almostNoFluid       = 1.e-1;
        Array<T,2> zeroVelocity (0.,0.);

        T rho = (T)1;
	
	plint radiusSquared = pow(nx / 5, 2);

        // Reduce the density where the fluid should not be. 
	T distanceFromCenter = pow(iX - nx/2, 2) + pow(iY - ny/2, 2) + pow(iZ - nz/2, 2);
        if ( (topLayer && distanceFromCenter > radiusSquared) || (!topLayer && distanceFromCenter <= radiusSquared) ) {
            rho = almostNoFluid;
        }

        iniCellAtEquilibrium(cell, rho, zeroVelocity);
    }
private:
    plint nx;
    plint ny;
    plint nz;
    bool topLayer;
    T force;
};

void rayleighTaylorSetup( MultiBlockLattice3D<T, DESCRIPTOR>& innerFluid,
                          MultiBlockLattice3D<T, DESCRIPTOR>& outerFluid,
                          T rho0, T rho1,
                          T force )
{
    // The setup is: heavy fluid in the center, light around it. 
    int nx = innerFluid.getNx();
    int ny = innerFluid.getNy();
    int nz = innerFluid.getNz();
       
    // Initialize center fluid.
    applyIndexed(innerFluid, Box3D(0, nx-1, 0, ny-1, 0, nz-1),
                 new TwoFluidInitializer<T,DESCRIPTOR>(nx, ny, nz, true, force) );
    // Initialize outer fluid.
    applyIndexed(outerFluid, Box3D(0, nx-1, 0, ny-1, 0, nz-1),
                 new TwoFluidInitializer<T,DESCRIPTOR>(nx, ny, nz, false, T()) );

    outerFluid.initialize();
    innerFluid.initialize();
}

void writeGifs(MultiBlockLattice3D<T, DESCRIPTOR>& innerFluid,
               MultiBlockLattice3D<T, DESCRIPTOR>& outerFluid, int iT)
{
    const plint imSize = 600;
    const plint nx = innerFluid.getNx();
    const plint ny = innerFluid.getNy();
    const plint nz = innerFluid.getNz();
    Box3D slice(0, nx-1, 0, ny-1, nz/2, nz/2);

    ImageWriter<T> imageWriter("leeloo.map");
    imageWriter.writeScaledGif(createFileName("rho_heavy_", iT, 6),
                               *computeDensity(innerFluid, slice), imSize,imSize);
    imageWriter.writeScaledGif(createFileName("rho_light_", iT, 6),
                               *computeDensity(outerFluid, slice), imSize,imSize);
}

void writeVTK(MultiBlockLattice3D<T,DESCRIPTOR>& lattice,
	      plint iter)
{
  double dx = 0.002; // 1/N
  double dt = 0.00004; // dx2 -> [www.lbmethod.org]
  VtkImageOutput3D<double> vtkOut(createFileName("vtk", iter, 6), dx);
  vtkOut.writeData<float>(*computeVelocityNorm(lattice), "velocityNorm", dx/dt);
  vtkOut.writeData<float>(*computeDensity(lattice), "density", 1.);
  //vtkOut.writeData<float>(*subtract(*computeDensity(lattice),(T)1.), "density", 1.);
  vtkOut.writeData<3,float>(*computeVelocity(lattice), "velocity", dx/dt);
}


int main(int argc, char *argv[])
{
    plbInit(&argc, &argv);
    global::directories().setOutputDir("./tmp/");
    
    const T omega1 = 1.0;
    const T omega2 = 1.2;
    const int nx   = 50;
    const int ny   = 50;
    const int nz   = 50;
    const T G      = 0.32;
    T force        = 0.;//1.e-3;
    const int maxIter  = 10000;
    const int saveIter = 5;
    const int vtkIter = 100;
    const int statIter = 50;

    // Use regularized BGK dynamics to improve numerical stability (but note that
    //   BGK dynamics works well too).
    MultiBlockLattice3D<T, DESCRIPTOR> innerFluid (
            nx,ny,nz, new ExternalMomentRegularizedBGKdynamics<T, DESCRIPTOR>(omega1) );
    MultiBlockLattice3D<T, DESCRIPTOR> outerFluid (
            nx,ny,nz, new ExternalMomentRegularizedBGKdynamics<T, DESCRIPTOR>(omega2) );

    // Make all directions periodic.
    innerFluid.periodicity().toggle(0, true);
    innerFluid.periodicity().toggle(1, true);
    innerFluid.periodicity().toggle(2, true); 
    outerFluid.periodicity().toggle(0, true);
    outerFluid.periodicity().toggle(1, true);
    outerFluid.periodicity().toggle(2, true);

    T rho1 = 1.; // Fictitious density experienced by the partner fluid on a Bounce-Back node.
    T rho0 = 0.; // Fictitious density experienced by the partner fluid on a Bounce-Back node.
    
    // Store a pointer to all lattices (two in the present application) in a vector to
    //   create the Shan/Chen coupling therm. The heavy fluid being at the first place
    //   in the vector, the coupling term is going to be executed at the end of the call
    //   to collideAndStream() or stream() for the heavy fluid.
    vector<MultiBlockLattice3D<T, DESCRIPTOR>* > blockLattices;
    blockLattices.push_back(&innerFluid);
    blockLattices.push_back(&outerFluid);
    
    // The argument "constOmegaValues" to the Shan/Chen processor is optional,
    //   and is used for efficiency reasons only. It tells the data processor
    //   that the relaxation times are constant, and that their inverse must be
    //   computed only once.
    std::vector<T> constOmegaValues;
    constOmegaValues.push_back(omega1);
    constOmegaValues.push_back(omega2);
    plint processorLevel = 1;
    integrateProcessingFunctional (
				   new ShanChenMultiComponentProcessor3D<T,DESCRIPTOR>(G),//,constOmegaValues),
            Box3D(0,nx-1,1,ny-2,0,nz-1),
            blockLattices, processorLevel );

    rayleighTaylorSetup(innerFluid, outerFluid, rho0, rho1, force);
	
    pcout << "Starting simulation" << endl;
    // Main loop over time iterations.
    for (int iT=0; iT<maxIter; ++iT) {
        if (iT%saveIter==0) {
            writeGifs(innerFluid, outerFluid, iT);
        }
        if (iT%vtkIter==0) {
            writeVTK(innerFluid, iT);
        }

        // Time iteration for the light fluid.
        outerFluid.collideAndStream();
        // Time iteration for the heavy fluid must come after the light fluid,
        //   because the coupling is executed here. You should understand this as follows.
        //   The effect of the coupling is to compute the interaction force between
        //   species, and to precompute density and momentum for each species. This must
        //   be executed *before* collide-and-streaming the fluids, because the collision
        //   step needs to access all these values. In the present case, it is done after
        //   both collide-and-stream step, which means, before the collide-and-stream of
        //   the next iteration (it's the same if you are before or after; the important
        //   point is not to be between the two collide-and-streams of the light and heavy
        //   fluid. As for the initial condition, the coupling is initially performed once
        //   during the function call to innerFluid.initialize().
        innerFluid.collideAndStream();

        if (iT%statIter==0) {
            pcout << "Average energy fluid one = "
                  << getStoredAverageEnergy<T>(innerFluid);
            pcout << ", average energy fluid two = "
                  << getStoredAverageEnergy<T>(outerFluid) << endl;
        }
    }
}