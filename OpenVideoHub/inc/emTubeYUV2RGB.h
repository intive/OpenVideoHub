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

#ifndef EMTUBE_YUV2RGB_H
#define EMTUBE_YUV2RGB_H

#include <e32base.h>
#include <gdi.h>

class CEmTubeYUV2RGB : public CBase
	{
	public:
		enum TRotation
		{
			ERotationNone = 0,
			ERotation90
		};

		enum TScale
		{
			EScaleNone = 0,
			EScaleUp,
			EScaleDown
		};
	
	public:
		static CEmTubeYUV2RGB* NewL();
		static CEmTubeYUV2RGB* NewLC();
		virtual ~CEmTubeYUV2RGB();

	private:
		CEmTubeYUV2RGB();
		void ConstructL();

		void Scale(TUint32* aSrc, TInt aSrcWidth, TInt aSrcHeight, TUint32* aDst, TInt aDstWidth, TInt aDstHeight, TInt aDstStride);
		void ScaleRotate(TUint32* aSrc, TInt aSrcWidth, TInt aSrcHeight, TUint32* aDst, TInt aDstWidth, TInt aDstHeight, TInt aDstStride);

	public:
		void Init( TDisplayMode aMode, TRotation aRotation, TScale aScale );

		void Convert(TUint8* aSrc[3],
				   TInt aSrcWidth,
				   TInt aSrcHeight,
				   TInt aSrcStride,
				   TInt aSrcStrideUV,
				   TInt aDstWidth,
				   TInt aDstHeight,
				   TInt aDstStride,
					 TAny* aDst);

	private:
		void ConvertRGB32(TUint8* aSrc[3],
						TInt aSrcWidth,
						TInt aSrcHeight,
						TInt aSrcStride,
						TInt aSrcStrideUV,
						TInt aDstWidth,
						TInt aDstHeight,
						TInt aDstStride,
						TAny* aDst);

		void ConvertScaleRGB32_rgb(
						TUint8* aSrc[3],
						TInt aSrcWidth,
						TInt aSrcHeight,
						TInt aSrcStride,
						TInt aSrcStrideUV,
						TInt aDstWidth,
						TInt aDstHeight,
						TInt aDstStride,
						TAny* aDst);

		void ConvertScaleRGB32_yuv(
						TUint8* aSrc[3],
						TInt aSrcWidth,
						TInt aSrcHeight,
						TInt aSrcStride,
						TInt aSrcStrideUV,
						TInt aDstWidth,
						TInt aDstHeight,
						TInt aDstStride,
						TAny* aDst);

		void ConvertRotateRGB32(TUint8* aSrc[3],
								TInt aSrcWidth,
								TInt aSrcHeight,
								TInt aSrcStride,
								TInt aSrcStrideUV,
								TInt aDstWidth,
								TInt aDstHeight,
								TInt aDstStride,
								TAny* aDst);

		void ConvertScaleRotateRGB32_rgb( TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst);

		void ConvertScaleRotateRGB32_yuv( TUint8* aSrc[3],
									TInt aSrcWidth,
									TInt aSrcHeight,
									TInt aSrcStride,
									TInt aSrcStrideUV,
									TInt aDstWidth,
									TInt aDstHeight,
									TInt aDstStride,
									TAny* aDst);
	private:
		TDisplayMode iMode;

		void (CEmTubeYUV2RGB::*iConvertRoutine)(TUint8* aSrc[3],
								TInt aSrcWidth,
								TInt aSrcHeight,
								TInt aSrcStride,
								TInt aSrcStrideUV,
								TInt aDstWidth,
								TInt aDstHeight,
								TInt aDstStride,
								TAny* aDst);

		TInt16 iCrv[256];
		TInt16 iCbu[256];
		TInt8 iCgu[256];
		TInt8 iCgv[256];
		TUint8 *iClipTablePtr;
		TUint8 iClipTable[1024];

		TUint32* iTmpData;

	};

#endif // EMTUBE_YUV2RGB_H
