/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
*/

#include "W3DDevice/GameClient/WaterGridSimulation.h"

#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"

#include <algorithm>
#include <cmath>

WaterGridSimulation::WaterGridSimulation() :
	m_enabled(FALSE),
	m_surfaceOverride(FALSE),
	m_inMotion(FALSE),
	m_gridDirectionX(1.0f, 0.0f),
	m_gridDirectionY(1.0f, 0.0f),
	m_gridOrigin(0.0f, 0.0f),
	m_transform(1),
	m_minGridHeight(0.0f),
	m_maxGridHeight(0.0f),
	m_gridChangeMaxRange(0.0f),
	m_gridChangeAtt0(0.0f),
	m_gridChangeAtt1(0.0f),
	m_gridChangeAtt2(0.0f),
	m_gridCellSize(10.0f),
	m_gridCellsX(128),
	m_gridCellsY(128)
{
}

WaterGridSimulation::~WaterGridSimulation() = default;

void WaterGridSimulation::resizeSamples()
{
	const std::size_t width = static_cast<std::size_t>(m_gridCellsX + 3);
	const std::size_t height = static_cast<std::size_t>(m_gridCellsY + 3);
	m_samples.assign(width * height, WaterGridSample());
}

std::size_t WaterGridSimulation::sampleIndex(Int x, Int y) const
{
	return static_cast<std::size_t>(y + 1) *
		static_cast<std::size_t>(m_gridCellsX + 3) +
		static_cast<std::size_t>(x + 1);
}

void WaterGridSimulation::Set_Enabled(Bool enabled)
{
	m_enabled = enabled;
	m_surfaceOverride = FALSE;
	if (enabled && m_samples.empty())
	{
		resizeSamples();
		Reset();
	}
}

void WaterGridSimulation::Reset()
{
	for (WaterGridSample &sample : m_samples)
	{
		sample.velocity = 0.0f;
		sample.height = 0.0f;
		sample.preferredHeight = 0.0f;
		sample.status = AT_REST;
	}
	m_inMotion = FALSE;
	m_surfaceOverride = FALSE;
}

void WaterGridSimulation::Update()
{
	if (!m_enabled || !m_inMotion)
		return;

	constexpr Real preferred_height_fudge = 1.0f;
	constexpr Real at_rest_velocity_fudge = 1.0f;
	constexpr Real water_dampening = 0.93f;
	m_inMotion = FALSE;

	for (WaterGridSample &sample : m_samples)
	{
		if ((sample.status & IN_MOTION) == 0)
			continue;

		sample.velocity *= water_dampening;
		if (sample.height < sample.preferredHeight)
			sample.velocity -= TheGlobalData->m_gravity * 3.0f;
		else
			sample.velocity += TheGlobalData->m_gravity * 3.0f;
		sample.height += sample.velocity;

		if (std::fabs(sample.height - sample.preferredHeight) <
			preferred_height_fudge &&
			std::fabs(sample.velocity) < at_rest_velocity_fudge)
		{
			sample.status &= static_cast<UnsignedByte>(~IN_MOTION);
			sample.height = sample.preferredHeight;
			sample.velocity = 0.0f;
		}
		else
		{
			m_inMotion = TRUE;
		}
	}
}

void WaterGridSimulation::Set_Height_Clamps(Real min_height, Real max_height)
{
	m_minGridHeight = min_height;
	m_maxGridHeight = max_height;
}

void WaterGridSimulation::Set_Change_Attenuation(Real a, Real b, Real c,
	Real range)
{
	m_gridChangeAtt0 = a;
	m_gridChangeAtt1 = b;
	m_gridChangeAtt2 = c;
	m_gridChangeMaxRange = range / m_gridCellSize;
}

