/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Subset of the mss32.dll API as used by the W3D engine.
 *
 * @copyright This is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            3 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#ifndef STRICT
#define STRICT
#endif

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#else
typedef struct WAVEOUT* HWAVEOUT;
typedef struct WAVEHDR* LPWAVEHDR;
typedef struct WAVEFORMAT *LPWAVEFORMAT;
#include <stdint.h>

typedef struct WAVEFORMAT
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
} WAVEFORMAT;

typedef struct PCMWAVEFORMAT
{
    WAVEFORMAT wf;
    uint16_t wBitsPerSample;
} PCMWAVEFORMAT;

#define WAVE_FORMAT_PCM 0x0001

typedef HWAVEOUT *LPHWAVEOUT;
#endif

#if !defined _MSC_VER
#if !defined(__stdcall)
#if defined __has_attribute && __has_attribute(stdcall)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif /* !defined __stdcall */
#endif /* !defined COMPILER_MSVC */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long U32;
typedef long S32;
typedef float F32;

/*
 * These will have been structs in the real SDK headers, but are only accessed through pointers
 * so we don't care unless access to the internals is required.
 */
typedef struct h3DPOBJECT
{
    unsigned int junk;
} h3DPOBJECT;
typedef h3DPOBJECT *H3DPOBJECT;
typedef H3DPOBJECT H3DSAMPLE;
typedef struct _SAMPLE *HSAMPLE;
typedef struct _STREAM *HSTREAM;
typedef struct _DIG_DRIVER
{
    char pad[168];
    int emulated_ds; // We only care about this particular member for our purposes.
} DIG_DRIVER;
typedef struct _DIG_DRIVER *HDIGDRIVER;
typedef struct _AUDIO *HAUDIO;
typedef struct _HMDIDRIVER *HMDIDRIVER;
typedef struct _HDLSDEVICE *HDLSDEVICE;
typedef void *HPROVIDER;
typedef S32 HTIMER;
typedef U32 HPROENUM;
typedef S32 M3DRESULT;

typedef void *AILLPDIRECTSOUND;
typedef void *AILLPDIRECTSOUNDBUFFER;

typedef struct _AILSOUNDINFO
{
    S32 format;
    const void *data_ptr;
    U32 data_len;
    U32 rate;
    S32 bits;
    S32 channels;
    U32 samples;
    U32 block_size;
    const void *initial_ptr;
} AILSOUNDINFO;

typedef enum
{
    DP_ASI_DECODER = 0, // Must be "ASI codec stream" provider
    DP_FILTER, // Must be "MSS pipeline filter" provider
    DP_MERGE, // Must be "MSS mixer" provider
    N_SAMPLE_STAGES, // Placeholder for end of list (= # of valid stages)
    SAMPLE_ALL_STAGES // Used to signify all pipeline stages, for shutdown
} SAMPLESTAGE;

#define AILCALLBACK __stdcall

#define AIL_set_3D_object_user_data AIL_set_3D_user_data
#define AIL_3D_object_user_data AIL_3D_user_data
#define AIL_3D_open_listener AIL_open_3D_listener

/*
 * Various callback typedefs.
 */
typedef U32(__stdcall *AIL_file_open_callback)(const char *, void**);
typedef void(__stdcall *AIL_file_close_callback)(void*);
typedef S32(__stdcall *AIL_file_seek_callback)(void*, S32, U32);
typedef U32(__stdcall *AIL_file_read_callback)(void*, void *, U32);
typedef void(__stdcall *AIL_stream_callback)(HSTREAM);
typedef void(__stdcall *AIL_3dsample_callback)(H3DPOBJECT);
typedef void(__stdcall *AIL_sample_callback)(HSAMPLE);

#define DIG_USE_WAVEOUT 15
#define AIL_LOCK_PROTECTION 18
#define WAVE_FORMAT_IMA_ADPCM 0x11
#define ENVIRONMENT_GENERIC 0
#define HPROENUM_FIRST 0
#define AIL_NO_ERROR 0
#define AIL_FILE_SEEK_BEGIN 0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END 2
#define AIL_3D_2_SPEAKER 0
#define AIL_3D_HEADPHONE 1
#define AIL_3D_SURROUND 2
#define AIL_3D_4_SPEAKER 3
#define AIL_3D_51_SPEAKER 4
#define AIL_3D_71_SPEAKER 5
#define AIL_MSS_version(a, b)
#define M3D_NOERR 0

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO  0
#endif

