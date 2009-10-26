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

#ifndef VIDEO_HANDLER_H
#define VIDEO_HANDLER_H

class MVideoHandler
{
	public:
		virtual void PlaybackFinished() = 0;
		virtual void PlaybackReady() = 0;
		virtual void PositionUpdate( TInt32 aCurrent, TInt32 aTotal ) = 0;

		virtual void FileOpenedL( TInt aError ) = 0;
		virtual void FileClosedL( TInt aError ) = 0;

		virtual void HandleOsdButtonPressedL( TInt aCommand ) = 0;
		virtual void HandleOsdButtonReleasedL( TInt aCommand ) = 0;
		virtual void HandleOsdButtonRepeatL( TInt aCommand ) = 0;
		virtual void HandleOsdSliderPositionChangeL( TInt aCommand, TInt aValue ) = 0;

};

#endif //VIDEO_HANDLER_H
