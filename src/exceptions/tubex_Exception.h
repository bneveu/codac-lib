/* ============================================================================
 *  tubex-lib - Exception class
 * ============================================================================
 *  Copyright : Copyright 2017 Simon Rohou
 *  License   : This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 *
 *  Author(s) : Simon Rohou
 *  Bug fixes : -
 *  Created   : 2015
 * ---------------------------------------------------------------------------- */

#ifndef __TUBEX_EXCEPTION_H__
#define __TUBEX_EXCEPTION_H__

#include <iostream>
#include <exception>
#include <string>
#include <sstream>

namespace tubex
{
  /**
   * \brief Exception abstract class.
   *
   * Thrown when necessary.
   */
  class Exception : public std::exception
  {
    public:

      Exception() {};
      Exception(const std::string& function_name, const std::string& custom_message);
      ~Exception() throw() {};

      virtual const char* what() const throw();

    protected:

      std::string m_what_msg;
  };

  std::ostream& operator<<(std::ostream& os, const Exception& e);
}

#endif