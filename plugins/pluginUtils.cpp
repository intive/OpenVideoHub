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

#include <utf.h>
#include <EscapeUtils.h>

#include "pluginUtils.h"


void CPluginUtils::AddUrlParamL( RBuf& aBuf, const TDesC& aParam, const TDesC& aValue, TBool aLast)
	{
	aBuf.ReAllocL( aBuf.Length() + 1 + aParam.Length() + aValue.Length() );
	aBuf.Append( aParam );
	aBuf.Append( _L("=") );
	aBuf.Append( aValue );

	if( !aLast )
		{
		aBuf.ReAllocL( aBuf.Length() + 1 );
		aBuf.Append( _L("&") );
		}
	}

void CPluginUtils::AddUrlParamL( RBuf& aBuf, const TDesC& aParam, TInt aValue, TBool aLast )
	{
	TBuf<12> t;
	t.Format( _L("%d"), aValue );
	aBuf.ReAllocL( aBuf.Length() + 1 + aParam.Length() + t.Length() );
	aBuf.Append( aParam );
	aBuf.Append( _L("=") );
	aBuf.Append( t );

	if( !aLast )
		{
		aBuf.ReAllocL( aBuf.Length() + 1 );
		aBuf.Append( _L("&") );
		}
	}

void CPluginUtils::AddMultipartParamL( RBuf& aBuf, const TDesC& aParam, const TDesC& aValue )
	{
	aBuf.ReAllocL( aBuf.Length() + KBoundary().Length() + aParam.Length() + aValue.Length() + 2 + 2 + 2 + 2 + 2 + 1 + KDisposition().Length() );
	aBuf.Append( _L("--") );
	aBuf.Append( KBoundary() );
	aBuf.Append( KCrLf );
	aBuf.Append( KDisposition() );
	aBuf.Append( aParam );
	aBuf.Append( _L("\"") );
	aBuf.Append( KCrLf );
	aBuf.Append( KCrLf );
	aBuf.Append( aValue );
	aBuf.Append( KCrLf );
	}

void CPluginUtils::AddMultipartParamL( RBuf& aBuf, const TDesC& aParam, TInt aValue )
	{
	TBuf<12> t;
	t.Format( _L("%d"), aValue );

	aBuf.ReAllocL( aBuf.Length() + KBoundary().Length() + aParam.Length() + t.Length() + 2 + 2 + 2 + 2 + 2 + 1 + KDisposition().Length() );
	aBuf.Append( _L("--") );
	aBuf.Append( KBoundary() );
	aBuf.Append( KCrLf );
	aBuf.Append( KDisposition() );
	aBuf.Append( aParam );
	aBuf.Append( _L("\"") );
	aBuf.Append( KCrLf );
	aBuf.Append( KCrLf );
	aBuf.Append( t );
	aBuf.Append( KCrLf );
	}

HBufC* CPluginUtils::ConvertUtfToUnicodeL( const TDesC8& aUtf7 )
	{

	RBuf output;
	CleanupClosePushL( output );
	
	TBuf16<20> outputBuffer;
	TPtrC8 remainderOfUtf7( aUtf7 );

	for(;;)
		{
		const TInt returnValue = CnvUtfConverter::ConvertToUnicodeFromUtf8(outputBuffer, remainderOfUtf7);
		if (returnValue==CnvUtfConverter::EErrorIllFormedInput)
			return NULL;
		else if (returnValue<0)
			return NULL;
        
		output.ReAllocL( output.Length() + outputBuffer.Length() );
		output.Append( outputBuffer );

        if (returnValue == 0)
            break;

        remainderOfUtf7.Set(remainderOfUtf7.Right(returnValue));
		}

	HBufC* ret = output.AllocL();
	
	CleanupStack::PopAndDestroy( &output );
	return ret;
	}
