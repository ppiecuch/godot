
#include "core/reference.h"
#include "core/variant.h"

/// PlayFab singleton wrapper

class PlayFab : public Object {
	GDCLASS(PlayFab, Object);

public:
	PlayFab *get_instance();

	PlayFab();
};
