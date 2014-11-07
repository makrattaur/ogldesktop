#ifndef G_H_SCOPEDCOMINITIALISATION
#define G_H_SCOPEDCOMINITIALISATION

class ScopedComInitialisation
{
public:
	enum ApartementType
	{
		MTA,
		STA
	};
	ScopedComInitialisation(ApartementType type = ApartementType::MTA);
	~ScopedComInitialisation();

	bool IsInitialised();
private:
	bool m_initialised;
};


#endif

