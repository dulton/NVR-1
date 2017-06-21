#ifndef _ISO_FORMAT_H_
#define _ISO_FORMAT_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iflytype.h"

/* Unsigned 8 bit */
#ifndef IFLYMP4_RAW
#define IFLYMP4_RAW "raw "
#endif

/* IMA4 */
#define IFLYMP4_IMA4 "ima4"

/* Twos compliment 8, 16, 24 */
#define IFLYMP4_TWOS "twos"

/* ulaw */
#define IFLYMP4_ULAW "ulaw"

typedef char				bit8;
typedef short				bit16;
typedef int					bit32;

#define _VERSION_			0
#if(_VERSION_==0)
typedef UINT32				UINTVAR;
typedef INT32				INTVAR;
#else
typedef UINT64				UINTVAR;
typedef INT64				INTVAR;
#endif

#define TRUE				1
#define FALSE				0

#define Track_enabled		0x000001
#define Track_in_movie		0x000002
#define Track_in_preview	0x000004

#define HEADER_LENGTH		8
#define MAXTRACKS			128

#define TRACK_VIDEO			1
#define TRACK_AUDIO			2

typedef struct _Atom
{
	INT32 start;      /* byte start in file */
	INT32 end;        /* byte endpoint in file */
	INT32 size;
	char  type[4];
}iflymp4_atom_t;

typedef struct _FullAtom
{
	iflymp4_atom_t atom;
	UINT8  version;
	UINT32 flags:24;
}iflymp4_fullatom_t;

typedef struct
{
	INT32 timeScale;
}iflymp4_tims_t;

typedef struct _FileTypeAtom
{
	iflymp4_atom_t atom;
	char major_brand[4]; /*is a brand identifier, a printable four character code, registered with ISO*/
	UINT32 minor_version; /*is an informative integer for the minor version of the major brand*/
	char compatible_brands[16]; /*is a list, to the end of the atom, of brands*/
}iflymp4_ftyp_t;

typedef struct _MovieHeaderAtom
{
	iflymp4_fullatom_t	fullatom;
	UINTVAR			creation_time;
	UINTVAR			modification_time;
	UINT32			timescale;
	UINTVAR			duration;
	INT32			rate;
	INT16			volume;
	bit16			reserved1;
	UINT32			reserved2[2];
	INT32			matrix[9];
	bit32			pre_defined[6];
	UINT32			next_track_ID;
}iflymp4_mvhd_t;

typedef struct _TrackHeaderAtom
{
	iflymp4_fullatom_t	fullatom;
	UINTVAR			creation_time;
	UINTVAR			modification_time;
	UINT32			track_ID;
	UINT32			reserved1;
	UINTVAR			duration;
	UINT32			reserved2[2];
	INT16			layer;
	INT16			alternate_group;
	INT16			volume;
	UINT16			reserved3;
	INT32			matrix[9];
	UINT32			width;
	UINT32			height;
	BOOL			is_video;
	BOOL			is_audio;
}iflymp4_tkhd_t;

typedef struct
{
	UINTVAR segment_duration;
	INTVAR	media_time;
	UINT16  media_rate_integer;
	UINT16  media_rate_fraction;
}iflymp4_elst_table_t;

typedef struct _EditListAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32	entry_count;
	iflymp4_elst_table_t *table;
}iflymp4_elst_t;

typedef struct _EditAtom
{
	iflymp4_atom_t atom;
	iflymp4_elst_t elst;
}iflymp4_edts_t;

typedef struct _MediaHeaderAtom
{
	iflymp4_fullatom_t  fullatom;
	UINTVAR			creation_time;
	UINTVAR			modification_time;
	UINT32			timescale;
	UINTVAR			duration;
	UINT16          language; /*包含1位pad与15位language*/
	UINT16			pre_defined;
}iflymp4_mdhd_t;

typedef struct _HandlerAtom
{
	iflymp4_fullatom_t	fullatom;
	char			pre_defined[4];
	char			handler_type[4];
	UINT32			reserved[3];
	char			name[26];
}iflymp4_hdlr_t;

typedef struct _VideoMediaHeaderAtom
{
	iflymp4_fullatom_t fullatom;
	UINT16  graphicsmode;
	UINT16	opcolor[3];
}iflymp4_vmhd_t;

typedef struct _SoundMediaHeaderAtom
{
	iflymp4_fullatom_t fullatom;
	UINT16  balance;
	UINT16	reserved;
}iflymp4_smhd_t;

typedef struct
{
	iflymp4_fullatom_t fullatom;
	UINT16 graphicsmode;
	UINT16 opcolor[3];
	UINT16 balance;
	UINT16 reserved;
}iflymp4_gmin_t;

typedef struct
{
	iflymp4_atom_t atom;
	iflymp4_gmin_t gmin;
}iflymp4_gmhd_t;

typedef struct _NullMediaHeaderAtom
{
	iflymp4_fullatom_t fullatom;
}iflymp4_nmhd_t;

typedef struct _DataEntryUrlAtom
{
	iflymp4_fullatom_t fullatom;
	char    location[256];
}iflymp4_url_t;

typedef struct _DataEntryUrnAtom
{
	iflymp4_fullatom_t fullatom;
	char    name[256];
	char    location[256];
}iflymp4_urn_t;

typedef struct
{
	iflymp4_fullatom_t fullatom;
	char *data_reference;
}iflymp4_dref_table_t;

typedef struct _DataReferenceAtom
{
	iflymp4_fullatom_t fullatom;
	UINT32	entry_count;
	iflymp4_dref_table_t *table;
}iflymp4_dref_t;

