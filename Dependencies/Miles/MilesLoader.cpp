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

#include "MilesLoader.h"
#include "mss/mss.h"

// mss32.dll exports decorated __stdcall names, which only exist in the 32 bit Windows build.
#if defined(_WIN32) && !defined(_WIN64)
#define MILES_LOADER_SUPPORTED 1
#else
#define MILES_LOADER_SUPPORTED 0
#endif


namespace
{

typedef F32 (__stdcall *AIL_3D_sample_volume_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_set_3D_sample_volume_t)(H3DSAMPLE sample, F32 volume);
typedef void (__stdcall *AIL_end_3D_sample_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_resume_3D_sample_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_stop_3D_sample_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_start_3D_sample_t)(H3DSAMPLE sample);
typedef U32 (__stdcall *AIL_3D_sample_loop_count_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_set_3D_sample_offset_t)(H3DSAMPLE sample, U32 offset);
typedef U32 (__stdcall *AIL_3D_sample_length_t)(H3DSAMPLE sample);
typedef U32 (__stdcall *AIL_3D_sample_offset_t)(H3DSAMPLE sample);
typedef S32 (__stdcall *AIL_3D_sample_playback_rate_t)(H3DSAMPLE sample);
typedef void (__stdcall *AIL_set_3D_sample_playback_rate_t)(H3DSAMPLE sample, S32 playback_rate);
typedef S32 (__stdcall *AIL_set_3D_sample_file_t)(H3DSAMPLE sample, const void* file_image);
typedef HPROVIDER (__stdcall *AIL_set_sample_processor_t)(HSAMPLE sample, SAMPLESTAGE pipeline_stage, HPROVIDER provider);
typedef void (__stdcall *AIL_set_filter_sample_preference_t)(HSAMPLE sample, const char* name, const void* val);
typedef void (__stdcall *AIL_release_sample_handle_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_close_3D_provider_t)(HPROVIDER lib);
typedef S32 (__stdcall *AIL_set_preference_t)(U32 number, S32 value);
typedef S32 (__stdcall *AIL_waveOutOpen_t)(HDIGDRIVER* driver, LPHWAVEOUT* waveout, S32 id, LPWAVEFORMAT format);
typedef void (__stdcall *AIL_waveOutClose_t)(HDIGDRIVER driver);
typedef void (__stdcall *AIL_set_3D_sample_loop_count_t)(H3DSAMPLE sample, U32 count);
typedef void (__stdcall *AIL_set_stream_playback_rate_t)(HSTREAM stream, S32 rate);
typedef S32 (__stdcall *AIL_stream_playback_rate_t)(HSTREAM stream);
typedef void (__stdcall *AIL_stream_ms_position_t)(HSTREAM sample, S32* total_milliseconds, S32* current_milliseconds);
typedef void (__stdcall *AIL_set_stream_ms_position_t)(HSTREAM stream, S32 pos);
typedef S32 (__stdcall *AIL_stream_loop_count_t)(HSTREAM stream);
typedef void (__stdcall *AIL_set_stream_loop_block_t)(HSTREAM stream, S32 loop_start, S32 loop_end);
typedef void (__stdcall *AIL_set_stream_loop_count_t)(HSTREAM stream, S32 count);
typedef void (__stdcall *AIL_close_stream_t)(HSTREAM stream);
typedef void (__stdcall *AIL_pause_stream_t)(HSTREAM stream, S32 onoff);
typedef AIL_stream_callback (__stdcall *AIL_register_stream_callback_t)(HSTREAM stream, AIL_stream_callback callback);
typedef AIL_3dsample_callback (__stdcall *AIL_register_3D_EOS_callback_t)(H3DSAMPLE sample, AIL_3dsample_callback EOS);
typedef AIL_sample_callback (__stdcall *AIL_register_EOS_callback_t)(HSAMPLE sample, AIL_sample_callback EOS);
typedef void (__stdcall *AIL_start_stream_t)(HSTREAM stream);
typedef void (__stdcall *AIL_set_sample_playback_rate_t)(HSAMPLE sample, S32 playback_rate);
typedef S32 (__stdcall *AIL_sample_playback_rate_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_sample_ms_position_t)(HSAMPLE sample, S32* total_ms, S32* current_ms);
typedef void (__stdcall *AIL_set_sample_ms_position_t)(HSAMPLE sample, S32 pos);
typedef S32 (__stdcall *AIL_sample_loop_count_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_set_sample_loop_count_t)(HSAMPLE sample, S32 count);
typedef void (__stdcall *AIL_end_sample_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_resume_sample_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_stop_sample_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_start_sample_t)(HSAMPLE sample);
typedef void (__stdcall *AIL_init_sample_t)(HSAMPLE sample);
typedef S32 (__stdcall *AIL_set_named_sample_file_t)(HSAMPLE sample, const char* file_name, const void* file_image, S32 file_size, S32 block);
typedef void (__stdcall *AIL_set_3D_sample_effects_level_t)(H3DSAMPLE sample, F32 effect_level);
typedef void (__stdcall *AIL_set_3D_sample_distances_t)(H3DSAMPLE sample, F32 max_dist, F32 min_dist);
typedef void (__stdcall *AIL_set_3D_velocity_vector_t)(H3DPOBJECT obj, F32 x, F32 y, F32 z);
typedef void (__stdcall *AIL_set_3D_position_t)(H3DPOBJECT obj, F32 X, F32 Y, F32 Z);
typedef void (__stdcall *AIL_set_3D_orientation_t)(H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up);
typedef S32 (__stdcall *AIL_WAV_info_t)(const void* data, AILSOUNDINFO* info);
typedef void (__stdcall *AIL_stop_timer_t)(HTIMER timer);
typedef void (__stdcall *AIL_release_timer_handle_t)(HTIMER timer);
typedef void (__stdcall *AIL_shutdown_t)(void);
typedef S32 (__stdcall *AIL_enumerate_filters_t)(HPROENUM* next, HPROVIDER* dest, char** name);
typedef void (__stdcall *AIL_set_file_callbacks_t)(AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);
typedef void (__stdcall *AIL_release_3D_sample_handle_t)(H3DSAMPLE sample);
typedef H3DSAMPLE (__stdcall *AIL_allocate_3D_sample_handle_t)(HPROVIDER lib);
typedef void (__stdcall *AIL_set_3D_user_data_t)(H3DPOBJECT obj, U32 index, S32 value);
typedef void (__stdcall *AIL_unlock_t)(void);
typedef void (__stdcall *AIL_unlock_mutex_t)(void);
typedef void (__stdcall *AIL_lock_t)(void);
typedef void (__stdcall *AIL_lock_mutex_t)(void);
typedef void (__stdcall *AIL_set_3D_speaker_type_t)(HPROVIDER lib, S32 speaker_type);
typedef void (__stdcall *AIL_close_3D_listener_t)(H3DPOBJECT listener);
typedef S32 (__stdcall *AIL_enumerate_3D_providers_t)(HPROENUM* next, HPROVIDER* dest, char** name);
typedef M3DRESULT (__stdcall *AIL_open_3D_provider_t)(HPROVIDER lib);
typedef char* (__stdcall *AIL_last_error_t)(void);
typedef H3DPOBJECT (__stdcall *AIL_open_3D_listener_t)(HPROVIDER lib);
typedef S32 (__stdcall *AIL_3D_user_data_t)(H3DPOBJECT obj, U32 index);
typedef S32 (__stdcall *AIL_sample_user_data_t)(HSAMPLE sample, U32 index);
typedef HSAMPLE (__stdcall *AIL_allocate_sample_handle_t)(HDIGDRIVER dig);
typedef void (__stdcall *AIL_set_sample_user_data_t)(HSAMPLE sample, U32 index, S32 value);
typedef S32 (__stdcall *AIL_decompress_ADPCM_t)(const AILSOUNDINFO *info, void **outdata, U32 *outsize);
typedef void (__stdcall *AIL_get_DirectSound_info_t)(HSAMPLE sample, AILLPDIRECTSOUND *lplpDS, AILLPDIRECTSOUNDBUFFER *lplpDSB);
typedef void (__stdcall *AIL_mem_free_lock_t)(void *ptr);
typedef HSTREAM (__stdcall *AIL_open_stream_t)(HDIGDRIVER dig, const char *filename, S32 stream_mem);
typedef S32 (__stdcall *AIL_startup_t)(void);
typedef void (__stdcall *AIL_quick_unload_t)(HAUDIO audio);
typedef HAUDIO (__stdcall *AIL_quick_load_and_play_t)(const char *filename, U32 loop_count, S32 wait_request);
typedef void (__stdcall *AIL_quick_set_volume_t)(HAUDIO audio, F32 volume, F32 extravol);
typedef S32 (__stdcall *AIL_quick_startup_t)(S32 use_digital, S32 use_MIDI, U32 output_rate, S32 output_bits, S32 output_channels);
typedef void (__stdcall *AIL_quick_handles_t)(HDIGDRIVER *pdig, HMDIDRIVER *pmdi, HDLSDEVICE *pdls);
typedef void (__stdcall *AIL_sample_volume_pan_t)(HSAMPLE sample, F32 *volume, F32 *pan);
typedef void (__stdcall *AIL_set_3D_sample_occlusion_t)(H3DSAMPLE sample, F32 occlusion);
typedef char * (__stdcall *AIL_set_redist_directory_t)(const char *dir);
typedef S32 (__stdcall *AIL_set_sample_file_t)(HSAMPLE sample, const void *file_image, S32 block);
typedef void (__stdcall *AIL_set_sample_volume_pan_t)(HSAMPLE sample, F32 volume, F32 pan);
typedef void (__stdcall *AIL_set_stream_volume_pan_t)(HSTREAM stream, F32 volume, F32 pan);
typedef void (__stdcall *AIL_stream_volume_pan_t)(HSTREAM stream, F32 *volume, F32 *pan);
typedef U32 (__stdcall *AIL_get_timer_highest_delay_t)(void);

AIL_3D_sample_volume_t AIL_3D_sample_volumePtr = nullptr;
AIL_set_3D_sample_volume_t AIL_set_3D_sample_volumePtr = nullptr;
AIL_end_3D_sample_t AIL_end_3D_samplePtr = nullptr;
AIL_resume_3D_sample_t AIL_resume_3D_samplePtr = nullptr;
AIL_stop_3D_sample_t AIL_stop_3D_samplePtr = nullptr;
AIL_start_3D_sample_t AIL_start_3D_samplePtr = nullptr;
AIL_3D_sample_loop_count_t AIL_3D_sample_loop_countPtr = nullptr;
AIL_set_3D_sample_offset_t AIL_set_3D_sample_offsetPtr = nullptr;
AIL_3D_sample_length_t AIL_3D_sample_lengthPtr = nullptr;
AIL_3D_sample_offset_t AIL_3D_sample_offsetPtr = nullptr;
AIL_3D_sample_playback_rate_t AIL_3D_sample_playback_ratePtr = nullptr;
AIL_set_3D_sample_playback_rate_t AIL_set_3D_sample_playback_ratePtr = nullptr;
AIL_set_3D_sample_file_t AIL_set_3D_sample_filePtr = nullptr;
AIL_set_sample_processor_t AIL_set_sample_processorPtr = nullptr;
AIL_set_filter_sample_preference_t AIL_set_filter_sample_preferencePtr = nullptr;
AIL_release_sample_handle_t AIL_release_sample_handlePtr = nullptr;
AIL_close_3D_provider_t AIL_close_3D_providerPtr = nullptr;
AIL_set_preference_t AIL_set_preferencePtr = nullptr;
AIL_waveOutOpen_t AIL_waveOutOpenPtr = nullptr;
AIL_waveOutClose_t AIL_waveOutClosePtr = nullptr;
AIL_set_3D_sample_loop_count_t AIL_set_3D_sample_loop_countPtr = nullptr;
AIL_set_stream_playback_rate_t AIL_set_stream_playback_ratePtr = nullptr;
AIL_stream_playback_rate_t AIL_stream_playback_ratePtr = nullptr;
AIL_stream_ms_position_t AIL_stream_ms_positionPtr = nullptr;
AIL_set_stream_ms_position_t AIL_set_stream_ms_positionPtr = nullptr;
AIL_stream_loop_count_t AIL_stream_loop_countPtr = nullptr;
AIL_set_stream_loop_block_t AIL_set_stream_loop_blockPtr = nullptr;
AIL_set_stream_loop_count_t AIL_set_stream_loop_countPtr = nullptr;
AIL_close_stream_t AIL_close_streamPtr = nullptr;
AIL_pause_stream_t AIL_pause_streamPtr = nullptr;
AIL_register_stream_callback_t AIL_register_stream_callbackPtr = nullptr;
AIL_register_3D_EOS_callback_t AIL_register_3D_EOS_callbackPtr = nullptr;
AIL_register_EOS_callback_t AIL_register_EOS_callbackPtr = nullptr;
AIL_start_stream_t AIL_start_streamPtr = nullptr;
AIL_set_sample_playback_rate_t AIL_set_sample_playback_ratePtr = nullptr;
AIL_sample_playback_rate_t AIL_sample_playback_ratePtr = nullptr;
AIL_sample_ms_position_t AIL_sample_ms_positionPtr = nullptr;
AIL_set_sample_ms_position_t AIL_set_sample_ms_positionPtr = nullptr;
AIL_sample_loop_count_t AIL_sample_loop_countPtr = nullptr;
AIL_set_sample_loop_count_t AIL_set_sample_loop_countPtr = nullptr;
AIL_end_sample_t AIL_end_samplePtr = nullptr;
AIL_resume_sample_t AIL_resume_samplePtr = nullptr;
AIL_stop_sample_t AIL_stop_samplePtr = nullptr;
AIL_start_sample_t AIL_start_samplePtr = nullptr;
AIL_init_sample_t AIL_init_samplePtr = nullptr;
AIL_set_named_sample_file_t AIL_set_named_sample_filePtr = nullptr;
AIL_set_3D_sample_effects_level_t AIL_set_3D_sample_effects_levelPtr = nullptr;
AIL_set_3D_sample_distances_t AIL_set_3D_sample_distancesPtr = nullptr;
AIL_set_3D_velocity_vector_t AIL_set_3D_velocity_vectorPtr = nullptr;
AIL_set_3D_position_t AIL_set_3D_positionPtr = nullptr;
AIL_set_3D_orientation_t AIL_set_3D_orientationPtr = nullptr;
AIL_WAV_info_t AIL_WAV_infoPtr = nullptr;
AIL_stop_timer_t AIL_stop_timerPtr = nullptr;
AIL_release_timer_handle_t AIL_release_timer_handlePtr = nullptr;
AIL_shutdown_t AIL_shutdownPtr = nullptr;
AIL_enumerate_filters_t AIL_enumerate_filtersPtr = nullptr;
AIL_set_file_callbacks_t AIL_set_file_callbacksPtr = nullptr;
AIL_release_3D_sample_handle_t AIL_release_3D_sample_handlePtr = nullptr;
AIL_allocate_3D_sample_handle_t AIL_allocate_3D_sample_handlePtr = nullptr;
AIL_set_3D_user_data_t AIL_set_3D_user_dataPtr = nullptr;
AIL_unlock_t AIL_unlockPtr = nullptr;
AIL_unlock_mutex_t AIL_unlock_mutexPtr = nullptr;
AIL_lock_t AIL_lockPtr = nullptr;
AIL_lock_mutex_t AIL_lock_mutexPtr = nullptr;
AIL_set_3D_speaker_type_t AIL_set_3D_speaker_typePtr = nullptr;
AIL_close_3D_listener_t AIL_close_3D_listenerPtr = nullptr;
AIL_enumerate_3D_providers_t AIL_enumerate_3D_providersPtr = nullptr;
AIL_open_3D_provider_t AIL_open_3D_providerPtr = nullptr;
AIL_last_error_t AIL_last_errorPtr = nullptr;
AIL_open_3D_listener_t AIL_open_3D_listenerPtr = nullptr;
AIL_3D_user_data_t AIL_3D_user_dataPtr = nullptr;
AIL_sample_user_data_t AIL_sample_user_dataPtr = nullptr;
AIL_allocate_sample_handle_t AIL_allocate_sample_handlePtr = nullptr;
AIL_set_sample_user_data_t AIL_set_sample_user_dataPtr = nullptr;
AIL_decompress_ADPCM_t AIL_decompress_ADPCMPtr = nullptr;
AIL_get_DirectSound_info_t AIL_get_DirectSound_infoPtr = nullptr;
AIL_mem_free_lock_t AIL_mem_free_lockPtr = nullptr;
AIL_open_stream_t AIL_open_streamPtr = nullptr;
AIL_startup_t AIL_startupPtr = nullptr;
AIL_quick_unload_t AIL_quick_unloadPtr = nullptr;
AIL_quick_load_and_play_t AIL_quick_load_and_playPtr = nullptr;
AIL_quick_set_volume_t AIL_quick_set_volumePtr = nullptr;
AIL_quick_startup_t AIL_quick_startupPtr = nullptr;
AIL_quick_handles_t AIL_quick_handlesPtr = nullptr;
AIL_sample_volume_pan_t AIL_sample_volume_panPtr = nullptr;
AIL_set_3D_sample_occlusion_t AIL_set_3D_sample_occlusionPtr = nullptr;
AIL_set_redist_directory_t AIL_set_redist_directoryPtr = nullptr;
AIL_set_sample_file_t AIL_set_sample_filePtr = nullptr;
AIL_set_sample_volume_pan_t AIL_set_sample_volume_panPtr = nullptr;
AIL_set_stream_volume_pan_t AIL_set_stream_volume_panPtr = nullptr;
AIL_stream_volume_pan_t AIL_stream_volume_panPtr = nullptr;
AIL_get_timer_highest_delay_t AIL_get_timer_highest_delayPtr = nullptr;

int ReferenceCount = 0;
bool Failed = false;
unsigned long LastError = 0;

#if MILES_LOADER_SUPPORTED
HMODULE Module = HMODULE(nullptr);

// Resolves one export into its matching function pointer. The exported names are decorated, so
// they carry the __stdcall argument byte count and must be spelled out verbatim.
#define MILES_RESOLVE(name, decorated) \
	name##Ptr = reinterpret_cast<name##_t>(::GetProcAddress(Module, decorated))

static void resolveAll()
{
	MILES_RESOLVE(AIL_3D_sample_volume, "_AIL_3D_sample_volume@4");
	MILES_RESOLVE(AIL_set_3D_sample_volume, "_AIL_set_3D_sample_volume@8");
	MILES_RESOLVE(AIL_end_3D_sample, "_AIL_end_3D_sample@4");
	MILES_RESOLVE(AIL_resume_3D_sample, "_AIL_resume_3D_sample@4");
	MILES_RESOLVE(AIL_stop_3D_sample, "_AIL_stop_3D_sample@4");
	MILES_RESOLVE(AIL_start_3D_sample, "_AIL_start_3D_sample@4");
	MILES_RESOLVE(AIL_3D_sample_loop_count, "_AIL_3D_sample_loop_count@4");
	MILES_RESOLVE(AIL_set_3D_sample_offset, "_AIL_set_3D_sample_offset@8");
	MILES_RESOLVE(AIL_3D_sample_length, "_AIL_3D_sample_length@4");
	MILES_RESOLVE(AIL_3D_sample_offset, "_AIL_3D_sample_offset@4");
	MILES_RESOLVE(AIL_3D_sample_playback_rate, "_AIL_3D_sample_playback_rate@4");
	MILES_RESOLVE(AIL_set_3D_sample_playback_rate, "_AIL_set_3D_sample_playback_rate@8");
	MILES_RESOLVE(AIL_set_3D_sample_file, "_AIL_set_3D_sample_file@8");
	MILES_RESOLVE(AIL_set_sample_processor, "_AIL_set_sample_processor@12");
	MILES_RESOLVE(AIL_set_filter_sample_preference, "_AIL_set_filter_sample_preference@12");
	MILES_RESOLVE(AIL_release_sample_handle, "_AIL_release_sample_handle@4");
	MILES_RESOLVE(AIL_close_3D_provider, "_AIL_close_3D_provider@4");
	MILES_RESOLVE(AIL_set_preference, "_AIL_set_preference@8");
	MILES_RESOLVE(AIL_waveOutOpen, "_AIL_waveOutOpen@16");
	MILES_RESOLVE(AIL_waveOutClose, "_AIL_waveOutClose@4");
	MILES_RESOLVE(AIL_set_3D_sample_loop_count, "_AIL_set_3D_sample_loop_count@8");
	MILES_RESOLVE(AIL_set_stream_playback_rate, "_AIL_set_stream_playback_rate@8");
	MILES_RESOLVE(AIL_stream_playback_rate, "_AIL_stream_playback_rate@4");
	MILES_RESOLVE(AIL_stream_ms_position, "_AIL_stream_ms_position@12");
	MILES_RESOLVE(AIL_set_stream_ms_position, "_AIL_set_stream_ms_position@8");
	MILES_RESOLVE(AIL_stream_loop_count, "_AIL_stream_loop_count@4");
	MILES_RESOLVE(AIL_set_stream_loop_block, "_AIL_set_stream_loop_block@12");
	MILES_RESOLVE(AIL_set_stream_loop_count, "_AIL_set_stream_loop_count@8");
	MILES_RESOLVE(AIL_close_stream, "_AIL_close_stream@4");
	MILES_RESOLVE(AIL_pause_stream, "_AIL_pause_stream@8");
	MILES_RESOLVE(AIL_register_stream_callback, "_AIL_register_stream_callback@8");
	MILES_RESOLVE(AIL_register_3D_EOS_callback, "_AIL_register_3D_EOS_callback@8");
	MILES_RESOLVE(AIL_register_EOS_callback, "_AIL_register_EOS_callback@8");
	MILES_RESOLVE(AIL_start_stream, "_AIL_start_stream@4");
	MILES_RESOLVE(AIL_set_sample_playback_rate, "_AIL_set_sample_playback_rate@8");
	MILES_RESOLVE(AIL_sample_playback_rate, "_AIL_sample_playback_rate@4");
	MILES_RESOLVE(AIL_sample_ms_position, "_AIL_sample_ms_position@12");
	MILES_RESOLVE(AIL_set_sample_ms_position, "_AIL_set_sample_ms_position@8");
	MILES_RESOLVE(AIL_sample_loop_count, "_AIL_sample_loop_count@4");
	MILES_RESOLVE(AIL_set_sample_loop_count, "_AIL_set_sample_loop_count@8");
	MILES_RESOLVE(AIL_end_sample, "_AIL_end_sample@4");
	MILES_RESOLVE(AIL_resume_sample, "_AIL_resume_sample@4");
	MILES_RESOLVE(AIL_stop_sample, "_AIL_stop_sample@4");
	MILES_RESOLVE(AIL_start_sample, "_AIL_start_sample@4");
	MILES_RESOLVE(AIL_init_sample, "_AIL_init_sample@4");
	MILES_RESOLVE(AIL_set_named_sample_file, "_AIL_set_named_sample_file@20");
	MILES_RESOLVE(AIL_set_3D_sample_effects_level, "_AIL_set_3D_sample_effects_level@8");
	MILES_RESOLVE(AIL_set_3D_sample_distances, "_AIL_set_3D_sample_distances@12");
	MILES_RESOLVE(AIL_set_3D_velocity_vector, "_AIL_set_3D_velocity_vector@16");
	MILES_RESOLVE(AIL_set_3D_position, "_AIL_set_3D_position@16");
	MILES_RESOLVE(AIL_set_3D_orientation, "_AIL_set_3D_orientation@28");
	MILES_RESOLVE(AIL_WAV_info, "_AIL_WAV_info@8");
	MILES_RESOLVE(AIL_stop_timer, "_AIL_stop_timer@4");
	MILES_RESOLVE(AIL_release_timer_handle, "_AIL_release_timer_handle@4");
	MILES_RESOLVE(AIL_shutdown, "_AIL_shutdown@0");
	MILES_RESOLVE(AIL_enumerate_filters, "_AIL_enumerate_filters@12");
	MILES_RESOLVE(AIL_set_file_callbacks, "_AIL_set_file_callbacks@16");
	MILES_RESOLVE(AIL_release_3D_sample_handle, "_AIL_release_3D_sample_handle@4");
	MILES_RESOLVE(AIL_allocate_3D_sample_handle, "_AIL_allocate_3D_sample_handle@4");
	MILES_RESOLVE(AIL_set_3D_user_data, "_AIL_set_3D_user_data@12");
	MILES_RESOLVE(AIL_unlock, "_AIL_unlock@0");
	MILES_RESOLVE(AIL_unlock_mutex, "_AIL_unlock_mutex@0");
	MILES_RESOLVE(AIL_lock, "_AIL_lock@0");
	MILES_RESOLVE(AIL_lock_mutex, "_AIL_lock_mutex@0");
	MILES_RESOLVE(AIL_set_3D_speaker_type, "_AIL_set_3D_speaker_type@8");
	MILES_RESOLVE(AIL_close_3D_listener, "_AIL_close_3D_listener@4");
	MILES_RESOLVE(AIL_enumerate_3D_providers, "_AIL_enumerate_3D_providers@12");
	MILES_RESOLVE(AIL_open_3D_provider, "_AIL_open_3D_provider@4");
	MILES_RESOLVE(AIL_last_error, "_AIL_last_error@0");
	MILES_RESOLVE(AIL_open_3D_listener, "_AIL_open_3D_listener@4");
	MILES_RESOLVE(AIL_3D_user_data, "_AIL_3D_user_data@8");
	MILES_RESOLVE(AIL_sample_user_data, "_AIL_sample_user_data@8");
	MILES_RESOLVE(AIL_allocate_sample_handle, "_AIL_allocate_sample_handle@4");
	MILES_RESOLVE(AIL_set_sample_user_data, "_AIL_set_sample_user_data@12");
	MILES_RESOLVE(AIL_decompress_ADPCM, "_AIL_decompress_ADPCM@12");
	MILES_RESOLVE(AIL_get_DirectSound_info, "_AIL_get_DirectSound_info@12");
	MILES_RESOLVE(AIL_mem_free_lock, "_AIL_mem_free_lock@4");
	MILES_RESOLVE(AIL_open_stream, "_AIL_open_stream@12");
	MILES_RESOLVE(AIL_startup, "_AIL_startup@0");
	MILES_RESOLVE(AIL_quick_unload, "_AIL_quick_unload@4");
	MILES_RESOLVE(AIL_quick_load_and_play, "_AIL_quick_load_and_play@12");
	MILES_RESOLVE(AIL_quick_set_volume, "_AIL_quick_set_volume@12");
	MILES_RESOLVE(AIL_quick_startup, "_AIL_quick_startup@20");
	MILES_RESOLVE(AIL_quick_handles, "_AIL_quick_handles@12");
	MILES_RESOLVE(AIL_sample_volume_pan, "_AIL_sample_volume_pan@12");
	MILES_RESOLVE(AIL_set_3D_sample_occlusion, "_AIL_set_3D_sample_occlusion@8");
	MILES_RESOLVE(AIL_set_redist_directory, "_AIL_set_redist_directory@4");
	MILES_RESOLVE(AIL_set_sample_file, "_AIL_set_sample_file@12");
	MILES_RESOLVE(AIL_set_sample_volume_pan, "_AIL_set_sample_volume_pan@12");
	MILES_RESOLVE(AIL_set_stream_volume_pan, "_AIL_set_stream_volume_pan@12");
	MILES_RESOLVE(AIL_stream_volume_pan, "_AIL_stream_volume_pan@12");
	MILES_RESOLVE(AIL_get_timer_highest_delay, "_AIL_get_timer_highest_delay@0");
}
#undef MILES_RESOLVE

static void freeResources()
{
	if (Module != HMODULE(nullptr)) {
		::FreeLibrary(Module);
		Module = HMODULE(nullptr);
	}

	AIL_3D_sample_volumePtr = nullptr;
	AIL_set_3D_sample_volumePtr = nullptr;
	AIL_end_3D_samplePtr = nullptr;
	AIL_resume_3D_samplePtr = nullptr;
	AIL_stop_3D_samplePtr = nullptr;
	AIL_start_3D_samplePtr = nullptr;
	AIL_3D_sample_loop_countPtr = nullptr;
	AIL_set_3D_sample_offsetPtr = nullptr;
	AIL_3D_sample_lengthPtr = nullptr;
	AIL_3D_sample_offsetPtr = nullptr;
	AIL_3D_sample_playback_ratePtr = nullptr;
	AIL_set_3D_sample_playback_ratePtr = nullptr;
	AIL_set_3D_sample_filePtr = nullptr;
	AIL_set_sample_processorPtr = nullptr;
	AIL_set_filter_sample_preferencePtr = nullptr;
	AIL_release_sample_handlePtr = nullptr;
	AIL_close_3D_providerPtr = nullptr;
	AIL_set_preferencePtr = nullptr;
	AIL_waveOutOpenPtr = nullptr;
	AIL_waveOutClosePtr = nullptr;
	AIL_set_3D_sample_loop_countPtr = nullptr;
	AIL_set_stream_playback_ratePtr = nullptr;
	AIL_stream_playback_ratePtr = nullptr;
	AIL_stream_ms_positionPtr = nullptr;
	AIL_set_stream_ms_positionPtr = nullptr;
	AIL_stream_loop_countPtr = nullptr;
	AIL_set_stream_loop_blockPtr = nullptr;
	AIL_set_stream_loop_countPtr = nullptr;
	AIL_close_streamPtr = nullptr;
	AIL_pause_streamPtr = nullptr;
	AIL_register_stream_callbackPtr = nullptr;
	AIL_register_3D_EOS_callbackPtr = nullptr;
	AIL_register_EOS_callbackPtr = nullptr;
	AIL_start_streamPtr = nullptr;
	AIL_set_sample_playback_ratePtr = nullptr;
	AIL_sample_playback_ratePtr = nullptr;
	AIL_sample_ms_positionPtr = nullptr;
	AIL_set_sample_ms_positionPtr = nullptr;
	AIL_sample_loop_countPtr = nullptr;
	AIL_set_sample_loop_countPtr = nullptr;
	AIL_end_samplePtr = nullptr;
	AIL_resume_samplePtr = nullptr;
	AIL_stop_samplePtr = nullptr;
	AIL_start_samplePtr = nullptr;
	AIL_init_samplePtr = nullptr;
	AIL_set_named_sample_filePtr = nullptr;
	AIL_set_3D_sample_effects_levelPtr = nullptr;
	AIL_set_3D_sample_distancesPtr = nullptr;
	AIL_set_3D_velocity_vectorPtr = nullptr;
	AIL_set_3D_positionPtr = nullptr;
	AIL_set_3D_orientationPtr = nullptr;
	AIL_WAV_infoPtr = nullptr;
	AIL_stop_timerPtr = nullptr;
	AIL_release_timer_handlePtr = nullptr;
	AIL_shutdownPtr = nullptr;
	AIL_enumerate_filtersPtr = nullptr;
	AIL_set_file_callbacksPtr = nullptr;
	AIL_release_3D_sample_handlePtr = nullptr;
	AIL_allocate_3D_sample_handlePtr = nullptr;
	AIL_set_3D_user_dataPtr = nullptr;
	AIL_unlockPtr = nullptr;
	AIL_unlock_mutexPtr = nullptr;
	AIL_lockPtr = nullptr;
	AIL_lock_mutexPtr = nullptr;
	AIL_set_3D_speaker_typePtr = nullptr;
	AIL_close_3D_listenerPtr = nullptr;
	AIL_enumerate_3D_providersPtr = nullptr;
	AIL_open_3D_providerPtr = nullptr;
	AIL_last_errorPtr = nullptr;
	AIL_open_3D_listenerPtr = nullptr;
	AIL_3D_user_dataPtr = nullptr;
	AIL_sample_user_dataPtr = nullptr;
	AIL_allocate_sample_handlePtr = nullptr;
	AIL_set_sample_user_dataPtr = nullptr;
	AIL_decompress_ADPCMPtr = nullptr;
	AIL_get_DirectSound_infoPtr = nullptr;
	AIL_mem_free_lockPtr = nullptr;
	AIL_open_streamPtr = nullptr;
	AIL_startupPtr = nullptr;
	AIL_quick_unloadPtr = nullptr;
	AIL_quick_load_and_playPtr = nullptr;
	AIL_quick_set_volumePtr = nullptr;
	AIL_quick_startupPtr = nullptr;
	AIL_quick_handlesPtr = nullptr;
	AIL_sample_volume_panPtr = nullptr;
	AIL_set_3D_sample_occlusionPtr = nullptr;
	AIL_set_redist_directoryPtr = nullptr;
	AIL_set_sample_filePtr = nullptr;
	AIL_set_sample_volume_panPtr = nullptr;
	AIL_set_stream_volume_panPtr = nullptr;
	AIL_stream_volume_panPtr = nullptr;
	AIL_get_timer_highest_delayPtr = nullptr;
}
#endif // MILES_LOADER_SUPPORTED

} // namespace


