// Copyright 2015 by Martin Moene
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// array_view2d is inspired on https://github.com/rhysd/array_view by Linda_pp.

#ifndef NOSTD_ARRAY_VIEW2D_OUTPUT_H_INCLUDED
#define NOSTD_ARRAY_VIEW2D_OUTPUT_H_INCLUDED

#include <ostream>

namespace nostd {

template< typename T >
std::ostream & operator<<(std::ostream & os, array_view2d<T> const & av)
{
    os << "{ ";

    if ( ! av.empty() )
    {
        typename array_view2d<T>::const_iterator pos = av.begin();
        typename array_view2d<T>::const_iterator const end = av.end();
        
        while ( true )
        {
            os << *pos;
            if ( ++pos != end ) os << ", ";
            else                break;
        }
    }
    return os << " }";
}

} // namespace nostd

#endif // NOSTD_ARRAY_VIEW2D_OUTPUT_H_INCLUDED

// End of file
