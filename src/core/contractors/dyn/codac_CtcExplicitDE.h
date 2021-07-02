/*
 *  CtcODE class
 * ----------------------------------------------------------------------------
 * 	\date       2020
 *  \authors  	Victor Reyes, Gilles Trombettoni
 */

#ifndef __CODAC_CTCEXPLICITDE_H__
#define __CODAC_CTCEXPLICITDE_H__

#include "codac_CtcIntegration.h"
#include "codac_DynCtc.h"
#include <vector>

namespace codac
{

	class CtcExplicitDE : public DynCtc{

	public:

		CtcExplicitDE(std::vector<double> input_times, CtcIntegration ctc_integration);
		/*
		 * This method performs a contraction for the TubeVector x.
		 * Note that the timesteps between the Tubes of x must be identically the same.
		 */
		void contract(TubeVector& x, TubeVector& v);

	private:

		std::vector<double> input_times;
		CtcIntegration ctc_integration;
	};
}

#endif /* SRC_CORE_CONTRACTORS_CODAC_CTCODE_H_ */
