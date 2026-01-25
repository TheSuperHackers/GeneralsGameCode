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

// ObjectPreview.cpp : implementation file
//

#include <rinfo.h>
#include <camera.h>
#include <light.h>

#include "StdAfx.h"
#include "resource.h"

#include "Lib/BaseType.h"

#include "ObjectPreview.h"
#include "WorldBuilderDoc.h"
#include "WHeightMapEdit.h"
#include "ObjectOptions.h"
#include "addplayerdialog.h"
#include "CUndoable.h"
#include "wbview3d.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "Common/PlayerTemplate.h"
#include "Common/FileSystem.h"
#include "GameLogic/SidesList.h"
#include "GameClient/Color.h"

#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "WW3D2/dx8wrapper.h"
#include "WWLib/TARGA.h"

/////////////////////////////////////////////////////////////////////////////
// ObjectPreview

ObjectPreview::ObjectPreview()
{
	m_tTempl = nullptr;
}

ObjectPreview::~ObjectPreview()
{
}

void ObjectPreview::SetThingTemplate(const ThingTemplate *tTempl)
{
	m_tTempl = tTempl;
}


BEGIN_MESSAGE_MAP(ObjectPreview, CWnd)
	//{{AFX_MSG_MAP(ObjectPreview)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define SWATCH_OFFSET 20

#define PREVIEW_WIDTH 128
#define PREVIEW_HEIGHT 128

static UnsignedByte * saveSurface(IDirect3DSurface8 *surface)
{
	D3DSURFACE_DESC desc;
	IDirect3DSurface8 *tempSurface;

	surface->GetDesc(&desc);

	LPDIRECT3DDEVICE8 m_pDev=DX8Wrapper::_Get_D3D_Device8();

	HRESULT hr=m_pDev->CreateImageSurface(  desc.Width,desc.Height,desc.Format, &tempSurface);

	hr=m_pDev->CopyRects(surface,nullptr,0,tempSurface,nullptr);

	D3DLOCKED_RECT lrect;

	DX8_ErrorCode(tempSurface->LockRect(&lrect,nullptr,D3DLOCK_READONLY));

	unsigned int x,y,index,index2,width,height;

	width=desc.Width;
	height=desc.Height;

#ifdef CAPTURE_TO_TARGA
	char image[3*PREVIEW_WIDTH*PREVIEW_HEIGHT];
	//bytes are mixed in targa files, not rgb order.
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			// index for image
			index=3*(x+y*width);
			// index for fb
			index2=y*lrect.Pitch+4*x;

			image[index]=*((char *) lrect.pBits + index2+2);
			image[index+1]=*((char *) lrect.pBits + index2+1);
			image[index+2]=*((char *) lrect.pBits + index2+0);
		}
	}

	Targa targ;
	memset(&targ.Header,0,sizeof(targ.Header));
	targ.Header.Width=width;
	targ.Header.Height=height;
	targ.Header.PixelDepth=24;
	targ.Header.ImageType=TGA_TRUECOLOR;
	targ.SetImage(image);
	targ.YFlip();

	targ.Save("ObjectPreview.tga",TGAF_IMAGE,false);

	return nullptr;

#else

	static UnsignedByte bgraImage[3*PREVIEW_WIDTH*PREVIEW_HEIGHT];
	//bmp is same byte order
	for (y=0; y<height; y++)
	{
		for (x=0; x<width; x++)
		{
			// index for image
			index=3*(x+y*width);
			// index for fb
			index2=y*lrect.Pitch+4*x;

			bgraImage[index]=*((UnsignedByte *) lrect.pBits + index2+0);
			bgraImage[index+1]=*((UnsignedByte *) lrect.pBits + index2+1);
			bgraImage[index+2]=*((UnsignedByte *) lrect.pBits + index2+2);
			//bgraImage[index+3]=0;
		}
	}

	//Flip the image
	UnsignedByte *ptr,*ptr1;
	UnsignedByte  v,v1;

	for (y = 0; y < (height >> 1); y++)
	{
		/* Compute address of lines to exchange. */
		ptr = (bgraImage + ((width * y) * 3));
		ptr1 = (bgraImage + ((width * (height - 1)) * 3));
		ptr1 -= ((width * y) * 3);

		/* Exchange all the pixels on this scan line. */
		for (x = 0; x < (width * 3); x++)
			{
			v = *ptr;
			v1 = *ptr1;
			*ptr = v1;
			*ptr1 = v;
			ptr++;
			ptr1++;
			}
	}

	tempSurface->Release();

	return bgraImage;
#endif
}