bool MilesLoader::isLoaded()
{
#if MILES_LOADER_SUPPORTED
	return Module != HMODULE(nullptr);
#else
	return false;
#endif
}


bool MilesLoader::isFailed()
{
	return Failed;
}


unsigned long MilesLoader::getLastError()
{
	return LastError;
}


bool MilesLoader::load()
{
	// Always increment the reference count.
	++ReferenceCount;

	// Optimization: return early if it failed before.
	if (Failed)
		return false;

	// Return early if someone else already loaded it.
	if (ReferenceCount > 1)
		return true;

#if MILES_LOADER_SUPPORTED
	// Load mss32.dll by name, so that the usual module search order applies.
	Module = ::LoadLibraryA("mss32.dll");
	if (Module == HMODULE(nullptr)) {
		LastError = ::GetLastError();
		Failed = true;
		return false;
	}

	resolveAll();
	return true;
#else
	Failed = true;
	return false;
#endif
}


void MilesLoader::unload()
{
	if (ReferenceCount > 0)
		--ReferenceCount;

	if (ReferenceCount > 0)
		return;

#if MILES_LOADER_SUPPORTED
	freeResources();
#endif
	Failed = false;
	LastError = 0;
}


// The Miles functions below stand in for the imports of mss32.dll. An unresolved function returns
// the same neutral value the Miles SDK stub library returned.


