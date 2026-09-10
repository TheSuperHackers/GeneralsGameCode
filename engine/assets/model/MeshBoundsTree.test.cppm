module;
#define BOOST_TEST_MODULE MeshBoundsTreeTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <vector>
export module Assets.MeshBoundsTree.Tests;
import Assets.Math;
import Assets.MeshBoundsTree;
using namespace Assets;

BOOST_AUTO_TEST_CASE(sample_order_and_front_first_polygon_order_are_preserved)
{
	std::vector<Vector3f> vertices;
	std::vector<std::array<std::uint32_t,3>> triangles;
	for (unsigned i=0;i<8;++i) {
		vertices.insert(vertices.end(),{{float(i),0,0},{float(i),1,0},{float(i),0,1}});
		triangles.push_back({i*3,i*3+1,i*3+2});
	}
	unsigned calls=0;
	MeshBoundsTree tree;
	BOOST_REQUIRE(Build_Mesh_Bounds_Tree(vertices,triangles,[&] { return calls++%3==0 ? 3u : 0u; },tree));
	// Root samples x=3, then its five-polygon front child samples x=6.
	// Both runs consume all candidates, even when subsequent costs tie.
	BOOST_CHECK_EQUAL(calls,39);
	const std::vector<unsigned> expected{6,7,3,4,5,0,1,2};
	BOOST_CHECK_EQUAL_COLLECTIONS(tree.polygon_indices.begin(),tree.polygon_indices.end(),expected.begin(),expected.end());
	BOOST_REQUIRE_EQUAL(tree.nodes.size(),5);
	BOOST_CHECK_EQUAL(tree.nodes[0].first,1);
	BOOST_CHECK_EQUAL(tree.nodes[0].second,4);
	BOOST_CHECK_EQUAL(tree.nodes[1].first,2);
	BOOST_CHECK_EQUAL(tree.nodes[1].second,3);
	BOOST_CHECK_EQUAL(tree.nodes[2].first,0);
	BOOST_CHECK_EQUAL(tree.nodes[2].second,2);
	BOOST_CHECK_EQUAL(tree.nodes[3].first,2);
	BOOST_CHECK_EQUAL(tree.nodes[3].second,3);
	BOOST_CHECK_EQUAL(tree.nodes[4].first,5);
	BOOST_CHECK_EQUAL(tree.nodes[4].second,3);
	BOOST_CHECK_EQUAL(tree.nodes[0].bounds.minimum.x,0);
	BOOST_CHECK_EQUAL(tree.nodes[0].bounds.maximum.x,7);
	BOOST_CHECK_EQUAL(tree.nodes[1].bounds.minimum.x,3);
	BOOST_CHECK_EQUAL(tree.nodes[1].bounds.maximum.x,7);
	vertices.clear();triangles.clear();
	BOOST_CHECK_EQUAL(tree.nodes[4].bounds.maximum.x,2);
}

BOOST_AUTO_TEST_CASE(coplanar_triangles_and_invalid_sources_do_not_change_sampling_contract)
{
	std::vector<Vector3f> vertices{{-0.0f,0,0},{0,1,0},{0,0,1}};
	std::vector<std::array<std::uint32_t,3>> triangles(7,{0,1,2});
	unsigned calls=0;
	MeshBoundsTree tree;
	BOOST_REQUIRE(Build_Mesh_Bounds_Tree(vertices,triangles,[&] { ++calls;return 0u; },tree));
	BOOST_CHECK_EQUAL(calls,21);
	BOOST_REQUIRE_EQUAL(tree.nodes.size(),1);
	BOOST_CHECK(tree.nodes[0].leaf);
	BOOST_CHECK_EQUAL(tree.nodes[0].second,7);
	BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(tree.nodes[0].bounds.minimum.x),0x80000000u);
	triangles[0][0]=3;
	BOOST_CHECK(!Build_Mesh_Bounds_Tree(vertices,triangles,[&] { ++calls;return 0u; },tree));
	BOOST_CHECK_EQUAL(calls,21);
	BOOST_CHECK_EQUAL(tree.nodes[0].second,7);
	triangles[0][0]=0;vertices[0].x=(std::numeric_limits<float>::infinity)();
	BOOST_CHECK(!Build_Mesh_Bounds_Tree(vertices,triangles,[&] { ++calls;return 0u; },tree));
	BOOST_CHECK_EQUAL(calls,21);
}

BOOST_AUTO_TEST_CASE(large_tree_retains_every_polygon_and_bounds_each_subtree)
{
	std::vector<Vector3f> vertices;
	std::vector<std::array<std::uint32_t,3>> triangles;
	for(unsigned i=0;i<24000;++i) {
		const float x=float(i%200),y=float(i/200);
		vertices.insert(vertices.end(),{{x,y,0},{x+.25f,y,0},{x,y+.25f,1}});
		triangles.push_back({i*3,i*3+1,i*3+2});
	}
	std::uint32_t state=17;
	MeshBoundsTree tree;
	BOOST_REQUIRE(Build_Mesh_Bounds_Tree(vertices,triangles,[&] { state=state*1664525u+1013904223u;return state; },tree));
	auto indices=tree.polygon_indices;
	std::sort(indices.begin(),indices.end());
	BOOST_REQUIRE_EQUAL(indices.size(),triangles.size());
	for(unsigned i=0;i<indices.size();++i) BOOST_CHECK_EQUAL(indices[i],i);
	for(const auto& node:tree.nodes) {
		BOOST_CHECK(node.bounds.Is_Valid());
		const auto contains=[&](Vector3f point) {
			return point.x>=node.bounds.minimum.x && point.x<=node.bounds.maximum.x &&
				point.y>=node.bounds.minimum.y && point.y<=node.bounds.maximum.y &&
				point.z>=node.bounds.minimum.z && point.z<=node.bounds.maximum.z;
		};
		if(node.leaf) {
			BOOST_REQUIRE(node.first+node.second<=tree.polygon_indices.size());
			for(unsigned i=0;i<node.second;++i)
				for(const auto vertex:triangles[tree.polygon_indices[node.first+i]]) BOOST_CHECK(contains(vertices[vertex]));
		} else {
			BOOST_REQUIRE(node.first<tree.nodes.size());BOOST_REQUIRE(node.second<tree.nodes.size());
			for(const auto child:{node.first,node.second}) {
				BOOST_CHECK(contains(tree.nodes[child].bounds.minimum));
				BOOST_CHECK(contains(tree.nodes[child].bounds.maximum));
			}
		}
	}
}
