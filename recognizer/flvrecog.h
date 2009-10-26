
#ifndef FLV_RECOG_H
#define FLV_RECOG_H

#include <apmrec.h>

class CFlvRecognizer: public CApaDataRecognizerType
	{
public:
	CFlvRecognizer();
	TUint PreferredBufSize();
	TDataType SupportedDataTypeL(TInt) const;   
	static CApaDataRecognizerType* CreateRecognizerL();
private:
	void DoRecognizeL(const TDesC&, const TDesC8&);
	};

#endif


