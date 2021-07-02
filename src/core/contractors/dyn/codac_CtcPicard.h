/** 
 *  CtcPicard class
 * ----------------------------------------------------------------------------
 *  \date       2018
 *  \author     Simon Rohou
 *  \copyright  Copyright 2020 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#ifndef __CODAC_CTCPICARD_H__
#define __CODAC_CTCPICARD_H__

#include "codac_DynCtc.h"
#include "codac_TFnc.h"
#include "codac_Slice.h"

namespace codac
{
  /**
   * \brief CtcPicard class.
   *
   * \todo Restricted tdomain feature?
   */
  class CtcPicard : public DynCtc
  {
    public:

      CtcPicard(float delta = 1.1);

      void contract(std::vector<Domain*>& v_domains);
      
      void contract(const TFnc& f,
                    Tube& x,
                    TimePropag t_propa = TimePropag::FORWARD | TimePropag::BACKWARD);
      void contract(const TFnc& f,
                    TubeVector& x,
                    TimePropag t_propa = TimePropag::FORWARD | TimePropag::BACKWARD);
      int picard_iterations() const;

      /* the Picard algorithm for the kth slices of the tubeVector tubr */
      void contract_picard_slice(const TFnc& f,
                               TubeVector& tube,
                               int k,
			       TimePropag t_propa);
      void  set_picard_subslices (int nsubslices);


    protected:
      void contract_picard_tubeslice(const TFnc& f,
                               TubeVector& tube,
                               int & k,
			       TimePropag t_propa);
      

      void contract_kth_slices(const TFnc& f,
                               TubeVector& tube,
                               int k,
                               TimePropag t_propa);
      void guess_kth_slices_envelope(const TFnc& f,
                               TubeVector& tube,
                               int k,
                               TimePropag t_propa);

      float m_delta;
      int m_picard_iterations = 0;
      int m_picard_subslices=500;
      double m_picard_minimum_domainsize=1.e-20;
  };
}

#endif
