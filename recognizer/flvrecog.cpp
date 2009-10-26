#include <eikenv.h>
#include <ImplementationProxy.h> 
#include "flvrecog.h"

const TInt KNumDataTypes = 1;
const TUid KDllUid = {0x2001A7E4};
const TInt KImplementationUid = 0x2001A7E5;

_LIT8(KDataTypeFLV, "application/openvideohub");

CFlvRecognizer::CFlvRecognizer():CApaDataRecognizerType(KDllUid, CApaDataRecognizerType::EHigh)
	{
	iCountDataTypes = KNumDataTypes;
	}

TUint CFlvRecognizer::PreferredBufSize()
	{
	return 8;
	}

TDataType CFlvRecognizer::SupportedDataTypeL(TInt /*aIndex*/) const
	{
	return TDataType(KDataTypeFLV);
	}

void CFlvRecognizer::DoRecognizeL(const TDesC& aName, const TDesC8& aBuffer)
	{
	iConfidence = ENotRecognized;

	if( aBuffer.Length() > 4 )
		{
		if (aBuffer[0] == 'F' && aBuffer[1] == 'L' && aBuffer[2] == 'V' && aBuffer[3] < 5 )
			{
			iConfidence = ECertain;
			iDataType = TDataType(KDataTypeFLV); 
			}
		}
	else
		{
		TParse parse;
		parse.Set(aName,NULL,NULL);
		TPtrC ext = parse.Ext(); // extract the extension from the filename

		if( ext.CompareF(_L(".flv")) == 0 )
			{
			iConfidence = ECertain;
			iDataType = TDataType(KDataTypeFLV); 
			}
		}
    }

CApaDataRecognizerType* CFlvRecognizer::CreateRecognizerL()
	{
	return new (ELeave) CFlvRecognizer;
	}

const TImplementationProxy ImplementationTable[] = 
    {
	IMPLEMENTATION_PROXY_ENTRY(KImplementationUid,CFlvRecognizer::CreateRecognizerL)
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);
    return ImplementationTable;
    }
