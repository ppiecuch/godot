/*************************************************************************/
/*  PlayFabSettings.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/* Auto-generated with SDKGenerator (don't manually edit) */

#ifndef PLAYFABSETTINGS_H
#define PLAYFABSETTINGS_H

struct PlayFabConfiguration {
	const String ProductionEnvironmentURL = ".playfabapi.com";
	const String AD_TYPE_IDFA = "Idfa";
	const String AD_TYPE_ANDROID_ID = "Adid";

	// The name of a customer vertical. This is only for customers running a private cluster. Generally you shouldn't touch this
	String VerticalName;

	// You must set this value for PlayFabSdk to work properly (Found in the Game
	// Manager for your title, at the PlayFab Website)
	String TitleId;

	// You must set this value for Admin/Server/Matchmaker to work properly (Found in the Game
	// Manager for your title, at the PlayFab Website)
	String DeveloperSecretKey;

	/// Client specifics

	// Set this to the appropriate AD_TYPE_X constant below
	String AdvertisingIdType;

	// Set this to corresponding device value
	String AdvertisingIdValue;

	// DisableAdvertising is provided for completeness, but changing it is not suggested
	// Disabling this may prevent your advertising-related PlayFab marketplace partners
	// from working correctly
	bool DisableAdvertising = false;

	struct InternalSettings {
		// This is automatically populated by the PlayFabAuthenticationApi.GetEntityToken method.
		Dictionary EntityToken;

		// This is automatically populated by any PlayFabClientApi.Login method.
		String ClientSessionTicket;
		String SdkVersionString = "GodotSdk-08.2022";
		Dictionary RequestGetParams;

		InternalSettings() {
			RequestGetParams = helper::dict("sdk", SdkVersionString);
		}
	};

	InternalSettings _internalSettings;

	void reset() {
		_internalSettings = InternalSettings();
	}
};

PlayFabConfiguration &PlayFabSettings();

#endif // PLAYFABSETTINGS_H
