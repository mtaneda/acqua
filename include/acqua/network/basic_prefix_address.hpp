/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <iostream>
#include <boost/operators.hpp>

namespace acqua { namespace network {

template <typename T>
class basic_prefix_address
    : private boost::totally_ordered< basic_prefix_address<T> >
    , private boost::unit_steppable< basic_prefix_address<T> >
    , private boost::additive2< basic_prefix_address<T>, long int>
{
public:
    using address_type = T;
    using masklen_type = typename T::masklen_type;

public:
    basic_prefix_address();

    basic_prefix_address(basic_prefix_address const & rhs);

    basic_prefix_address(basic_prefix_address && rhs);

    explicit basic_prefix_address(address_type const & address, masklen_type masklen);

    void assign(address_type const & address, masklen_type masklen);

    masklen_type masklen() const
    {
        return masklen_;
    }

    address_type address() const
    {
        return address_;
    }

    address_type netmask() const;

    basic_prefix_address & operator=(basic_prefix_address const & rhs);

    basic_prefix_address & operator=(basic_prefix_address && rhs);

    basic_prefix_address & operator++();

    basic_prefix_address & operator--();

    basic_prefix_address & operator+=(long int num);

    basic_prefix_address & operator-=(long int num);

    template <typename T_>
    friend bool operator==(basic_prefix_address<T_> const & lhs, basic_prefix_address<T_> const & rhs);

    template <typename T_>
    friend bool operator<(basic_prefix_address<T_> const & lhs, basic_prefix_address<T_> const & rhs);

    template <typename T_, typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_prefix_address<T_> const & rhs);

    template <typename T_>
    friend std::size_t hash_value(basic_prefix_address<T_> const & rhs);

private:
    template <typename Ch, typename Tr>
    void to_string(std::basic_ostream<Ch, Tr> & os) const;

private:
    masklen_type masklen_;
    address_type address_;
};

} }

#include <acqua/network/impl/basic_prefix_address.ipp>