F32 __stdcall AIL_3D_sample_volume(H3DSAMPLE sample);
void __stdcall AIL_set_3D_sample_volume(H3DSAMPLE sample, F32 volume);
void __stdcall AIL_end_3D_sample(H3DSAMPLE sample);
void __stdcall AIL_resume_3D_sample(H3DSAMPLE sample);
void __stdcall AIL_stop_3D_sample(H3DSAMPLE sample);
void __stdcall AIL_start_3D_sample(H3DSAMPLE sample);
U32 __stdcall AIL_3D_sample_loop_count(H3DSAMPLE sample);
void __stdcall AIL_set_3D_sample_offset(H3DSAMPLE sample, U32 offset);
U32 __stdcall AIL_3D_sample_length(H3DSAMPLE sample);
U32 __stdcall AIL_3D_sample_offset(H3DSAMPLE sample);
S32 __stdcall AIL_3D_sample_playback_rate(H3DSAMPLE sample);
void __stdcall AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, S32 playback_rate);
S32 __stdcall AIL_set_3D_sample_file(H3DSAMPLE sample, const void* file_image);
HPROVIDER __stdcall AIL_set_sample_processor(HSAMPLE sample, SAMPLESTAGE pipeline_stage, HPROVIDER provider);
void __stdcall AIL_set_filter_sample_preference(HSAMPLE sample, const char* name, const void* val);
void __stdcall AIL_release_sample_handle(HSAMPLE sample);
void __stdcall AIL_close_3D_provider(HPROVIDER lib);
S32 __stdcall AIL_set_preference(U32 number, S32 value);
S32 __stdcall AIL_waveOutOpen(HDIGDRIVER* driver, LPHWAVEOUT* waveout, S32 id, LPWAVEFORMAT format);
void __stdcall AIL_waveOutClose(HDIGDRIVER driver);
void __stdcall AIL_set_3D_sample_loop_count(H3DSAMPLE sample, U32 count);
void __stdcall AIL_set_stream_playback_rate(HSTREAM stream, S32 rate);
S32 __stdcall AIL_stream_playback_rate(HSTREAM stream);
void __stdcall AIL_stream_ms_position(HSTREAM sample, S32* total_milliseconds, S32* current_milliseconds);
void __stdcall AIL_set_stream_ms_position(HSTREAM stream, S32 pos);
S32 __stdcall AIL_stream_loop_count(HSTREAM stream);
void __stdcall AIL_set_stream_loop_block(HSTREAM stream, S32 loop_start, S32 loop_end);
void __stdcall AIL_set_stream_loop_count(HSTREAM stream, S32 count);
void __stdcall AIL_close_stream(HSTREAM stream);
void __stdcall AIL_pause_stream(HSTREAM stream, S32 onoff);
AIL_stream_callback __stdcall AIL_register_stream_callback(HSTREAM stream, AIL_stream_callback callback);
AIL_3dsample_callback __stdcall AIL_register_3D_EOS_callback(H3DSAMPLE sample, AIL_3dsample_callback EOS);
AIL_sample_callback __stdcall AIL_register_EOS_callback(HSAMPLE sample, AIL_sample_callback EOS);
void __stdcall AIL_start_stream(HSTREAM stream);
void __stdcall AIL_set_sample_playback_rate(HSAMPLE sample, S32 playback_rate);
S32 __stdcall AIL_sample_playback_rate(HSAMPLE sample);
void __stdcall AIL_sample_ms_position(HSAMPLE sample, S32* total_ms, S32* current_ms);
void __stdcall AIL_set_sample_ms_position(HSAMPLE sample, S32 pos);
S32 __stdcall AIL_sample_loop_count(HSAMPLE sample);
void __stdcall AIL_set_sample_loop_count(HSAMPLE sample, S32 count);
void __stdcall AIL_end_sample(HSAMPLE sample);
void __stdcall AIL_resume_sample(HSAMPLE sample);
void __stdcall AIL_stop_sample(HSAMPLE sample);
void __stdcall AIL_start_sample(HSAMPLE sample);
void __stdcall AIL_init_sample(HSAMPLE sample);
S32 __stdcall AIL_set_named_sample_file(
    HSAMPLE sample, const char* file_name, const void* file_image, S32 file_size, S32 block);