// return an array of BGRA pixels
static UnsignedByte * generatePreview( const ThingTemplate *tt )
{
	// find the default model to preview
	RenderObjClass *model = nullptr;
	Real scale = 1.0f;
	AsciiString modelName = "No Model Name";
	if (tt)
	{
		ModelConditionFlags state;
		state.clear();
		WbView3d *p3View = CWorldBuilderDoc::GetActiveDoc()->GetActive3DView();
		modelName = p3View->getBestModelName(tt, state);
		scale = tt->getAssetScale();
	}
	// set render object, or create if we need to
	if( modelName.isEmpty() == FALSE &&
			strncmp( modelName.str(), "No ", 3 ) )
	{
	 	WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();
		model = pMgr->Create_Render_Obj(modelName.str());
		if (model)
		{
			const AABoxClass bbox = model->Get_Bounding_Box();
//			Real height = bbox.Extent.Z;
			const SphereClass sphere = model->Get_Bounding_Sphere();
			Real dist = sphere.Radius*0.5;
			model->Set_Position(Vector3(-sphere.Center.X, -sphere.Center.Y, -sphere.Center.Z));

			// Create reflection texture
			TextureClass *objectTexture = DX8Wrapper::Create_Render_Target (PREVIEW_WIDTH, PREVIEW_HEIGHT);
			if (!objectTexture)
			{
				model->Release_Ref();
				return nullptr;
			}

			// Set the render target
			DX8Wrapper::Set_Render_Target_With_Z(objectTexture);

			// create the camera
			Bool orthoCamera = false;
			CameraClass *camera = NEW_REF( CameraClass, () );
			Matrix3D camTran;
			camTran.Look_At(Vector3(dist*2,dist*2,dist),Vector3(0.0f, 0.0f, 0.0f),0);
			camera->Set_Transform( camTran);

			Vector2 minVec = Vector2( -1, -1 );
			Vector2 maxVec = Vector2( +1, +1 );
			camera->Set_View_Plane( minVec, maxVec );
			camera->Set_Clip_Planes( 0.995f, 600.0f );
			if (orthoCamera)
				camera->Set_Projection_Type( CameraClass::ORTHO );

			// Clear the backbuffer
			WW3D::Begin_Render(true,true,Vector3(0.5f,0.5f,0.5f));
			//WW3D::Begin_Render(true,true,Vector3(1.0f,1.0f,1.0f));

			RenderInfoClass rinfo(*camera);
			LightEnvironmentClass lightEnv;
			rinfo.light_environment = &lightEnv;
			lightEnv.Reset(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));

			WW3D::Render(*model, rinfo);

			WW3D::End_Render(false);

			// Change the rendertarget back to the main backbuffer
			DX8Wrapper::Set_Render_Target((IDirect3DSurface8 *)nullptr);

			SurfaceClass *surface = objectTexture->Get_Surface_Level();
			UnsignedByte *data = saveSurface(surface->Peek_D3D_Surface());

			REF_PTR_RELEASE(surface);

			REF_PTR_RELEASE(objectTexture);
			REF_PTR_RELEASE(camera);
			return data;
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
// ObjectPreview message handlers

void ObjectPreview::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect clientRect;
	GetClientRect(&clientRect);

	CRect previewRect;
	previewRect = clientRect;

	CBrush brush;
	brush.CreateSolidBrush(RGB(0,0,0));

	UnsignedByte *pData;
	pData = generatePreview(m_tTempl);
	if (pData) {
		DrawMyTexture(&dc, previewRect.top, previewRect.left, previewRect.Width(), previewRect.Height(), pData);
	} else {
		dc.FillSolidRect(&previewRect, RGB(128,128,128));
	}
	dc.FrameRect(&previewRect, &brush);
}

void ObjectPreview::DrawMyTexture(CDC *pDc, int top, int left, Int width, Int height, UnsignedByte *rgbData)
{
	// Just blast about some dib bits.

	LPBITMAPINFO pBI;
//	long bytes = sizeof(BITMAPINFO);
 	pBI = new BITMAPINFO;
	pBI->bmiHeader.biSize = sizeof(pBI->bmiHeader);
	pBI->bmiHeader.biWidth = PREVIEW_WIDTH;
	pBI->bmiHeader.biHeight = PREVIEW_HEIGHT; /* match display top left == 0,0 */
	pBI->bmiHeader.biPlanes = 1;
	pBI->bmiHeader.biBitCount = 24;
	pBI->bmiHeader.biCompression = BI_RGB;
	pBI->bmiHeader.biSizeImage = (PREVIEW_WIDTH*PREVIEW_HEIGHT)*(pBI->bmiHeader.biBitCount/8);
	pBI->bmiHeader.biXPelsPerMeter = 1000;
	pBI->bmiHeader.biYPelsPerMeter = 1000;
	pBI->bmiHeader.biClrUsed = 0;
	pBI->bmiHeader.biClrImportant = 0;

	//::Sleep(10);
	//int val=::StretchDIBits(pDc->m_hDC, left, top, width, height, 0, 0, PREVIEW_WIDTH, PREVIEW_HEIGHT, rgbData, pBI,
	//	DIB_RGB_COLORS, SRCCOPY);
	/*int val=*/::StretchDIBits(pDc->m_hDC, left, top, width, height, PREVIEW_WIDTH/4, PREVIEW_HEIGHT/4, PREVIEW_WIDTH/2, PREVIEW_HEIGHT/2, rgbData, pBI,
		DIB_RGB_COLORS, SRCCOPY);
	delete(pBI);
}


