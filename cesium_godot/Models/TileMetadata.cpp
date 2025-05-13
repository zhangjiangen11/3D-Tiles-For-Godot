#include "TileMetadata.h"

void TileMetadata::init(size_t tableCount) {
	this->m_tables.reserve(tableCount);
}

const Dictionary& TileMetadata::get_table(int32_t index) const {
	return this->m_tables.at(index);
}


int32_t TileMetadata::get_table_count() const {
	return this->m_tables.size();
}

void TileMetadata::add_table(const CesiumGltf::PropertyTableView& tableView) {
	CesiumPropertyTable_t table{};
	// table.reserve(tableView.size());
	tableView.forEachProperty([this, table](const std::string& propertyId, auto propertyValue) mutable {
		table.get_or_add(propertyId.c_str(), this->make_metadata_value(propertyValue));
	});
}