F32 __stdcall AIL_3D_sample_volume(H3DSAMPLE sample)
{
	return AIL_3D_sample_volumePtr != nullptr ? AIL_3D_sample_volumePtr(sample) : 0.0f;
}

void __stdcall AIL_set_3D_sample_volume(H3DSAMPLE sample, F32 volume)
{
	if (AIL_set_3D_sample_volumePtr != nullptr)
		AIL_set_3D_sample_volumePtr(sample, volume);
}

void __stdcall AIL_end_3D_sample(H3DSAMPLE sample)
{
	if (AIL_end_3D_samplePtr != nullptr)
		AIL_end_3D_samplePtr(sample);
}

void __stdcall AIL_resume_3D_sample(H3DSAMPLE sample)
{
	if (AIL_resume_3D_samplePtr != nullptr)
		AIL_resume_3D_samplePtr(sample);
}

void __stdcall AIL_stop_3D_sample(H3DSAMPLE sample)
{
	if (AIL_stop_3D_samplePtr != nullptr)
		AIL_stop_3D_samplePtr(sample);
}

void __stdcall AIL_start_3D_sample(H3DSAMPLE sample)
{
	if (AIL_start_3D_samplePtr != nullptr)
		AIL_start_3D_samplePtr(sample);
}

U32 __stdcall AIL_3D_sample_loop_count(H3DSAMPLE sample)
{
	return AIL_3D_sample_loop_countPtr != nullptr ? AIL_3D_sample_loop_countPtr(sample) : 0;
}

