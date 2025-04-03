#include "Cesium3DTile.h"
#include "Utils/CesiumMathUtils.h"
#include "glm/ext/vector_double3.hpp"

const glm::dvec3& Cesium3DTile::get_original_position() {
	return this->m_originalPosition;
}

void Cesium3DTile::set_original_position(const glm::dvec3& position) {
	this->m_originalPosition = position;
}


void Cesium3DTile::apply_position_on_globe(const glm::dvec3& engineOrigin) {
	glm::dvec3 globalPos = this->m_originalPosition - engineOrigin;
	this->set_global_position(CesiumMathUtils::from_glm_vec3(globalPos));
}

void Cesium3DTile::_bind_methods() {
	// We do not want to bind anything, but we need to register the class
}
