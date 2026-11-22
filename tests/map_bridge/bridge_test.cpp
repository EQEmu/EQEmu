#include "rust_raycast_mesh.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <limits>
struct Release { void operator()(RaycastMesh* e) const { if(e) e->release(); } };
int main() {
    std::string error;
    using Engine=std::unique_ptr<RaycastMesh,Release>;
    std::vector<std::array<float,3>> v{{0,0,0},{4,0,0},{0,4,0}};
    float a[]{1,1,2}, b[]{1,1,-2}, p[]{99,99,99}, n[]{99,99,99}, d=99;
    for (int iteration=0; iteration<64; ++iteration) {
        Engine e(createRustRaycastMesh(v,{0,1,2},error)); assert(e);
        assert(e->getBoundMin()[2]==0 && e->getBoundMax()[0]==4);
        assert(e->raycast(a,b,p,n,&d)); assert(p[2]==0 && n[2]==1 && d==2);
        assert(e->raycast(a,b,nullptr,nullptr,nullptr));
        float outside[]{8,8,2}; p[0]=n[0]=d=99;
        assert(!e->raycast(outside,b,p,n,&d)); assert(p[0]==99 && n[0]==99 && d==99);
        assert(!e->raycast(a,a,p,n,&d));
        float shortA[]{1,1,1e-6f}, shortB[]{1,1,-1e-6f};
        assert(!e->raycast(shortA,shortB,nullptr,nullptr,nullptr));
        Engine reversed(createRustRaycastMesh(v,{0,2,1,0,1,2},error));
        assert(reversed->raycast(a,b,p,n,&d) && n[2]==-1);
        Engine invalid(createRustRaycastMesh(v,{3,0,1},error)); assert(!invalid && !error.empty());
        assert(e->raycast(a,b,p,n,&d));
        Engine empty(createRustRaycastMesh({}, {},error)); assert(empty);
        assert(!empty->getBoundMin() && !empty->getBoundMax());
        assert(!empty->raycast(a,b,p,n,&d));
        auto bad=v; bad[0][0]=std::numeric_limits<float>::quiet_NaN();
        assert(!createRustRaycastMesh(bad,{0,1,2},error));
        assert(!createRustRaycastMesh(v,{0,1},error));
        float invalidQuery[]{INFINITY,0,0}; bool threw=false;
        try { e->raycast(invalidQuery,b,p,n,&d); } catch(const std::exception&) { threw=true; }
        assert(threw);
    }
    std::cout << "64 bridge lifecycle iterations passed\n";
}