typedef struct _DataInformationAtom
{
	iflymp4_atom_t atom;
	iflymp4_dref_t dref;
}iflymp4_dinf_t;

typedef struct
{
	iflymp4_atom_t	atom;
	INT8   reserved1[6];
	UINT16 data_reference_index;
	
	/* video description */
	UINT16 pre_defined1;
	UINT16 reserved2;
	UINT32 pre_defined2[3];
	UINT16 width;
	UINT16 height;
	UINT32 dpi_horizontal;
	UINT32 dpi_vertical;
	UINT32 reserved3;
	UINT16 frame_count;
	char   compressor_name[32];
	UINT16 depth;
	INT16  pre_defined3;
	
	/* audio description */
	UINT32 reserved4[2];
	UINT16 channels;
	UINT16 samplesize;
	UINT16 pre_defined4;
	UINT16 reserved5;
	UINT32 samplerate;
}iflymp4_stsd_table_t;

typedef struct _SampleDescriptionAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32	entry_count;
	iflymp4_stsd_table_t *table;
}iflymp4_stsd_t;

/* sample to chunk */
typedef struct
{
	UINT32 first_chunk;
	UINT32 samples_per_chunk;
	UINT32 sample_description_index;
}iflymp4_stsc_table_t;

typedef struct _SampleToChunkAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32 entry_count;
	UINT32 entry_allocated;
	iflymp4_stsc_table_t *table;
}iflymp4_stsc_t;

typedef struct
{
	UINT32 entry_size;
}iflymp4_stsz_table_t;

typedef struct _SampleSizeAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32 sample_size;
	UINT32 entry_count;
	UINT32 entry_allocated;
	iflymp4_stsz_table_t *table;
}iflymp4_stsz_t;

typedef struct
{
	UINT32 offset;
}iflymp4_stco_table_t;

typedef struct _ChunkOffsetAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32 entry_count;
	UINT32 entry_allocated;
	iflymp4_stco_table_t *table;
}iflymp4_stco_t;

typedef struct
{
	UINT32 sample_count;
	INT32  sample_delta;
}iflymp4_stts_table_t;

typedef struct _TimeToSampleAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32		entry_count;
	UINT32		entry_allocated;
	iflymp4_stts_table_t* table;
}iflymp4_stts_t;

typedef struct
{
	UINT32 sample_count;
	INT32  sample_delta;
}iflymp4_ctts_table_t;

typedef struct _CompositionOffsetAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32		entry_count;
	UINT32		entry_allocated;
	iflymp4_ctts_table_t* table;
}iflymp4_ctts_t;

typedef struct
{
	UINT32 sample_number;
}iflymp4_stss_table_t;

typedef struct _SyncSampleAtom
{
	iflymp4_fullatom_t	fullatom;
	UINT32 entry_count;
	UINT32 entry_allocated;
	iflymp4_stss_table_t *table;
}iflymp4_stss_t;

typedef struct _SampleTableAtom
{
	iflymp4_atom_t atom;
	iflymp4_stsd_t stsd;
	iflymp4_stts_t stts;
	
	iflymp4_stsc_t stsc;
	iflymp4_stsz_t stsz;
	iflymp4_stco_t stco;

	iflymp4_stss_t stss;
}iflymp4_stbl_t;

typedef struct _MediaInformationAtom
{
	iflymp4_atom_t atom;

	BOOL is_video;
	BOOL is_audio;
	iflymp4_vmhd_t vmhd;
	iflymp4_smhd_t smhd;
	
	iflymp4_gmhd_t gmhd;
	iflymp4_nmhd_t nmhd;

	iflymp4_hdlr_t hdlr;
	
	iflymp4_dinf_t dinf;

	iflymp4_stbl_t stbl;
}iflymp4_minf_t;

typedef struct _MediaAtom
{
	iflymp4_atom_t atom;
	iflymp4_mdhd_t mdhd;
	iflymp4_hdlr_t hdlr;
	iflymp4_minf_t minf;
}iflymp4_mdia_t;

typedef struct _TrackAtom
{
	iflymp4_atom_t atom;
	iflymp4_tkhd_t tkhd;
	iflymp4_edts_t edts;
	iflymp4_mdia_t mdia;
	INT32 channels;            /* number of audio channels in the track */
	INT32 current_position;   /* current sample in output file */
	INT32 current_chunk;      /* current chunk in output file */
}iflymp4_trak_t;

typedef struct _CopyrightAtom
{
	iflymp4_atom_t atom;
	UINT16 language;
	char *notice;
	UINT32 notice_len;
}iflymp4_cprt_t;

typedef struct _UserDataAtom
{
	iflymp4_atom_t atom;
	/*iflymp4_cprt_t cprt;*/
	char data[183];
}iflymp4_udta_t;

typedef struct _MovieAtom
{
	iflymp4_atom_t atom;
	iflymp4_mvhd_t mvhd;

	iflymp4_trak_t *trak[MAXTRACKS];
	INT32 total_tracks;

	/*iflymp4_udta_t udta;*/
}iflymp4_moov_t;

typedef struct _MediaDataAtom
{
	iflymp4_atom_t atom;
}iflymp4_mdat_t;

typedef struct
{
	FILE *stream;
	INT32 total_length;
	iflymp4_mdat_t mdat;
	iflymp4_moov_t moov;
	INT32 rd;
	INT32 wr;
	
	/* for begining and ending frame writes where the user wants to write the  */
	/* file descriptor directly */
	INT32 offset;
	
	/* I/O */
	INT32 file_position;      /* Current position of file descriptor */

	INT32 last_frame;
	INT32 last_start;
	INT32 last_stts_index;
	
}iflymp4_t;

/* return TRUE if the file is a mp4 file */
BOOL iflymp4_check_sig(const char *path);

