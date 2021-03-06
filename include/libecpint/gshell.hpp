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

#ifndef GSHELL_HEAD
#define GSHELL_HEAD

#include <vector>

namespace libecpint {

	/** 
	  * \struct GaussianShell
	  * \brief Lightweight container for contracted shell of Gaussian basis functions
	  */
	struct GaussianShell {
		std::vector<double> exps; ///< Exponents of primitives
		std::vector<double> coeffs; ///< Coefficients in contraction of primitives
		
		/// Pointer to xyz coordinates of basis function (pointer so as to update if geometry changes)
		double* centerVec; 
		
		int l; ///< Angular momentum of shell
	
		/** 
		  * Constructs a GaussianShell
		  * @param A - xyz coordinates of shell
		  * @param l - angular momentum of shell
		  */ 
		GaussianShell(double* A, int l);
		
		/**
		  * Adds a Gaussian primitive to the shell
		  * @param exp - the exponent
		  * @param c - the contraction coefficient
		  */
		void addPrim(double exp, double c);
		
		/// @return the number of primitives
		int nprimitive() const { return exps.size(); }
		
		/// @return the number of cartesian basis functions in a shell with this angular momentum 
		int ncartesian() const { return ((l+1)*(l+2))/2; }
		
		/// @return the xyz coordinates of the shell
		double* center() const { return centerVec; };
		
		/**
		  * @param i - the index of the primitive of interest
		  * @return the exponent of the ith primitive
		  */ 
		double exp(int i) const { return exps[i]; }
		
		/**
		  * @param i - the index of the primitive of interest
		  * @return the contraction coefficient of the ith primitive
		  */ 
		double coef(int i) const { return coeffs[i]; }
		
		/// @return the angular momentum of the shell
		int am() const { return l; }
	};

	/**
	* \class ShellPairData
	* \brief Stores the (shifted, for derivs) angular momenta, number of cartesians in a shell pair, and shifted centers
	*/
	struct ShellPairData {
		int LA; 		///< Angular momentum of shellA, shifted if deriv > 0
		int LB;			///< Angular momentum of shellB, shifted if deriv > 0
		int maxLBasis;	///< Maximum angular momentum in the orbital basis
		int ncartA;		///< No. of cartesian BFs in shellA
		int ncartB;		///< No. of cartesian BFs in shellB	
		double A[3];	///< xyz coords of shellA relative to ECP
		double B[3];	///< xyz coords of shellB relative to ECP
		double A2;		///< Square of distance from ECP to shellA
		double Am; 		///< Magnitude of distance from ECP to shellA
		double B2;		///< Square of distance from ECP to shellB
		double Bm;		///< Magnitude of distance from ECP to shellB
		double RAB2; 	///< Square of distance between shells A and B
		double RABm;	///< Magnitude of distance between shells A and B
	};

}

#endif