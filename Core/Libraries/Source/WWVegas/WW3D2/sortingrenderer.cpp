/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/sortingrenderer.cpp                    $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 * 06/27/02 KM Changes to max texture stage caps																*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "sortingrenderer.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "Renderer.h"
#include "vertmaterial.h"
#include "texture.h"
#include "d3d8.h"
#include "statistics.h"
#include <WWDebug/wwprofile.h>
#include <Utility/cmath_adapter.h>
#include <algorithm>
#include <list>
#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <cstring>
#include <vector>
#include <atomic>
#include <cfenv>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <new>
#include <system_error>
#include <thread>


bool SortingRendererClass::_EnableTriangleDraw=true;
static unsigned DEFAULT_SORTING_POLY_COUNT = 16384;	// (count * 3) must be less than 65536
static unsigned DEFAULT_SORTING_VERTEX_COUNT = 32768;	// count must be less than 65536

void SortingRendererClass::SetMinVertexBufferSize( unsigned val )
{
	DEFAULT_SORTING_VERTEX_COUNT = val;
	DEFAULT_SORTING_POLY_COUNT = val/2;	//typically have 2:1 vertex:triangle ratio.
}

struct ShortVectorIStruct
{
	unsigned short i;
	unsigned short j;
	unsigned short k;
};

struct TempIndexStruct
{
	ShortVectorIStruct tri;
	unsigned short idx;
	float z;
};

bool operator <(const TempIndexStruct &l, const TempIndexStruct &r) { return l.z < r.z; }
bool operator <=(const TempIndexStruct &l, const TempIndexStruct &r) { return l.z <= r.z; }
bool operator >(const TempIndexStruct &l, const TempIndexStruct &r) { return l.z > r.z; }
bool operator >=(const TempIndexStruct &l, const TempIndexStruct &r) { return l.z >= r.z; }
bool operator ==(const TempIndexStruct &l, const TempIndexStruct &r) { return l.z == r.z; }
// ----------------------------------------------------------------------------
static
void InsertionSort(TempIndexStruct *begin, TempIndexStruct *end)
{
	for (TempIndexStruct *iter = begin + 1; iter < end; ++iter) {
		TempIndexStruct val = iter[0];
		TempIndexStruct *insert = iter;
		while (insert != begin && insert[-1] > val) {
			insert[0] = insert[-1];
			insert -= 1;
		}
		insert[0] = val;
	}
}

// ----------------------------------------------------------------------------
static
void Sort(TempIndexStruct *begin, TempIndexStruct *end)
{
	const int diff = end - begin;
	if (diff <= 16) {
		// Insertion sort has less overhead for small arrays
		InsertionSort(begin, end);
	} else {
		// Choose the median of begin, mid, and (end - 1) as the partitioning element.
		// Rearrange so that *(begin + 1) <= *begin <= *(end - 1).  These will be guard
		// elements.
		TempIndexStruct *mid = begin + diff/2;
		std::swap(mid[0], begin[1]);
		if (begin[1] > end[-1]) {
			std::swap(begin[1], end[-1]);
		}
		if (begin[0] > end[-1]) {
			std::swap(begin[0], end[-1]);
		}
		if (begin[1] > begin[0]) {
			std::swap(begin[1], begin[0]);
		}

		// *begin is now the partitioning element
		TempIndexStruct *begin1 = begin + 1;	// TODO: Temp fix until I find out who is passing me NaN
		TempIndexStruct *end1 = end - 1;			// TODO: Temp fix until I find out who is passing me NaN
		TempIndexStruct *left = begin + 1;
		TempIndexStruct *right = end - 1;
		for (;;) {
#if 0		// TODO: Temp fix until I find out who is passing me NaN.
			do ++left; while (left[0] < begin[0]);		// Scan up to find element >= than partition
			do --right; while (right[0] > begin[0]);	// Scan down to find element <= than partition
#else
			do ++left; while (left < end1 && left[0] < begin[0]);		// Scan up to find element >= than partition
			do --right; while (right > begin1 && right[0] > begin[0]);	// Scan down to find element <= than partition
#endif
			if (right < left) break;									// Pointers crossed.  Partitioning completed.
			std::swap(left[0], right[0]);							// Exchange elements.
		}
		std::swap(begin[0], right[0]);							// Insert partition element

		// Sort the smaller subarray first then the larger
		if (right - begin > end - (right + 1)) {
			Sort(right + 1, end);
			Sort(begin, right);
		} else {
			Sort(begin, right);
			Sort(right + 1, end);
		}
	}
}

// ----------------------------------------------------------------------------

class SortingNodeStruct
{
	W3DMPO_CODE(SortingNodeStruct)

public:
	RenderStateStruct sorting_state;

	Vector3 transformed_center;
	unsigned short start_index;			// First index used in the ib
	unsigned short polygon_count;			// Polygon count to process (3 indices = one polygon)
	unsigned short min_vertex_index;		// First index used in the vb
	unsigned short vertex_count;			// Number of vertices used in vb
	bool billboard_quads;
};

