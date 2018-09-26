#ifndef TWOLABELSEED_H
#define TWOLABELSEED_H

#include"pointlistgeometry.h"
#include<QSize>


/**
 * \brief 
 */
class TwoLabelSeed
{
public:
	TwoLabelSeed();
	~TwoLabelSeed();

	void setSeeds(const PointListGeometry& foregroundSeed, const PointListGeometry& backgroundSeed);

	bool isSeedPoint(size_t index) const;

	bool isForegroundSeed(size_t index) const;

	void setToForegroundSeed(size_t index);

	void initialize(QSize dim);

	unsigned char* getSeedBuffer();

private:

	unsigned char* seedBuffer_;
	PointListGeometry foregroundSeed_, backgroundSeed_;
};

#endif // TWOLABELSEED_H
