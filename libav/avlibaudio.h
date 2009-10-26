#ifndef AV_LIB_AUDIOH
#define AV_LIB_AUDIOH

#ifndef __GNUC__
#include <e32def.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_CODEC_MP3 0
#define AUDIO_CODEC_UNKNOWN -1

IMPORT_C void AV_AudioRegister( void );
IMPORT_C void AV_AudioCleanup( void );

//audio decoding
typedef struct sample
{
	int	outsize;
	int	frequency;
	int	channels;
	int	bits;
	int	flags;
}SAMPLE; 

IMPORT_C void AUDIO_Close(void *handle);
IMPORT_C void *AUDIO_Init(int freq, int channels, int bits, int codec_id);
IMPORT_C void *AUDIO_Decode(void *handle, void *data, void *out, int size);

#ifdef __cplusplus
}
#endif

#endif
