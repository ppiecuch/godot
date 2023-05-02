#include "core/vector.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

namespace Filo {
	struct Utils {
		static bool Catenary(const Vector2 &p1, const Vector2 &p2, real_t l, Vector<Vector2> &points) {
			real_t r = p1.x;
			real_t s = p1.y;
			real_t u = p2.x;
			real_t v = p2.y;

			// swap values if p1 is to the right of p2:
			if (r > u){
				real_t temp = r;
				r = u;
				u = temp;
				temp = s;
				s = v;
				v = temp;
			}

			// find z:
			real_t z = 0.005f;
			real_t target = Math::sqrt(l*l-(v-s)*(v-s))/(u-r);
			while(Math::sinh(z)/z < target){
				z += 0.005;
			}

			if (z > 0.005) {
				real_t a = (u-r)/2/z;
				real_t p = (r+u-a*Math::log((l+v-s)/(l-v+s)))/2;
				real_t q = (v+s-l*(real_t)Math::cosh(z)/(real_t)Math::sinh(z))/2;

				int segments = points.size();
				real_t inc = (u-r)*(1/(segments-1));

				for (int i = 0; i < segments; ++i)
				{
					real_t x = r+inc*i;
					points.write[i] = Vector2(x,a*(real_t)Math::cosh((x-p)/a)+q);
				}
				return true;
			} else {
				return false;
			}
		}

		static bool Sinusoid(const Vector3 &origin, const Vector3 &direction, real_t l, unsigned frequency, Vector<Vector3> &points) {
			const Vector3 Forward = Vector3(0, 0, -1);

			Vector3 ndirection = direction;
			real_t magnitude = ndirection.length();
			if (magnitude > 1e-4) {

				ndirection /= magnitude;
				Vector3 ortho = ndirection.cross(Forward);

				int segments = points.size();
				real_t inc = magnitude / (segments - 1);

				real_t d = frequency * 4;
				real_t d2 = d * d;

				// analytic approx to amplitude from wave arc length.
				real_t amplitude = Math::sqrt(l * l / d2 - magnitude * magnitude/d2);

				if (Math::is_nan(amplitude)) {
					return false;
				}
				for (int i = 0; i < segments; ++i) {
					real_t pctg = i/(real_t)(segments - 1);
					points.write[i] = origin + ndirection * inc * i + ortho * Math::sin(pctg * Math_PI*2 * frequency) * amplitude;
				}
				return true;
			} else {
				return false;
			}
		}

		// Modulo operator that also follows intuition for negative arguments. That is , -1 mod 3 = 2, not -1.
		static real_t Mod(real_t a, real_t b) { return a - b * Math::floor(a / b); }

		static Vector3 Rotate2D(const Vector3 &v, real_t angle) {
			return Vector3(
					v.x * Math::cos(angle) - v.y * Math::sin(angle),
					v.x * Math::sin(angle) + v.y * Math::cos(angle),
					v.z
				);
		}
	};
} // namespace
