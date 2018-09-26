#ifndef CRWCRALGORITHM_H
#define CRWCRALGORITHM_H

#ifdef USE_GPU
#include"crwcrgpusolver.h"
#else
#include"crwcrsolver.h"
#endif

#include "twolabelseed.h"
#include <QObject>
#include <QSizeF>


/**
 * \brief CRWCR algorithm
 */
class CRWCRAlgorithm : public QObject
{
Q_OBJECT
public:
	explicit CRWCRAlgorithm(QObject* parent = nullptr);
	~CRWCRAlgorithm();

	void setImage(const QImage& data);

signals:

	void segmentationDone(float*);
	void segmentationTime(int);

public slots:

	void process();

	void setSeeds(const PointListGeometry& foregroundseed, const PointListGeometry& backgroundseed);

	void setPreProcessState(int state)
	{
		isPreProcess_ = state > 0;
	}

private:
	TwoLabelSeed* twoLabelSeed_;
	CRWCRSolver* solver_;

	bool isPreProcess_;
	float* image_;
	QSize dim_;
};

#endif  // CRWCRALGORITHM_H