typedef std::list<SortingNodeStruct*> SortingNodeStructList;
static SortingNodeStructList sorted_list;
static SortingNodeStructList unsorted_list;
static SortingNodeStructList clean_list;
static unsigned total_sorting_vertices;

static SortingNodeStruct* Get_Sorting_Struct()
{
	if (!clean_list.empty()) {
		SortingNodeStruct* state = clean_list.front();
		clean_list.pop_front();
		return state;
	}
	return W3DNEW SortingNodeStruct();
}

// ----------------------------------------------------------------------------
//
// Temporary arrays for the sorting system
//
// ----------------------------------------------------------------------------

static TempIndexStruct* temp_index_array;
static unsigned temp_index_array_count;

// Adapted from Zero Hour Reforged (Olcay Seygan and contributors): stable
// three-pass radix sort of depth keys. Keep the old sort for small pools.
static std::vector<TempIndexStruct> depth_sort_scratch;

static std::uint32_t Depth_Sort_Key(float depth)
{
	static_assert(sizeof(float) == sizeof(std::uint32_t));
	static_assert(std::numeric_limits<float>::is_iec559);
	const auto bits = std::bit_cast<std::uint32_t>(depth);
	const auto magnitude = bits & 0x7fffffffu;
	// Both signed zeroes compare equal, so keep their original relative order.
	if (magnitude == 0)
		return 0x80000000u;
	return (bits & 0x80000000u) ? 0x7fffffffu - magnitude : 0x80000000u + magnitude;
}

static TempIndexStruct *Sort_By_Depth(TempIndexStruct *items, unsigned count)
{
	if (count < 256)
	{
		if (count > 1)
			Sort(items, items + count);
		return items;
	}

	constexpr unsigned bitsPerPass = 11;
	constexpr unsigned buckets = 1u << bitsPerPass;
	constexpr unsigned passes = 3;
	std::array<std::array<unsigned, buckets>, passes> histogram{};
	for (unsigned i = 0; i < count; ++i)
	{
		// Preserve the existing guarded comparison sort for malformed depths.
		if (!std::isfinite(items[i].z))
		{
			Sort(items, items + count);
			return items;
		}
		const auto key = Depth_Sort_Key(items[i].z);
		for (unsigned pass = 0; pass < passes; ++pass)
			++histogram[pass][(key >> (pass * bitsPerPass)) & (buckets - 1)];
	}

	depth_sort_scratch.resize(count);
	TempIndexStruct *from = items;
	TempIndexStruct *to = depth_sort_scratch.data();
	for (unsigned pass = 0; pass < passes; ++pass)
	{
		auto &offsets = histogram[pass];
		unsigned offset = 0;
		for (auto &bucket : offsets)
		{
			const unsigned size = bucket;
			bucket = offset;
			offset += size;
		}
		for (unsigned i = 0; i < count; ++i)
		{
			const auto key = Depth_Sort_Key(from[i].z);
			to[offsets[(key >> (pass * bitsPerPass)) & (buckets - 1)]++] = from[i];
		}
		std::swap(from, to);
	}
	return from;
}

static TempIndexStruct* Get_Temp_Index_Array(unsigned count)
{
	if (count < DEFAULT_SORTING_POLY_COUNT)
		count = DEFAULT_SORTING_POLY_COUNT;
	if (count>temp_index_array_count) {
		delete[] temp_index_array;
		temp_index_array=W3DNEWARRAY TempIndexStruct[count];
		temp_index_array_count=count;
	}
	return temp_index_array;
}

// ----------------------------------------------------------------------------
//
// Insert triangles to the sorting system.
//
// ----------------------------------------------------------------------------

