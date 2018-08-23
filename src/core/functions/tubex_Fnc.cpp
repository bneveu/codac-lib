/* ============================================================================
 *  tubex-lib - Fnc class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#include "tubex_Fnc.h"
#include "tubex_Slice.h"
#include "tubex_TubeVector.h"
#include "tubex_Tube.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  Fnc::Fnc()
  {

  }

  Fnc::Fnc(int n, int m) : m_nb_vars(n), m_img_dim(m)
  {

  }

  Fnc::~Fnc()
  {

  }
  
  const Fnc& Fnc::operator=(const Fnc& f)
  {
    m_nb_vars = f.m_nb_vars;
    m_img_dim = f.m_img_dim;
  }

  int Fnc::nb_vars() const
  {
    return m_nb_vars;
  }

  int Fnc::image_dim() const
  {
    return m_img_dim;
  }

  const Tube Fnc::eval(const TubeVector& x) const
  {
    // todo: optimize this?
    return eval_vector(x)[0];
  }

  const TubeVector Fnc::eval_vector(const TubeVector& x) const
  {
    // todo: check dim x regarding f. f.imgdim can be of 0 and then x 1 in order to keep slicing pattern
    TubeVector y(x); // keeping x's slicing
    y.resize(image_dim());

    if(x.is_empty())
    {
      y.set_empty();
      return y;
    }

    IntervalVector result(y.size());
    
    Slice **v_y_slices = new Slice*[y.size()];
    for(int i = 0 ; i < y.size() ; i++)
      v_y_slices[i] = y[i].get_first_slice();

    while(v_y_slices[0] != NULL)
    {
      result = eval_vector(v_y_slices[0]->domain(), x);
      for(int i = 0 ; i < y.size() ; i++)
        v_y_slices[i]->set_envelope(result[i]);

      result = eval_vector(v_y_slices[0]->domain().lb(), x);
      for(int i = 0 ; i < y.size() ; i++)
        v_y_slices[i]->set_input_gate(result[i]);
      
      for(int i = 0 ; i < y.size() ; i++)
        v_y_slices[i] = v_y_slices[i]->next_slice();
    }
    
    for(int i = 0 ; i < y.size() ; i++)
      v_y_slices[i] = y[i].get_last_slice();

      result = eval_vector(v_y_slices[0]->domain().ub(), x);
    for(int i = 0 ; i < y.size() ; i++)
      v_y_slices[i]->set_output_gate(result[i]);

    delete[] v_y_slices;
    return y;
  }
}