void __stdcall AIL_set_3D_sample_offset(H3DSAMPLE sample, U32 offset)
{
	if (AIL_set_3D_sample_offsetPtr != nullptr)
		AIL_set_3D_sample_offsetPtr(sample, offset);
}

U32 __stdcall AIL_3D_sample_length(H3DSAMPLE sample)
{
	return AIL_3D_sample_lengthPtr != nullptr ? AIL_3D_sample_lengthPtr(sample) : 0;
}

U32 __stdcall AIL_3D_sample_offset(H3DSAMPLE sample)
{
	return AIL_3D_sample_offsetPtr != nullptr ? AIL_3D_sample_offsetPtr(sample) : 0;
}

S32 __stdcall AIL_3D_sample_playback_rate(H3DSAMPLE sample)
{
	return AIL_3D_sample_playback_ratePtr != nullptr ? AIL_3D_sample_playback_ratePtr(sample) : 0;
}

void __stdcall AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, S32 playback_rate)
{
	if (AIL_set_3D_sample_playback_ratePtr != nullptr)
		AIL_set_3D_sample_playback_ratePtr(sample, playback_rate);
}

S32 __stdcall AIL_set_3D_sample_file(H3DSAMPLE sample, const void* file_image)
{
	return AIL_set_3D_sample_filePtr != nullptr ? AIL_set_3D_sample_filePtr(sample, file_image) : 0;
}

