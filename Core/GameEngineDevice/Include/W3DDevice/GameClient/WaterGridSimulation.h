/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	Gameplay-owned water-grid simulation. This state is intentionally
**	independent of scene objects, textures, and render backends.
*/

#pragma once

#include <vector>

#include "WWLib/always.h"
#include "WWMath/matrix3d.h"
#include "WWMath/vector2.h"
#include "Common/Snapshot.h"
#include "W3DDevice/GameClient/WaterGeometry.h"

class Xfer;

struct WaterGridSample
{
	Real height = 0.0f;
	Real velocity = 0.0f;
	UnsignedByte status = 0;
	UnsignedByte preferredHeight = 0;
};

class WaterGridSimulation final : public Snapshot
{
public:
	enum SampleStatus
	{
		AT_REST = 0x00,
		IN_MOTION = 0x01
	};

	WaterGridSimulation();
	~WaterGridSimulation();

	void Reset();
	void Update();

	void Set_Enabled(Bool enabled);
	Bool Is_Enabled() const { return m_enabled; }
	void Set_Surface_Override(Bool enabled) { m_surfaceOverride = enabled; }
	Bool Is_Surface_Override() const { return m_surfaceOverride; }

	void Set_Height_Clamps(Real min_height, Real max_height);
	void Set_Change_Attenuation(Real a, Real b, Real c, Real range);
	void Add_Velocity(Real world_x, Real world_y, Real velocity,
		Real preferred_height);
	void Change_Height(Real world_x, Real world_y, Real delta);

	void Set_Transform(Real angle, Real x, Real y, Real z);
	void Set_Transform(const Matrix3D &transform);
	const Matrix3D &Get_Transform() const { return m_transform; }

	void Set_Resolution(Real cells_x, Real cells_y, Real cell_size);
	void Get_Resolution(Real *cells_x, Real *cells_y, Real *cell_size) const;
	Bool World_To_Grid(Real world_x, Real world_y, Real &grid_x,
		Real &grid_y) const;
	void Build_Render_Data(WaterGridRenderData *data) const;

	void Set_Vertex_Height(Int x, Int y, Real value);
	Real Get_Vertex_Height(Int x, Int y) const;

	Int Get_Cells_X() const { return m_gridCellsX; }
	Int Get_Cells_Y() const { return m_gridCellsY; }
	Real Get_Cell_Size() const { return m_gridCellSize; }
	const WaterGridSample *Get_Samples() const { return m_samples.data(); }
	UnsignedInt Get_Sample_Count() const
	{
		return static_cast<UnsignedInt>(m_samples.size());
	}

protected:
	virtual void crc(Xfer *xfer) override;
	virtual void xfer(Xfer *xfer) override;
	virtual void loadPostProcess() override;

private:
	void resizeSamples();
	std::size_t sampleIndex(Int x, Int y) const;

	std::vector<WaterGridSample> m_samples;
	Bool m_enabled;
	Bool m_surfaceOverride;
	Bool m_inMotion;
	Vector2 m_gridDirectionX;
	Vector2 m_gridDirectionY;
	Vector2 m_gridOrigin;
	Matrix3D m_transform;
	Real m_minGridHeight;
	Real m_maxGridHeight;
	Real m_gridChangeMaxRange;
	Real m_gridChangeAtt0;
	Real m_gridChangeAtt1;
	Real m_gridChangeAtt2;
	Real m_gridCellSize;
	Int m_gridCellsX;
	Int m_gridCellsY;
};
