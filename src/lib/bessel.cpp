/* 
 *      Copyright (c) 2017 Robert Shaw
 *		This file is a part of Libecpint.
 *
 *      Permission is hereby granted, free of charge, to any person obtaining
 *      a copy of this software and associated documentation files (the
 *      "Software"), to deal in the Software without restriction, including
 *      without limitation the rights to use, copy, modify, merge, publish,
 *      distribute, sublicense, and/or sell copies of the Software, and to
 *      permit persons to whom the Software is furnished to do so, subject to
 *      the following conditions:
 *
 *      The above copyright notice and this permission notice shall be
 *      included in all copies or substantial portions of the Software.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *      EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *      NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *      LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *      OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *      WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "bessel.hpp"
#include "mathutil.hpp"
#include <cmath>
#include <iostream>

namespace libecpint {

	// Constructor
	BesselFunction::BesselFunction() {}
	BesselFunction::BesselFunction(int _lMax, int _N, int _order, const double accuracy)
	{
		init(_lMax, _N, _order, accuracy);
	}

	void BesselFunction::init(int _lMax, int _N, int _order, const double accuracy) {
		// Check parameters
		lMax = _lMax > -1 ? _lMax : 0;
		N = _N > 0 ? _N : 1;
		order = _order > 0 ? _order : 1;
	
		// Allocate arrays
		K = new double*[N+1];
		for (int i = 0; i < N+1; i++) K[i] = new double[lMax + TAYLOR_CUT + 1];
		C = new double[lMax+TAYLOR_CUT];
	
		// Tabulate values
		tabulate(accuracy);
	}

	BesselFunction::~BesselFunction() {
		for (int i = 0; i < N+1; i++) delete[](K[i]);
		delete[](K);
		delete[](C);
	}

	// Tabulate the bessel function values
	int BesselFunction::tabulate(const double accuracy) {
		int retval = 0; // 0 for success, -1 for not converged
		// Series expansion for bessel function, K, is given by:
		// K_l(z) ~ z^l sum_{j=0 to infty} F_j(z) / (2j + 2l + 1)!! 
		// where F_j(z) = e^(-z) * (z^2/2)^j / j!
		int lmax = lMax + TAYLOR_CUT;
	
		double F[order + 1]; // F_j above
	
		K[0][0] = 1.0;
		double z, z2; // z and z^2 / 2
		double ratio; // F_j(z) / (2j+1)!!
		for (int i = 0; i <= N; i++) {
			// Calculate K(z) at equally spaced points z = 16/N to 16
			z = i / (N/16.0);
			z2 = z * z / 2.0;
		
			F[0] = exp(-z);
			ratio = F[0] / DFAC[0];
			K[i][0] = ratio;
		
			// Series expansion for K_0(z)
			int l = order;
			int j;
			for (j = 1; j <= l; j++) {
			
				if (ratio < accuracy) {
					// Reached convergence
					break;
				} 
			
				F[j] = F[j-1] * z2 / ((double)j);
				ratio = F[j] / DFAC[2*j+1];
				K[i][0] += ratio;
			}
			//if ( ratio > accuracy ) { retval = -1; break; } // Not converged

			// Calculate K_l from K_0
			z2 = z;
			for (l=1; l<=lmax; l++) {
				ratio = 0;
				for (int m=0; m < j; m++) ratio += F[m]/DFAC[2*l + 2*m + 1]; 
				K[i][l] = z2 * ratio;
				z2 *= z; 
			}
	
		}
	
		// Determine coefficients for derivative recurrence
		for (int i = 1; i<lmax; i++) C[i] = i/(2.0*i + 1.0);
	
		return retval;
	}	

	// Calculate modified spherical Bessel function K_l(z), weighted with an exponential factor e^(-z)
	// for l = 0 to lMax. This restricts K(z) to the interval [0,1].
	void BesselFunction::calculate(const double z, int maxL, std::vector<double> &values) {
		if (lMax < maxL) {
			std::cout << "Asked for " << maxL << " but only initialised to maximum L = " << lMax << "\n";
			maxL = lMax;
		}
		values.assign(maxL + 1, 0.0);
	
		// Set K_0(z) = 1.0, and K_l(z) = 0.0 (for l != 0) if z <= 0
		if (z <= 0) values[0] = 1.0;
		// Zeroth order case
		// K_l(z) ~ (1-z)*z^l / (2l + 1)!!
		else if (z < SMALL) { 
			values[0] = 1.0 - z;
			for (int l = 1; l <= maxL; l++) values[l] = values[l-1]*z/(2.0*l+1.0);
		} 
		// Large z case
		// K_l(z) ~ R_l(-z)/(2z)
		// where R_l(z) = sum_{k=0 to l} T_l,k(z)
		// where T_l,k(z) = (l+k)!/[k!(l-k)!] * (2z)^{-k}
		else if (z > 16.0) {
			values[0] = 0.5/z;
			for (int l = 1; l <= maxL; l++) {
				values[l] = values[0];
				double Rl = 1.0;
				double Tlk = 1.0;
				double cof = 1.0;
				for (int k = 1; k <= l; k++) {
					cof = (l-k+1)*(l+k)/((double)k);
					Tlk *= - cof * values[0];
					Rl += Tlk;
				}
				values[l] *= Rl;
			}
		} 
		// SMALL < z < 16 
		// Use Taylor series around pretabulated values in class
		// 5 terms is usually sufficient for machine accuracy
		else {
			int maxLambda = maxL + TAYLOR_CUT;
			double scale = N/16.0;
		
			// Index of abscissa z in table
			int index = floor(z * scale + 0.5);
			double dz = z - index/scale; // z - z0
		
			if (fabs(dz) < 1e-12) { // z is one of the tabulated points
				for (int l = 0; l <= maxL; l++) values[l] = K[index][l];
			} else {
				// Determine the necessary derivatives from
				// K_l^(n+1) = C_l K_(l-1)^(n) + (C_l + 1/(2l+1))K_(l+1)^(n) - K_l^(n)
				double dK[TAYLOR_CUT+1][maxLambda];
		
				// Copy K values into dK
				for (int l = 0; l < maxLambda; l++)
					dK[0][l] = K[index][l];
			
				// Then the rest
				for (int n = 1; n < TAYLOR_CUT+1; n++) { 
					dK[n][0] = dK[n-1][1] - dK[n-1][0];
					for (int l = 1; l <= maxLambda - n; l++) 
						dK[n][l] = C[l]*dK[n-1][l-1] + (C[l] + 1.0/(2.0*l + 1.0))*dK[n-1][l+1] - dK[n-1][l];
				}
			
		
				// Calculate (dz)^n/n! terms just once
				double dzn[TAYLOR_CUT+1];
				dzn[0] = 1.0;
				for (int n = 1; n < TAYLOR_CUT + 1; n++)
					dzn[n] = dzn[n-1] * dz / ((double) n);
		
				// Now tabulate the values through Taylor seris
				// K(z) ~ sum_{n=0 to 5} K^(n)(z0)(z-z0)^n / n!
				for (int l = 0; l <= maxL; l++) {
					values[l] = 0.0;
					for (int n = 0; n < TAYLOR_CUT+1; n++)
						values[l] += dzn[n] * dK[n][l]; 
				}
			}
		}
	}
}