/* call this first to open the file and create all the objects */
iflymp4_t* iflymp4_open(char *filename, INT32 rd, INT32 wr, INT32 append);

/* make the iflymp4 file streamable */
INT32 iflymp4_make_streamable(char *in_path, char *out_path);

/* Set various options in the file. */
INT32 iflymp4_set_time_scale(iflymp4_t *file, INT32 timescale);
INT32 iflymp4_set_copyright(iflymp4_t *file, char *string);
INT32 iflymp4_set_name(iflymp4_t *file, char *string);
INT32 iflymp4_set_info(iflymp4_t *file, char *string);
INT32 iflymp4_get_time_scale(iflymp4_t *file);
char* iflymp4_get_copyright(iflymp4_t *file);
char* iflymp4_get_name(iflymp4_t *file);
char* iflymp4_get_info(iflymp4_t *file);

/* Read all the information about the file. */
/* Requires a MOOV atom be present in the file. */
/* If no MOOV atom exists return 1 else return 0. */
INT32 iflymp4_read_info(iflymp4_t *file);

/* set up tracks in a new file after opening and before writing */
/* returns the number of iflymp4 tracks allocated */
/* audio is stored two channels per iflymp4 track */
INT32 iflymp4_set_audio(iflymp4_t *file, INT32 channels, INT32 sample_rate, INT32 bits, INT32 sample_size, INT32 time_scale, INT32 sample_duration, char *compressor);

/* Samplerate can be set after file is created */
INT32 iflymp4_set_framerate(iflymp4_t *file, float framerate);

/* video is stored one layer per iflymp4 track */
INT32 iflymp4_set_video(iflymp4_t *file, INT32 tracks, INT32 frame_w, INT32 frame_h, float frame_rate, INT32 time_scale, char *compressor);

/* Set the depth of the track. */
INT32 iflymp4_set_depth(iflymp4_t *file, INT32 depth, INT32 track);

INT32 iflymp4_write(iflymp4_t *file);

INT32 iflymp4_close(iflymp4_t *file);
INT32 iflymp4_destroy(iflymp4_t *file);
INT32 iflymp4_delete(iflymp4_t *file);