void SortingRendererClass::Insert_Triangles(
	const SphereClass& bounding_sphere,
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count,
	bool billboard_quads)
{
	if (!WW3D::Is_Sorting_Enabled()) {
		DX8Wrapper::Draw_Triangles(start_index,polygon_count,min_vertex_index,vertex_count);
		return;
	}

	SNAPSHOT_SAY(("SortingRenderer::Insert(start_i: %d, polygons : %d, min_vi: %d, vertex_count: %d)",
		start_index,polygon_count,min_vertex_index,vertex_count));


	DX8_RECORD_SORTING_RENDER(polygon_count,vertex_count);

	SortingNodeStruct* state=Get_Sorting_Struct();

	DX8Wrapper::Get_Render_State(state->sorting_state);

 	WWASSERT(
		((state->sorting_state.index_buffer_type==BUFFER_TYPE_SORTING || state->sorting_state.index_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING) &&
		(state->sorting_state.vertex_buffer_types[0]==BUFFER_TYPE_SORTING || state->sorting_state.vertex_buffer_types[0]==BUFFER_TYPE_DYNAMIC_SORTING)));


	state->start_index=start_index;
	state->polygon_count=polygon_count;
	state->min_vertex_index=min_vertex_index;
	state->vertex_count=vertex_count;
	state->billboard_quads = billboard_quads;

	if (bounding_sphere.Is_Valid())
	{
		const Matrix4x4 mtx=To_Matrix4x4(state->sorting_state.view)*To_Matrix4x4(state->sorting_state.world);
		Matrix4x4::Transform_Vector(mtx,bounding_sphere.Center,&state->transformed_center);

		Insert_To_Sorted_List(state);
	}
	else
	{
		// TheSuperHackers @perf stephanmeesters 04/07/2026 Nodes without bounding information do not require sorting.
		state->transformed_center = Vector3(0.0f, 0.0f, 0.0f);
		unsorted_list.push_back(state);
	}

#ifdef DEBUG_CRASHING
	SortingVertexBufferClass* vertex_buffer=static_cast<SortingVertexBufferClass*>(state->sorting_state.vertex_buffers[0]);
	WWASSERT(vertex_buffer);
	WWASSERT(state->vertex_count<=vertex_buffer->Get_Vertex_Count());

	unsigned short* indices=nullptr;
	SortingIndexBufferClass* index_buffer=static_cast<SortingIndexBufferClass*>(state->sorting_state.index_buffer);
	WWASSERT(index_buffer);
	indices=index_buffer->index_buffer;
	WWASSERT(indices);
	indices+=state->start_index;
	indices+=state->sorting_state.iba_offset;

	for (int i=0;i<state->polygon_count;++i) {
		unsigned short idx1=indices[i*3]-state->min_vertex_index;
		unsigned short idx2=indices[i*3+1]-state->min_vertex_index;
		unsigned short idx3=indices[i*3+2]-state->min_vertex_index;
		WWASSERT(idx1<state->vertex_count);
		WWASSERT(idx2<state->vertex_count);
		WWASSERT(idx3<state->vertex_count);
	}
#endif // DEBUG_CRASHING
}

// ----------------------------------------------------------------------------
//
// Insert triangles to the sorting system, with no bounding information.
//
// ----------------------------------------------------------------------------

void SortingRendererClass::Insert_Triangles(
	unsigned short start_index,
	unsigned short polygon_count,
	unsigned short min_vertex_index,
	unsigned short vertex_count)
{
	Insert_Triangles(SphereClass(),start_index,polygon_count,min_vertex_index,vertex_count);
}

// ----------------------------------------------------------------------------
//
// Flush all sorting polygons.
//
// ----------------------------------------------------------------------------

void SortingRendererClass::Insert_Quads(unsigned short quad_count, unsigned short vertex_count)
{
	WWASSERT(static_cast<unsigned>(quad_count) * 4 == vertex_count);
	Insert_Triangles(SphereClass(), 0, quad_count * 2, 0, vertex_count, true);
}

void Release_Refs(SortingNodeStruct* state)
{
	int i;
	for (i=0;i<MAX_VERTEX_STREAMS;++i) {
		REF_PTR_RELEASE(state->sorting_state.vertex_buffers[i]);
	}
	REF_PTR_RELEASE(state->sorting_state.index_buffer);
	REF_PTR_RELEASE(state->sorting_state.material);
	for (i=0;i<DX8Wrapper::Get_Current_Caps()->Get_Max_Textures_Per_Pass();++i)
	{
		REF_PTR_RELEASE(state->sorting_state.Textures[i]);
	}
}

static unsigned overlapping_node_count;
static unsigned overlapping_polygon_count;
static unsigned overlapping_entry_count;
static constexpr unsigned short QUAD_ENTRY = 0xffffu;
static unsigned overlapping_vertex_count;
static const unsigned MAX_OVERLAPPING_NODES=4096;
static SortingNodeStruct* overlapping_nodes[MAX_OVERLAPPING_NODES];

// ----------------------------------------------------------------------------

void SortingRendererClass::Insert_To_Sorted_List(SortingNodeStruct *state)
{
	/// @todo lorenzen sez use a bucket sort here... and stop copying so much data so many times

	for (SortingNodeStructList::iterator node = sorted_list.begin(); node != sorted_list.end(); ++node)
	{
		if (state->transformed_center.Z > (*node)->transformed_center.Z) {
			sorted_list.insert(node, state);
			return;
		}
	}

	sorted_list.push_back(state);
}

// ----------------------------------------------------------------------------

void SortingRendererClass::Insert_To_Sorting_Pool(SortingNodeStruct* state)
{
	if (overlapping_node_count>=MAX_OVERLAPPING_NODES) {
		Release_Refs(state);
		delete state;
		WWASSERT(0);
		return;
	}

	overlapping_nodes[overlapping_node_count]=state;
	overlapping_vertex_count+=state->vertex_count;
	overlapping_polygon_count+=state->polygon_count;
	overlapping_entry_count += state->billboard_quads ? state->polygon_count / 2 : state->polygon_count;
	overlapping_node_count++;
}

// ----------------------------------------------------------------------------
//static unsigned prevLight = 0xffffffff;

