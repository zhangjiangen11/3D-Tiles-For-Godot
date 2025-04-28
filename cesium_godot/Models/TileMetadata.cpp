#include "TileMetadata.h"

void TileMetadata::init(size_t tableCount) {
	this->m_tables.reserve(tableCount);
}

void TileMetadata::add_table(const CesiumGltf::PropertyTableView& tableView) {
	CesiumPropertyTable_t table{};
	table.reserve(tableView.size());
	tableView.forEachProperty([this, table](const std::string& propertyId, auto propertyValue) mutable {
		table.emplace(propertyId, this->make_metadata_value(propertyValue));
	});
}