Bool WaterGridSimulation::World_To_Grid(Real world_x, Real world_y,
	Real &grid_x, Real &grid_y) const
{
	if (m_gridCellSize == 0.0f)
		return FALSE;

	const Real dx = world_x - m_gridOrigin.X;
	const Real dy = world_y - m_gridOrigin.Y;
	const Real inverse_cell_size = 1.0f / m_gridCellSize;
	grid_x = inverse_cell_size *
		(dx * m_gridDirectionX.X + dy * m_gridDirectionX.Y);
	grid_y = inverse_cell_size *
		(dx * m_gridDirectionY.X + dy * m_gridDirectionY.Y);

	return grid_x >= 0.0f && grid_x <= m_gridCellsX - 1 &&
		grid_y >= 0.0f && grid_y <= m_gridCellsY - 1;
}

void WaterGridSimulation::Add_Velocity(Real world_x, Real world_y,
	Real velocity, Real preferred_height)
{
	if (!m_enabled)
		return;

	Real grid_x = 0.0f;
	Real grid_y = 0.0f;
	if (!World_To_Grid(world_x, world_y, grid_x, grid_y))
		return;

	const Int min_x = std::max<Int>(0,
		static_cast<Int>(std::floor(grid_x - m_gridChangeMaxRange)));
	const Int max_x = std::min<Int>(m_gridCellsX,
		static_cast<Int>(std::ceil(grid_x + m_gridChangeMaxRange)));
	const Int min_y = std::max<Int>(0,
		static_cast<Int>(std::floor(grid_y - m_gridChangeMaxRange)));
	const Int max_y = std::min<Int>(m_gridCellsY,
		static_cast<Int>(std::ceil(grid_y + m_gridChangeMaxRange)));

	for (Int y = min_y; y <= max_y; ++y)
	{
		for (Int x = min_x; x <= max_x; ++x)
		{
			WaterGridSample &sample = m_samples[sampleIndex(x, y)];
			sample.preferredHeight = static_cast<UnsignedByte>(preferred_height);
			sample.velocity += velocity;
			sample.status |= IN_MOTION;
		}
	}
	m_inMotion = TRUE;
	m_surfaceOverride = TRUE;
}

void WaterGridSimulation::Change_Height(Real world_x, Real world_y,
	Real delta)
{
	Real grid_x = 0.0f;
	Real grid_y = 0.0f;
	if (!World_To_Grid(world_x, world_y, grid_x, grid_y))
		return;

	const Int min_x = std::max<Int>(0,
		static_cast<Int>(std::floor(grid_x - m_gridChangeMaxRange)));
	const Int max_x = std::min<Int>(m_gridCellsX,
		static_cast<Int>(std::ceil(grid_x + m_gridChangeMaxRange)));
	const Int min_y = std::max<Int>(0,
		static_cast<Int>(std::floor(grid_y - m_gridChangeMaxRange)));
	const Int max_y = std::min<Int>(m_gridCellsY,
		static_cast<Int>(std::ceil(grid_y + m_gridChangeMaxRange)));

	for (Int y = min_y; y <= max_y; ++y)
	{
		for (Int x = min_x; x <= max_x; ++x)
		{
			WaterGridSample &sample = m_samples[sampleIndex(x, y)];
			const Real dx = grid_x - static_cast<Real>(x);
			const Real dy = grid_y - static_cast<Real>(y);
			const Real distance = std::sqrt(dx * dx + dy * dy);
			const Real denominator = m_gridChangeAtt0 +
				m_gridChangeAtt1 * distance +
				distance * distance * m_gridChangeAtt2;
			if (denominator == 0.0f)
				continue;
			sample.height += delta / denominator;
			sample.height = std::max(m_minGridHeight,
			std::min(m_maxGridHeight, sample.height));
		}
	}
}

void WaterGridSimulation::Set_Transform(Real angle, Real x, Real y, Real z)
{
	Matrix3D transform(1);
	transform.Rotate_Z(angle);
	transform.Set_Translation(Vector3(x, y, z));
	Set_Transform(transform);
}

