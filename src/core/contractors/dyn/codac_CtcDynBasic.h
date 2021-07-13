/*
 * codac_CtcDynBasic.h
 *
 *  Created on: Mar 6, 2020
 *      Author: Victor Reyes
 *      		Gilles Trombettoni
 */

#ifndef __CODAC_CTCDYNBASIC_H__
#define __CODAC_CTCDYNBASIC_H__

#include "codac_DynCtc.h"
#include "codac_Slice.h"
#include "ibex_Function.h"
#include "codac_CtcDeriv.h"
#include <vector>
#include <time.h>

namespace codac
{

	class CtcDynBasic : public DynCtc{

	public:

		CtcDynBasic(const TFnc& fnc, double prec = 0.);
		/*
		 * This method performs a contraction for the TubeVector x.
		 * Note that the timesteps between the Tubes of x must be identically the same.
		 */
		bool contract(std::vector<Slice*> x_slice, std::vector<Slice*> v_slice, TimePropag t_propa);
                void contract(std::vector<Domain*>& v_domains);
		/*
		 * ctc_fwd manages to make an evaluation of the current Slices in order to contract and update v
		 */
		void ctc_fwd(Slice &x, Slice &v, std::vector<Slice*> x_slice, std::vector<Slice*> v_slice, int pos);
		/*
		 *  used to obtain the current precision of the iterative method.
		 */
		double get_prec();
		/*
		 * changes the value of the precision
		 */
		void set_prec(double prec);

		bool get_reasoning_slice();

		void set_reasoning_slice(bool reasoning_slice = true);



	private:

		bool m_reasoning_slice = true;
		CtcDeriv ctc_deriv;
		const TFnc& fnc;
		double prec;
	};
}

#endif /* SRC_CORE_CONTRACTORS_CODAC_CTCDYNBASIC_H_ */

