/**************************************************************************/
/*  imath.cpp                                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "imath.h"

#include "core/math/math_funcs.h"
#include "core/os/memory.h"

int BitReverse(int i, int l) {
#ifdef _M_IX86
	__asm {
		mov edx,i
		xor eax,eax
		mov ecx,l

	ici:
		rcr edx,1
		rcl eax,1
		dec cx
		jnz ici
	}
#else
	int k = 0;
	for (int j = 0; j < l; j++) {
		k = (k << 1) + (i & 1);
		i >>= 1;
	}
	return k;
#endif
}

int Ror(int i, int p) {
#ifdef _M_IX86
	__asm {
		mov ecx,p
		mov eax,i
		ror eax,cl
		// retour dans eax
	}
#else
	p &= 0x3f;
	return (i >> p) + (i << (32 - p));
#endif
}

int Rol(int i, int p) {
#ifdef _M_IX86
	__asm {
		mov ecx,p
		mov eax,i
		rol eax,cl
		// retour dans eax
	}
#else
	p &= 0x3f;
	return (i << p) + (i >> (32 - p));
#endif
}

int Sign(int i) {
	if (i > 0) {
		return +1;
	} else if (i < 0) {
		return -1;
	} else {
		return 0;
	}
}

uint_t Abs(int i) {
	if (i < 0) {
		return -i;
	} else {
		return i;
	}
}

// Calcule le plus grand facteur commun entre a et b avec l'algorithme d'Euclide
uint_t pgfc(uint_t a, uint_t b) {
	while (b) {
		const int r = (a % b);
		a = b;
		b = r;
	}
	return a;
}

// Cette routine simplifie la fraction a/b
// par exemple 6/-8 ---> -3/4
// ...en O ( Log2( Max(a,b))^2 ) en pire cas
// a cause d'Euclide. Pas super rapide a cause
// des conversions de signe.
void SimplifyFraction(int &a, int &b) {
	int i, s = (a < 0) ^ (b < 0);
	a = Abs(a);
	b = Abs(b);
	while ((i = pgfc(a, b)) > 1) { // si le pgfc est 1, alors a et b sont relativ. premier.
		a /= i;
		b /= i;
	}
	if (s) {
		a = -a;
	}
}

// Exponentiation discrete en O(log(exposant))
uint_t ExpoDiscrete(uint_t a, uint_t p) {
	if (p == 0) {
		return 1;
	} else {
		uint_t i = 1, y = a;
		while (p) {
			if (p & 1) {
				i *= y;
			}
			y *= y;
			p >>= 1;
		}
		return i;
	}
}

// Exponentiation discrete en O(log(exposant))
// sauf que les multiplications intermediaires
// sont faites avec un modulo
uint_t ModExpoDiscrete(uint_t a, uint_t p, uint_t m) {
	if (p == 0) {
		return 1;
	} else {
		unsigned i = 1, y = a;
		while (p) {
			if (p & 1) {
				i = (i * y) % m;
			}
			y *= y;
			y %= m;
			p >>= 1;
		}
		return i;
	}
}

// Calcule le Plafond(Log2(i))
uint_t CompactLog2(uint_t i) {
	int j = 0;
	do {
		j++;
		i >>= 1;
	} while (i);
	return j;
}

// Calcule le Plafond(Log2(i))
// en O(Log(Log(32))==5, ce qui est a peu pres
// 3-4 fois plus rapide que CompactLog2
uint_t Log2(uint_t i) {
#if _M_IX86 > 200
	__asm {
	tester:

		mov ebx,i
		or  ebx,ebx
		jnz nonzero

	zero:

		xor eax,eax
		inc eax
		jmp sortie

	nonzero:

		bsr eax,ebx
		inc eax

	sortie:
	}
#else
	int n = 32;
	if ((i & 0xffff0000) == 0) {
		n -= 16;
		i <<= 16;
	}
	if ((i & 0xff000000) == 0) {
		n -= 8;
		i <<= 8;
	}
	if ((i & 0xf0000000) == 0) {
		n -= 4;
		i <<= 4;
	}
	if ((i & 0xc0000000) == 0) {
		n -= 2;
		i <<= 2;
	}
	if ((i & 0x80000000) == 0) {
		n--;
	}
	return n;
#endif
}

// Calcule le Plafond(Log10(i))
uint_t Log10(uint_t i) {
	int n = 0;
	do {
		n++;
		i /= 10;
	} while (i);
	return n;
}

// En passant, sur certaines machines, le sqrt(float)
// est BEAUCOUP plus rapide que cette routine (dix
// fois!!!). Et c'est probablement à cause de la mul-
// tiplication. Donc, n'utiliser que sur une machine
// moins bien montée qu'un SGI ou Pentium II, Pro,
// etc.
uint_t FloorSqrt(uint_t x) {
	uint_t y = 0;
	uint_t base = 32768;
	x++;
	while (base) {
		uint_t t = y + base;
		if ((t * t) < x) {
			y = t;
		}
		base >>= 1;
	}
	return y;
}

// Celle-ci est passablement plus rapide que
// FloorSqrt, mais elle ne calcule pas le
// plancher(Sqrt(i)) mais le round, ce qui en
// ceraines occasion peut etre un probleme,
// alors il faudrait alors utiliser FloorSqrt().
// Cette routine est attribuée a Jamie McCarthy
// (quoique ce n'est pas certain) d'un groupe Mac
uint_t Sqrt(uint_t i) {
	uint_t r = 0;
	uint_t m = 0x40000000;

	do {
		uint_t nr = r + m;
		if (nr <= i) {
			i -= nr;
			r = nr + m;
		}
		r >>= 1;
		m >>= 2;
	} while (m);

	if (i > r) {
		r++;
	}
	return r;
}

// Je trouve la version de stdlib un peu
// ennuyante du fait que l'on doit déclarer
// un struct. Deplus, pour etre certain que
// le code est bien écrit, je le refais moi-meme.
void DivMod(int a, int b, int &q, int &r) {
#if _M_IX86 > 200
	// Microsoft C v5.0 ne paire pas un div et un mod qui
	// se suivent (pour quelle raison? GNU C le fait, et
	// aussi le compilo d'intel... faque... )
	__asm {
		mov eax,a
		cdq
		mov ebx,b
		div ebx
						// write back
		mov ebx,q // puisque q est pointeur
		mov [ebx],eax
		mov ebx,r // puisque r est pointeur
		mov [ebx],edx
	}
#else
	q = a / b; // en esperant que le compilo les paire
	r = a % b;
#endif
}

// Version usigned: l'extension est differente
void UDivMod(uint_t a, uint_t b, uint_t &q, uint_t &r) {
#if _M_IX86 > 200
	__asm {
		mov eax,a
		xor edx,edx
		mov ebx,b
		div ebx
						// write back
		mov ebx,q // puisque q est pointeur
		mov [ebx],eax
		mov ebx,r // puisque r est pointeur
		mov [ebx],edx
	}
#else
	q = a / b;
	r = a % b;
#endif
}

// Ici, on a toutes les routines pour les fonctions
// de factorisation, facteurs, premiers, etc.

// Les nombres premiers sont encodés en differentiel dans
// cette table a cause de :
//
// 1) Il y en a 6541, à un 32 bit chaque, c'est gros!
// 2) L'algorithme de factorisation parcourre la table séquentiellement de toutes facons!
// 3) L'algo change si peu! (en fait, c'est assez subtil comme différence)
unsigned char dpremiers[6541] =
#include "dprimes.inl"
		;

// Produit la factorisation canonique (decomposition
// en facteurs premiers avec multiplicite)
//
// Un nombre a AU PLUS Log2(N) facteurs, donc le ta-
// bleau doit pouvoir contenir au moins 32 ints.
// Le pire temps de l'algorithme est O(Zeta(Sqrt(N)))
// Ce qui n'est pas trop mal, si on considère N (et
// non pas log(N), le nombre de bits!)
//
// Zeta(x) =~= x / ln x
void Factorise(uint_t i, uint_t facteurs[], int &factptr) {
	factptr = 0;
	if (i < 2) {
		return;
	}
	uint_t q, r, f = 2, p = 0;

	do {
		//  q = i / f;
		//  r = i % f;

		UDivMod(i, f, q, r);

		if (r == 0) {
			i = q;
			facteurs[factptr++] = f;
		} else {
			f += dpremiers[p++];
		}
	} while (f <= q);

	if (i > 1) {
		facteurs[factptr++] = i;
	}
}

// Détermine si un nombre est premier avec
// un test court pour environ 83-85 % des nombres
// et la factorisation à bras pour les nombres
// vraiment premiers ou composites dans les
// facteurs 2,3,...,19,23.
int Premier(uint_t i) {
	uint_t t = pgfc(i, FastDivTest);

	if ((t > 1) && (t != i)) { // i.e., relativement premier a FastDivTest, soit 223092870
		return false;
	} else {
		uint_t facteurs[32];
		int factptr;

		Factorise(i, facteurs, factptr);
		return factptr == 1;
	}
}

// Détermine si deux nombres sont relativement premiers
int RelativementPremier(uint_t a, uint_t b) {
	return pgfc(a, b) == 1;
}

// Retourne un entier relativement premier a A, tel
// que b mod a =~= sqrt(a), pratique pour les hashs
//
// L'algorithme est assez nul, mais c'est correct
// puisque la probabilite qu'il n'y ait pas de nom-
// bres relativement premiers a A autour de Sqrt(A)
// est assez faible. Pour un meme a, retourne tou-
// jours le meme b.
uint_t RelativementPremierSqrt(uint_t a) {
	uint_t t = Sqrt(a), i = 0;
	int c1, c2;
	do {
		i++;
		c1 = RelativementPremier(a, t + i);
		c2 = RelativementPremier(a, t - i);
	} while ((!c1) && (!c2));
	if (c1) {
		return t + i;
	} else {
		return t - i;
	}
}

// Retourne un entier relativement premier a A, tel
// que b mod a =~= Phi^-1 a, pratique pour les hashs.
uint_t RelativementPremierPhi(uint_t a) {
	uint_t t = (int)(InverseGoldenRatio * a + 0.5); // sqrt(a);
	uint_t i = 0;
	int c1, c2;
	do {
		i++;
		c1 = RelativementPremier(a, t + i);
		c2 = RelativementPremier(a, t - i);
	} while ((!c1) && (!c2));
	if (c1) {
		return t + i;
	} else {
		return t - i;
	}
}

// Calcule les facteurs propres de i, le plus efficacement possible
void FacteursPropres(uint_t i, uint_t **facteursPropres, int &factptr) {
	uint_t facteurs[32];
	uint_t puiss[32][32];
	int top[32];
	int regs[32];
	int primefactors;

	Factorise(i, facteurs, primefactors);

	int k = 0, l = 0;
	while (k < primefactors) {
		top[l] = 1;
		puiss[l][0] = 1;

		uint_t t = facteurs[k];
		while ((k < primefactors) && (t == facteurs[k])) {
			puiss[l][top[l]] = t;
			top[l]++;
			k++;
		}
		l++;
	}
	factptr = 1;
	for (k = 0; k < l; k++) {
		factptr *= top[k];
		for (int j = 1; j < top[k]; j++) {
			puiss[k][j] *= puiss[k][j - 1];
		}
	}
	for (int j = 0; j < l; j++) {
		regs[j] = 0;
	}
	(*facteursPropres) = (uint_t *)memalloc(factptr * sizeof(uint_t));
	for (k = 0; k < factptr; k++) {
		uint_t s = 1;

		for (int j = 0; j < l; j++) {
			s *= puiss[j][regs[j]];
		}
		(*facteursPropres)[k] = s;
		regs[0]++;
		int j = 0;
		while ((regs[j] == top[j]) && (j < l)) {
			regs[j] = 0;
			j++;
			regs[j]++;
		}
	}
}

// Calcule un élément primitif / géné-
// rateur sur un corps Zq. Comme l'al-
// go est probabiliste, il ne retourne
// pas toujours la meme valeur. Il
// change le randseed. Version L V
uint_t GenerateurStochastique(uint_t q) {
	uint_t facteurs[32];
	uint_t m[32]; // 10-12, en pire cas
	int f, i, j, ok;
	uint_t l;

	Factorise(q - 1, facteurs, f);

	// facteurs uniques
	i = 1, j = 1, l = facteurs[0];
	while (i < f) {
		if (facteurs[i] != l) {
			l = facteurs[j] = facteurs[i];
			j++;
		}
		i++;
	}
	f = j;
	for (i = 0; i < f; i++) {
		m[i] = (q - 1) / facteurs[i];
	}
	int k = 0;
	do {
		l = 0;
		while ((l == 0) && (k < 1000)) {
			k++;
			while ((j = (Math::rand() % q)) < 2)
				;
			for (l = 0, i = 0; i < f; i++) {
				l += (ModExpoDiscrete(j, m[i], q) == 1);
			}
		}
		ok = ModExpoDiscrete(j, q - 1, q) == 1;
	} while ((!ok) && (k < 1000));

	if (ok) {
		return j;
	} else {
		return 0xffffffff;
	}
}

// Celle-ci genere le plus petit generateur
// pour un corps mod q.
uint_t PlusPetitGenerateur(uint_t q) {
	uint_t facteurs[32];
	uint_t m[32]; // 10-12, en pire cas
	int f, i, j, ok;
	uint_t l, k;

	Factorise(q - 1, facteurs, f);

	// facteurs uniques
	i = 1, j = 1, l = facteurs[0];
	while (i < f) {
		if (facteurs[i] != l) {
			l = facteurs[j] = facteurs[i];
			j++;
		}
		i++;
	}
	f = j;
	for (i = 0; i < f; i++) {
		m[i] = (q - 1) / facteurs[i];
	}
	k = 1;
	do {
		l = 0;
		while ((l == 0) && (k < q)) {
			k++;
			for (l = 0, i = 0; i < f; i++) {
				l += (ModExpoDiscrete(k, m[i], q) == 1);
			}
		}
		ok = ModExpoDiscrete(k, q - 1, q) == 1;
	} while ((!ok) && (k < q));
	if ((ok) && (k < q)) {
		return k;
	} else {
		return 0xffffffff;
	}
}

// Ce programme prend un generateur, puis
// pond une n-ième racine de l'unite
uint_t NiemeRacineUniteStochastique(uint_t q, uint_t m) {
	if ((q - 1) % m) {
		return 0xffffffff;
	} else {
		uint_t g, k, c = 0;
		do {
			g = GenerateurStochastique(q);
			k = ModExpoDiscrete(g, (q - 1) / m, q);
			c++;
		} while (((g == 1) || (k == 1)) && (c < 10000));
		if (c < 10000) {
			return k;
		} else {
			return -1;
		}
	}
}

uint_t PlusPetiteNiemeRacineUnite(uint_t q, uint_t m) {
	if ((q - 1) % m) {
		return 0xffffffff;
	} else {
		return ModExpoDiscrete(PlusPetitGenerateur(q), (q - 1) / m, q);
	}
}
