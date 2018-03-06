/* ============================================================================
 *  tubex-lib - CtcDeriv class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#ifndef CtcDeriv_HEADER
#define CtcDeriv_HEADER

#include "tubex_Ctc.h"

namespace tubex
{
  /**
   * \brief CtcDeriv class.
   */
  class CtcDeriv : Ctc
  {
    public:

      CtcDeriv();
      bool contract(Tube& x, const Tube& v, const ibex::Interval& x0 = ibex::Interval::ALL_REALS);
      bool contractFwd(Tube& x, const Tube& v, const ibex::Interval& x0 = ibex::Interval::ALL_REALS);
      bool contractBwd(Tube& x, const Tube& v);

    protected:

  };
}

#endif