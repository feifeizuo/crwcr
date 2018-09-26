#ifndef CRWCRGPUSOLVER_H
#define CRWCRGPUSOLVER_H

#include"twolabelseed.h"
#include"singleton.h"

class CRWCRSolver
{
public:
	CRWCRSolver(const float* image,int width, int height);
	~CRWCRSolver();

	void setSeed(TwoLabelSeed* seed);

	void solve();

	float* generateProbabilityImage()const;

	int getUseTime()const;

private:

	void normalize(float* data, size_t length);

	void calculateWeight();

	void calculateGradient();

	Parameters& parameters_;

	int time_;

	TwoLabelSeed* seeds_;
	unsigned char* d_seedBuffer_;

	const float *image_;
	int width_, height_;
	size_t numPixels_;

	// weight
	float* d_wx, *d_wy;

	float* d_grad;
	float* d_solution, *solution_;
	float* d_matCen, *d_matSub, *d_rVec, *d_matUp;

};

#endif // !CRWCRGPUSOLVER_H