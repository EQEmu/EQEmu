#pragma once

#include "zone/water_map.h"

#pragma pack(push)
#pragma pack(1)
typedef struct ZBSP_Node {
	int32 node_number;
	float normal[3], splitdistance;
	int32 region;
	int32 special;
	int32 left, right;
} ZBSP_Node;

#pragma pack(pop)

class WaterMapV1 : public WaterMap
{
public:
	WaterMapV1();
	~WaterMapV1();
	
	virtual WaterRegionType ReturnRegionType(const glm::vec3& location) const;
	virtual bool InWater(const glm::vec3& location) const;
	virtual bool InVWater(const glm::vec3& location) const;
	virtual bool InLava(const glm::vec3& location) const;
	virtual bool InLiquid(const glm::vec3& location) const;
	virtual bool InPvP(const glm::vec3& location) const;
	virtual bool InZoneLine(const glm::vec3& location) const;
	
protected:
	virtual bool Load(FILE *fp);

private:
	WaterRegionType BSPReturnRegionType(int32 node_number, const glm::vec3& location) const;
	ZBSP_Node* BSP_Root;

	friend class WaterMap;
};
