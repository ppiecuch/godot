#ifndef FEM_MAJOR_TYPES_HPP
#define FEM_MAJOR_TYPES_HPP

#include "arr_size.hpp"
#include "data.hpp"
#include "read.hpp"
#include "variant.hpp"
#include "write.hpp"

namespace fem { namespace major_types {

  using fem::arr;
  using fem::arr_index;
  using fem::arr_cref;
  using fem::arr_ref;
  using fem::arr_size;
  using fem::arr_1d;
  using fem::arr_2d;
  using fem::arr_3d;
  using fem::common_read;
  using fem::common_variant;
  using fem::common_write;
  using fem::datum;
  using fem::dimension;
  using fem::dim1;
  using fem::equivalence;
  using fem::local_equivalences;
  using fem::read_loop;
  using fem::save_equivalences;
  using fem::star;
  using fem::str_arr_cref;
  using fem::str_arr_ref;
  using fem::str_cref;
  using fem::str_index;
  using fem::str_ref;
  using fem::values;
  using fem::write_loop;

}} // fem::major_types

#endif // GUARD
