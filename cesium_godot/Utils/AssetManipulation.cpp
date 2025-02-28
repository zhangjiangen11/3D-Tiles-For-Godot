#include "AssetManipulation.h"

#include "../Models/CesiumGlobe.h"
#include "Models/CesiumGDConfig.h"
#include "Models/CesiumGDCreditSystem.h"
#include "Models/CesiumGDRasterOverlay.h"
#include "Models/CesiumGDTileset.h"
#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/array.hpp"
#include "magic_enum.hpp"
#include <cstdint>
#include <winnt.h>

const char* CESIUM_GLOBE_NAME = "CesiumGeoreference";
const char* CESIUM_TILESET_NAME = "Cesium3DTileset";

CesiumGeoreference* Godot3DTiles::AssetManipulation::find_or_create_globe(Node* baseNode) {
  Node* root = get_root_of_edit_scene(baseNode);
	CesiumGeoreference* globe = nullptr;
	int32_t count = root->get_child_count();
	for (int32_t i = 0; i < count; i++) {
		Node* child = root->get_child(i);
		CesiumGeoreference* foundChild = Object::cast_to<CesiumGeoreference>(child);
		if (foundChild != nullptr) {
			return foundChild;
		}
	}
	
	//Create a globe
	globe = memnew(CesiumGeoreference);
	globe->set_name(CESIUM_GLOBE_NAME);
	globe->set_rotation_degrees(Vector3(-90.0, 0.0, 0.0));
	root->add_child(globe, true);
	globe->set_owner(root);
	return globe;
}

CesiumGDConfig* Godot3DTiles::AssetManipulation::find_or_create_config_node(Node* baseNode) {
	Node* root = baseNode->get_tree()->get_root();
	CesiumGDConfig* result = find_node_in_scene<CesiumGDConfig>(root);
	if (result != nullptr) {
		return result;
	}
	result = memnew(CesiumGDConfig);
	root->add_child(result);
	result->set_owner(root);
	return result;
}

CesiumGDCreditSystem* Godot3DTiles::AssetManipulation::find_or_create_credit_system(Node* baseNode, bool deferred) {
	// HACK: assume we will always have this root as a node 3d on the child of the window
	Node* root = baseNode->get_tree()->get_root();
	CesiumGDCreditSystem* result = find_node_in_scene<CesiumGDCreditSystem>(root);
	if (result != nullptr) {
		return result;
	}
	
	result = memnew(CesiumGDCreditSystem);
	CesiumGeoreference* globe = find_node_in_scene<CesiumGeoreference>(root);
	ERR_FAIL_COND_V_MSG(globe == nullptr, nullptr, "No CesiumGeoreference found in scene, please add one manually or with the Cesium Panel!");
	constexpr bool readableName = true;
	if (deferred) {
		globe->get_parent_node_3d()->call_deferred("add_child", result, readableName, godot::Node::INTERNAL_MODE_FRONT);
		result->call_deferred("set_owner", globe->get_parent_node_3d());
	}
	else {
		globe->get_parent_node_3d()->add_child(result, readableName, godot::Node::INTERNAL_MODE_FRONT);
		result->set_owner(globe->get_parent_node_3d());
	}
	return result;
}

Node* Godot3DTiles::AssetManipulation::get_root_of_edit_scene(Node* baseNode) {
  return Object::cast_to<Node>(baseNode->get_tree()->get_edited_scene_root());
}


void Godot3DTiles::AssetManipulation::instantiate_tileset(Node* baseNode, int32_t tilesetType) {
	Node* root = get_root_of_edit_scene(baseNode);
	Cesium3DTileset* tileset = memnew(Cesium3DTileset);
	root->add_child(tileset, true);
	
	
	TilesetType actualType = static_cast<TilesetType>(tilesetType);
	CesiumIonRasterOverlay* rasterOverlay = nullptr;
	Cesium3DTileset* extraTileset = nullptr;
	
	constexpr int32_t cesiumWorldTerrainId = 1;
	constexpr int32_t bingMapsAerialWithLabelsId = 3;
	constexpr int32_t osmBuildingsId = 96188;
	constexpr int32_t bingRoadsId = 4;
	
	switch(actualType) {
		case Godot3DTiles::AssetManipulation::TilesetType::Blank:
			break;
		case Godot3DTiles::AssetManipulation::TilesetType::BingMapsAerialWithLabels:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumIonRasterOverlay);
			rasterOverlay->set_asset_id(bingMapsAerialWithLabelsId);
			break;
		
		case Godot3DTiles::AssetManipulation::TilesetType::BingMapsRoads:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumIonRasterOverlay);
			rasterOverlay->set_asset_id(bingRoadsId);
			break;
			
		case Godot3DTiles::AssetManipulation::TilesetType::OsmBuildings:
			tileset->set_ion_asset_id(cesiumWorldTerrainId);
			rasterOverlay = memnew(CesiumIonRasterOverlay);
			rasterOverlay->set_asset_id(bingMapsAerialWithLabelsId);
			extraTileset = memnew(Cesium3DTileset);
			extraTileset->set_ion_asset_id(osmBuildingsId);
			break;
		case Godot3DTiles::AssetManipulation::TilesetType::GooglePhotorealistic:
			tileset->set_ion_asset_id(2275207);
			break;
		default:
			ERR_PRINT(String("Tileset type not implemented: ") + magic_enum::enum_name(actualType).data());
			break;	
	}

	if (extraTileset != nullptr) {
		root->add_child(extraTileset, true);
	}

	if (rasterOverlay != nullptr) {
		tileset->add_child(rasterOverlay, true);
		rasterOverlay->set_owner(root);
	}
	
}


void Godot3DTiles::AssetManipulation::instantiate_dynamic_cam(Node* baseNode) {
	const char* georefCameraScript = "res://addons/cesium_godot/scripts/georeference_camera_controller.gd";
	const char* trueOriginCameraScript = "res://addons/cesium_godot/scripts/cesium_camera_controller.gd";
	Node* root = get_root_of_edit_scene(baseNode);
	CesiumGeoreference* globe = find_or_create_globe(baseNode);
	Camera3D* camera = memnew(Camera3D);
	root->add_child(camera, true);
	camera->set_owner(root);
	auto originType = static_cast<CesiumGeoreference::OriginType>(globe->get_origin_type());
	Ref<Resource> script = ResourceLoader::get_singleton()->load(georefCameraScript, "Script");
	camera->set_script(script);
	camera->set("tilesets", find_all_tilesets(baseNode));
	camera->set("globe_node", globe);
}



Cesium3DTileset* find_first_tileset(Node* baseNode) {
	//Get a globe
	return nullptr;
}


Array Godot3DTiles::AssetManipulation::find_all_tilesets(Node* baseNode) {
	// All tilesets will be inside the globe
	CesiumGeoreference* globeNode = find_or_create_globe(baseNode);
	int32_t count = globeNode->get_child_count();
	Array results;
	for (int32_t i = 0; i < count; i++) {
		Node* child = globeNode->get_child(i);
		auto* foundTileset = Object::cast_to<Cesium3DTileset>(child);
		if (foundTileset == nullptr) continue;
		results.append(foundTileset);
	}
	return results;
}
