/*
**	Command & Conquer Generals(tm)
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

#include "texturefile.h"
#include "textureloader.h"
#include "ww3d.h"
#include "wwstring.h"
#include	"RAWFILE.H"
#include	"ffactory.h"
#include "nstrdup.h"
#include "texfcach.h"
#include "assetmgr.h"

#ifdef WW3D_DX8
#include <srCore.hpp>
#include <srSurfaceIOManager.hpp>
#include "texture.h"

static TextureFileClass* head;
static bool mipmaps=true;
static int texture_count;
static int locked_surface_count;
static int texture_id;

/*
** Definitions of static members:
*/
unsigned int TextureFileClass::_CurrentTimeStamp = 0U;
float TextureFileClass::_SwitchThreshold = 0.75f;	// Default value

// ----------------------------------------------------------------------------

static int Calculate_Size(srColorSurfaceIFace* surface,int red_factor=0)
{
	// Set performance statistics
	int p=surface->getPitch();
	int h=surface->getHeight();
	int size=0;
	p<<=red_factor;
	h<<=red_factor;
	while (h) {
		size+=p*h;
		p>>=1;
		h>>=1;
	}
	return size;
}

// ----------------------------------------------------------------------------

void TextureFileClass::Switch_Mipmaping_Debug()
{
	mipmaps=!mipmaps;
	TextureFileClass* n=head;
	while (n) {
		if (mipmaps) n->params.setMipmap(MIPMAP_DEFAULT);
		else n->params.setMipmap(MIPMAP_NONE);
		n=n->Succ;
	}
}

// ----------------------------------------------------------------------------

int TextureFileClass::Get_Total_Locked_Surface_Size()
{
	int total_locked_surface_size=0;
	TextureFileClass* n=head;
	while (n) {
		total_locked_surface_size+=n->Get_Locked_Surface_Size();
		n=n->Succ;
	}
	return total_locked_surface_size;
}

// ----------------------------------------------------------------------------

