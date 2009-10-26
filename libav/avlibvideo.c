#include "avcodec.h"

#include "avlibvideo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern AVCodec vp6f_decoder;
extern AVCodec vp6a_decoder;
extern AVCodec flv_decoder;
extern AVCodec flashsv_decoder;

EXPORT_C void AV_VideoRegister( void )
	{
	register_avcodec( &vp6f_decoder );
	register_avcodec( &vp6a_decoder );
	register_avcodec( &flv_decoder );
//	register_avcodec( &flashsv_decoder );
	}

EXPORT_C void AV_VideoCleanup( void )
	{
	}

typedef struct lavc_video_handle
{
	int width, height;
	AVCodec *codec;
	AVCodecContext *c;
	AVFrame *picture;
	IMAGE v;
	unsigned char *buf_input;
	int buf_size;
}LAVC_V_HANDLE;


EXPORT_C void VIDEO_Close(void *handle)
{
	LAVC_V_HANDLE * h = handle;

	if( h )
	{
		if(h->c)
		{
			if( h->c->codec )
				avcodec_close(h->c);

			if(h->c->extradata)
				av_free(h->c->extradata);

			av_free(h->c);
		}

		if(h->picture)
			av_free(h->picture);
		
		av_free( h );
	}
}

EXPORT_C void * VIDEO_Init(int width, int height, void *extraData, int extraDataSize, int codec_id )
{
	LAVC_V_HANDLE *h;

	h = av_malloc(sizeof(LAVC_V_HANDLE));
	if(!h) return NULL;
	memset(h, 0, sizeof(LAVC_V_HANDLE));

	h->c = avcodec_alloc_context();
	h->c->idct_algo = FF_IDCT_AUTO;

	h->width = width;
	h->height = height;
	h->picture = NULL;

	avcodec_init();

	h->codec = NULL;
	h->c->error_resilience = 0;
	h->c->error_concealment = 0;
	h->c->workaround_bugs = 0; //FF_BUG_AUTODETECT;

	h->c->flags |= CODEC_FLAG_EMU_EDGE;
	h->c->flags2 |= CODEC_FLAG2_FAST;
	h->c->skip_loop_filter = AVDISCARD_ALL;

	if(extraDataSize)
	{
		h->c->extradata_size = extraDataSize;
		h->c->extradata = av_malloc(extraDataSize);
		if(!h->c->extradata)
		{
			VIDEO_Close(h);
			return NULL;
		}
		memcpy(h->c->extradata, extraData, extraDataSize);
	}

	switch( codec_id )
	{
	case VIDEO_CODEC_FLV:
		codec_id = CODEC_ID_FLV1;
	break;
	
	case VIDEO_CODEC_VP6F:
		codec_id = CODEC_ID_VP6F;
	break;

	case VIDEO_CODEC_VP6A:
		codec_id = CODEC_ID_VP6A;
	break;

	default:
		codec_id = CODEC_ID_NONE;
	break;
	}

	h->codec = avcodec_find_decoder( (enum CodecID)codec_id );
	if (!h->codec)
	{
		VIDEO_Close(h);
		return NULL;
	}

	h->c->width = width;
	h->c->height = height;
	h->c->draw_horiz_band = 0;

	if((h->picture = avcodec_alloc_frame()) == NULL)
	{
		VIDEO_Close(h);
		return NULL;
	}

	if (avcodec_open(h->c, h->codec) < 0)
	{
		VIDEO_Close(h);
		return NULL;
	}

	return (void *)h;
}

EXPORT_C void VIDEO_SetOptions(void *handle, int option, int value)
{
	LAVC_V_HANDLE * h = handle;

	if( option == VIDEO_OPTION_GRAY )
	{
		if( value )
			h->c->flags |= CODEC_FLAG_GRAY;
		else
			h->c->flags &= (~CODEC_FLAG_GRAY);
	}
	else if( option == VIDEO_OPTION_LOWRES )
	{
		if( value == 1 )
			h->c->lowres = value;
		else
			h->c->lowres = 0;
	}
	else
	{
	//do nothing
	}
}

EXPORT_C void *VIDEO_Decode(void *handle, void *data, int size, int skip)
{
	LAVC_V_HANDLE * h = handle;

	int got_picture = 0, len = 0;

	if(data)
	{
		h->buf_input = data;
		h->buf_size = size;
	}

	len = avcodec_decode_video(h->c, h->picture, &got_picture, h->buf_input, h->buf_size);

	if (len < 0)
	{
		return NULL;
	}
	h->buf_input += len;
	h->buf_size -= len;

	if(got_picture)
	{
		h->v.output[0] = h->picture->data[0];
		h->v.output[1] = h->picture->data[1];
		h->v.output[2] = h->picture->data[2];
		h->v.width = h->width;
		h->v.height = h->height;
		h->v.stride = h->picture->linesize[0];
		h->v.uv_stride = h->picture->linesize[1];
	}
	return &h->v;
}

#ifdef __cplusplus
}
#endif
