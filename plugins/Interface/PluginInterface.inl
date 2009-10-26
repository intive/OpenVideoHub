
inline CPluginInterface::CPluginInterface()
	{
	}

inline CPluginInterface::~CPluginInterface()
	{
	REComSession::DestroyedImplementation(iDtor_ID_Key);
	}

inline CPluginInterface* CPluginInterface::NewL( TUid aUid )
	{
	return REINTERPRET_CAST( CPluginInterface*,
					REComSession::CreateImplementationL( aUid, 
					_FOFF( CPluginInterface, iDtor_ID_Key ) ) );
	}

inline CPluginInterface* CPluginInterface::NewL(const TDesC8& aMatch, TUid aUid )
    {
    TEComResolverParams resolverParams;
    resolverParams.SetDataType( aMatch );
    resolverParams.SetWildcardMatch( ETrue );
    return reinterpret_cast<CPluginInterface*>
                  (REComSession::CreateImplementationL( aUid,
                  _FOFF(CPluginInterface, iDtor_ID_Key ), 
                  NULL, resolverParams) );
    }

inline void CPluginInterface::ListAllImplementationsL( RImplInfoPtrArray& aImplInfoArray )
	{
	REComSession::ListImplementationsL( KPluginInterfaceUid, aImplInfoArray );
	}
