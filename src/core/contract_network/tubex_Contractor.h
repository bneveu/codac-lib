/** 
 *  \file
 *  Contractor class
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Simon Rohou
 *  \copyright  Copyright 2020 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#ifndef __TUBEX_CONTRACTOR_H__
#define __TUBEX_CONTRACTOR_H__

#include <functional>
#include "ibex_Ctc.h"
#include "tubex_Ctc.h"
#include "tubex_Domain.h"
#include "tubex_ContractorNetwork.h"

namespace ibex
{
  class Ctc;
}

namespace tubex
{
  enum class ContractorType { COMPONENT, EQUALITY, IBEX, TUBEX };

  class Domain;
  class DomainSingleton;
  class ContractorNetwork;
  class Ctc;

  class Contractor
  {
    public:

      Contractor(ContractorType type);
      Contractor(ibex::Ctc& ctc);
      Contractor(tubex::Ctc& ctc);
      Contractor(const Contractor& ac);
      ~Contractor();

      ContractorType type() const;

      bool is_active() const;
      void set_active(bool active);

      std::vector<Domain*>& domains();

      bool operator==(const Contractor& x) const;
      bool comes_from(const Contractor& x) const;

      void contract();

      const std::string& name();
      void set_name(const std::string& name);


    protected:

      const ContractorType m_type;
      double m_active = true;

      union
      {
        std::reference_wrapper<ibex::Ctc> m_ibex_ctc;
        std::reference_wrapper<tubex::Ctc> m_tubex_ctc;
      };

      std::vector<Domain*> m_domains;

      std::string m_name = "?";
  };
}

#endif