int TextureFileClass::Get_Total_Texture_Size()
{
	int total_texture_size=0;
	TextureFileClass* n=head;
	while (n) {
		total_texture_size+=n->Get_Texture_Size();
		n=n->Succ;
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureFileClass::Get_Total_Non_Reduced_Texture_Size()
{
	int total_texture_size=0;
	TextureFileClass* n=head;
	while (n) {
		total_texture_size+=n->Get_Non_Reduced_Texture_Size();
		n=n->Succ;
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureFileClass::Get_Total_Texture_Count()
{
	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureFileClass::Get_Total_Locked_Surface_Count()
{
	return locked_surface_count;
}

StringClass TextureFileClass::List_Missing_Files()	// Print out a list of missing files
{
	StringClass s(0,true);

	TextureFileClass* n=head;
	s=
		"Missing textures\n\n"
		"id      name\n"
		"------------------------\n";
	int missing_count=0;
	while (n) {
		if (n->Get_File_Error()) {
			StringClass tmp;
			tmp.Format("%4.4d  %s\n",n->ID(),n->Get_File_Name());
			s+=tmp;
		}
		n=n->Succ;
	}
	if (!missing_count) s="No missing textures\n\n";
	else {
		StringClass tmp;
		tmp.Format("\nTotal missing textures: %d",missing_count);
		s+=tmp;
	}
	return s;
}

// ----------------------------------------------------------------------------

TextureFileClass::TextureFileClass(const char * filename) :
	LockedSurface(0),
	LockedSurfaceReductionFactor(0),
	LockedSurfaceSize(0),
	TextureSize(0),
	NonReducedTextureSize(0),
	CurrentReductionFactor(0),
	DesiredReductionFactor(0.0f),
	TimeStampOfLastRequestReductionCall(_CurrentTimeStamp - 1),
	TimeStampOfLastProcessReductionCall(_CurrentTimeStamp - 1),
	FileName(0),
	TextureFrameHandle(getNewFrameHandle()),
	TempSurfacePtr(0),
	id(texture_id++),
	flash(false),
	flash_store_surface(NULL),
	file_error(false)
{
	if(filename && *filename) {
		FileName = nstrdup(filename);
	} else {
		FileName = 0;
	}

	if(FileName) {
		setName(FileName);
		if (!TextureLoader::Texture_File_Exists(FileName)) {
			file_error=true;
		}
	}

	// Load locked surface (or not) according to current locked surface reduction factor set in
	// texture loader
	Load_Locked_Surface();

	texture_count++;
	Succ=head;
	head=this;

	flags.clear(GENERATESURFACE_FAILURE);
	flags.set(DIRTY_DEFAULTS);

	if (WW3D::Get_Texture_Thumbnail_Mode()!=WW3D::TEXTURE_THUMBNAIL_MODE_RESIZING) {
		ReductionEnabled=false;
	}

}

TextureFileClass::~TextureFileClass()
{
	texture_count--;
	if (flash_store_surface) flash_store_surface->release();
	if (LockedSurface) locked_surface_count--;
	TextureFileClass* n=head;
	if (n==this) {
		head=Succ;
	}
	else {
		while (n) {
			if (n->Succ==this) {
				n->Succ=Succ;
				break;
			}
			n=n->Succ;
		}
	}

	invalidate();

	if (FileName) delete [] FileName;
	if (LockedSurface) LockedSurface->release();
}

// Copy CTor and assignment operator assert for now - if anyone hits the
// assert we might need to actually implement them 8^).
TextureFileClass::TextureFileClass(const TextureFileClass & src)
{
	WWASSERT(0);
}

TextureFileClass & TextureFileClass::operator = (const TextureFileClass &that)
{
	if (this != &that) {

		srTexture::operator = (that);

		WWASSERT(0);

	}
	return *this;
}

srTextureIFace::FrameHandle TextureFileClass::getTextureFrameHandle(void)
{
	if (flags.test(GENERATESURFACE_FAILURE))
		return 0;

	return TextureFrameHandle;
}

void TextureFileClass::getMipmapData(MultiRequest& m)
{
	// First some assertions...
	WWASSERT(m.largeLOD <= m.smallLOD && m.smallLOD < MAX_LOD);
	WWASSERT_PRINT(m.levels[m.largeLOD],"TextureFileClass::getMipmapData() -- NULL surface passed in!\n");
	if (!m.levels[m.largeLOD])	return;

	/*
	** If the temporary surface pointer points to a surface (this will be a new surface which just
	** got loaded in this frame) use it and release it - done.
	** Otherwise, if we have a locked surface, then normally the current reduction factor should
	** be equal or greater to the locked surface reduction factor so we can use the locked surface
	** (possibly scaled down). If the current reduction factor is less than that of the locked
	** surface, this means that in the past we have loaded up a surface larger than the locked
	** surface, and it has just been thrown out of the API/GERD/HW texture cache. In this case we
	** have no choice but to scale the locked surface up to fill the surface handed to us, and we
	** will request a background load task for the real surface.
	** If there is no locked surface, this means that the texture is never reduced or loaded in
	** the background - we perform a normal blocking load of the surface, use it, and release it
	** (if you want behavior similar to "cached mode" in srTextureFile you should set the locked
	** surface reduction factor to 0, instead of having no locked surface).
	*/
	if (TempSurfacePtr) {
		Fill_Multi_Request_From_Surface(m, TempSurfacePtr);
		Release_Temp_Surface();
	} else {
		TextureSize=0;
		NonReducedTextureSize=0;
		if (LockedSurface) {
			Fill_Multi_Request_From_Surface(m, LockedSurface);
			if (CurrentReductionFactor < LockedSurfaceReductionFactor) {
				WWDEBUG_SAY(("Scaling locked surface up!\n"));
				CurrentReductionFactor = LockedSurfaceReductionFactor;
				float des_red_factor = floor(DesiredReductionFactor + 0.5f);
				WWASSERT(des_red_factor >= 0.0f);
				unsigned int int_desired_reduction_factor = (int)des_red_factor;

				// If int_desired_reduction_factor is smaller than the locked surface reduction
				// factor (meaning we want a larger surface), request to load it.
				if (int_desired_reduction_factor < LockedSurfaceReductionFactor) {
					texture_loader_info.reduction_factor = int_desired_reduction_factor;
					TextureLoader::Add_Load_Task(this);
				}
			}
			else {
				// This else clause should only be entered in cases where Process_Reduction() is not
				// called every frame on the texture for some reason. Such textures will not resize,
				// but this at least ensures that they will get loaded.
				Process_Reduction();
			}
		} else {
			Load_Temp_Surface();
			if (TempSurfacePtr) {
				TextureSize=Calculate_Size(TempSurfacePtr);
				NonReducedTextureSize=TextureSize;

				Fill_Multi_Request_From_Surface(m, TempSurfacePtr);
				Release_Temp_Surface();
			}
		}
	}
}

void TextureFileClass::invalidate(void)
{
	Release_Temp_Surface();
	invalidateFrameHandle(TextureFrameHandle);
	flags.clear(GENERATESURFACE_FAILURE);	
}

void TextureFileClass::setupDefaultValues(void)
{
	if (flags.test(DIRTY_DEFAULTS))
	{
		flags.clear (DIRTY_DEFAULTS);

		/*
		** If the temporary surface pointer points to a surface (this will be a new surface which
		** just got loaded in this frame by Apply_New_Surface) get defaults from it - done.
		** Otherwise, if we have a locked surface, then normally the current reduction factor should
		** be equal or greater to the locked surface reduction factor so we can get defaults from
		** the locked surface (possibly scaled down). If the current reduction factor is less than
		** that of the locked surface, this means that in the past we have loaded up a surface
		** larger than the locked surface, and it has just been thrown out of the API/GERD/HW
		** texture cache. In this case we set the current reduction factor to be equal to that of
		** the locked surface, and get defaults from the locked surface.
		** If there is no locked surface, this means that the texture is never reduced or loaded in
		** the background - we perform a normal blocking load of the surface, store it in the temp
		** surface pointer (so getMipmapData can also use it), and get defaults from it.
		*/
		if (TempSurfacePtr) {
			setupDefaultValuesFromSurface(TempSurfacePtr);
			TextureSize=Calculate_Size(TempSurfacePtr);
			NonReducedTextureSize=Calculate_Size(TempSurfacePtr,CurrentReductionFactor);
		} else {
			if (LockedSurface) {
				setupDefaultValuesFromSurface(LockedSurface);
				if (CurrentReductionFactor >= LockedSurfaceReductionFactor) {
					// Scale size down by difference between current and locked reduction factors
					unsigned int diff = CurrentReductionFactor - LockedSurfaceReductionFactor;
					defaultDimensions.width = defaultDimensions.width >> diff;
					if (defaultDimensions.width == 0) defaultDimensions.width = 1;
					defaultDimensions.height = defaultDimensions.height >> diff;
					if (defaultDimensions.height == 0) defaultDimensions.height = 1;
				} else {
					CurrentReductionFactor = LockedSurfaceReductionFactor;
				}
				TextureSize=0;
				NonReducedTextureSize=0;
			} else {
				Load_Temp_Surface();
				if (TempSurfacePtr) {
					setupDefaultValuesFromSurface(TempSurfacePtr);
					TextureSize=Calculate_Size(TempSurfacePtr);
					NonReducedTextureSize=TextureSize;
				}
			}
		}
	}
}

void TextureFileClass::Apply_New_Surface()
{
	invalidate();
	TempSurfacePtr = texture_loader_info.new_surface;
	texture_loader_info.new_surface = 0;
	flags.set(DIRTY_DEFAULTS);
	CurrentReductionFactor = texture_loader_info.reduction_factor;
}

// This is used by an object to request a reduction factor on all its textures. Only the smallest
// reduction factor requested in a given frame is preserved.
void TextureFileClass::Request_Reduction(float reduction_factor)
{
	if (!ReductionEnabled) return;

	if (reduction_factor < 0.0f) reduction_factor = 0.0f;

	// If this is the first request in this time-stamp, overwrite desired reduction factor -
	// otherwise current factor is the lesser of current and requested.
	if (TimeStampOfLastRequestReductionCall != _CurrentTimeStamp) {

		DesiredReductionFactor = reduction_factor;

		// Update time stamp
		TimeStampOfLastRequestReductionCall = _CurrentTimeStamp;

	} else {
		DesiredReductionFactor = MIN(reduction_factor, DesiredReductionFactor);
	}
}

// This is used during rendering - if a textures desired reduction level is "different enough"
// from its current one, we fix it: either by updating from the locked surface (if the reduction
// factor is equal or greater to that of the locked surface) or by asking the texture loader to
// load a new reduction level of the texture in the background. 
// NOTE - if there is no locked surface, we do nothing (since textures without locked surfaces do
// not perform reduction).
void TextureFileClass::Process_Reduction(void)
{
	// Check if this is the first Process_Reduction() call this frame - otherwise do nothing
	if (TimeStampOfLastProcessReductionCall != _CurrentTimeStamp) {

		// If we are doing reductions and the difference between the current and desired reduction
		// factors is above the threshold, fix it.
		if (LockedSurface && fabs(float(CurrentReductionFactor) - DesiredReductionFactor) > _SwitchThreshold) {

			// Find the integer desired reduction factor
			float des_red_factor = floor(DesiredReductionFactor + 0.5f);
			WWASSERT(des_red_factor >= 0.0f);
			unsigned int int_desired_reduction_factor = (int)des_red_factor;
			if (int_desired_reduction_factor >= LockedSurfaceReductionFactor) {
				// Can update from the locked surface:
				CurrentReductionFactor = int_desired_reduction_factor;
				invalidate();
			} else {
				// Need to load bigger surface
				texture_loader_info.reduction_factor = int_desired_reduction_factor;
				TextureLoader::Add_Load_Task(this);
			}
		}

		// Update time stamp
		TimeStampOfLastProcessReductionCall = _CurrentTimeStamp;
	
	}
}

void TextureFileClass::_Set_Switch_Threshold(float switch_threshold)
{
	_SwitchThreshold = MIN(switch_threshold, 0.999);
	_SwitchThreshold = MAX(_SwitchThreshold, 0.5);
}

float TextureFileClass::_Get_Switch_Threshold(void)
{
	return _SwitchThreshold;
}

void TextureFileClass::Load_Temp_Surface(void)
{
	if (TempSurfacePtr) invalidate();
	
	if (!FileName) return;

	TempSurfacePtr = 0;

	//
	//	Use the texture file caching mechanism to load the surface.  If no cache
	// is initialized, then simply load the surface from the file.
	//
	TextureFileCache* cache=WW3DAssetManager::Get_Instance()->Texture_File_Cache();
	if (cache != NULL) {
		cache->Validate_Texture(FileName);
		TempSurfacePtr = cache->Get_Surface(
			Get_File_Name(),
			0); // No reduction!
	} else {
		TempSurfacePtr = ::Load_Surface(FileName);
	}
}

void TextureFileClass::Release_Temp_Surface(void)
{
	if (TempSurfacePtr)
	{
		TempSurfacePtr->release();
		TempSurfacePtr = 0;
	}
}

void TextureFileClass::Fill_Multi_Request_From_Surface(MultiRequest& m, srColorSurfaceIFace* surface)
{
	m.levels[m.largeLOD]->copy (*surface);
	for (LOD i = m.largeLOD+1; i <= m.smallLOD; i++)
	{
		WWASSERT(m.levels[i] && m.levels[i-1]);
		if (m.levels[i] && m.levels[i-1])							// just debug check
			m.levels[i]->copy (*(m.levels[i-1]));
	}
}

// Debug features

// Make texture flash (Warning! Slow, for debug only!!!)
void TextureFileClass::Set_Texture_Flash(bool b)
{
	flash=b;
	if (flash) {
		Load_Temp_Surface();
		flash_store_surface=TempSurfacePtr;
		flash_store_surface->addReference();
	}
	else {
		// Restore the original surface... the surface will be released after applying to use
		invalidateFrameHandle(TextureFrameHandle);
		Release_Temp_Surface();
		TempSurfacePtr=flash_store_surface;
		flash_store_surface=NULL;
	}
}

// Return texture flash state
bool TextureFileClass::Get_Texture_Flash() const
{
	return flash;
}

TextureFileClass* TextureFileClass::Get_Texture(int id)
{
	TextureFileClass* n=head;
	while (n) {
		if (n->ID()==id) return n;
		n=n->Succ;
	}
	return NULL;
}

static unsigned latest_time;
static bool flash_state;
static unsigned flash_counter;
const int flash_time=500;

void TextureFileClass::Update_Texture_Flash()
{
	int cur_time=WW3D::Get_Sync_Time();
	int delta=cur_time-latest_time;
	latest_time=cur_time;
	flash_counter+=delta;
	if (flash_counter>flash_time) {
		flash_counter-=flash_time;
		if (flash_counter>flash_time) flash_counter=0;
		flash_state=!flash_state;
	}
	else return;

	srColorSurface* s=NULL;
	if (!flash_state) {
		int w=srCore.getSurface()->getWidth();
		srColorSurfaceIFace::PixelFormat pf;
		srCore.getSurface()->getPixelFormat(pf);
		s=W3DNEW srColorSurface(pf,w,w);
		s->copy(*srCore.getSurface());
	}

	TextureFileClass* n=head;
	while (n) {
		if (n->flash) {
			n->invalidateFrameHandle(n->TextureFrameHandle);
			n->Release_Temp_Surface();
			if (s) {
				n->TempSurfacePtr=s; 
				s->addReference(); // Surface will be release after the use
			}
			else {
				n->TempSurfacePtr=n->flash_store_surface;
				n->TempSurfacePtr->addReference(); // Surface will be release after the use
			}
		}
		n=n->Succ;
	}
	s->release();
}

// If false is passed, the texture will load the largest LOD size and stick to that

void TextureFileClass::Enable_Reduction(bool b)
{
	if (ReductionEnabled==b) return;
	ReductionEnabled=b;
	if (b) {
		Load_Locked_Surface();
	}
	else {
		if (LockedSurface) LockedSurface->release();
		LockedSurface=NULL;
		LockedSurfaceReductionFactor=0;
		CurrentReductionFactor=0;
		DesiredReductionFactor=0;

		Load_Temp_Surface();
		if (TempSurfacePtr) {
			setupDefaultValuesFromSurface(TempSurfacePtr);
			TextureSize=Calculate_Size(TempSurfacePtr);
			NonReducedTextureSize=TextureSize;
		}
	}
}

void TextureFileClass::Load_Locked_Surface()
{
	if (LockedSurface) LockedSurface->release();
	LockedSurface=NULL;
	LockedSurfaceReductionFactor=0;
	if (FileName) {
		TextureLoader::Load_Locked_Surface_Immediate(FileName, LockedSurface, LockedSurfaceReductionFactor);

		if (LockedSurface) {

			// If we have a locked surface, set the current reduction factor to be equal to the
			// locked surface reduction factor (this is the highest resolution that we can render
			// with at this time)
			CurrentReductionFactor = LockedSurfaceReductionFactor;

			LockedSurfaceSize=Calculate_Size(LockedSurface);
			locked_surface_count++;

		}
	}
}

#endif //WW3D_DX8
