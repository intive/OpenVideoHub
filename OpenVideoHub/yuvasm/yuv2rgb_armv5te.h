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

#ifndef __YUV2RGB_ARMV5TE_H__
#define __YUV2RGB_ARMV5TE_H__

#ifdef __cplusplus
extern "C" {
#endif

	void	YUV2RGB_Convert
		(
		 TUint8* aSrc[3],
		 TInt aSrcWidth,
		 TInt aSrcHeight,
		 TInt aSrcStride,
		 TInt aSrcStrideUV,
		 TInt /*aDstWidth*/,
		 TInt /*aDstHeight*/,
		 TInt aDstStride,
		 TAny* aDst
		);

	void	YUV2RGB_ConvertRotate
		(
		 TUint8* aSrc[3],
		 TInt aSrcWidth,
		 TInt aSrcHeight,
		 TInt aSrcStride,
		 TInt aSrcStrideUV,
		 TInt aDstWidth,
		 TInt /*aDstHeight*/,
		 TInt aDstStride,
		 TAny* aDst
		);

	void	YUV2RGB_ConvertScaleYUV
		(
		 TUint8* aSrc[3],
		 TInt aSrcWidth,
		 TInt aSrcHeight,
		 TInt aSrcStride,
		 TInt aSrcStrideUV,
		 TInt aDstWidth,
		 TInt aDstHeight,
		 TInt aDstStride,
		 TAny* aDst
		);

	void	YUV2RGB_ConvertRotateScaleYUV
		(
		 TUint8* aSrc[3],
		 TInt aSrcWidth,
		 TInt aSrcHeight,
		 TInt aSrcStride,
		 TInt aSrcStrideUV,
		 TInt aDstWidth,
		 TInt aDstHeight,
		 TInt aDstStride,
		 TAny* aDst
		);

	void	YUV2RGB_ScaleUpRGB
		(
		 TAny* aDst,
		 TAny* aSrc,
		 TInt aDX,
		 TInt aDY,
		 TInt aDstWidth,
		 TInt aDstStride,
		 TInt aDstSize
		);

#ifdef __cplusplus
}
#endif

#endif