void WaterGridSimulation::Set_Transform(const Matrix3D &transform)
{
	m_transform = transform;
	m_gridOrigin.X = transform.Get_X_Translation();
	m_gridOrigin.Y = transform.Get_Y_Translation();
	m_gridDirectionX.X = transform.Get_X_Vector().X;
	m_gridDirectionX.Y = transform.Get_X_Vector().Y;
	m_gridDirectionY.X = transform.Get_Y_Vector().X;
	m_gridDirectionY.Y = transform.Get_Y_Vector().Y;
}

void WaterGridSimulation::Set_Resolution(Real cells_x, Real cells_y,
	Real cell_size)
{
	const Int new_cells_x = static_cast<Int>(cells_x);
	const Int new_cells_y = static_cast<Int>(cells_y);
	m_gridCellSize = cell_size;
	if (m_gridCellsX == new_cells_x && m_gridCellsY == new_cells_y)
		return;

	m_gridCellsX = new_cells_x;
	m_gridCellsY = new_cells_y;
	if (m_enabled)
	{
		resizeSamples();
		Reset();
	}
}

void WaterGridSimulation::Get_Resolution(Real *cells_x, Real *cells_y,
	Real *cell_size) const
{
	if (cells_x != nullptr)
		*cells_x = m_gridCellsX;
	if (cells_y != nullptr)
		*cells_y = m_gridCellsY;
	if (cell_size != nullptr)
		*cell_size = m_gridCellSize;
}

void WaterGridSimulation::Build_Render_Data(WaterGridRenderData *data) const
{
	if (data == nullptr)
		return;

	data->enabled = m_enabled;
	data->surface_override = m_surfaceOverride;
	data->cells_x = m_gridCellsX;
	data->cells_y = m_gridCellsY;
	data->cell_size = m_gridCellSize;
	data->transform = m_transform;
	data->heights.resize(m_samples.size());
	for (std::size_t i = 0; i < m_samples.size(); ++i)
		data->heights[i] = static_cast<float>(m_samples[i].height);
}

void WaterGridSimulation::Set_Vertex_Height(Int x, Int y, Real value)
{
	if (x < 0 || x > m_gridCellsX || y < 0 || y > m_gridCellsY ||
		m_samples.empty())
		return;
	m_samples[sampleIndex(x, y)].height = value;
}

Real WaterGridSimulation::Get_Vertex_Height(Int x, Int y) const
{
	if (x < 0 || x > m_gridCellsX || y < 0 || y > m_gridCellsY ||
		m_samples.empty())
		return 0.0f;
	return m_samples[sampleIndex(x, y)].height +
		m_transform.Get_Z_Translation();
}

void WaterGridSimulation::crc(Xfer *xfer)
{
}

void WaterGridSimulation::xfer(Xfer *xfer)
{
	XferVersion current_version = 1;
	XferVersion version = current_version;
	xfer->xferVersion(&version, current_version);

	Int cells_x = m_gridCellsX;
	xfer->xferInt(&cells_x);
	if (cells_x != m_gridCellsX)
	{
		DEBUG_CRASH(("WaterGridSimulation::xfer - cells X mismatch"));
		throw SC_INVALID_DATA;
	}

	Int cells_y = m_gridCellsY;
	xfer->xferInt(&cells_y);
	if (cells_y != m_gridCellsY)
	{
		DEBUG_CRASH(("WaterGridSimulation::xfer - cells Y mismatch"));
		throw SC_INVALID_DATA;
	}

	if (m_samples.empty() && m_enabled)
		resizeSamples();
	for (WaterGridSample &sample : m_samples)
	{
		xfer->xferReal(&sample.height);
		xfer->xferReal(&sample.velocity);
		xfer->xferUnsignedByte(&sample.status);
		xfer->xferUnsignedByte(&sample.preferredHeight);
	}
}

void WaterGridSimulation::loadPostProcess()
{
}