HPROVIDER __stdcall AIL_set_sample_processor(HSAMPLE sample, SAMPLESTAGE pipeline_stage, HPROVIDER provider)
{
	return AIL_set_sample_processorPtr != nullptr
		? AIL_set_sample_processorPtr(sample, pipeline_stage, provider)
		: nullptr;
}

void __stdcall AIL_set_filter_sample_preference(HSAMPLE sample, const char* name, const void* val)
{
	if (AIL_set_filter_sample_preferencePtr != nullptr)
		AIL_set_filter_sample_preferencePtr(sample, name, val);
}

void __stdcall AIL_release_sample_handle(HSAMPLE sample)
{
	if (AIL_release_sample_handlePtr != nullptr)
		AIL_release_sample_handlePtr(sample);
}

void __stdcall AIL_close_3D_provider(HPROVIDER lib)
{
	if (AIL_close_3D_providerPtr != nullptr)
		AIL_close_3D_providerPtr(lib);
}

S32 __stdcall AIL_set_preference(U32 number, S32 value)
{
	return AIL_set_preferencePtr != nullptr ? AIL_set_preferencePtr(number, value) : 0;
}

S32 __stdcall AIL_waveOutOpen(HDIGDRIVER* driver, LPHWAVEOUT* waveout, S32 id, LPWAVEFORMAT format)
{
	if (AIL_waveOutOpenPtr != nullptr)
		return AIL_waveOutOpenPtr(driver, waveout, id, format);

	if (driver != nullptr)
		*driver = nullptr;
	if (waveout != nullptr)
		*waveout = nullptr;
	return 1;
}

