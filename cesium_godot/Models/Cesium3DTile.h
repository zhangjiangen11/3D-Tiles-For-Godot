#ifndef CESIUM_3D_TILE
#define CESIUM_3D_TILE

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/concave_polygon_shape3d.hpp"
using namespace godot;
#endif

#include <glm/ext/vector_double3.hpp>

class CesiumGeoreference;

class Cesium3DTile : public MeshInstance3D {
	GDCLASS(Cesium3DTile, MeshInstance3D)
	
public:
	
	/// @brief Provides the position as it was passed in from the gltf parser
	const glm::dvec3& get_original_position();
	
	void set_original_position(const glm::dvec3& position);
	
	void apply_position_on_globe(const glm::dvec3& engineOrigin);

	void generate_tile_collision();
	
private:

	Ref<ConcavePolygonShape3D> create_trimesh_shape_inverse_winding();	

	Node* create_collision_node_custom_trimesh();
	
	glm::dvec3 m_originalPosition;

protected:

	static void _bind_methods();
	
};


#endif
