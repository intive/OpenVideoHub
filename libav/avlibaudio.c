#include "config.h"
#include "avcodec.h"

#include "avlibaudio.h"

#ifdef __cplusplus
extern "C" {
#endif

extern AVCodec pcm_s8_decoder;
extern AVCodec pcm_s16be_decoder;
extern AVCodec pcm_s16le_decoder;
extern AVCodec pcm_s16le_decoder;
extern AVCodec mp3_decoder;
extern AVCodec adpcm_swf_decoder;

EXPORT_C void AV_AudioRegister( void )
	{
	avcodec_init();
//	register_avcodec( &pcm_s8_decoder );
//	register_avcodec( &pcm_s16be_decoder );
//	register_avcodec( &pcm_s16le_decoder );
//	register_avcodec( &pcm_s16le_decoder );
	register_avcodec( &mp3_decoder );
//	register_avcodec( &adpcm_swf_decoder );
	}

EXPORT_C void AV_AudioCleanup( void )
	{
	}

typedef struct lavc_audio_handle
{
	SAMPLE s;
	AVCodecContext *context;
	AVCodec *codec;
	int freq, channels;
	int position, remaining;
	unsigned char buf[32768];
}LAVC_A_HANDLE;

EXPORT_C void AUDIO_Close(void *handle)
{
	LAVC_A_HANDLE *h = handle;
	if(h)
	{
		if(h->context)
		{
			avcodec_close(h->context);
			av_free(h->context);
		}
		av_free(h);
	}
}

EXPORT_C void *AUDIO_Init(int freq, int channels, int bits, int codec_id)
{
	LAVC_A_HANDLE *h;

	h = av_malloc(sizeof(LAVC_A_HANDLE));
	if(!h) return NULL;
	memset(h, 0, sizeof(LAVC_A_HANDLE));

	h->position = 0;
	h->remaining = 0;
	h->freq = freq;
	h->channels = channels;

	avcodec_init();

	h->context = avcodec_alloc_context();
	h->context->channels = channels;
	h->context->sample_rate = freq;
	h->context->bit_rate = 0;//ext->avgBytesPerSec * 8;
	h->context->codec_tag = 0;//ext->ID;
	h->context->block_align = 0;//ext->blockAlign;
	h->context->bits_per_sample = bits;

	switch( codec_id )
	{
	case AUDIO_CODEC_MP3:
		codec_id = CODEC_ID_MP3;
	break;
	
	default:
		codec_id = CODEC_ID_NONE;
	break;
	}

	h->codec = avcodec_find_decoder( (enum CodecID)codec_id );

	if(h->codec == NULL)
	{
		AUDIO_Close(h);
		return NULL;
	}
	h->context->codec_id = h->codec->id;

	if (avcodec_open(h->context, h->codec) < 0)
	{
		AUDIO_Close(h);
		return NULL;
	}

	return h;
}

EXPORT_C void * AUDIO_Decode(void *handle, void *data, void *out, int size)
{
	LAVC_A_HANDLE *h = handle;
	int i;
	int outsize = 0, dec = 0;
	unsigned char *d = data, *outptr = out;
	int minsize = size;

	if(size > 0)
	{
		if(h->remaining)
		{
			memcpy(h->buf, h->buf + h->position, h->remaining);
		}
		memcpy(h->buf + h->remaining, data, size);
		h->remaining += size;
		h->position = 0;
		d = h->buf;
		size = h->remaining;

		while(size && size >= minsize)
		{
			outsize = 200000;
			i = avcodec_decode_audio2(h->context, (short *)outptr, &outsize, d, size);
			if(i < 0)
			{
				h->s.outsize = dec;
				h->s.channels = h->channels;
				h->s.frequency = h->freq;
				h->s.bits = 16;
				return &h->s;
			}
			minsize = i;
			d += i;
			size -= i;
			h->remaining -= i;
			h->position += i;
			dec += outsize;
			outptr += outsize;
		}
	}

	h->s.outsize = dec;
	h->s.channels = h->channels;
	h->s.frequency = h->freq;
	h->s.bits = 16;

	return &h->s;
}

#ifdef __cplusplus
}
#endif

#define MAX_NEG_CROP 1024
uint8_t ff_cropTbl[256 + 2 * MAX_NEG_CROP] = {0, };

void dsputil_static_init(void)
{
	int i;
    for(i=0;i<256;i++) ff_cropTbl[i + MAX_NEG_CROP] = i;
    for(i=0;i<MAX_NEG_CROP;i++) {
        ff_cropTbl[i] = 0;
        ff_cropTbl[i + MAX_NEG_CROP + 256] = 255;
    }
}