void __stdcall AIL_waveOutClose(HDIGDRIVER driver)
{
	if (AIL_waveOutClosePtr != nullptr)
		AIL_waveOutClosePtr(driver);
}

void __stdcall AIL_set_3D_sample_loop_count(H3DSAMPLE sample, U32 count)
{
	if (AIL_set_3D_sample_loop_countPtr != nullptr)
		AIL_set_3D_sample_loop_countPtr(sample, count);
}

void __stdcall AIL_set_stream_playback_rate(HSTREAM stream, S32 rate)
{
	if (AIL_set_stream_playback_ratePtr != nullptr)
		AIL_set_stream_playback_ratePtr(stream, rate);
}

S32 __stdcall AIL_stream_playback_rate(HSTREAM stream)
{
	return AIL_stream_playback_ratePtr != nullptr ? AIL_stream_playback_ratePtr(stream) : 0;
}

void __stdcall AIL_stream_ms_position(HSTREAM sample, S32* total_milliseconds, S32* current_milliseconds)
{
	if (AIL_stream_ms_positionPtr != nullptr)
	{
		AIL_stream_ms_positionPtr(sample, total_milliseconds, current_milliseconds);
		return;
	}

	if (total_milliseconds != nullptr)
		*total_milliseconds = 0;
	if (current_milliseconds != nullptr)
		*current_milliseconds = 0;
}

void __stdcall AIL_set_stream_ms_position(HSTREAM stream, S32 pos)
{
	if (AIL_set_stream_ms_positionPtr != nullptr)
		AIL_set_stream_ms_positionPtr(stream, pos);
}

S32 __stdcall AIL_stream_loop_count(HSTREAM stream)
{
	return AIL_stream_loop_countPtr != nullptr ? AIL_stream_loop_countPtr(stream) : 0;
}

void __stdcall AIL_set_stream_loop_block(HSTREAM stream, S32 loop_start, S32 loop_end)
{
	if (AIL_set_stream_loop_blockPtr != nullptr)
		AIL_set_stream_loop_blockPtr(stream, loop_start, loop_end);
}

void __stdcall AIL_set_stream_loop_count(HSTREAM stream, S32 count)
{
	if (AIL_set_stream_loop_countPtr != nullptr)
		AIL_set_stream_loop_countPtr(stream, count);
}

void __stdcall AIL_close_stream(HSTREAM stream)
{
	if (AIL_close_streamPtr != nullptr)
		AIL_close_streamPtr(stream);
}

void __stdcall AIL_pause_stream(HSTREAM stream, S32 onoff)
{
	if (AIL_pause_streamPtr != nullptr)
		AIL_pause_streamPtr(stream, onoff);
}

AIL_stream_callback __stdcall AIL_register_stream_callback(HSTREAM stream, AIL_stream_callback callback)
{
	return AIL_register_stream_callbackPtr != nullptr ? AIL_register_stream_callbackPtr(stream, callback) : nullptr;
}

AIL_3dsample_callback __stdcall AIL_register_3D_EOS_callback(H3DSAMPLE sample, AIL_3dsample_callback EOS)
{
	return AIL_register_3D_EOS_callbackPtr != nullptr ? AIL_register_3D_EOS_callbackPtr(sample, EOS) : nullptr;
}

AIL_sample_callback __stdcall AIL_register_EOS_callback(HSAMPLE sample, AIL_sample_callback EOS)
{
	return AIL_register_EOS_callbackPtr != nullptr ? AIL_register_EOS_callbackPtr(sample, EOS) : nullptr;
}

void __stdcall AIL_start_stream(HSTREAM stream)
{
	if (AIL_start_streamPtr != nullptr)
		AIL_start_streamPtr(stream);
}

