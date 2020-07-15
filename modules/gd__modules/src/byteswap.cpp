#include "byteswap.h"


Byteswap::Byteswap() {
    std::cout << "Byteswapper instantiated!" << std::endl;
}

float Byteswap::reverseFloat(float inFloat) {
    float retVal;
    char *floatToConvert = ( char* ) & inFloat;
    char *returnFloat = ( char* ) & retVal;

    // swap the bytes into a temporary buffer
    returnFloat[0] = floatToConvert[3];
    returnFloat[1] = floatToConvert[2];
    returnFloat[2] = floatToConvert[1];
    returnFloat[3] = floatToConvert[0];
    
    return retVal;

}

int Byteswap::reverseInt( int inInt )
{
   int retVal;
   char *intToConvert = ( char* ) & inInt;
   char *returnInt = ( char* ) & retVal;

   // swap the bytes into a temporary buffer
   returnInt[0] = intToConvert[3];
   returnInt[1] = intToConvert[2];
   returnInt[2] = intToConvert[1];
   returnInt[3] = intToConvert[0];

   return retVal;
};


void Byteswap::_bind_methods() {
    ClassDB::bind_method("reverseFloat", &Byteswap::reverseFloat);
    ClassDB::bind_method("reverseInt", &Byteswap::reverseInt);
}
