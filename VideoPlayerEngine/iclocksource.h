#ifndef H_ICLOCKSOURCE
#define H_ICLOCKSOURCE

class IClockSource
{
public:
	virtual double GetPosition() = 0; // In seconds.
protected:
	~IClockSource() { };
};

#endif
