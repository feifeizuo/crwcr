#ifndef CRWCRSOLVER_H
#define CRWCRSOLVER_H

#include<string>
#include "singleton.h"
#include"twolabelseed.h"


/**
 * \brief Implement CRWCR algorithm
 */
class CRWCRSolver
{
public:
	CRWCRSolver(const float* image, int width, int height);
	~CRWCRSolver();

	void setSeed(TwoLabelSeed* seed);

	void solve();

	float* generateProbabilityImage() const;

	float getUseTime() const;

private:

	void initialization();

	void prcorrection();

	void TDMA(double* a, double* b, double* c, double* d, double* x, int numRow);

	void normalize(float* data, size_t length);

	void calculateWeight();

	void calculateGradient();

	Parameters& parameters_;

	TwoLabelSeed* seeds_;

	const float* image_;
	int width_, height_;
	size_t numPixels_;

	// weight
	float *wx_, *wy_;

	float* grad_;
	float* solution_;
	int time_;
};

#endif // !CRWCRSOLVER_H
