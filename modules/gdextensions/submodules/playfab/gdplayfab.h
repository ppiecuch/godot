
#include "core/reference.h"
#include "core/variant.h"

/// PlayFab singleton wrapper

class PlayFab : public Object {
	GDCLASS(PlayFab, Object);

public:
	PlayFab *get_instance();

	// https://docs.microsoft.com/en-us/rest/api/playfab/admin/?view=playfab-rest

	// Account Management
	// Authentication
	// Characters
	// Content
	// Custom Server Management
	// Matchmaking
	// Play Stream
	// Player Data Management
	// Player Item Management
	// Scheduled Task
	// Server-Side Cloud Script
	// Shared Group Data
	// Title-Wide Data Management

	PlayFab();
};