static void Apply_Render_State(RenderStateStruct& render_state)
{
	DX8Wrapper::Set_Shader(render_state.shader);

	DX8Wrapper::Set_Material(render_state.material);

	for (int i=0;i<DX8Wrapper::Get_Current_Caps()->Get_Max_Textures_Per_Pass();++i)
	{
		DX8Wrapper::Set_Texture(i,render_state.Textures[i]);
	}

	DX8Wrapper::_Set_DX8_Transform(D3DTS_WORLD,render_state.world);
	DX8Wrapper::_Set_DX8_Transform(D3DTS_VIEW,render_state.view);


	if (!render_state.material->Get_Lighting())
		return;	//no point changing lights if they are ignored.
  //prevLight = render_state.lightsHash;

	if (render_state.LightEnable[0]) {
		DX8Wrapper::Set_DX8_Light(0,&render_state.Lights[0]);
		if (render_state.LightEnable[1]) {
			DX8Wrapper::Set_DX8_Light(1,&render_state.Lights[1]);
			if (render_state.LightEnable[2]) {
				DX8Wrapper::Set_DX8_Light(2,&render_state.Lights[2]);
				if (render_state.LightEnable[3]) {
					DX8Wrapper::Set_DX8_Light(3,&render_state.Lights[3]);
				}
				else {
					DX8Wrapper::Set_DX8_Light(3,nullptr);
				}
			}
			else {
				DX8Wrapper::Set_DX8_Light(2,nullptr);
			}
		}
		else {
			DX8Wrapper::Set_DX8_Light(1,nullptr);
		}
	}
	else {
		DX8Wrapper::Set_DX8_Light(0,nullptr);
	}


}

// ----------------------------------------------------------------------------

// Conservative adaptation of Reforged's state-run grouping: only adjacent
// unlit billboard entries with identical captured state can share a draw.
// Never move an entry across another depth entry.
static bool Same_Billboard_Run(const SortingNodeStruct *a, const SortingNodeStruct *b)
{
	if (a == b)
		return true;
	if (!a->billboard_quads || !b->billboard_quads)
		return false;
	const auto &x = a->sorting_state;
	const auto &y = b->sorting_state;
	if (x.shader.Get_Bits() != y.shader.Get_Bits() || x.material != y.material ||
		!x.material || x.material->Get_Lighting())
		return false;
	if (std::memcmp(&x.world, &y.world, sizeof(x.world)) ||
		std::memcmp(&x.view, &y.view, sizeof(x.view)))
		return false;
	const int textureCount = DX8Wrapper::Get_Current_Caps()->Get_Max_Textures_Per_Pass();
	for (int i = 0; i < textureCount; ++i)
	{
		if (x.Textures[i] != y.Textures[i] ||
			(x.Textures[i] && x.Textures[i]->Get_Pool() == TextureBaseClass::POOL_DEFAULT))
			return false;
	}
	return true;
}

// Render-only adaptation of Reforged's fork/join JobSystem. Workers only read
// immutable CPU vertex data and write disjoint preallocated depth entries.
// Opt in for A/B testing until the target's threading and FPU behavior are verified.
namespace
{
struct QuadDepthJob
{
	const VertexFormatXYZNDUV2 *vertices;
	TempIndexStruct *entries;
	unsigned count, nodeId, vertexOffset;
	float x, y, z, w;
};

QuadDepthJob quadDepthJobs[MAX_OVERLAPPING_NODES];

void FillQuadDepth(const QuadDepthJob &job)
{
	for (unsigned q = 0; q < job.count; ++q)
	{
		const auto *v = job.vertices + q * 4;
		auto &entry = job.entries[q];
		entry.tri = {static_cast<unsigned short>(q * 4 + job.vertexOffset), 0, QUAD_ENTRY};
		entry.idx = static_cast<unsigned short>(job.nodeId);
		entry.z = (job.x * (v[0].x + v[1].x + v[2].x + v[3].x)
			+ job.y * (v[0].y + v[1].y + v[2].y + v[3].y)
			+ job.z * (v[0].z + v[1].z + v[2].z + v[3].z)) * 0.25f + job.w;
	}
}

void CheckQuadDepth([[maybe_unused]] const QuadDepthJob &job)
{
#ifdef DEBUG_CRASHING
	for (unsigned q = 0; q < job.count; ++q)
		DEBUG_ASSERTCRASH(std::isfinite(job.entries[q].z), ("Quad has invalid center"));
#endif
}

constexpr unsigned minParallelNodes = 5;
constexpr unsigned minParallelQuads = 4096;
constexpr unsigned maxRenderWorkers = 8;

unsigned QuadDepthWorkerCount()
{
	static const unsigned workerCount = [] {
		const char *value = std::getenv("GENERALS_RENDER_WORKERS");
		if (!value)
			return 0u;
		char *end = nullptr;
		const long requested = std::strtol(value, &end, 10);
		if (end == value || *end || requested <= 0)
			return 0u;
		return static_cast<unsigned>(std::min(requested, static_cast<long>(maxRenderWorkers)));
	}();
	return workerCount;
}

class RenderQuadPool
{
	std::mutex mutex;
	std::condition_variable ready, done;
	std::array<std::thread, maxRenderWorkers> workers;
	unsigned workerCount = 0;
	std::atomic<unsigned> next{0};
	unsigned count = 0, permits = 0, busy = 0;
	bool stopping = false;
	std::fenv_t environment{};

