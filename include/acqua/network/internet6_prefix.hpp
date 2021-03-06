/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/basic_prefix_address.hpp>

namespace acqua { namespace network {

using internet6_prefix = basic_prefix_address<internet6_address>;

} }
