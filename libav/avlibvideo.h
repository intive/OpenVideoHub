#ifndef AV_LIB_VIDEO_H
#define AV_LIB_VIDEO_H

#ifndef __GNUC__
#include <e32def.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_CODEC_FLV 0
#define VIDEO_CODEC_FLASHSV 1
#define VIDEO_CODEC_VP6F 2
#define VIDEO_CODEC_VP6A 3
#define VIDEO_CODEC_UNKNOWN -1

IMPORT_C void AV_VideoRegister( void );
IMPORT_C void AV_VideoCleanup( void );

//video decoding
typedef struct image
{
	unsigned char	*output[3];
	unsigned int	width;
	unsigned int	height;
	unsigned int	stride;
	unsigned int	uv_stride;
}IMAGE;

#define VIDEO_OPTION_GRAY 1
#define VIDEO_OPTION_LOWRES 2

IMPORT_C void VIDEO_Close(void *handle);
IMPORT_C void *VIDEO_Init(int width, int height, void *extraData, int extraDataSize, int codec_id );
IMPORT_C void *VIDEO_Decode(void *handle, void *data, int size, int skip);
IMPORT_C void VIDEO_SetOptions(void *handle, int option, int value);


#ifdef __cplusplus
}
#endif

#endif
