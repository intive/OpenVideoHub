/*
 * Copyright (c) 2007-2009 BLStream Oy.
 *
 * This file is part of OpenVideoHub.
 *
 * OpenVideoHub is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * OpenVideoHub is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpenVideoHub; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <fbs.h>
#include <eikenv.h>

#ifndef __WINS__
#define USE_ARMV5_CODE
#include "yuv2rgb_armv5te.h"
#endif

#include "emTubeYUV2RGB.h"

CEmTubeYUV2RGB* CEmTubeYUV2RGB::NewL()
	{
	CEmTubeYUV2RGB* self = CEmTubeYUV2RGB::NewLC();
	CleanupStack::Pop( self );
	return self;
	}

CEmTubeYUV2RGB* CEmTubeYUV2RGB::NewLC()
	{
	CEmTubeYUV2RGB* self = new ( ELeave ) CEmTubeYUV2RGB();
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

CEmTubeYUV2RGB::~CEmTubeYUV2RGB()
	{
	delete [] iTmpData;
	iTmpData = NULL;
	}

CEmTubeYUV2RGB::CEmTubeYUV2RGB()
	{
	}

void CEmTubeYUV2RGB::ConstructL()
	{
	}

//b = y + ( u * 29032) >> 14;
//g = y + ((u * -11277) + (-23401 * v) >> 15;
//r = y + ( v * 22970) >> 14;

//b = y + ( u * 132798) >> 16;
//g = y + ((u * -24759)  +  (-53109 * v) >> 16;
//r = y + ( v * 104448);

void CEmTubeYUV2RGB::Init( TDisplayMode aMode, TRotation aRotation, TScale aScale )
	{
	iMode = aMode;

#ifndef USE_ARMV5_CODE
	delete [] iTmpData;
	iTmpData = NULL;

	TInt crv,cbu,cgu,cgv;
	TInt i;

#if 1
	crv = 104448;
	cbu = 132798;
	cgu = 24759;
	cgv = 53109;

	for ( i = 0; i < 256; i++ )
		{
		iCrv[i] = ((i-128) * crv) >> 16;
		iCbu[i] = ((i-128) * cbu) >> 16;
		iCgu[i] = -(((i-128) * cgu) >> 16);
		iCgv[i] = -(((i-128) * cgv) >> 16);
		}
#else
	crv = 22970;
	cbu = 29032;
	cgu = 11277;
	cgv = 23401;

	for ( i = 0; i < 256; i++ )
		{
		iCrv[i] = ((i-128) * crv) >> 14;
		iCbu[i] = ((i-128) * cbu) >> 14;
		iCgu[i] = -(((i-128) * cgu) >> 15);
		iCgv[i] = -(((i-128) * cgv) >> 15);
		}
#endif

	iClipTablePtr = &iClipTable[0];
	iClipTablePtr += 384;
	for ( i = -384; i < 640; i++ )
		iClipTablePtr[i] = (i > 255 ) ? 255 : ((i < 0) ? 0 : i );
#endif //USE_ARMV5_CODE

	iConvertRoutine = NULL;

	switch( iMode )
		{
		case EColor16MU:
			if( aScale != EScaleNone )
				{
				switch( aScale )
					{
					case EScaleUp:
						{
						if( aRotation )
							{
							iConvertRoutine = &CEmTubeYUV2RGB::ConvertScaleRotateRGB32_rgb;
							}
						else
							{
							iConvertRoutine = &CEmTubeYUV2RGB::ConvertScaleRGB32_rgb;
							}
						}
					break;

					case EScaleDown:
						{
						if( aRotation )
							{
							iConvertRoutine = &CEmTubeYUV2RGB::ConvertScaleRotateRGB32_yuv;
							}
						else
							{
							iConvertRoutine = &CEmTubeYUV2RGB::ConvertScaleRGB32_yuv;
							}
						}
					break;

					default:
					break;

					}
				}
			else
				{
				if( aRotation )
					{
					iConvertRoutine = &CEmTubeYUV2RGB::ConvertRotateRGB32;
					}
				else
					{
					iConvertRoutine = &CEmTubeYUV2RGB::ConvertRGB32;
					}
				}
		break;

		default:
		break;
		}

	if( !iConvertRoutine )
		User::Panic( _L("unknown pxl format"), iMode );
	}

void CEmTubeYUV2RGB::ConvertRGB32(TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt /*aDstWidth*/,
									TInt /*aDstHeight*/,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	YUV2RGB_Convert(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, 0, 0, aDstStride, aDst);
