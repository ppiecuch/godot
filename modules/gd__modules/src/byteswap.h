/* osc.h */

#ifndef BYTESWAP_H
#define BYTESWAP_H

#include "core/reference.h"
#include <cstdlib>
#include <iostream>

class Byteswap : public Reference {
  GDCLASS(Byteswap, Reference);

 protected:
  static void _bind_methods();

 public:
  Byteswap();

  float reverseFloat(float);
  int reverseInt(int);
};

#endif // BYTESWAP_H
