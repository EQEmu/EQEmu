#include "zone/map.h"
#include "zone/zone.h"
#include <cassert>
#include <fstream>
#include <limits>
#include <iostream>
#include <cmath>
#include <filesystem>
bool near(float a,float b) { return std::abs(a-b)<0.02f; }
Zone* zone=nullptr;
const ZoneConfig* Config=nullptr;
int map_test_adjust=1;
// Tiny V1 files exercise the production decoder and production ownership swap.
void fixture(const char* path, float z, bool empty=false) {
	std::ofstream out(path,std::ios::binary);
	auto write=[&](const auto& v){out.write(reinterpret_cast<const char*>(&v),sizeof(v));};
	uint32_t version=0x01000000, count=empty?0:1; uint16_t nodes=0; uint32_t facelist=0;
	write(version);write(count);write(nodes);write(facelist);
	if(!empty) { float face[]{0,0,z,4,0,z,0,4,z,0,0,1,0}; for(auto f:face) write(f); }
}
int main() {
	fixture("triangle.map",0);fixture("moved.map",10);
	fixture("empty.map",0,true);fixture("bad.map",std::numeric_limits<float>::quiet_NaN());
	// A stale native cache must neither override decoded geometry nor be rewritten.
	{ std::ofstream cache("triangle.mmf"); cache<<"stale native cache"; }
	{ std::ofstream cache("missing.mmf"); cache<<"stale cache without a map"; }
	Map cache_only; assert(!cache_only.Load("missing.map"));
	Map m; glm::vec3 start(1,1,2), result(99), norm(99);float distance=99;
	assert(m.FindBestZ(start,nullptr)==BEST_Z_INVALID);
	assert(!m.CheckLoS(start,{1,1,-2}));
	assert(m.Load("triangle.map"));
	assert(near(m.FindBestZ(start,&result),0) && start.z==3);
	assert(near(m.FindClosestZ(start,nullptr),0));
	assert(near(m.FindGround(start,nullptr),0));
	glm::vec3 below(1,1,-2);assert(near(m.FindCeiling(below,nullptr),0));
	assert(m.LineIntersectsZone(start,below,1,nullptr));
	assert(!m.CheckLoS(start,below));
	assert(m.DoCollisionCheck(start,below,norm,distance) && norm.z==1 && distance==3);
	assert(!m.Load("bad.map"));assert(near(m.FindGround(start,nullptr),0));
	assert(!m.Load("missing.map"));assert(near(m.FindGround(start,nullptr),0));
	assert(m.Load("moved.map"));assert(near(m.FindCeiling(start,nullptr),10));
	Zone environment; zone=&environment; environment.newzone_data.max_z=5;
	assert(m.FindBestZ(start,nullptr)==BEST_Z_INVALID);
	environment.newzone_data.max_z=0;environment.newzone_data.underworld=11;
	glm::vec3 above(1,1,12);assert(m.FindGround(above,nullptr)==BEST_Z_INVALID);
	assert(m.Load("empty.map"));zone=nullptr;
	result=glm::vec3(99);norm=glm::vec3(99);distance=99;
	assert(m.FindBestZ(start,&result)==BEST_Z_INVALID && result.z==99);
	assert(m.FindClosestZ(start,nullptr)==BEST_Z_INVALID);
	assert(m.FindGround(start,nullptr)==BEST_Z_INVALID && m.FindCeiling(start,nullptr)==BEST_Z_INVALID);
	assert(m.CheckLoS(start,below));assert(!m.LineIntersectsZone(start,below,1,nullptr));
	assert(!m.LineIntersectsZoneNoZLeaps(start,below,1,nullptr));
	assert(!m.DoCollisionCheck(start,below,norm,distance) && norm.z==99 && distance==99);
	assert(!m.Load("bad.map"));assert(m.CheckLoS(start,below));
	std::ifstream cache("triangle.mmf"); std::string contents; std::getline(cache,contents);
	assert(contents=="stale native cache");
	assert(!std::filesystem::exists("moved.mmf"));
	std::cout<<"production Map wrapper and reload assertions passed\n";
}