#else
	TInt cols = aSrcWidth >> 1, rows = aSrcHeight >> 1;
	TInt x, y;
	TUint8 *pu, *pv;
	TUint8 *py1, *py2;
	TInt16 t;
	TInt16 u, v;
	TUint32 rgb;
	TInt mod, mod_y, mod_uv;
	TUint32* rgbp1, *rgbp2;
	TInt c11, c21, c41;

	rgbp1 = (TUint32*)aDst;
	rgbp2 = rgbp1 + aDstStride;

	py1 = (TUint8*)aSrc[0];
	py2 = (TUint8*)( aSrc[0] + aSrcStride );
	pu = aSrc[1];
	pv = aSrc[2];

//unroll loop
	cols >>= 1;
	cols &= ~1;

	mod = aDstStride + (aDstStride - (cols<<2));
	mod_uv = aSrcStrideUV - ( (cols<<2) >> 1 );
	mod_y = aSrcStride + (aSrcStride - (cols<<2));

	unsigned char *clipTablePtr = iClipTablePtr;

	TUint8* tmp;

	for (y = 0; y < rows; y++)
		{
		for (x = 0; x < cols; x++)
			{

			u = *pu++;
			c41 = iCbu[u];
			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];

			v = *pv++;
			c21 = iCgu[u];
			c21 += iCgv[v];
			rgb |= (tmp[c21] << 8);
			c11 = iCrv[v];
			rgb |= (tmp[c11] << 16);
			*rgbp1++ = rgb;

			t = *py1++;
			tmp = &clipTablePtr[t];

			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp1++ = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp2++ = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp2++ = rgb;
//second row

			u = *pu++;
			c41 = iCbu[u];
			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];

			v = *pv++;
			c21 = iCgu[u];
			c21 += iCgv[v];
			rgb |= (tmp[c21] << 8);
			c11 = iCrv[v];
			rgb |= (tmp[c11] << 16);
			*rgbp1++ = rgb;

			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp1++ = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp2++ = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*rgbp2++ = rgb;
			}

		py1 += mod_y;
		py2 += mod_y;

		pu += mod_uv;
		pv += mod_uv;

		rgbp1 += mod;
		rgbp2 += mod;
		}
#endif
	}

void CEmTubeYUV2RGB::ConvertScaleRGB32_rgb(
									TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	TUint32 * sDst = (TUint32 *) aDst + (aDstHeight - aSrcHeight) * aDstStride + (aDstWidth - aSrcWidth);
	YUV2RGB_Convert(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, 0, 0, aDstStride, sDst);
	YUV2RGB_ScaleUpRGB(aDst, sDst, (aSrcWidth << 16) / aDstWidth, (aSrcHeight << 16) / aDstHeight, aDstWidth, aDstStride, aDstHeight * aDstStride);
#else
	if( !iTmpData )
		iTmpData = new TUint32[ aSrcWidth * aSrcHeight ];

	ConvertRGB32( aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, aDstHeight, aSrcWidth, iTmpData );
	Scale( iTmpData, aSrcWidth, aSrcHeight, (TUint32*)aDst, aDstWidth, aDstHeight, aDstStride );
#endif
	}

void CEmTubeYUV2RGB::ConvertScaleRGB32_yuv(
									TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	YUV2RGB_ConvertScaleYUV(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, aDstHeight, aDstStride, aDst);
#else
//TODO this one doesn't work properly on arm metacafe.flv
//and on 320x240 videos in portrait mode on N95
	int c;
	TInt32 u, v, t0;
	TUint32 rgb;
	TInt pos;
	TInt newPos;
	TInt newPos_uv;
	TInt dx, dy, dx_uv, dy_uv, x, x_uv, y, y_uv, i, j;

	dx = (aSrcWidth << 16) / aDstWidth;
	dy = (aSrcHeight << 16) / aDstHeight;
	dx_uv = dx >> 1;
	dy_uv = dy >> 1;

	TUint8* yp = aSrc[0];
	TUint8* up = aSrc[1];
	TUint8* vp = aSrc[2];

	TUint32* dst = (TUint32*)aDst;

	TUint8* tmp;
	TUint8* tmp1 = iClipTablePtr;

	y = 0;
	y_uv = 0;

	TInt dstMod = aDstStride - aDstWidth;
	aDstWidth >>= 1;

	for(i=0;i<aDstHeight;i++)
		{
		x = 0;
		x_uv = 0;

		newPos = ( y >> 16 ) * aSrcStride;
		newPos_uv = ( y_uv >> 16 ) * aSrcStrideUV;

		for(j=0;j<aDstWidth;j++)
			{
			t0 = yp[ newPos + (x >> 16) ];
			tmp = &tmp1[t0];
			pos = newPos_uv + (x_uv >> 16);

			x += dx;
			x_uv += dx_uv;

			u = up[ pos ];


			c = iCbu[u];
			rgb = tmp[c];

			v = vp[ pos ];
			c = iCgu[u];
			c += iCgv[v];
			rgb |= ( tmp[c] << 8 );

			c = iCrv[v];
			rgb |= ( tmp[c] << 16 );

			*dst++ = rgb;

//second loop

			t0 = yp[ newPos + (x >> 16) ];
			tmp = &tmp1[t0];
			pos = newPos_uv + (x_uv >> 16);

			x += dx;
			x_uv += dx_uv;

			u = up[ pos ];


			c = iCbu[u];
			rgb = tmp[c];

			v = vp[ pos ];
			c = iCgu[u];
			c += iCgv[v];
			rgb |= ( tmp[c] << 8 );

			c = iCrv[v];
			rgb |= ( tmp[c] << 16 );

			*dst++ = rgb;
			}

		dst += dstMod;
		y += dy;
		y_uv += dy_uv;
		}
#endif
	}

