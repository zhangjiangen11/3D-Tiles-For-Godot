#ifndef CESIUM_3D_TILE
#define CESIUM_3D_TILE

#include "CesiumGltf/Model.h"
#include "Models/TileMetadata.h"
#include "godot_cpp/variant/dictionary.hpp"
#include <cstdint>
#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
using namespace godot;
#endif

#include <glm/ext/vector_double3.hpp>

namespace CesiumGltf {
	class Model;
	
	class ExtensionModelExtStructuralMetadata;
}

class CesiumGeoreference;

class Cesium3DTile : public MeshInstance3D {
	GDCLASS(Cesium3DTile, MeshInstance3D)
	
public:
	
	/// @brief Provides the position as it was passed in from the gltf parser
	const glm::dvec3& get_original_position();
	
	void set_original_position(const glm::dvec3& position);
	
	
	void apply_position_on_globe(const glm::dvec3& engineOrigin);

	void generate_tile_collision();
	
	void add_metadata(const CesiumGltf::Model* model, const CesiumGltf::ExtensionModelExtStructuralMetadata* metadata);
	
	const Dictionary& get_metadata_table(int32_t idx) const;

	int32_t get_table_count() const;
	
private:

	Ref<ConcavePolygonShape3D> create_trimesh_shape_inverse_winding();	

	Node* create_collision_node_custom_trimesh();

	TileMetadata m_metadata;
	
	glm::dvec3 m_originalPosition;

protected:

	static void _bind_methods();
	
};


#endif
