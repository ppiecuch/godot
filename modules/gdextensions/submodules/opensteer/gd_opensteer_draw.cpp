#include "OpenSteer/Vec3.h"
#include "OpenSteer/LocalSpace.h"

#include <sstream>

namespace OpenSteer {
	// OpenSteer system/drawing interface
	namespace Draw {
		void drawCameraLookAt (const Vec3& cameraPosition, const Vec3& pointToLookAt, const Vec3& up)
		{
		}
		void drawLine (const Vec3& startPoint, const Vec3& endPoint, const Vec3& color)
		{
		}
		void drawLine (const Vec3& startPoint, const Vec3& endPoint, const Vec3& color, const float alpha)
		{
		}
		void drawWideLine (const Vec3& startPoint, const Vec3& endPoint, const Vec3& color, const float width)
		{
		}
		void drawLineGrid (int hseg, int vseg, const Vec3& center, const Vec3& color)
		{
		}
		void drawCircle (const float radius, const Vec3& axis, const Vec3& center, const Vec3& color, const int segments, const bool filled, const bool in3d)
		{
		}
        void drawCircle (const float radius, const Vec3& center, const Vec3& color, const int segments, const bool filled)
		{
		}
		void drawQuadrangle (const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4, const Vec3& color)
		{
		}
		void drawCheckerboardGrid (const float size, const int subsquares, const Vec3& center, const Vec3& color1, const Vec3& color2)
		{
		}
		void drawBox (const AbstractLocalSpace& localSpace, const Vec3& size, const Vec3& color)
		{
		}
        void drawCircle (const AbstractLocalSpace& localSpace, const Vec3& color, float radius, bool filled, float up_offset)
		{
		}
        void drawTextAt2dLocation(const std::ostringstream& text, const Vec3& position, const Vec3& color)
		{
		}
        void drawTextAt2dLocation(const char *text, const Vec3& position, const Vec3& color)
		{
		}
        void drawTextAt3dLocation(const std::ostringstream& text, const Vec3& position, const Vec3& color)
		{
		}
        void drawTextAt3dLocation(const char *text, const Vec3& position, const Vec3& color)
		{
		}
	} // namespace Draw
} // namespace OpenSteer