void __stdcall AIL_set_sample_playback_rate(HSAMPLE sample, S32 playback_rate)
{
	if (AIL_set_sample_playback_ratePtr != nullptr)
		AIL_set_sample_playback_ratePtr(sample, playback_rate);
}

S32 __stdcall AIL_sample_playback_rate(HSAMPLE sample)
{
	return AIL_sample_playback_ratePtr != nullptr ? AIL_sample_playback_ratePtr(sample) : 0;
}

void __stdcall AIL_sample_ms_position(HSAMPLE sample, S32* total_ms, S32* current_ms)
{
	if (AIL_sample_ms_positionPtr != nullptr)
	{
		AIL_sample_ms_positionPtr(sample, total_ms, current_ms);
		return;
	}

	if (total_ms != nullptr)
		*total_ms = 0;
	if (current_ms != nullptr)
		*current_ms = 0;
}

void __stdcall AIL_set_sample_ms_position(HSAMPLE sample, S32 pos)
{
	if (AIL_set_sample_ms_positionPtr != nullptr)
		AIL_set_sample_ms_positionPtr(sample, pos);
}

S32 __stdcall AIL_sample_loop_count(HSAMPLE sample)
{
	return AIL_sample_loop_countPtr != nullptr ? AIL_sample_loop_countPtr(sample) : 0;
}

void __stdcall AIL_set_sample_loop_count(HSAMPLE sample, S32 count)
{
	if (AIL_set_sample_loop_countPtr != nullptr)
		AIL_set_sample_loop_countPtr(sample, count);
}

void __stdcall AIL_end_sample(HSAMPLE sample)
{
	if (AIL_end_samplePtr != nullptr)
		AIL_end_samplePtr(sample);
}

void __stdcall AIL_resume_sample(HSAMPLE sample)
{
	if (AIL_resume_samplePtr != nullptr)
		AIL_resume_samplePtr(sample);
}

void __stdcall AIL_stop_sample(HSAMPLE sample)
{
	if (AIL_stop_samplePtr != nullptr)
		AIL_stop_samplePtr(sample);
}

void __stdcall AIL_start_sample(HSAMPLE sample)
{
	if (AIL_start_samplePtr != nullptr)
		AIL_start_samplePtr(sample);
}

void __stdcall AIL_init_sample(HSAMPLE sample)
{
	if (AIL_init_samplePtr != nullptr)
		AIL_init_samplePtr(sample);
}

S32 __stdcall AIL_set_named_sample_file(HSAMPLE sample, const char* file_name, const void* file_image, S32 file_size, S32 block)
{
	return AIL_set_named_sample_filePtr != nullptr
		? AIL_set_named_sample_filePtr(sample, file_name, file_image, file_size, block)
		: 0;
}

void __stdcall AIL_set_3D_sample_effects_level(H3DSAMPLE sample, F32 effect_level)
{
	if (AIL_set_3D_sample_effects_levelPtr != nullptr)
		AIL_set_3D_sample_effects_levelPtr(sample, effect_level);
}

void __stdcall AIL_set_3D_sample_distances(H3DSAMPLE sample, F32 max_dist, F32 min_dist)
{
	if (AIL_set_3D_sample_distancesPtr != nullptr)
		AIL_set_3D_sample_distancesPtr(sample, max_dist, min_dist);
}

void __stdcall AIL_set_3D_velocity_vector(H3DPOBJECT obj, F32 x, F32 y, F32 z)
{
	if (AIL_set_3D_velocity_vectorPtr != nullptr)
		AIL_set_3D_velocity_vectorPtr(obj, x, y, z);
}

void __stdcall AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z)
{
	if (AIL_set_3D_positionPtr != nullptr)
		AIL_set_3D_positionPtr(obj, X, Y, Z);
}

void __stdcall AIL_set_3D_orientation(H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up)
{
	if (AIL_set_3D_orientationPtr != nullptr)
		AIL_set_3D_orientationPtr(obj, X_face, Y_face, Z_face, X_up, Y_up, Z_up);
}

S32 __stdcall AIL_WAV_info(const void* data, AILSOUNDINFO* info)
{
	if (AIL_WAV_infoPtr != nullptr)
		return AIL_WAV_infoPtr(data, info);

	if (info != nullptr)
	{
		info->format = 0;
		info->data_ptr = nullptr;
		info->data_len = 0;
		info->rate = 0;
		info->bits = 0;
		info->channels = 0;
		info->samples = 0;
		info->block_size = 0;
		info->initial_ptr = nullptr;
	}
	return 0;
}

void __stdcall AIL_stop_timer(HTIMER timer)
{
	if (AIL_stop_timerPtr != nullptr)
		AIL_stop_timerPtr(timer);
}

void __stdcall AIL_release_timer_handle(HTIMER timer)
{
	if (AIL_release_timer_handlePtr != nullptr)
		AIL_release_timer_handlePtr(timer);
}

void __stdcall AIL_shutdown(void)
{
	if (AIL_shutdownPtr != nullptr)
		AIL_shutdownPtr();
}

S32 __stdcall AIL_enumerate_filters(HPROENUM* next, HPROVIDER* dest, char** name)
{
	if (AIL_enumerate_filtersPtr != nullptr)
		return AIL_enumerate_filtersPtr(next, dest, name);

	if (dest != nullptr)
		*dest = nullptr;
	if (name != nullptr)
		*name = nullptr;
	return 0;
}

void __stdcall AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb, AIL_file_seek_callback seekcb, AIL_file_read_callback readcb)
{
	if (AIL_set_file_callbacksPtr != nullptr)
		AIL_set_file_callbacksPtr(opencb, closecb, seekcb, readcb);
}

void __stdcall AIL_release_3D_sample_handle(H3DSAMPLE sample)
{
	if (AIL_release_3D_sample_handlePtr != nullptr)
		AIL_release_3D_sample_handlePtr(sample);
}

H3DSAMPLE __stdcall AIL_allocate_3D_sample_handle(HPROVIDER lib)
{
	return AIL_allocate_3D_sample_handlePtr != nullptr ? AIL_allocate_3D_sample_handlePtr(lib) : nullptr;
}

void __stdcall AIL_set_3D_user_data(H3DPOBJECT obj, U32 index, S32 value)
{
	if (AIL_set_3D_user_dataPtr != nullptr)
		AIL_set_3D_user_dataPtr(obj, index, value);
}

void __stdcall AIL_unlock(void)
{
	if (AIL_unlockPtr != nullptr)
		AIL_unlockPtr();
}

void __stdcall AIL_unlock_mutex(void)
{
	if (AIL_unlock_mutexPtr != nullptr)
		AIL_unlock_mutexPtr();
}

void __stdcall AIL_lock(void)
{
	if (AIL_lockPtr != nullptr)
		AIL_lockPtr();
}

void __stdcall AIL_lock_mutex(void)
{
	if (AIL_lock_mutexPtr != nullptr)
		AIL_lock_mutexPtr();
}

void __stdcall AIL_set_3D_speaker_type(HPROVIDER lib, S32 speaker_type)
{
	if (AIL_set_3D_speaker_typePtr != nullptr)
		AIL_set_3D_speaker_typePtr(lib, speaker_type);
}

void __stdcall AIL_close_3D_listener(H3DPOBJECT listener)
{
	if (AIL_close_3D_listenerPtr != nullptr)
		AIL_close_3D_listenerPtr(listener);
}

