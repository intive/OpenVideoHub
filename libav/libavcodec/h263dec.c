/*
 * H.263 decoder
 * Copyright (c) 2001 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file h263dec.c
 * H.263 decoder.
 */

#include "avcodec.h"
#include "dsputil.h"
#include "mpegvideo.h"
#include "h263_parser.h"
#include "mpeg4video_parser.h"
#include "msmpeg4.h"

//#define DEBUG
//#define PRINT_FRAME_TIME

av_cold int ff_h263_decode_init(AVCodecContext *avctx)
{
    MpegEncContext *s = avctx->priv_data;

    s->avctx = avctx;
    s->out_format = FMT_H263;

    s->width  = avctx->coded_width;
    s->height = avctx->coded_height;
    s->workaround_bugs= avctx->workaround_bugs;

    // set defaults
    MPV_decode_defaults(s);
    s->quant_precision=5;
    s->decode_mb= ff_h263_decode_mb;
    s->low_delay= 1;
    avctx->pix_fmt= PIX_FMT_YUV420P;
    s->unrestricted_mv= 1;

    /* select sub codec */
    switch(avctx->codec->id) {
    case CODEC_ID_H263:
        s->unrestricted_mv= 0;
        break;
    case CODEC_ID_MPEG4:
//        s->decode_mb= ff_mpeg4_decode_mb;
        s->time_increment_bits = 4; /* default value for broken headers */
        s->h263_pred = 1;
        s->low_delay = 0; //default, might be overriden in the vol header during header parsing
        break;
    case CODEC_ID_MSMPEG4V1:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=1;
        break;
    case CODEC_ID_MSMPEG4V2:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=2;
        break;
    case CODEC_ID_MSMPEG4V3:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=3;
        break;
    case CODEC_ID_WMV1:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=4;
        break;
    case CODEC_ID_WMV2:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=5;
        break;
    case CODEC_ID_VC1:
    case CODEC_ID_WMV3:
        s->h263_msmpeg4 = 1;
        s->h263_pred = 1;
        s->msmpeg4_version=6;
        break;
    case CODEC_ID_H263I:
        break;
    case CODEC_ID_FLV1:
    s->h263_flv = 1;
        break;
    default:
        return -1;
    }
    s->codec_id= avctx->codec->id;

    /* for h263, we allocate the images after having read the header */
    if (avctx->codec->id != CODEC_ID_H263 && avctx->codec->id != CODEC_ID_MPEG4)
        if (MPV_common_init(s) < 0)
            return -1;

    if (ENABLE_MSMPEG4_DECODER && s->h263_msmpeg4)
        ff_msmpeg4_decode_init(s);
    else
    h263_decode_init_vlc(s);

    return 0;
}

av_cold int ff_h263_decode_end(AVCodecContext *avctx)
{
    MpegEncContext *s = avctx->priv_data;

    MPV_common_end(s);
    return 0;
}

/**
 * returns the number of bytes consumed for building the current frame
 */
static int get_consumed_bytes(MpegEncContext *s, int buf_size){
    int pos= (get_bits_count(&s->gb)+7)>>3;

    if(s->divx_packed){
        //we would have to scan through the whole buf to handle the weird reordering ...
        return buf_size;
    }else if(s->flags&CODEC_FLAG_TRUNCATED){
        pos -= s->parse_context.last_index;
        if(pos<0) pos=0; // padding is not really read so this might be -1
        return pos;
    }else{
        if(pos==0) pos=1; //avoid infinite loops (i doubt that is needed but ...)
        if(pos+10>buf_size) pos=buf_size; // oops ;)

        return pos;
    }
}