	void fill()
	{
		for (;;)
		{
			const unsigned jobIndex = next.fetch_add(1, std::memory_order_relaxed);
			if (jobIndex >= count)
				break;
			FillQuadDepth(quadDepthJobs[jobIndex]);
		}
	}

	void worker()
	{
		for (;;)
		{
			{
				std::unique_lock<std::mutex> lock(mutex);
				ready.wait(lock, [this] { return stopping || permits != 0; });
				if (stopping)
					return;
				--permits;
				++busy;
			}
			// Each fork inherits the render caller's floating-point environment.
			// If restoration fails, leave the work for the caller/other workers.
			if (std::fesetenv(&environment) == 0)
				fill();
			{
				std::lock_guard<std::mutex> lock(mutex);
				--busy;
			}
			done.notify_one();
		}
	}

public:
	explicit RenderQuadPool(unsigned requested)
	{
		for (; workerCount < requested; ++workerCount)
		{
			try
			{
				workers[workerCount] = std::thread([this] { worker(); });
			}
			catch (const std::system_error &)
			{
				break;
			}
			catch (const std::bad_alloc &)
			{
				break;
			}
		}
	}

	~RenderQuadPool()
	{
		{
			std::lock_guard<std::mutex> lock(mutex);
			stopping = true;
		}
		ready.notify_all();
		for (unsigned i = 0; i < workerCount; ++i)
			workers[i].join();
	}

	void run(unsigned jobCount)
	{
		if (workerCount == 0 || std::fegetenv(&environment) != 0)
		{
			for (unsigned i = 0; i < jobCount; ++i)
				FillQuadDepth(quadDepthJobs[i]);
			return;
		}
		{
			std::lock_guard<std::mutex> lock(mutex);
			count = jobCount;
			next.store(0, std::memory_order_relaxed);
			permits = workerCount;
		}
		ready.notify_all();
		fill();
		{
			std::unique_lock<std::mutex> lock(mutex);
			permits = 0;
			done.wait(lock, [this] { return busy == 0; });
		}
	}
};

std::optional<RenderQuadPool> quadDepthPool;

void RunQuadDepthJobs(unsigned jobCount, unsigned quadCount)
{
	if (jobCount >= minParallelNodes && quadCount >= minParallelQuads)
	{
		if (!quadDepthPool)
			quadDepthPool.emplace(QuadDepthWorkerCount());
		quadDepthPool->run(jobCount);
	}
	else
	{
		for (unsigned j = 0; j < jobCount; ++j)
			FillQuadDepth(quadDepthJobs[j]);
	}

	for (unsigned j = 0; j < jobCount; ++j)
		CheckQuadDepth(quadDepthJobs[j]);
}
} // namespace