S32 __stdcall AIL_enumerate_3D_providers(HPROENUM* next, HPROVIDER* dest, char** name)
{
	if (AIL_enumerate_3D_providersPtr != nullptr)
		return AIL_enumerate_3D_providersPtr(next, dest, name);

	if (dest != nullptr)
		*dest = nullptr;
	if (name != nullptr)
		*name = nullptr;
	return 0;
}

M3DRESULT __stdcall AIL_open_3D_provider(HPROVIDER lib)
{
	return AIL_open_3D_providerPtr != nullptr ? AIL_open_3D_providerPtr(lib) : 1;
}

char* __stdcall AIL_last_error(void)
{
	return AIL_last_errorPtr != nullptr ? AIL_last_errorPtr() : nullptr;
}

H3DPOBJECT __stdcall AIL_open_3D_listener(HPROVIDER lib)
{
	return AIL_open_3D_listenerPtr != nullptr ? AIL_open_3D_listenerPtr(lib) : nullptr;
}

S32 __stdcall AIL_3D_user_data(H3DPOBJECT obj, U32 index)
{
	return AIL_3D_user_dataPtr != nullptr ? AIL_3D_user_dataPtr(obj, index) : 0;
}

S32 __stdcall AIL_sample_user_data(HSAMPLE sample, U32 index)
{
	return AIL_sample_user_dataPtr != nullptr ? AIL_sample_user_dataPtr(sample, index) : 0;
}

HSAMPLE __stdcall AIL_allocate_sample_handle(HDIGDRIVER dig)
{
	return AIL_allocate_sample_handlePtr != nullptr ? AIL_allocate_sample_handlePtr(dig) : nullptr;
}

void __stdcall AIL_set_sample_user_data(HSAMPLE sample, U32 index, S32 value)
{
	if (AIL_set_sample_user_dataPtr != nullptr)
		AIL_set_sample_user_dataPtr(sample, index, value);
}

S32 __stdcall AIL_decompress_ADPCM(const AILSOUNDINFO *info, void **outdata, U32 *outsize)
{
	if (AIL_decompress_ADPCMPtr != nullptr)
		return AIL_decompress_ADPCMPtr(info, outdata, outsize);

	if (outdata != nullptr)
		*outdata = nullptr;
	if (outsize != nullptr)
		*outsize = 0;
	return 0;
}

void __stdcall AIL_get_DirectSound_info(HSAMPLE sample, AILLPDIRECTSOUND *lplpDS, AILLPDIRECTSOUNDBUFFER *lplpDSB)
{
	if (AIL_get_DirectSound_infoPtr != nullptr)
	{
		AIL_get_DirectSound_infoPtr(sample, lplpDS, lplpDSB);
		return;
	}

	if (lplpDS != nullptr)
		*lplpDS = nullptr;
	if (lplpDSB != nullptr)
		*lplpDSB = nullptr;
}

void __stdcall AIL_mem_free_lock(void *ptr)
{
	if (AIL_mem_free_lockPtr != nullptr)
		AIL_mem_free_lockPtr(ptr);
}

HSTREAM __stdcall AIL_open_stream(HDIGDRIVER dig, const char *filename, S32 stream_mem)
{
	return AIL_open_streamPtr != nullptr ? AIL_open_streamPtr(dig, filename, stream_mem) : nullptr;
}

S32 __stdcall AIL_startup(void)
{
	return AIL_startupPtr != nullptr ? AIL_startupPtr() : 0;
}

void __stdcall AIL_quick_unload(HAUDIO audio)
{
	if (AIL_quick_unloadPtr != nullptr)
		AIL_quick_unloadPtr(audio);
}

HAUDIO __stdcall AIL_quick_load_and_play(const char *filename, U32 loop_count, S32 wait_request)
{
	return AIL_quick_load_and_playPtr != nullptr
		? AIL_quick_load_and_playPtr(filename, loop_count, wait_request)
		: nullptr;
}

void __stdcall AIL_quick_set_volume(HAUDIO audio, F32 volume, F32 extravol)
{
	if (AIL_quick_set_volumePtr != nullptr)
		AIL_quick_set_volumePtr(audio, volume, extravol);
}

S32 __stdcall AIL_quick_startup(S32 use_digital, S32 use_MIDI, U32 output_rate, S32 output_bits, S32 output_channels)
{
	return AIL_quick_startupPtr != nullptr
		? AIL_quick_startupPtr(use_digital, use_MIDI, output_rate, output_bits, output_channels)
		: 0;
}

void __stdcall AIL_quick_handles(HDIGDRIVER *pdig, HMDIDRIVER *pmdi, HDLSDEVICE *pdls)
{
	if (AIL_quick_handlesPtr != nullptr)
	{
		AIL_quick_handlesPtr(pdig, pmdi, pdls);
		return;
	}

	if (pdig != nullptr)
		*pdig = nullptr;
	if (pmdi != nullptr)
		*pmdi = nullptr;
	if (pdls != nullptr)
		*pdls = nullptr;
}

void __stdcall AIL_sample_volume_pan(HSAMPLE sample, F32 *volume, F32 *pan)
{
	if (AIL_sample_volume_panPtr != nullptr)
	{
		AIL_sample_volume_panPtr(sample, volume, pan);
		return;
	}

	if (volume != nullptr)
		*volume = 0.0f;
	if (pan != nullptr)
		*pan = 0.5f;
}

void __stdcall AIL_set_3D_sample_occlusion(H3DSAMPLE sample, F32 occlusion)
{
	if (AIL_set_3D_sample_occlusionPtr != nullptr)
		AIL_set_3D_sample_occlusionPtr(sample, occlusion);
}

char * __stdcall AIL_set_redist_directory(const char *dir)
{
	return AIL_set_redist_directoryPtr != nullptr ? AIL_set_redist_directoryPtr(dir) : nullptr;
}

S32 __stdcall AIL_set_sample_file(HSAMPLE sample, const void *file_image, S32 block)
{
	return AIL_set_sample_filePtr != nullptr ? AIL_set_sample_filePtr(sample, file_image, block) : 0;
}

void __stdcall AIL_set_sample_volume_pan(HSAMPLE sample, F32 volume, F32 pan)
{
	if (AIL_set_sample_volume_panPtr != nullptr)
		AIL_set_sample_volume_panPtr(sample, volume, pan);
}

void __stdcall AIL_set_stream_volume_pan(HSTREAM stream, F32 volume, F32 pan)
{
	if (AIL_set_stream_volume_panPtr != nullptr)
		AIL_set_stream_volume_panPtr(stream, volume, pan);
}

void __stdcall AIL_stream_volume_pan(HSTREAM stream, F32 *volume, F32 *pan)
{
	if (AIL_stream_volume_panPtr != nullptr)
	{
		AIL_stream_volume_panPtr(stream, volume, pan);
		return;
	}

	if (volume != nullptr)
		*volume = 0.0f;
	if (pan != nullptr)
		*pan = 0.5f;
}

U32 __stdcall AIL_get_timer_highest_delay(void)
{
	return AIL_get_timer_highest_delayPtr != nullptr ? AIL_get_timer_highest_delayPtr() : 0;
}