static int decode_slice(MpegEncContext *s){
    const int part_mask= s->partitioned_frame ? (AC_END|AC_ERROR) : 0x7F;
    const int mb_size= 16>>s->avctx->lowres;
    s->last_resync_gb= s->gb;
    s->first_slice_line= 1;

    s->resync_mb_x= s->mb_x;
    s->resync_mb_y= s->mb_y;

    ff_set_qscale(s, s->qscale);

    if(s->partitioned_frame){
        const int qscale= s->qscale;

        /* restore variables which were modified */
        s->first_slice_line=1;
        s->mb_x= s->resync_mb_x;
        s->mb_y= s->resync_mb_y;
        ff_set_qscale(s, qscale);
    }

    for(; s->mb_y < s->mb_height; s->mb_y++) {
        /* per-row end of slice checks */
        ff_init_block_index(s);
        for(; s->mb_x < s->mb_width; s->mb_x++) {
            int ret;

            ff_update_block_index(s);

            if(s->resync_mb_x == s->mb_x && s->resync_mb_y+1 == s->mb_y){
                s->first_slice_line=0;
            }

            /* DCT & quantize */

            s->mv_dir = MV_DIR_FORWARD;
            s->mv_type = MV_TYPE_16X16;
//            s->mb_skipped = 0;
//printf("%d %d %06X\n", ret, get_bits_count(&s->gb), show_bits(&s->gb, 24));
            ret= s->decode_mb(s, s->block);

            if (s->pict_type!=FF_B_TYPE)
                ff_h263_update_motion_val(s);

            if(ret<0){
                const int xy= s->mb_x + s->mb_y*s->mb_stride;
                if(ret==SLICE_END){
                    MPV_decode_mb(s, s->block);

                    if(++s->mb_x >= s->mb_width){
                        s->mb_x=0;
                        ff_draw_horiz_band(s, s->mb_y*mb_size, mb_size);
                        s->mb_y++;
                    }
                    return 0;
                }else if(ret==SLICE_NOEND){
                    av_log(s->avctx, AV_LOG_ERROR, "Slice mismatch at MB: %d\n", xy);
                    return -1;
                }
                return -1;
            }

            MPV_decode_mb(s, s->block);
        }

        ff_draw_horiz_band(s, s->mb_y*mb_size, mb_size);

        s->mb_x= 0;
    }
    return -1;
}