void SortingRendererClass::Flush_Sorting_Pool()
{
	if (!overlapping_node_count) return;

	SNAPSHOT_SAY(("SortingSystem - Flush"));

	// Fill dynamic index buffer with sorting index buffer vertices
	TempIndexStruct* tis=Get_Temp_Index_Array(overlapping_entry_count);

	unsigned vertexAllocCount = overlapping_vertex_count;
	if (DynamicVBAccessClass::Get_Default_Vertex_Count() < DEFAULT_SORTING_VERTEX_COUNT)
		vertexAllocCount = DEFAULT_SORTING_VERTEX_COUNT;	//make sure that we force the DX8 dynamic vertex buffer to maximum size
	if (overlapping_vertex_count > vertexAllocCount)
		vertexAllocCount = overlapping_vertex_count;
	// Reforged (Olcay Seygan and contributors): sort globally, then stream geometry
	// in 16-bit-safe batches. Keep the original shared-vertex fast path for small pools.
	const bool streamed = vertexAllocCount > 65535u ||
		(DEFAULT_SORTING_VERTEX_COUNT != 1 && vertexAllocCount > DEFAULT_SORTING_VERTEX_COUNT);
	std::optional<DynamicVBAccessClass> dyn_vb_access;
	std::vector<const VertexFormatXYZNDUV2 *> nodeVertices;
	if (streamed)
		nodeVertices.resize(overlapping_node_count);
	else
		dyn_vb_access.emplace(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, static_cast<unsigned short>(vertexAllocCount));
	{
		std::optional<DynamicVBAccessClass::WriteLockClass> lock;
		VertexFormatXYZNDUV2 *dest_verts = nullptr;
		if (!streamed)
		{
			lock.emplace(&*dyn_vb_access);
			dest_verts = static_cast<VertexFormatXYZNDUV2 *>(lock->Get_Formatted_Vertex_Array());
		}

		const bool collectQuadDepthJobs = overlapping_node_count >= minParallelNodes
			&& overlapping_entry_count >= minParallelQuads && QuadDepthWorkerCount() != 0;
		unsigned quadJobCount = 0;
		unsigned queuedQuadCount = 0;
		unsigned polygon_array_offset=0;
		unsigned vertex_array_offset=0;
		for (unsigned node_id=0;node_id<overlapping_node_count;++node_id) {
			SortingNodeStruct* state=overlapping_nodes[node_id];
			VertexFormatXYZNDUV2* src_verts=nullptr;
			SortingVertexBufferClass* vertex_buffer=static_cast<SortingVertexBufferClass*>(state->sorting_state.vertex_buffers[0]);
			WWASSERT(vertex_buffer);
			src_verts=vertex_buffer->VertexBuffer;
			WWASSERT(src_verts);
			src_verts+=state->sorting_state.vba_offset;
			src_verts+=state->sorting_state.index_base_offset;
			src_verts+=state->min_vertex_index;

			// If you have a crash in here and "dest_verts" points to illegal memory area,
			// it is because D3D is in illegal state, and the only known cure is rebooting.
			// This illegal state is usually caused by Quake3-engine powered games such as MOHAA.
			if (streamed)
				nodeVertices[node_id] = src_verts;
			else
			{
				memcpy(dest_verts, src_verts, sizeof(VertexFormatXYZNDUV2)*state->vertex_count);
				dest_verts += state->vertex_count;
			}

			const Matrix4x4 mtx=To_Matrix4x4(state->sorting_state.view)*To_Matrix4x4(state->sorting_state.world);

			// Adapted from Reforged (Olcay Seygan and contributors): one depth entry per billboard quad.
			if (state->billboard_quads)
			{
				const unsigned quadCount = state->polygon_count / 2u;
				const QuadDepthJob job{src_verts, tis + polygon_array_offset, quadCount,
					node_id, streamed ? 0u : vertex_array_offset,
					mtx[2][0], mtx[2][1], mtx[2][2], mtx[2][3]};
				if (collectQuadDepthJobs)
				{
					quadDepthJobs[quadJobCount++] = job;
					queuedQuadCount += quadCount;
				}
				else
				{
					FillQuadDepth(job);
					CheckQuadDepth(job);
				}
				if (!streamed)
					state->min_vertex_index = vertex_array_offset;
				polygon_array_offset += quadCount;
				vertex_array_offset += state->vertex_count;
				continue;
			}

			unsigned short* indices=nullptr;
			SortingIndexBufferClass* index_buffer=static_cast<SortingIndexBufferClass*>(state->sorting_state.index_buffer);
			WWASSERT(index_buffer);
			indices=index_buffer->index_buffer;
			WWASSERT(indices);
			indices+=state->start_index;
			indices+=state->sorting_state.iba_offset;

			if (mtx[2][0] == 0.0f && mtx[2][1] == 0.0f && mtx[2][3] == 0.0f && mtx[2][2] == 1.0f) {
				// The common case for particle systems.
				for (int i=0;i<state->polygon_count;++i) {
					unsigned short idx1=indices[i*3]-state->min_vertex_index;
					unsigned short idx2=indices[i*3+1]-state->min_vertex_index;
					unsigned short idx3=indices[i*3+2]-state->min_vertex_index;
					WWASSERT(idx1<state->vertex_count);
					WWASSERT(idx2<state->vertex_count);
					WWASSERT(idx3<state->vertex_count);
					const VertexFormatXYZNDUV2 *v1 = src_verts + idx1;
					const VertexFormatXYZNDUV2 *v2 = src_verts + idx2;
					const VertexFormatXYZNDUV2 *v3 = src_verts + idx3;
					unsigned array_index=i+polygon_array_offset;
					WWASSERT(array_index<overlapping_entry_count);
					TempIndexStruct *tis_ptr = tis + array_index;
					tis_ptr->tri.i = idx1 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->tri.j = idx2 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->tri.k = idx3 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->idx = node_id;
					tis_ptr->z = (v1->z + v2->z + v3->z)/3.0f;
					DEBUG_ASSERTCRASH((! std::isnan(tis_ptr->z) && std::isfinite(tis_ptr->z)), ("Triangle has invalid center"));
				}
			} else {
				for (int i=0;i<state->polygon_count;++i) {
					unsigned short idx1=indices[i*3]-state->min_vertex_index;
					unsigned short idx2=indices[i*3+1]-state->min_vertex_index;
					unsigned short idx3=indices[i*3+2]-state->min_vertex_index;
					WWASSERT(idx1<state->vertex_count);
					WWASSERT(idx2<state->vertex_count);
					WWASSERT(idx3<state->vertex_count);
					const VertexFormatXYZNDUV2 *v1 = src_verts + idx1;
					const VertexFormatXYZNDUV2 *v2 = src_verts + idx2;
					const VertexFormatXYZNDUV2 *v3 = src_verts + idx3;
					unsigned array_index=i+polygon_array_offset;
					WWASSERT(array_index<overlapping_entry_count);
					TempIndexStruct *tis_ptr = tis + array_index;
					tis_ptr->tri.i = idx1 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->tri.j = idx2 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->tri.k = idx3 + (streamed ? 0u : vertex_array_offset);
					tis_ptr->idx = node_id;
					tis_ptr->z = (mtx[2][0]*(v1->x + v2->x + v3->x) +
												mtx[2][1]*(v1->y + v2->y + v3->y) +
												mtx[2][2]*(v1->z + v2->z + v3->z))/3.0f + mtx[2][3];
					DEBUG_ASSERTCRASH((! std::isnan(tis_ptr->z) && std::isfinite(tis_ptr->z)), ("Triangle has invalid center"));
				}
			}

			if (!streamed)
				state->min_vertex_index=vertex_array_offset;

			polygon_array_offset+=state->polygon_count;
			vertex_array_offset+=state->vertex_count;
		}
		if (quadJobCount != 0)
		{
			// Only CPU sorting pages are accessed by workers. Release the GPU lock before waking them.
			lock.reset();
			RunQuadDepthJobs(quadJobCount, queuedQuadCount);
		}
	}

	tis = Sort_By_Depth(tis, overlapping_entry_count);

	// Keep quads intact across both 16-bit index and vertex upload boundaries.
	const unsigned vertexBudget = streamed && DEFAULT_SORTING_VERTEX_COUNT >= 4 && DEFAULT_SORTING_VERTEX_COUNT <= 65535u
		? DEFAULT_SORTING_VERTEX_COUNT : 65535u;
	unsigned chunkOffset = 0;
	while (chunkOffset < overlapping_entry_count)
	{
		unsigned chunkEnd = chunkOffset, indexCount = 0, vertexCount = 0;
		while (chunkEnd < overlapping_entry_count)
		{
			const bool quad = tis[chunkEnd].tri.k == QUAD_ENTRY;
			const unsigned nextIndices = quad ? 6 : 3;
			const unsigned nextVertices = quad ? 4 : 3;
			if (indexCount + nextIndices > 65535u || (streamed && vertexCount + nextVertices > vertexBudget))
				break;
			indexCount += nextIndices;
			vertexCount += nextVertices;
			++chunkEnd;
		}
		WWASSERT(chunkEnd > chunkOffset);

		std::optional<DynamicVBAccessClass> batchVb;
		if (streamed)
		{
			batchVb.emplace(BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, static_cast<unsigned short>(vertexCount));
			DynamicVBAccessClass::WriteLockClass lock(&*batchVb);
			auto *vertices = lock.Get_Formatted_Vertex_Array();
			unsigned cursor = 0;
			for (unsigned a = chunkOffset; a < chunkEnd; ++a)
			{
				const auto &entry = tis[a];
				const auto *src = nodeVertices[entry.idx];
				if (entry.tri.k == QUAD_ENTRY)
				{
					for (unsigned v = 0; v < 4; ++v)
						vertices[cursor++] = src[entry.tri.i + v];
				}
				else
				{
					vertices[cursor++] = src[entry.tri.i];
					vertices[cursor++] = src[entry.tri.j];
					vertices[cursor++] = src[entry.tri.k];
				}
			}
		}

		DynamicIBAccessClass dyn_ib_access(BUFFER_TYPE_DYNAMIC_DX8, static_cast<unsigned short>(indexCount));
		{
			DynamicIBAccessClass::WriteLockClass lock(&dyn_ib_access);
			auto *indices = lock.Get_Index_Array();
			unsigned vertexCursor = 0;
			for (unsigned a = chunkOffset; a < chunkEnd; ++a)
			{
				const auto &entry = tis[a];
				const bool quad = entry.tri.k == QUAD_ENTRY;
				if (quad)
				{
					const unsigned first = streamed ? vertexCursor : entry.tri.i;
					for (unsigned offset : {0u, 1u, 2u, 2u, 3u, 0u})
						*indices++ = static_cast<unsigned short>(first + offset);
				}
				else
				{
					*indices++ = static_cast<unsigned short>(streamed ? vertexCursor : entry.tri.i);
					*indices++ = static_cast<unsigned short>(streamed ? vertexCursor + 1 : entry.tri.j);
					*indices++ = static_cast<unsigned short>(streamed ? vertexCursor + 2 : entry.tri.k);
				}
				vertexCursor += quad ? 4 : 3;
			}
		}

		DX8Wrapper::Set_Index_Buffer(dyn_ib_access, 0);
		DX8Wrapper::Set_Vertex_Buffer(streamed ? *batchVb : *dyn_vb_access);
		DX8Wrapper::Apply_Render_State_Changes();

		unsigned a = chunkOffset, indexCursor = 0, vertexCursor = 0;
		while (a < chunkEnd)
		{
			const unsigned nodeId = tis[a].idx;
			const unsigned firstIndex = indexCursor;
			const unsigned firstVertex = vertexCursor;
			SortingNodeStruct *state = overlapping_nodes[nodeId];
			unsigned minVertex = state->min_vertex_index;
			unsigned maxVertex = minVertex + state->vertex_count;
			do
			{
				const bool quad = tis[a].tri.k == QUAD_ENTRY;
				if (!streamed)
				{
					const auto *entryNode = overlapping_nodes[tis[a].idx];
					minVertex = std::min(minVertex, static_cast<unsigned>(entryNode->min_vertex_index));
					maxVertex = std::max(maxVertex, static_cast<unsigned>(entryNode->min_vertex_index) + entryNode->vertex_count);
				}
				indexCursor += quad ? 6 : 3;
				vertexCursor += quad ? 4 : 3;
				++a;
			} while (a < chunkEnd && Same_Billboard_Run(state, overlapping_nodes[tis[a].idx]));
			Apply_Render_State(state->sorting_state);
			DX8Wrapper::Draw_Triangles(firstIndex, (indexCursor - firstIndex) / 3,
				streamed ? firstVertex : minVertex,
				streamed ? vertexCursor - firstVertex : maxVertex - minVertex);
		}
		chunkOffset = chunkEnd;
	}

	// Release all references and return nodes back to the clean list for the frame...
	for (unsigned node_id=0;node_id<overlapping_node_count;++node_id) {
		SortingNodeStruct* state=overlapping_nodes[node_id];
		Release_Refs(state);
		clean_list.push_front(state);
	}
	overlapping_node_count=0;
	overlapping_polygon_count=0;
	overlapping_entry_count=0;
	overlapping_vertex_count=0;

	SNAPSHOT_SAY(("SortingSystem - Done flushing"));

}

