#include "polyfonts.h"

#include "core/color.h"

class PolyGodot : public PolyFont {
	float width;
	Color color;
public:
	virtual void beginStringDraw() const;
	virtual void doneStringDraw() const;
	virtual bool polyDrawElements(int mode, coord_vector &indices) const;
	virtual void setWidth(float w);
	virtual void setColor(float R, float G, float B, float A);

	PolyGodot();
	PolyGodot(const pffont *f);
	~PolyGodot();
};