void CEmTubeYUV2RGB::ConvertScaleRotateRGB32_yuv(
									TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	YUV2RGB_ConvertRotateScaleYUV(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, aDstHeight, aDstStride, aDst);
#else
	int c;
	TInt16 u, v, t0;
	TUint32 rgb;
	TInt pos;
	TInt newPos;
	TInt newPos_uv;
	TInt dx, dy, dx_uv, dy_uv, x, x_uv, y, y_uv, i, j;

	dx = (aSrcWidth << 16) / aDstHeight;
	dy = (aSrcHeight << 16) / aDstWidth;
	dx_uv = dx >> 1;
	dy_uv = dy >> 1;

	TUint8* yp = aSrc[0];
	TUint8* up = aSrc[1];
	TUint8* vp = aSrc[2];

	TUint32* dst = (TUint32*)aDst;
	dst += ( aDstWidth - 1 );

	TUint8* tmp;
	TUint8* tmp1 = iClipTablePtr;
	aDstHeight >>= 1;

	y = 0;
	y_uv = 0;

	for(i=0;i<aDstWidth;i++)
		{
		x = 0;
		x_uv = 0;

		newPos = ( y >> 16 ) * aSrcStride;
		newPos_uv = ( y_uv >> 16 ) * aSrcStrideUV;

		TUint32 *dst1 = dst;

		for(j=0;j<aDstHeight;j++)
			{
			t0 = yp[ newPos + (x >> 16) ];
			tmp = &tmp1[t0];
			pos = newPos_uv + (x_uv >> 16);
			x += dx;
			x_uv += dx_uv;

			u = up[ pos ];
			c = iCbu[u];
			rgb = tmp[c];

			v = vp[ pos ];
			c = iCgu[u];
			c += iCgv[v];
			rgb |= ( tmp[ c ] << 8 );

			c = iCrv[v];
			rgb |= ( tmp[ c ] << 16 );

			*dst1 = rgb;
			dst1 += aDstStride;
//second row
			t0 = yp[ newPos + (x >> 16) ];
			tmp = &tmp1[t0];
			pos = newPos_uv + (x_uv >> 16);
			x += dx;
			x_uv += dx_uv;

			u = up[ pos ];
			c = iCbu[u];
			rgb = tmp[c];

			v = vp[ pos ];
			c = iCgu[u];
			c += iCgv[v];
			rgb |= ( tmp[ c ] << 8 );

			c = iCrv[v];
			rgb |= ( tmp[ c ] << 16 );

			*dst1 = rgb;
			dst1 += aDstStride;
			}

		dst--;
		y += dy;
		y_uv += dy_uv;
		}
#endif
	}

void CEmTubeYUV2RGB::ConvertScaleRotateRGB32_rgb(
									TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	TUint32 * sDst = (TUint32 *) aDst + (aDstHeight - aSrcWidth) * aDstStride;
	YUV2RGB_ConvertRotate(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, 0, aDstStride, sDst);
	sDst += aDstWidth - aSrcHeight;
	YUV2RGB_ScaleUpRGB(aDst, sDst, (aSrcHeight << 16) / aDstWidth, (aSrcWidth << 16) / aDstHeight, aDstWidth, aDstStride, aDstHeight * aDstStride);
#else
	if( !iTmpData )
		iTmpData = new TUint32[ aSrcWidth * aSrcHeight ];

	ConvertRGB32( aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, aDstHeight, aSrcWidth, iTmpData );
	ScaleRotate( iTmpData, aSrcWidth, aSrcHeight, (TUint32*)aDst, aDstWidth, aDstHeight, aDstStride );
#endif
	}