int ff_h263_decode_frame(AVCodecContext *avctx,
                             void *data, int *data_size,
                             const uint8_t *buf, int buf_size)
{
    MpegEncContext *s = avctx->priv_data;
    int ret;
    AVFrame *pict = data;

#ifdef PRINT_FRAME_TIME
uint64_t time= rdtsc();
#endif
#ifdef DEBUG
    av_log(avctx, AV_LOG_DEBUG, "*****frame %d size=%d\n", avctx->frame_number, buf_size);
    if(buf_size>0)
        av_log(avctx, AV_LOG_DEBUG, "bytes=%x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
#endif
    s->flags= avctx->flags;
    s->flags2= avctx->flags2;

    /* no supplementary picture */
    if (buf_size == 0) {
        /* special case for last picture */
        if (s->low_delay==0 && s->next_picture_ptr) {
            *pict= *(AVFrame*)s->next_picture_ptr;
            s->next_picture_ptr= NULL;

            *data_size = sizeof(AVFrame);
        }

        return 0;
    }

retry:

    init_get_bits(&s->gb, buf, buf_size*8);
    s->bitstream_buffer_size=0;

    if (!s->context_initialized) {
        if (MPV_common_init(s) < 0) //we need the idct permutaton for reading a custom matrix
            return -1;
    }

    /* We need to set current_picture_ptr before reading the header,
     * otherwise we cannot store anyting in there */
    if(s->current_picture_ptr==NULL || s->current_picture_ptr->data[0]){
        int i= ff_find_unused_picture(s, 0);
        s->current_picture_ptr= &s->picture[i];
    }

    ret = flv_h263_decode_picture_header(s);

    if(ret==FRAME_SKIPPED) return get_consumed_bytes(s, buf_size);

    /* skip if the header was thrashed */
    if (ret < 0){
        av_log(s->avctx, AV_LOG_ERROR, "header damaged\n");
        return -1;
    }

    avctx->has_b_frames= !s->low_delay;

        /* After H263 & mpeg4 header decode we have the height, width,*/
        /* and other parameters. So then we could init the picture   */
        /* FIXME: By the way H263 decoder is evolving it should have */
        /* an H263EncContext                                         */

    if (   s->width  != avctx->coded_width
        || s->height != avctx->coded_height) {
        /* H.263 could change picture size any time */
        ParseContext pc= s->parse_context; //FIXME move these demuxng hack to avformat
        s->parse_context.buffer=0;
        MPV_common_end(s);
        s->parse_context= pc;
    }
    if (!s->context_initialized) {
        avcodec_set_dimensions(avctx, s->width, s->height);

        goto retry;
    }

    if((s->codec_id==CODEC_ID_H263 || s->codec_id==CODEC_ID_H263P))
        s->gob_index = ff_h263_get_gob_height(s);

    // for hurry_up==5
    s->current_picture.pict_type= s->pict_type;
    s->current_picture.key_frame= s->pict_type == FF_I_TYPE;

    /* skip B-frames if we don't have reference frames */
    if(s->last_picture_ptr==NULL && (s->pict_type==FF_B_TYPE || s->dropable)) return get_consumed_bytes(s, buf_size);
    /* skip b frames if we are in a hurry */
    if(avctx->hurry_up && s->pict_type==FF_B_TYPE) return get_consumed_bytes(s, buf_size);
    if(   (avctx->skip_frame >= AVDISCARD_NONREF && s->pict_type==FF_B_TYPE)
       || (avctx->skip_frame >= AVDISCARD_NONKEY && s->pict_type!=FF_I_TYPE)
       ||  avctx->skip_frame >= AVDISCARD_ALL)
        return get_consumed_bytes(s, buf_size);
    /* skip everything if we are in a hurry>=5 */
    if(avctx->hurry_up>=5) return get_consumed_bytes(s, buf_size);

    if(s->next_p_frame_damaged){
        if(s->pict_type==FF_B_TYPE)
            return get_consumed_bytes(s, buf_size);
        else
            s->next_p_frame_damaged=0;
    }

    if((s->avctx->flags2 & CODEC_FLAG2_FAST) && s->pict_type==FF_B_TYPE){
        s->me.qpel_put= s->dsp.put_2tap_qpel_pixels_tab;
        s->me.qpel_avg= s->dsp.avg_2tap_qpel_pixels_tab;
    }else if((!s->no_rounding) || s->pict_type==FF_B_TYPE){
        s->me.qpel_put= s->dsp.put_qpel_pixels_tab;
        s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;
    }else{
        s->me.qpel_put= s->dsp.put_no_rnd_qpel_pixels_tab;
        s->me.qpel_avg= s->dsp.avg_qpel_pixels_tab;
    }

    if(MPV_frame_start(s, avctx) < 0)
        return -1;

    /* decode each macroblock */
    s->mb_x=0;
    s->mb_y=0;

    decode_slice(s);
    while(s->mb_y<s->mb_height){
            if(ff_h263_resync(s)<0)
                break;

        decode_slice(s);
    }

intrax8_decoded:

    MPV_frame_end(s);

assert(s->current_picture.pict_type == s->current_picture_ptr->pict_type);
assert(s->current_picture.pict_type == s->pict_type);
    if (s->pict_type == FF_B_TYPE || s->low_delay) {
        *pict= *(AVFrame*)s->current_picture_ptr;
    } else if (s->last_picture_ptr != NULL) {
        *pict= *(AVFrame*)s->last_picture_ptr;
    }

    if(s->last_picture_ptr || s->low_delay){
        *data_size = sizeof(AVFrame);
        ff_print_debug_info(s, pict);
    }

    /* Return the Picture timestamp as the frame number */
    /* we subtract 1 because it is added on utils.c     */
    avctx->frame_number = s->picture_number - 1;

#ifdef PRINT_FRAME_TIME
av_log(avctx, AV_LOG_DEBUG, "%"PRId64"\n", rdtsc()-time);
#endif

    return get_consumed_bytes(s, buf_size);
}

AVCodec flv_decoder = {
    "flv",
    CODEC_TYPE_VIDEO,
    CODEC_ID_FLV1,
    sizeof(MpegEncContext),
    ff_h263_decode_init,
    NULL,
    ff_h263_decode_end,
    ff_h263_decode_frame,
    CODEC_CAP_DRAW_HORIZ_BAND | CODEC_CAP_DR1
};
