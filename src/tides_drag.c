/**
 * @file    tides_drag.c
 * @brief   Add drag forces due to slowly rotating tides raised on the primary body.
 * @author  Stanley A. Baronett <stanley.a.baronett@gmail.com>
 * 
 * @section     LICENSE
 * Copyright (c) 2015 Dan Tamayo, Hanno Rein
 *
 * This file is part of reboundx.
 *
 * reboundx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * reboundx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The section after the dollar signs gets built into the documentation by a script.  All lines must start with space * space like below.
 * Tables always must be preceded and followed by a blank line.  See http://docutils.sourceforge.net/docs/user/rst/quickstart.html for a primer on rst.
 * $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 *
 * $Tides$       // Effect category (must be the first non-blank line after dollar signs and between dollar signs to be detected by script).
 *
 * ======================= ===============================================
 * Authors                 S.A. Baronett
 * Implementation Paper    *In progress*
 * Based on                `Schroder & Smith 2008 <https://arxiv.org/abs/0801.4031>`_.
 * C Example               :ref:`c_example_tides_drag`.
 * Python Example          `TidesDrag.ipynb <https://github.com/dtamayo/reboundx/blob/master/ipython_examples/TidesDrag.ipynb>`_.
 * ======================= ===============================================
 *
 * ADD DESCRIPTION HERE
 * 
 * **Effect Parameters**
 * 
 * None
 *
 * **Particle Parameters**
 *
 * ============================ =========== ==================================================================
 * Field (C type)               Required    Description
 * ============================ =========== ==================================================================
 * PARAM (float)                Yes         DESCRIPTION
 * primary (int)                No          Set to 1 to specify the primary.  Defaults to treating particles[0] as primary if not set.
 * ============================ =========== ==================================================================
 * 
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "reboundx.h"

double R0 = 0.78;       // sun's physical radius in AU
double L0 = 869.5;      // solar luminosity
double Omega = 0;       // angular velocity of solar rotation
double lambda2 = 0.023; // depends on properties of convective envelope

void rebx_tides_drag(struct reb_simulation* const sim, struct rebx_force* const force, struct reb_particle* const particles, const int N){
    //FOR USE LATER W/ REGISTERED PARAMETERS
    // struct rebx_extras* const rebx = sim->extras; // to access add'l params
    // for (int i=0; i<N; i++){
    //     const double* migration_tau = rebx_get_param(rebx, particles[i].ap, "migration_tau");
    //     if (migration_tau != NULL){
    //         double tau = *migration_tau;
    //         particles[1].ax -= particles[1].vx/tau;
    //         particles[1].ay -= particles[1].vy/tau;
    //         particles[1].az -= particles[1].vz/tau;
    //     }
    //     else if (i == 1 && migration_tau == NULL){
    //         printf("NULL POINTER\n");
    //     }
    // }

    struct reb_orbit po = reb_tools_particle_to_orbit(sim->G, particles[1], particles[0]);

    const double dr = po.d;               // particle's radial distance
    const double vmag = po.v;             // particl's' relative velocity
    const double M0 = particles[0].m;     // primary's mass
    const double m = particles[1].m;      // particle's mass
    const double q = m/M0;                // mass ratio
    const double rratio = R0/dr;          // ratio of primary physical radius to orbital radius
    const double omega = po.n;            // angular velocity of orbiting particle
    const double t_f = cbrt(M0*R0*R0/L0); // convective friction time (Zahn 1989, Eq.15)
    
    // Equation (4) of Schroder & Smith (2008):
    // torque from retarded solar bulges for planar (i.e.
    // omega and Omega are parallel) and circular orbits
    const double torque = 6.*(lambda2/t_f)*q*q*M0*R0*R0*rratio*rratio*rratio*rratio*rratio*rratio*(Omega - omega);

    const double prefac = torque/m/dr/vmag; // consolidate prefactor for acceleration components

    // Apply torque to acceleration components
    particles[1].ax += prefac*particles[1].vx;
    particles[1].ay += prefac*particles[1].vy;
    particles[1].az += prefac*particles[1].vz;

    // Apply proportional (mass ratio, q) reverse torque to primary
    particles[0].ax -= q*prefac*particles[1].vx;
    particles[0].ay -= q*prefac*particles[1].vy;
    particles[0].az -= q*prefac*particles[1].vz;
}