// ----------------------------------------------------------------------------

void SortingRendererClass::Flush()
{
	WWPROFILE("SortingRenderer::Flush");
	Matrix4x4 old_view;
	Matrix4x4 old_world;
	Renderer::Get_Transform(RenderTransform_View,old_view);
	Renderer::Get_Transform(RenderTransform_World,old_world);

	// TheSuperHackers @perf stephanmeesters 04/07/2026
	// Splice nodes that have no bounding information (Z=0.0) at the correct location into the sorted list.
	SortingNodeStructList::iterator node = sorted_list.begin();
	while (node != sorted_list.end() && (*node)->transformed_center.Z > 0.0f) {
		++node;
	}
	sorted_list.splice(node, unsorted_list);

	while (!sorted_list.empty()) {
		SortingNodeStruct* state = sorted_list.front();
		sorted_list.pop_front();

		if ((state->sorting_state.index_buffer_type==BUFFER_TYPE_SORTING || state->sorting_state.index_buffer_type==BUFFER_TYPE_DYNAMIC_SORTING) &&
			(state->sorting_state.vertex_buffer_types[0]==BUFFER_TYPE_SORTING || state->sorting_state.vertex_buffer_types[0]==BUFFER_TYPE_DYNAMIC_SORTING)) {
			Insert_To_Sorting_Pool(state);
		}
		else {
			DX8Wrapper::Set_Render_State(state->sorting_state);
			DX8Wrapper::Draw_Triangles(state->start_index,state->polygon_count,state->min_vertex_index,state->vertex_count);
			DX8Wrapper::Release_Render_State();
			Release_Refs(state);
			clean_list.push_front(state);
		}
	}

	bool old_enable=DX8Wrapper::_Is_Triangle_Draw_Enabled();
	DX8Wrapper::_Enable_Triangle_Draw(_EnableTriangleDraw);
	Flush_Sorting_Pool();
	DX8Wrapper::_Enable_Triangle_Draw(old_enable);

	DX8Wrapper::Set_Index_Buffer(nullptr,0);
	DX8Wrapper::Set_Vertex_Buffer(nullptr);
	total_sorting_vertices=0;

	DynamicIBAccessClass::_Reset(false);
	DynamicVBAccessClass::_Reset(false);


	Renderer::Set_Transform(RenderTransform_View,old_view);
	Renderer::Set_Transform(RenderTransform_World,old_world);

}

// ----------------------------------------------------------------------------

void SortingRendererClass::Deinit()
{
	quadDepthPool.reset();

	//
	//	Flush the sorted list
	//
	while (!sorted_list.empty()) {
		delete sorted_list.front();
		sorted_list.pop_front();
	}

	//
	//	Flush the unsorted list
	//
	while (!unsorted_list.empty()) {
		delete unsorted_list.front();
		unsorted_list.pop_front();
	}

	//
	//	Flush the clean list
	//
	while (!clean_list.empty()) {
		delete clean_list.front();
		clean_list.pop_front();
	}

	delete[] temp_index_array;
	temp_index_array=nullptr;
	temp_index_array_count=0;
	std::vector<TempIndexStruct>().swap(depth_sort_scratch);
}
