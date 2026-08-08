/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// TheSuperHackers @refactor bobtista 10/04/2026 Render backend global owner.
// Holds the single g_renderBackend pointer and constructs/destroys the
// concrete backend instance.

#include "RenderBackend.h"
#include "DX8Backend.h"

#include "WWDebug/wwdebug.h"

IRenderBackend * g_renderBackend = nullptr;

void Init_Render_Backend()
{
    // Construction is single shot. WW3D::Init pairs with WW3D::Shutdown, which
    // destroys the backend again, so arriving here with a live backend means the
    // library lifecycle is unbalanced.
    WWASSERT(g_renderBackend == nullptr);
    if (g_renderBackend != nullptr)
    {
        return;
    }

    g_renderBackend = new DX8Backend();
}

void Shutdown_Render_Backend()
{
    if (g_renderBackend == nullptr)
    {
        return;
    }
    delete g_renderBackend;
    g_renderBackend = nullptr;
}