void CEmTubeYUV2RGB::ConvertRotateRGB32(TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt /*aDstHeight*/,
									TInt aDstStride,
									TAny* aDst)
	{
#ifdef USE_ARMV5_CODE
	YUV2RGB_ConvertRotate(aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV, aDstWidth, 0, aDstStride, aDst);
#else
	int c11, c21, c41;
	TInt cols = aSrcWidth >> 1, rows = aSrcHeight >> 1;
	TInt x, y;
	TUint8 *pu, *pv;
	TUint8 *py1, *py2;
	TInt16 t;
	TInt16 u, v;
	TUint32 rgb;
	TInt mod_y, mod_uv;
	TUint32* rgbp1;
	TInt dstStride2;

	rgbp1 = (TUint32*)aDst;
	rgbp1 += (aDstWidth - 1);

	py1 = (TUint8*)aSrc[0];
	py2 = (TUint8*)( aSrc[0] + aSrcStride );
	pu = aSrc[1];
	pv = aSrc[2];

	cols >>= 1;
	cols &= ~1;

	mod_uv = aSrcStrideUV - ( (cols<<2) >> 1 );
	mod_y = aSrcStride + (aSrcStride - (cols<<2));

	dstStride2 = ( aDstStride << 1);

	TUint8* tmp;
	unsigned char *clipTablePtr = iClipTablePtr;

	for (y = 0; y < rows; y++)
		{
		TUint32 *rgbp2 = rgbp1;

		for (x = 0; x < cols; x++)
			{
			u = *pu++;
			c41 = iCbu[u];
			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];

			v = *pv++;
			c21 = iCgu[u];
			c21 += iCgv[v];
			rgb |= (tmp[c21] << 8);

			c11 = iCrv[v];
			rgb |= (tmp[c11] << 16);
			*rgbp2 = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 - 1) = rgb;

			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 + aDstStride) = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 + aDstStride - 1) = rgb;

			rgbp2 += dstStride2;
//second
			u = *pu++;
			c41 = iCbu[u];
			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];

			v = *pv++;
			c21 = iCgu[u];
			c21 += iCgv[v];
			rgb |= (tmp[c21] << 8);

			c11 = iCrv[v];
			rgb |= (tmp[c11] << 16);
			*rgbp2 = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 - 1) = rgb;

			t = *py1++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 + aDstStride) = rgb;

			t = *py2++;
			tmp = &clipTablePtr[t];
			rgb = tmp[c41];
			rgb |= (tmp[c21] << 8);
			rgb |= (tmp[c11] << 16);
			*(rgbp2 + aDstStride - 1) = rgb;

			rgbp2 += dstStride2;
			}
		py1 += mod_y;
		py2 += mod_y;

		pu += mod_uv;
		pv += mod_uv;

		rgbp1 -= 2;
		}
#endif
 	}

#ifndef USE_ARMV5_CODE
void CEmTubeYUV2RGB::Scale(TUint32* aSrc,
							TInt aSrcWidth,
							TInt aSrcHeight,
							TUint32* aDst,
							TInt aDstWidth,
							TInt aDstHeight,
							TInt aDstStride)
	{
	TInt newPos;
	TInt dx, dy, x, y, i, j;

	dx = (aSrcWidth << 16) / aDstWidth;
	dy = (aSrcHeight << 16) / aDstHeight;

	TUint32* dst = (TUint32*)aDst;

	y = 0;

	TInt dstMod = aDstStride - aDstWidth;

	for(i=0;i<aDstHeight;i++)
		{
		x = 0;

		newPos = ( y >> 16 ) * aSrcWidth;
		TUint32* src = aSrc + newPos;

		for(j=0;j<aDstWidth>>1;j++)
			{
			*dst++ = src[ x >> 16 ];
			x += dx;
			*dst++ = src[ x >> 16 ];
			x += dx;
			}

		dst += dstMod;
		y += dy;
		}
	}

void CEmTubeYUV2RGB::ScaleRotate(TUint32* aSrc,
								TInt aSrcWidth,
								TInt aSrcHeight,
								TUint32* aDst,
								TInt aDstWidth,
								TInt aDstHeight,
								TInt aDstStride)
	{
	TInt newPos;
	TInt dx, dy, x, y, i, j;

	dx = (aSrcWidth << 16) / aDstHeight;
	dy = (aSrcHeight << 16) / aDstWidth;

	TUint32* dst = (TUint32*)aDst;
	dst += ( aDstWidth - 1 );

	y = 0;

	for(i=0;i<aDstWidth;i++)
		{
		x = 0;
		newPos = ( y >> 16 ) * aSrcWidth;

		TUint32 *dst1 = dst;
		TUint32 *src = aSrc + newPos;

		for(j=0;j<aDstHeight>>1;j++)
			{
			*dst1 = src[ x >> 16 ];
			x += dx;
			dst1 += aDstStride;

			*dst1 = src[ x >> 16 ];
			x += dx;
			dst1 += aDstStride;
			}

		dst--;
		y += dy;
		}
	}
#endif

void CEmTubeYUV2RGB::Convert(TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst)
	{

	if( iConvertRoutine )
		(this->*iConvertRoutine)( aSrc, aSrcWidth, aSrcHeight, aSrcStride, aSrcStrideUV,
							aDstWidth, aDstHeight, aDstStride, aDst);
	}
