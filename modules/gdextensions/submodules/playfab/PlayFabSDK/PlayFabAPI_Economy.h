/*************************************************************************/
/*  PlayFabAPI_Economy.h                                                 */
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

/* Auto-generated at Tue Aug 16 2022 20:53:56 GMT+0000 (Coordinated Universal Time) */

namespace Economy {


int AcceptTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Accept trade.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/accepttrade
	

	return _http_cli->request_append(
		"/Inventory/AcceptTrade",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int AddVirtualCurrencies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Increase virtual currencies amount.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/addvirtualcurrencies
	

	return _http_cli->request_append(
		"/Inventory/AddVirtualCurrencies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CancelTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Cancel trade.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/canceltrade
	

	return _http_cli->request_append(
		"/Inventory/CancelTrade",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ConsumeInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Consume inventory items.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/consumeinventoryitems
	

	return _http_cli->request_append(
		"/Inventory/ConsumeInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateDraftItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates a new item in the working catalog using provided metadata.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/createdraftitem
	

	return _http_cli->request_append(
		"/Catalog/CreateDraftItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int CreateUploadUrls(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates one or more upload URLs which can be used by the client to upload raw file data.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/createuploadurls
	

	return _http_cli->request_append(
		"/Catalog/CreateUploadUrls",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteEntityItemReviews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Deletes all reviews, helpfulness votes, and ratings submitted by the entity specified.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/deleteentityitemreviews
	

	return _http_cli->request_append(
		"/Catalog/DeleteEntityItemReviews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int DeleteItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Removes an item from working catalog and all published versions from the public catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/deleteitem
	

	return _http_cli->request_append(
		"/Catalog/DeleteItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetAccessTokens(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get Access Tokens
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getaccesstokens
	

	return _http_cli->request_append(
		"/Inventory/GetAccessTokens",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetCatalogConfig(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the configuration for the catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getcatalogconfig
	

	return _http_cli->request_append(
		"/Catalog/GetCatalogConfig",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetDraftItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves an item from the working catalog. This item represents the current working state of the item.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getdraftitem
	

	return _http_cli->request_append(
		"/Catalog/GetDraftItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetDraftItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a paginated list of the items from the draft catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getdraftitems
	

	return _http_cli->request_append(
		"/Catalog/GetDraftItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetEntityDraftItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves a paginated list of the items from the draft catalog created by the Entity.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getentitydraftitems
	

	return _http_cli->request_append(
		"/Catalog/GetEntityDraftItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetEntityItemReview(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the submitted review for the specified item by the authenticated entity.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getentityitemreview
	

	return _http_cli->request_append(
		"/Catalog/GetEntityItemReview",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get current inventory items.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getinventoryitems
	

	return _http_cli->request_append(
		"/Inventory/GetInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves an item from the public catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitem
	

	return _http_cli->request_append(
		"/Catalog/GetItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItemModerationState(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the moderation state for an item, including the concern category and string reason.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitemmoderationstate
	

	return _http_cli->request_append(
		"/Catalog/GetItemModerationState",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItemPublishStatus(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets the status of a publish of an item.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitempublishstatus
	

	return _http_cli->request_append(
		"/Catalog/GetItemPublishStatus",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItemReviews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a paginated set of reviews associated with the specified item.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitemreviews
	

	return _http_cli->request_append(
		"/Catalog/GetItemReviews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItemReviewSummary(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get a summary of all reviews associated with the specified item.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitemreviewsummary
	

	return _http_cli->request_append(
		"/Catalog/GetItemReviewSummary",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Retrieves items from the public catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/getitems
	

	return _http_cli->request_append(
		"/Catalog/GetItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetMarketplaceAccessToken(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get Access Tokens
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getmarketplaceaccesstoken
	

	return _http_cli->request_append(
		"/Inventory/GetMarketplaceAccessToken",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetReceiptIssuerCertificateAsJwk(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets receipt issuer certificate as JSON Web Key.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getreceiptissuercertificateasjwk
	

	return _http_cli->request_append(
		"/Inventory/GetReceiptIssuerCertificateAsJwk",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int GetReceiptIssuerCertificateAsPem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets receipt issuer certificate as PEM encoded certificate.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getreceiptissuercertificateaspem
	

	return _http_cli->request_append(
		"/Inventory/GetReceiptIssuerCertificateAsPem",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int GetReceiptIssuerCertificates(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Gets receipt issuer certificate as JSON Web Key and PEM encoded certificate.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getreceiptissuercertificates
	

	return _http_cli->request_append(
		"/Inventory/GetReceiptIssuerCertificates",
		dict_request,
		user_callback,
		dict_header_extra,
		Array(),
		Array()
	);
}

int GetTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get trade
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/gettrade
	

	return _http_cli->request_append(
		"/Inventory/GetTrade",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetTrades(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get trades
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/gettrades
	

	return _http_cli->request_append(
		"/Inventory/GetTrades",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GetVirtualCurrencies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Get current virtual currencies.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/getvirtualcurrencies
	

	return _http_cli->request_append(
		"/Inventory/GetVirtualCurrencies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int GrantInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Grant inventory items.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/grantinventoryitems
	

	return _http_cli->request_append(
		"/Inventory/GrantInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int OpenTrade(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Open trade.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/opentrade
	

	return _http_cli->request_append(
		"/Inventory/OpenTrade",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PublishDraftItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Initiates a publish of an item from the working catalog to the public catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/publishdraftitem
	

	return _http_cli->request_append(
		"/Catalog/PublishDraftItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PurchaseItemByFriendlyId(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Purchase an item, bundle or subscription by friendly id.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/purchaseitembyfriendlyid
	

	return _http_cli->request_append(
		"/Catalog/PurchaseItemByFriendlyId",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int PurchaseItemById(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Purchase an item, bundle or subscription by id.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/purchaseitembyid
	

	return _http_cli->request_append(
		"/Catalog/PurchaseItemById",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int Redeem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Redeem assets.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/redeem
	

	return _http_cli->request_append(
		"/Inventory/Redeem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RedeemInAppPurchase(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Redeem assets.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/redeeminapppurchase
	

	return _http_cli->request_append(
		"/Inventory/RedeemInAppPurchase",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int RefreshSubscriptions(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Refresh Subscriptions.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/refreshsubscriptions
	

	return _http_cli->request_append(
		"/Inventory/RefreshSubscriptions",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ReportItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Submit a report for an item, indicating in what way the item is inappropriate.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/reportitem
	

	return _http_cli->request_append(
		"/Catalog/ReportItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ReportItemReview(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Submit a report for a review
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/reportitemreview
	

	return _http_cli->request_append(
		"/Catalog/ReportItemReview",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int ReviewItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Creates or updates a review for the specified item.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/reviewitem
	

	return _http_cli->request_append(
		"/Catalog/ReviewItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SearchItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Executes a search against the public catalog using the provided search parameters and returns a set of paginated
	// results.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/searchitems
	

	return _http_cli->request_append(
		"/Catalog/SearchItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SearchTrades(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Search trades
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/searchtrades
	

	return _http_cli->request_append(
		"/Inventory/SearchTrades",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetInventoryItems(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set inventory items
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/setinventoryitems
	

	return _http_cli->request_append(
		"/Inventory/SetInventoryItems",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetItemModerationState(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Sets the moderation state for an item, including the concern category and string reason.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/setitemmoderationstate
	

	return _http_cli->request_append(
		"/Catalog/SetItemModerationState",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SetVirtualCurrencies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Set virtual currencies
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/setvirtualcurrencies
	

	return _http_cli->request_append(
		"/Inventory/SetVirtualCurrencies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SubmitItemReviewVote(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Submit a vote for a review, indicating whether the review was helpful or unhelpful.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/submititemreviewvote
	

	return _http_cli->request_append(
		"/Catalog/SubmitItemReviewVote",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int SubtractVirtualCurrencies(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Decrease virtual currencies amount.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/subtractvirtualcurrencies
	

	return _http_cli->request_append(
		"/Inventory/SubtractVirtualCurrencies",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int TakedownItemReviews(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Submit a request to takedown one or more reviews.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/takedownitemreviews
	

	return _http_cli->request_append(
		"/Catalog/TakedownItemReviews",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateCatalogConfig(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Updates the configuration for the catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/updatecatalogconfig
	

	return _http_cli->request_append(
		"/Catalog/UpdateCatalogConfig",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateDraftItem(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update the metadata for an item in the working catalog.
	// https://docs.microsoft.com/rest/api/playfab/economy/catalog/updatedraftitem
	

	return _http_cli->request_append(
		"/Catalog/UpdateDraftItem",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

int UpdateInventoryItemsProperties(Dictionary dict_request, UserCallback user_callback = nullptr, Dictionary dict_header_extra = Dictionary()) {
	
	// Update inventory Items.
	// https://docs.microsoft.com/rest/api/playfab/economy/inventory/updateinventoryitemsproperties
	

	return _http_cli->request_append(
		"/Inventory/UpdateInventoryItemsProperties",
		dict_request,
		user_callback,
		dict_header_extra,
		array(CHK_ENTITY_TOKEN, USE_AUTH_ENTITY_TOKEN),
		Array()
	);
}

} // namespace Economy
