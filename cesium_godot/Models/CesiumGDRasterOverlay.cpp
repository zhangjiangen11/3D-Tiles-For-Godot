#include "CesiumGDRasterOverlay.h"
#include "../Utils/NetworkUtils.h"
#include <CesiumRasterOverlays/IonRasterOverlay.h>
#include "CesiumGDTileset.h"
#include "CesiumGDConfig.h"

int64_t CesiumIonRasterOverlay::get_asset_id() const
{
	return this->m_assetId;
}

void CesiumIonRasterOverlay::set_asset_id(int64_t id)
{
	this->m_assetId = id;
}

void CesiumIonRasterOverlay::set_material_key(const String& key)
{
	this->m_materialKey = key;
}

const String& CesiumIonRasterOverlay::get_material_key() const
{
	return this->m_materialKey;
}

Error CesiumIonRasterOverlay::add_to_tileset(Cesium3DTileset* tilesetInstance)
{
	if (tilesetInstance == nullptr) return Error::ERR_INVALID_PARAMETER;
	this->m_configInstance = tilesetInstance->get_cesium_config();
	if (this->m_assetId <= 0) return Error::ERR_CANT_ACQUIRE_RESOURCE;

	//Overlay already added
	if (this->m_overlayInstance != nullptr) return Error::OK;

	this->create_and_add_overlay(tilesetInstance);
	return Error::OK;
}

void CesiumIonRasterOverlay::remove_from_tileset(Cesium3DTileset* tilesetInstance)
{

}

CesiumUtility::IntrusivePointer<CesiumRasterOverlays::IonRasterOverlay> CesiumIonRasterOverlay::get_overlay_instance()
{
	return this->m_overlayInstance;
}

void CesiumIonRasterOverlay::create_and_add_overlay(Cesium3DTileset* tilesetInstance)
{
	const String& ionAccessToken = this->m_configInstance->get_access_token();
	this->m_overlayInstance = new CesiumRasterOverlays::IonRasterOverlay(
		this->m_materialKey.utf8().get_data(),
		this->m_assetId,
		ionAccessToken.utf8().get_data(),
		{},
		this->m_configInstance->get_api_url().utf8().get_data()
	);
	tilesetInstance->add_overlay(this);
}

void CesiumIonRasterOverlay::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("set_material_key", "key"), &CesiumIonRasterOverlay::set_material_key);
	ClassDB::bind_method(D_METHOD("get_material_key"), &CesiumIonRasterOverlay::get_material_key);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "key"), "set_material_key", "get_material_key");


	ClassDB::bind_method(D_METHOD("set_asset_id", "id"), &CesiumIonRasterOverlay::set_asset_id);
	ClassDB::bind_method(D_METHOD("get_asset_id"), &CesiumIonRasterOverlay::get_asset_id);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "asset_id"), "set_asset_id", "get_asset_id");
}