/* get length information */
/* channel numbers start on 1 for audio and video */
INT32 iflymp4_audio_length(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_length(iflymp4_t *file, INT32 track);

/* get position information */
INT32 iflymp4_audio_position(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_position(iflymp4_t *file, INT32 track);

/* get file information */
INT32 iflymp4_video_tracks(iflymp4_t *file);
INT32 iflymp4_audio_tracks(iflymp4_t *file);

INT32 iflymp4_media_tracks(iflymp4_t *file);
INT32 iflymp4_track_type(iflymp4_t *file,INT32 track);
INT32 iflymp4_max_width(iflymp4_t *file);
INT32 iflymp4_max_height(iflymp4_t *file);

INT32 iflymp4_read_media_frame(iflymp4_t *file,INT32 track,INT32 frame,UINT8 *buf,INT32 len,INT32 *realLen,
							   INT32 *mediatype,INT32 *duration,INT32 *isKey);

INT32 iflymp4_set_trak_starttime(iflymp4_t *file,INT32 track,INT32 mediatype,INT32 start_time);
INT32 iflymp4_get_trak_starttime(iflymp4_t *file,INT32 track,INT32 mediatype);

INT32 iflymp4_audio_frame_size(iflymp4_t *file, INT32 frame, INT32 track);

BOOL iflymp4_has_audio(iflymp4_t *file);
INT32 iflymp4_audio_sample_rate(iflymp4_t *file, INT32 track);
INT32 iflymp4_audio_bits(iflymp4_t *file, INT32 track);
INT32 iflymp4_track_channels(iflymp4_t *file, INT32 track);
INT32 iflymp4_audio_time_scale(iflymp4_t *file, INT32 track);
INT32 iflymp4_audio_sample_duration(iflymp4_t *file, INT32 track);
char* iflymp4_audio_compressor(iflymp4_t *file, INT32 track);

BOOL iflymp4_has_video(iflymp4_t *file);
INT32 iflymp4_video_width(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_height(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_depth(iflymp4_t *file, INT32 track);
float iflymp4_video_frame_rate(iflymp4_t *file, INT32 track);
char* iflymp4_video_compressor(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_time_scale(iflymp4_t *file, INT32 track);
INT32 iflymp4_video_frame_time(iflymp4_t *file, INT32 track, INT32 frame,
							   INT32 *start_time, INT32 *duration);

/* number of bytes of raw data in this frame */
INT32 iflymp4_frame_size(iflymp4_t *file, INT32 frame, INT32 track);

/* get the iflymp4 track and channel that the audio channel beINT32s to */
/* channels and tracks start on 0 */
INT32 iflymp4_channel_location(iflymp4_t *file, INT32 *iflymp4_track, INT32 *iflymp4_channel, INT32 channel);

/* file positioning */
INT32 iflymp4_seek_end(iflymp4_t *file);
INT32 iflymp4_seek_start(iflymp4_t *file);

/* set position of file descriptor relative to a track */
INT32 iflymp4_set_audio_position(iflymp4_t *file, INT32 sample, INT32 track);
INT32 iflymp4_set_video_position(iflymp4_t *file, INT32 frame, INT32 track);

/* ========================== Access to raw data follows. */
/* write data for one iflymp4 track */
/* the user must handle conversion to the channels in this track */
INT32 iflymp4_write_audio(iflymp4_t *file, char *audio_buffer, INT32 samples, INT32 track);
INT32 iflymp4_write_audio_frame(iflymp4_t *file, UINT8 *audio_buffer, INT32 bytes, INT32 track);
INT32 iflymp4_write_audio_hint(iflymp4_t *file, UINT8 *hintBuffer, 
							   INT32 hintBufferSize, INT32 audioTrack, INT32 hintTrack, INT32 hintSampleDuration);

INT32 iflymp4_write_video_frame(iflymp4_t *file, UINT8 *video_buffer, INT32 bytes, INT32 track, UINT8 isKeyFrame, INT32 duration, INT32 renderingOffset);
INT32 iflymp4_write_video_hint(iflymp4_t *file, UINT8 *hint_buffer, INT32 hintBufferSize, INT32 videoTrack, INT32 hintTrack, INT32 hintSampleDuration, UINT8 isKeyFrame);

/* for writing a frame using a library that needs a file descriptor */
INT32 iflymp4_write_frame_init(iflymp4_t *file, INT32 track); /* call before fwrite */
FILE* iflymp4_get_fd(iflymp4_t *file);     /* return a file descriptor */
INT32 iflymp4_write_frame_end(iflymp4_t *file, INT32 track); /* call after fwrite */

/* For reading and writing audio to a file descriptor. */
INT32 iflymp4_write_audio_end(iflymp4_t *file, INT32 track, INT32 samples); /* call after fwrite */

/* Read an entire chunk. */
/* read the number of bytes starting at the byte_start in the specified chunk */
/* You must provide enough space to store the chunk. */
INT32 iflymp4_read_chunk(iflymp4_t *file, char *output, INT32 track, INT32 chunk, INT32 byte_start, INT32 byte_len);

/* read raw data */
INT32 iflymp4_read_audio(iflymp4_t *file, char *audio_buffer, INT32 samples, INT32 track);
INT32 iflymp4_read_audio_frame(iflymp4_t *file, UINT8 *audio_buffer, INT32 maxBytes, INT32 track);
INT32 iflymp4_read_frame(iflymp4_t *file, UINT8 *video_buffer, INT32 track);

/* for reading frame using a library that needs a file descriptor */
/* Frame caching doesn't work here. */
INT32 iflymp4_read_frame_init(iflymp4_t *file, INT32 track);
INT32 iflymp4_read_frame_end(iflymp4_t *file, INT32 track);


/* ===================== Access to built in codecs follows. */

/* If the codec for this track is supported in the library return 1. */
INT32 iflymp4_supported_video(iflymp4_t *file, INT32 track);
INT32 iflymp4_supported_audio(iflymp4_t *file, INT32 track);

/* The codec can generate the color model */
INT32 iflymp4_test_colormodel(iflymp4_t *file, 
							  INT32 colormodel, 
							  INT32 track);

/* Decode or encode the frame into a frame buffer. */
/* All the frame buffers passed to these functions are UINT8 */
/* rows with 3 bytes per pixel.  The byte order per 3 byte pixel is */
/* RGB. */
INT32 iflymp4_decode_video(iflymp4_t *file, UINT8 **row_pointers, INT32 track);
INT32 iflymp4_encode_video(iflymp4_t *file, UINT8 **row_pointers, INT32 track);
INT32 iflymp4_decode_scaled(iflymp4_t *file, 
							 INT32 in_x,                    /* Location of input frame to take picture */
							 INT32 in_y,
							 INT32 in_w,
							 INT32 in_h,
							 INT32 out_w,                   /* Dimensions of output frame */
							 INT32 out_h,
							 INT32 color_model,             /* One of the color models defined above */
							 UINT8 **row_pointers, 
							 INT32 track);

/* Dump the file structures for the currently opened file. */
INT32 iflymp4_dump(iflymp4_t *file);

/* Specify the number of cpus to utilize. */
INT32 iflymp4_set_cpus(iflymp4_t *file, INT32 cpus);

/* Specify whether to read contiguously or not. */
/* preload is the number of bytes to read ahead. */
INT32 iflymp4_set_preload(iflymp4_t *file, INT32 preload);

/* Test the 32 bit overflow */
INT32 iflymp4_test_position(iflymp4_t *file);

INT32 iflymp4_get_timescale(float frame_rate);



INT32 iflymp4_position(iflymp4_t *file);
INT32 iflymp4_set_position(iflymp4_t *file, INT32 position);
INT32 iflymp4_read_data(iflymp4_t *file, char *data, INT32 size);
INT32 iflymp4_write_data(iflymp4_t *file, char *data, INT32 size);
UINT64 iflymp4_read_int64(iflymp4_t *file);
INT32 iflymp4_read_int32(iflymp4_t *file);
INT32 iflymp4_read_int24(iflymp4_t *file);
INT16 iflymp4_read_int16(iflymp4_t *file);
char iflymp4_read_char(iflymp4_t *file);
INT32 iflymp4_read_char32(iflymp4_t *file, char *string);
INT32 iflymp4_write_int64(iflymp4_t *file, UINT64 value);
INT32 iflymp4_write_int32(iflymp4_t *file, INT32 value);
INT32 iflymp4_write_int24(iflymp4_t *file, INT32 number);
INT32 iflymp4_write_int16(iflymp4_t *file, INT16 number);
INT32 iflymp4_write_char(iflymp4_t *file, char x);
INT32 iflymp4_write_char32(iflymp4_t *file, char *string);
BOOL iflymp4_match_32(char *input, char *output);
INT32 iflymp4_print_chars(char *desc, char *input, INT32 len);
INT32 iflymp4_copy_char32(char *output, char *input);
UINT32 iflymp4_current_time();

INT32 iflymp4_atom_reset(iflymp4_atom_t *atom);
INT32 iflymp4_atom_read_header(iflymp4_t *file, iflymp4_atom_t *atom);
INT32 iflymp4_atom_write_header(iflymp4_t *file, iflymp4_atom_t *atom, char *text);
INT32 iflymp4_atom_write_footer(iflymp4_t *file, iflymp4_atom_t *atom);
BOOL iflymp4_atom_is(iflymp4_atom_t *atom, char *type);
INT32 iflymp4_atom_read_size(char *data);
UINT64 iflymp4_atom_read_size64(char *data);
INT32 iflymp4_atom_read_type(char *data, char *type);
INT32 iflymp4_atom_skip(iflymp4_t *file, iflymp4_atom_t *atom);

INT32 iflymp4_ftyp_init(iflymp4_ftyp_t *ftyp);
INT32 iflymp4_ftyp_delete(iflymp4_ftyp_t *ftyp);
INT32 iflymp4_read_ftyp(iflymp4_t *file, iflymp4_ftyp_t *ftyp, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_ftyp(iflymp4_t *file, iflymp4_ftyp_t *ftyp);

INT32 iflymp4_mdat_init(iflymp4_mdat_t *mdat);
INT32 iflymp4_mdat_delete(iflymp4_mdat_t *mdat);
INT32 iflymp4_mdat_dump(iflymp4_mdat_t *mdat);
INT32 iflymp4_read_mdat(iflymp4_t *file, iflymp4_mdat_t *mdat, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_mdat(iflymp4_t *file, iflymp4_mdat_t *mdat);

INT32 iflymp4_moov_init(iflymp4_moov_t *moov);
INT32 iflymp4_moov_delete(iflymp4_moov_t *moov);
INT32 iflymp4_moov_dump(iflymp4_moov_t *moov);
INT32 iflymp4_read_moov(iflymp4_t *file, iflymp4_moov_t *moov, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_moov(iflymp4_t *file, iflymp4_moov_t *moov);
INT32 iflymp4_shift_offsets(iflymp4_moov_t *moov, INT32 offset);

INT32 iflymp4_matrix_init(INT32 *matrix);
INT32 iflymp4_matrix_delete(INT32 *matrix);
INT32 iflymp4_read_matrix(iflymp4_t *file, INT32 *matrix);
INT32 iflymp4_matrix_dump(INT32 *matrix);
INT32 iflymp4_write_matrix(iflymp4_t *file, INT32 *matrix);

INT32 iflymp4_mvhd_init(iflymp4_mvhd_t *mvhd);
INT32 iflymp4_mvhd_delete(iflymp4_mvhd_t *mvhd);
INT32 iflymp4_mvhd_dump(iflymp4_mvhd_t *mvhd);
INT32 iflymp4_read_mvhd(iflymp4_t *file, iflymp4_mvhd_t *mvhd);
INT32 iflymp4_write_mvhd(iflymp4_t *file, iflymp4_mvhd_t *mvhd);

iflymp4_trak_t* iflymp4_add_trak(iflymp4_moov_t *moov);
INT32 iflymp4_read_trak(iflymp4_t *file, iflymp4_trak_t *trak, iflymp4_atom_t *trak_atom);

INT32 iflymp4_mdhd_init(iflymp4_mdhd_t *mdhd);
INT32 iflymp4_mdhd_init_video(iflymp4_t *file, iflymp4_mdhd_t *mdhd, INT32 timescale);
INT32 iflymp4_mdhd_init_audio(iflymp4_t *file, iflymp4_mdhd_t *mdhd, INT32 timescale);
INT32 iflymp4_mdhd_delete(iflymp4_mdhd_t *mdhd);
INT32 iflymp4_read_mdhd(iflymp4_t *file, iflymp4_mdhd_t *mdhd);
INT32 iflymp4_mdhd_dump(iflymp4_mdhd_t *mdhd);
INT32 iflymp4_write_mdhd(iflymp4_t *file, iflymp4_mdhd_t *mdhd);

INT32 iflymp4_elst_table_init(iflymp4_elst_table_t *table);
INT32 iflymp4_elst_table_delete(iflymp4_elst_table_t *table);
INT32 iflymp4_read_elst_table(iflymp4_t *file, iflymp4_elst_table_t *table);
INT32 iflymp4_write_elst_table(iflymp4_t *file, iflymp4_elst_table_t *table, INT32 duration);
INT32 iflymp4_elst_table_dump(iflymp4_elst_table_t *table);
INT32 iflymp4_elst_init(iflymp4_elst_t *elst);
INT32 iflymp4_elst_init_all(iflymp4_elst_t *elst);
INT32 iflymp4_elst_delete(iflymp4_elst_t *elst);
INT32 iflymp4_elst_dump(iflymp4_elst_t *elst);
INT32 iflymp4_read_elst(iflymp4_t *file, iflymp4_elst_t *elst);
INT32 iflymp4_write_elst(iflymp4_t *file, iflymp4_elst_t *elst, INT32 duration);

INT32 iflymp4_edts_init(iflymp4_edts_t *edts);
INT32 iflymp4_edts_delete(iflymp4_edts_t *edts);
INT32 iflymp4_edts_init_table(iflymp4_edts_t *edts);
INT32 iflymp4_read_edts(iflymp4_t *file, iflymp4_edts_t *edts, iflymp4_atom_t *edts_atom);
INT32 iflymp4_edts_dump(iflymp4_edts_t *edts);
INT32 iflymp4_write_edts(iflymp4_t *file, iflymp4_edts_t *edts, INT32 duration);

INT32 iflymp4_gmin_init(iflymp4_gmin_t *gmin);
INT32 iflymp4_gmin_delete(iflymp4_gmin_t *gmin);
INT32 iflymp4_gmin_dump(iflymp4_gmin_t *gmin);
INT32 iflymp4_read_gmin(iflymp4_t *file, iflymp4_gmin_t *gmin);
INT32 iflymp4_write_gmin(iflymp4_t *file, iflymp4_gmin_t *gmin);

INT32 iflymp4_gmhd_init(iflymp4_gmhd_t *gmhd);
INT32 iflymp4_vmhd_init_video(iflymp4_t *file, iflymp4_vmhd_t *vmhd, INT32 frame_w, INT32 frame_h, float frame_rate);
INT32 iflymp4_gmhd_delete(iflymp4_gmhd_t *gmhd);
INT32 iflymp4_gmhd_dump(iflymp4_gmhd_t *gmhd);
INT32 iflymp4_read_gmhd(iflymp4_t *file, iflymp4_gmhd_t *gmhd, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_gmhd(iflymp4_t *file, iflymp4_gmhd_t *gmhd);

INT32 iflymp4_vmhd_init(iflymp4_vmhd_t *vmhd);
INT32 iflymp4_vmhd_delete(iflymp4_vmhd_t *vmhd);
INT32 iflymp4_vmhd_dump(iflymp4_vmhd_t *vmhd);
INT32 iflymp4_read_vmhd(iflymp4_t *file, iflymp4_vmhd_t *vmhd);
INT32 iflymp4_write_vmhd(iflymp4_t *file, iflymp4_vmhd_t *vmhd);

INT32 iflymp4_smhd_init(iflymp4_smhd_t *smhd);
INT32 iflymp4_smhd_delete(iflymp4_smhd_t *smhd);
INT32 iflymp4_smhd_dump(iflymp4_smhd_t *smhd);
INT32 iflymp4_read_smhd(iflymp4_t *file, iflymp4_smhd_t *smhd);
INT32 iflymp4_write_smhd(iflymp4_t *file, iflymp4_smhd_t *smhd);

INT32 iflymp4_nmhd_init(iflymp4_nmhd_t *nmhd);
INT32 iflymp4_nmhd_delete(iflymp4_nmhd_t *nmhd);
INT32 iflymp4_nmhd_dump(iflymp4_nmhd_t *nmhd);
INT32 iflymp4_read_nmhd(iflymp4_t *file, iflymp4_nmhd_t *nmhd);
INT32 iflymp4_write_nmhd(iflymp4_t *file, iflymp4_nmhd_t *nmhd);

INT32 iflymp4_dref_init(iflymp4_dref_t *dref);
INT32 iflymp4_dref_init_all(iflymp4_dref_t *dref);
INT32 iflymp4_dref_delete(iflymp4_dref_t *dref);
INT32 iflymp4_dref_dump(iflymp4_dref_t *dref);
INT32 iflymp4_read_dref(iflymp4_t *file, iflymp4_dref_t *dref);
INT32 iflymp4_write_dref(iflymp4_t *file, iflymp4_dref_t *dref);

INT32 iflymp4_dinf_init(iflymp4_dinf_t *dinf);
INT32 iflymp4_dinf_delete(iflymp4_dinf_t *dinf);
INT32 iflymp4_dinf_init_all(iflymp4_dinf_t *dinf);
INT32 iflymp4_dinf_dump(iflymp4_dinf_t *dinf);
INT32 iflymp4_read_dinf(iflymp4_t *file, iflymp4_dinf_t *dinf, iflymp4_atom_t *dinf_atom);
INT32 iflymp4_write_dinf(iflymp4_t *file, iflymp4_dinf_t *dinf);

INT32 iflymp4_hdlr_init(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_hdlr_init_video(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_hdlr_init_audio(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_hdlr_init_data(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_hdlr_delete(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_hdlr_dump(iflymp4_hdlr_t *hdlr);
INT32 iflymp4_read_hdlr(iflymp4_t *file, iflymp4_hdlr_t *hdlr);
INT32 iflymp4_write_hdlr(iflymp4_t *file, iflymp4_hdlr_t *hdlr);

INT32 iflymp4_stbl_init(iflymp4_stbl_t *stbl);
INT32 iflymp4_stbl_init_video(iflymp4_t *file, iflymp4_stbl_t *stbl, INT32 frame_w, INT32 frame_h, 
							INT32 time_scale, float frame_rate, char *compressor);
INT32 iflymp4_stbl_init_audio(iflymp4_t *file, iflymp4_stbl_t *stbl, INT32 channels, INT32 sample_rate, 
							INT32 bits, INT32 sample_size,INT32 time_scale,INT32 sample_duration,char *compressor);
INT32 iflymp4_stbl_delete(iflymp4_stbl_t *stbl);
INT32 iflymp4_stbl_dump(void *minf_ptr, iflymp4_stbl_t *stbl);
INT32 iflymp4_read_stbl(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stbl_t *stbl, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_stbl(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stbl_t *stbl);



INT32 iflymp4_stsd_init_audio(iflymp4_t *file, iflymp4_stsd_t *stsd, INT32 channels, 
							INT32 sample_rate, INT32 bits, char *compressor);
INT32 iflymp4_stsd_init_video(iflymp4_t *file, iflymp4_stsd_t *stsd, INT32 frame_w, 
							INT32 frame_h, float frame_rate, char *compression);
INT32 iflymp4_stsd_table_init(iflymp4_stsd_table_t *table);
INT32 iflymp4_stsd_init_table(iflymp4_stsd_t *stsd);
INT32 iflymp4_stsd_table_delete(iflymp4_stsd_table_t *table);
INT32 iflymp4_stsd_delete(iflymp4_stsd_t *stsd);
INT32 iflymp4_stsd_audio_dump(iflymp4_stsd_table_t *table);
INT32 iflymp4_stsd_video_dump(iflymp4_stsd_table_t *table);
INT32 iflymp4_stsd_table_dump(void *minf_ptr, iflymp4_stsd_table_t *table);
INT32 iflymp4_stsd_dump(void *minf_ptr, iflymp4_stsd_t *stsd);
INT32 iflymp4_read_stsd_audio(iflymp4_t *file, iflymp4_stsd_table_t *table, iflymp4_atom_t *parent_atom);
INT32 iflymp4_read_stsd_video(iflymp4_t *file, iflymp4_stsd_table_t *table, iflymp4_atom_t *parent_atom);
INT32 iflymp4_read_stsd_table(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stsd_table_t *table);
INT32 iflymp4_write_stsd_audio(iflymp4_t *file, iflymp4_stsd_table_t *table);
INT32 iflymp4_write_stsd_video(iflymp4_t *file, iflymp4_stsd_table_t *table);
INT32 iflymp4_write_stsd_table(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stsd_table_t *table);
INT32 iflymp4_read_stsd(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stsd_t *stsd);
INT32 iflymp4_write_stsd(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_stsd_t *stsd);
INT32 iflymp4_tims_init(iflymp4_tims_t *tims);
INT32 iflymp4_read_tims(iflymp4_t *file, iflymp4_tims_t *tims);
INT32 iflymp4_write_tims(iflymp4_t *file, iflymp4_tims_t *tims);
INT32 iflymp4_stsd_init(iflymp4_stsd_t *stsd);



INT32 iflymp4_stts_init(iflymp4_stts_t *stts);
INT32 iflymp4_stts_init_table(iflymp4_stts_t *stts);
INT32 iflymp4_stts_init_video(iflymp4_t *file, iflymp4_stts_t *stts, INT32 time_scale, float frame_rate);
INT32 iflymp4_stts_init_audio(iflymp4_t *file, iflymp4_stts_t *stts, INT32 time_scale, INT32 sample_delta);
INT32 iflymp4_stts_init_hint(iflymp4_t *file, iflymp4_stts_t *stts, INT32 sample_delta);
INT32 iflymp4_stts_delete(iflymp4_stts_t *stts);
INT32 iflymp4_stts_dump(iflymp4_stts_t *stts);
INT32 iflymp4_read_stts(iflymp4_t *file, iflymp4_stts_t *stts);
INT32 iflymp4_write_stts(iflymp4_t *file, iflymp4_stts_t *stts);
INT32 iflymp4_update_stts(iflymp4_stts_t *stts, INT32 sample_delta);



INT32 iflymp4_stss_init(iflymp4_stss_t *stss);
INT32 iflymp4_stss_init_common(iflymp4_t *file, iflymp4_stss_t *stss);
INT32 iflymp4_stss_delete(iflymp4_stss_t *stss);
INT32 iflymp4_stss_dump(iflymp4_stss_t *stss);
INT32 iflymp4_read_stss(iflymp4_t *file, iflymp4_stss_t *stss);
INT32 iflymp4_write_stss(iflymp4_t *file, iflymp4_stss_t *stss);
INT32 iflymp4_update_stss(iflymp4_stss_t *stss, INT32 sample);



INT32 iflymp4_stco_init(iflymp4_stco_t *stco);
INT32 iflymp4_stco_delete(iflymp4_stco_t *stco);
INT32 iflymp4_stco_init_common(iflymp4_t *file, iflymp4_stco_t *stco);
INT32 iflymp4_stco_dump(iflymp4_stco_t *stco);
INT32 iflymp4_read_stco(iflymp4_t *file, iflymp4_stco_t *stco);
INT32 iflymp4_write_stco(iflymp4_t *file, iflymp4_stco_t *stco);
INT32 iflymp4_update_stco(iflymp4_stco_t *stco, INT32 chunk, INT32 offset);



INT32 iflymp4_stsc_init(iflymp4_stsc_t *stsc);
INT32 iflymp4_stsc_init_table(iflymp4_t *file, iflymp4_stsc_t *stsc);
INT32 iflymp4_stsc_init_video(iflymp4_t *file, iflymp4_stsc_t *stsc);
INT32 iflymp4_stsc_init_audio(iflymp4_t *file, iflymp4_stsc_t *stsc);
INT32 iflymp4_stsc_delete(iflymp4_stsc_t *stsc);
INT32 iflymp4_stsc_dump(iflymp4_stsc_t *stsc);
INT32 iflymp4_read_stsc(iflymp4_t *file, iflymp4_stsc_t *stsc);
INT32 iflymp4_write_stsc(iflymp4_t *file, iflymp4_stsc_t *stsc);
INT32 iflymp4_update_stsc(iflymp4_stsc_t *stsc, INT32 chunk, INT32 samples);



INT32 iflymp4_stsz_init(iflymp4_stsz_t *stsz);
INT32 iflymp4_stsz_init_video(iflymp4_t *file, iflymp4_stsz_t *stsz);
INT32 iflymp4_stsz_init_audio(iflymp4_t *file, iflymp4_stsz_t *stsz, INT32 sample_size);
INT32 iflymp4_stsz_delete(iflymp4_stsz_t *stsz);
INT32 iflymp4_stsz_dump(iflymp4_stsz_t *stsz);
INT32 iflymp4_read_stsz(iflymp4_t *file, iflymp4_stsz_t *stsz);
INT32 iflymp4_write_stsz(iflymp4_t *file, iflymp4_stsz_t *stsz);
INT32 iflymp4_update_stsz(iflymp4_stsz_t *stsz, INT32 sample, INT32 sample_size);



INT32 iflymp4_minf_init(iflymp4_minf_t *minf);
INT32 iflymp4_minf_init_video(iflymp4_t *file, iflymp4_minf_t *minf, INT32 frame_w, INT32 frame_h, 
								INT32 time_scale, float frame_rate, char *compressor);
INT32 iflymp4_minf_init_audio(iflymp4_t *file, iflymp4_minf_t *minf, INT32 channels, INT32 sample_rate, 
							INT32 bits, INT32 sample_size, INT32 time_scale, INT32 sample_duration, char *compressor);
INT32 iflymp4_minf_delete(iflymp4_minf_t *minf);
INT32 iflymp4_minf_dump(iflymp4_minf_t *minf);
INT32 iflymp4_read_minf(iflymp4_t *file, iflymp4_minf_t *minf, iflymp4_atom_t *parent_atom);
INT32 iflymp4_write_minf(iflymp4_t *file, iflymp4_minf_t *minf);


INT32 iflymp4_mdia_init(iflymp4_mdia_t *mdia);
INT32 iflymp4_mdia_init_video(iflymp4_t *file, iflymp4_mdia_t *mdia, INT32 frame_w, INT32 frame_h, 
								float frame_rate, INT32 time_scale, char *compressor);
INT32 iflymp4_mdia_init_audio(iflymp4_t *file, iflymp4_mdia_t *mdia, INT32 channels, INT32 sample_rate, 
							INT32 bits, INT32 sample_size, INT32 time_scale, INT32 sample_duration, char *compressor);
INT32 iflymp4_mdia_delete(iflymp4_mdia_t *mdia);
INT32 iflymp4_mdia_dump(iflymp4_mdia_t *mdia);
INT32 iflymp4_read_mdia(iflymp4_t *file, iflymp4_mdia_t *mdia, iflymp4_atom_t *trak_atom);
INT32 iflymp4_write_mdia(iflymp4_t *file, iflymp4_mdia_t *mdia);


INT32 iflymp4_tkhd_init(iflymp4_tkhd_t *tkhd);
INT32 iflymp4_tkhd_init_audio(iflymp4_t *file, iflymp4_tkhd_t *tkhd);
INT32 iflymp4_tkhd_init_video(iflymp4_t *file, iflymp4_tkhd_t *tkhd, INT32 frame_w, INT32 frame_h);
INT32 iflymp4_tkhd_delete(iflymp4_tkhd_t *tkhd);
INT32 iflymp4_tkhd_dump(iflymp4_tkhd_t *tkhd);
INT32 iflymp4_read_tkhd(iflymp4_t *file, iflymp4_tkhd_t *tkhd);
INT32 iflymp4_write_tkhd(iflymp4_t *file, iflymp4_tkhd_t *tkhd);

INT32 iflymp4_trak_init(iflymp4_trak_t *trak);
INT32 iflymp4_trak_init_audio(iflymp4_t *file, iflymp4_trak_t *trak, INT32 channels, INT32 sample_rate, 
							INT32 bits, INT32 sample_size, INT32 time_scale, INT32 sample_duration, char *compressor);
INT32 iflymp4_trak_init_video(iflymp4_t *file, iflymp4_trak_t *trak, INT32 frame_w, INT32 frame_h, 
							float frame_rate, INT32 time_scale, char *compressor);
iflymp4_trak_t* iflymp4_add_trak(iflymp4_moov_t *moov);
INT32 iflymp4_trak_duration(iflymp4_trak_t *trak, INT32 *duration, INT32 *timescale);
INT32 iflymp4_delete_trak(iflymp4_moov_t *moov, iflymp4_trak_t *trak);
INT32 iflymp4_trak_dump(iflymp4_trak_t *trak);
INT32 iflymp4_samples_to_bytes(iflymp4_trak_t *track, INT32 samples);
INT32 iflymp4_read_trak(iflymp4_t *file, iflymp4_trak_t *trak, iflymp4_atom_t *trak_atom);
INT32 iflymp4_write_trak(iflymp4_t *file, iflymp4_trak_t *trak, INT32 moov_time_scale);
INT32 iflymp4_trak_shift_offsets(iflymp4_trak_t *trak, INT32 offset);
INT32 iflymp4_trak_fix_counts(iflymp4_t *file, iflymp4_trak_t *trak);
INT32 iflymp4_track_samples(iflymp4_t *file, iflymp4_trak_t *trak);
INT32 iflymp4_track_end(iflymp4_trak_t *trak);

INT32 iflymp4_sample_of_chunk(iflymp4_trak_t *trak, INT32 chunk);
INT32 iflymp4_chunk_of_sample(INT32 *chunk_sample, INT32 *chunk, iflymp4_trak_t *trak, INT32 sample);
INT32 iflymp4_chunk_to_offset(iflymp4_trak_t *trak, INT32 chunk);
INT32 iflymp4_offset_to_chunk(INT32 *chunk_offset, iflymp4_trak_t *trak, INT32 offset);
INT32 iflymp4_offset_to_sample(iflymp4_trak_t *trak, INT32 offset);
INT32 iflymp4_sample_to_offset(iflymp4_trak_t *trak, INT32 sample);
INT32 iflymp4_update_tables(iflymp4_t *file, 
						  iflymp4_trak_t *trak, 
						  INT32 offset, 
						  INT32 chunk, 
						  INT32 sample, 
						  INT32 samples, 
						  INT32 sample_size,
						  INT32 sample_duration,
						  UINT8 isSyncSample,
						  INT32 renderingOffset);
INT32 iflymp4_chunk_samples(iflymp4_trak_t *trak, INT32 chunk);

#ifdef __cplusplus
}
#endif

#endif