void __stdcall AIL_set_3D_sample_effects_level(H3DSAMPLE sample, F32 effect_level);
void __stdcall AIL_set_3D_sample_distances(H3DSAMPLE sample, F32 max_dist, F32 min_dist);
void __stdcall AIL_set_3D_velocity_vector(H3DPOBJECT obj, F32 x, F32 y, F32 z);
void __stdcall AIL_set_3D_position(H3DPOBJECT obj, F32 X, F32 Y, F32 Z);
void __stdcall AIL_set_3D_orientation(
    H3DPOBJECT obj, F32 X_face, F32 Y_face, F32 Z_face, F32 X_up, F32 Y_up, F32 Z_up);
S32 __stdcall AIL_WAV_info(const void* data, AILSOUNDINFO* info);
void __stdcall AIL_stop_timer(HTIMER timer);
void __stdcall AIL_release_timer_handle(HTIMER timer);
void __stdcall AIL_shutdown(void);
S32 __stdcall AIL_enumerate_filters(HPROENUM* next, HPROVIDER* dest, char** name);
void __stdcall AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb,
    AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);
void __stdcall AIL_release_3D_sample_handle(H3DSAMPLE sample);
H3DSAMPLE __stdcall AIL_allocate_3D_sample_handle(HPROVIDER lib);
void __stdcall AIL_set_3D_user_data(H3DPOBJECT obj, U32 index, S32 value);
void __stdcall AIL_unlock(void);
void __stdcall AIL_unlock_mutex(void);
void __stdcall AIL_lock(void);
void __stdcall AIL_lock_mutex(void);
void __stdcall AIL_set_3D_speaker_type(HPROVIDER lib, S32 speaker_type);
void __stdcall AIL_close_3D_listener(H3DPOBJECT listener);
S32 __stdcall AIL_enumerate_3D_providers(HPROENUM* next, HPROVIDER* dest, char** name);
M3DRESULT __stdcall AIL_open_3D_provider(HPROVIDER lib);
char* __stdcall AIL_last_error(void);
H3DPOBJECT __stdcall AIL_open_3D_listener(HPROVIDER lib);
S32 __stdcall AIL_3D_user_data(H3DPOBJECT obj, U32 index);
S32 __stdcall AIL_sample_user_data(HSAMPLE sample, U32 index);
HSAMPLE __stdcall AIL_allocate_sample_handle(HDIGDRIVER dig);
void __stdcall AIL_set_sample_user_data(HSAMPLE sample, U32 index, S32 value);
S32 __stdcall AIL_decompress_ADPCM(const AILSOUNDINFO *info, void **outdata, U32 *outsize);
void __stdcall AIL_get_DirectSound_info(HSAMPLE sample, AILLPDIRECTSOUND *lplpDS, AILLPDIRECTSOUNDBUFFER *lplpDSB);
void __stdcall AIL_mem_free_lock(void *ptr);
HSTREAM __stdcall AIL_open_stream(HDIGDRIVER dig, const char *filename, S32 stream_mem);
S32 __stdcall AIL_startup(void);
void __stdcall AIL_quick_unload(HAUDIO audio);
HAUDIO __stdcall AIL_quick_load_and_play(const char *filename, U32 loop_count, S32 wait_request);
void __stdcall AIL_quick_set_volume(HAUDIO audio, F32 volume, F32 extravol);
S32 __stdcall AIL_quick_startup(
    S32 use_digital, S32 use_MIDI, U32 output_rate, S32 output_bits, S32 output_channels);
void __stdcall AIL_quick_handles(HDIGDRIVER *pdig, HMDIDRIVER *pmdi, HDLSDEVICE *pdls);
void __stdcall AIL_sample_volume_pan(HSAMPLE sample, F32 *volume, F32 *pan);
void __stdcall AIL_set_3D_sample_occlusion(H3DSAMPLE sample, F32 occlusion);
char *__stdcall AIL_set_redist_directory(const char *dir);
S32 __stdcall AIL_set_sample_file(HSAMPLE sample, const void *file_image, S32 block);
void __stdcall AIL_set_sample_volume_pan(HSAMPLE sample, F32 volume, F32 pan);
void __stdcall AIL_set_stream_volume_pan(HSTREAM stream, F32 volume, F32 pan);
void __stdcall AIL_stream_volume_pan(HSTREAM stream, F32 *volume, F32 *pan);
U32 __stdcall AIL_get_timer_highest_delay(void);

#ifdef __cplusplus
} // extern "C"
#endif
