#include "gd_bitblit.h"

BitBlit *BitBlit::singleton = nullptr;

BitBlit::BitBlit() {
	singleton = this;
}

BitBlit::~BitBlit() {
	singleton = nullptr;
}
