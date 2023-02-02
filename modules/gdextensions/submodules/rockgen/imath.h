#ifndef IMATHS_H
#define IMATHS_H

unsigned int CompactLog2(unsigned int i);
unsigned int Log2(unsigned int i);
unsigned int Log10(unsigned int i);
unsigned int FloorSqrt(unsigned int x);
unsigned int Sqrt(unsigned int i);
unsigned int iabs(int i);
int iSign(int i);

int Ror(int i,int p);
int Rol(int i,int p);

int BitReverse(int i, int l);

void DivMod(int a, int b, int & q, int & r);
void UDivMod(unsigned int a, unsigned int b, unsigned int & q, unsigned int & r);


const unsigned int FastDivTest = 223092870; // 2*3*5*7*11*13*17*19*23
const float GoldenRatio        = 1.6180339887; // (1+Sqrt(5)) / 2
const float InverseGoldenRatio = 0.6180339887;

unsigned int pgfc(unsigned int a, unsigned int b);
void SimplifyFraction(int &a, int &b);
unsigned int ExpoDiscrete(unsigned int a, unsigned int p);
unsigned int ModExpoDiscrete(unsigned int a, unsigned int p, unsigned int m);
void Factorise( unsigned int i, unsigned int facteurs[], int &factptr);
void FacteursPropres(unsigned int i, unsigned int **facteursPropres, int &factptr);
int Premier(unsigned int i); 
int RelativementPremier(unsigned int a, unsigned int b);

unsigned int RelativementPremierSqrt(unsigned a);
unsigned int RelativementPremierPhi(unsigned a);

unsigned int GenerateurStochastique(unsigned int q);
unsigned int PlusPetitGenerateur(unsigned int q);
unsigned int NiemeRacineUniteStochastique(unsigned int q, unsigned int m);
unsigned int PlusPetiteNiemeRacineUnite(unsigned int q, unsigned int m);

#endif // IMATHS_H
