#include <algorithm>
#include"crwcrkernel.cuh"
#include "crwcrgpusolver.h"

#pragma comment(lib, "cudart.lib")

CRWCRSolver::CRWCRSolver(const float * image, int width, int height) :
	image_(image),
	width_(width),
	height_(height),
	seeds_(nullptr),
	parameters_(Singleton<Parameters>::GetInstance())
{
	numPixels_ = width_ * height_;
	
	cudaMalloc((void**)&d_wx, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_wy, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_grad, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_seedBuffer_, numPixels_ * sizeof(unsigned char));
	cudaMalloc((void**)&d_matSub, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_matCen, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_matUp, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_rVec, numPixels_ * sizeof(float));
	cudaMalloc((void**)&d_solution, numPixels_ * sizeof(float));

	solution_ = new float[numPixels_];

	calculateWeight();
	calculateGradient();
}

CRWCRSolver::~CRWCRSolver()
{
	cudaFree(d_grad);
	cudaFree(d_wx);
	cudaFree(d_wy);
	cudaFree(d_solution);
	cudaFree(d_matCen);
	cudaFree(d_matUp);
	cudaFree(d_matSub);
	cudaFree(d_rVec);
	cudaFree(d_seedBuffer_);
	delete[] solution_;
	solution_ = nullptr;
}

void CRWCRSolver::setSeed(TwoLabelSeed * seed)
{
	seeds_ = seed;
}

void CRWCRSolver::solve()
{
	cudaMemcpy(d_seedBuffer_, seeds_->getSeedBuffer(), numPixels_ * sizeof(unsigned char), cudaMemcpyHostToDevice);

	cudaEvent_t start, stop;
	float time;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);

	const int blockDim = 8;

	dim3 dimBlock(blockDim, blockDim, 1);
	dim3 dimGrid(CRWCRGPU::iDivUp(width_,blockDim), CRWCRGPU::iDivUp(height_, blockDim), 1);

	dim3 dimB(blockDim, 1, 1);
	dim3 dimG(CRWCRGPU::iDivUp(width_*height_, blockDim), 1, 1);

	int2 dim = make_int2(width_, height_);

	for(int iter=0;iter<parameters_.maxIterations1D;iter++)
	{
		CRWCRGPU::rowSweepKernel << <dimGrid, dimBlock >> > (d_wx, d_grad,
			d_seedBuffer_, d_matSub, d_matCen, d_matUp, d_rVec,dim,parameters_.gamma1D,parameters_.lambda1D);
		CRWCRGPU::TDMARow << <dimG, dimB >> > (d_matSub, d_matCen, d_matUp, d_rVec, d_solution, dim);
		CRWCRGPU::increaseSeedsKernel<<<dimGrid, dimBlock>>>(d_solution, d_seedBuffer_,dim, parameters_.foreThreshold);
		
		CRWCRGPU::columnSweepKernel<< <dimGrid, dimBlock >> > (d_wy, d_grad,
			d_seedBuffer_, d_matSub, d_matCen, d_matUp, d_rVec, dim, parameters_.gamma1D, parameters_.lambda1D);
		CRWCRGPU::TDMAColumn << <dimG, dimB >> > (d_matSub, d_matCen, d_matUp, d_rVec, d_solution, dim);
		CRWCRGPU::increaseSeedsKernel<<<dimGrid, dimBlock>>>(d_solution, d_seedBuffer_,dim, parameters_.foreThreshold);
	}

	for (int iter = 0; iter < parameters_.maxIterations2D; iter++)
	{
		CRWCRGPU::PRRowKernel << <dimGrid, dimBlock >> > (d_wx, d_wy, d_grad,
			d_solution, d_seedBuffer_, d_matSub, d_matCen, d_matUp, d_rVec,dim,parameters_.gamma2D,
			parameters_.lambda2D,parameters_.dt);
		CRWCRGPU::TDMARow << <dimG, dimB >> > (d_matSub, d_matCen, d_matUp, d_rVec, d_solution, dim);

		CRWCRGPU::PRColumnKernel << <dimGrid, dimBlock >> > (d_wx, d_wy, d_grad,
			d_solution, d_seedBuffer_, d_matSub, d_matCen, d_matUp, d_rVec
			, dim, parameters_.gamma2D,	parameters_.lambda2D, parameters_.dt);
		CRWCRGPU::TDMAColumn << <dimG, dimB >> > (d_matSub, d_matCen, d_matUp, d_rVec, d_solution, dim);
	}

	cudaMemcpy(solution_, d_solution, numPixels_ * sizeof(float), cudaMemcpyDeviceToHost);

	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time,start,stop);
	time_ = time;
	
}

float * CRWCRSolver::generateProbabilityImage() const
{
	return solution_;
}

int CRWCRSolver::getUseTime()const
{
	return time_;
}

void CRWCRSolver::normalize(float * data, size_t length)
{
	float l = 1, u = 0;
	for (size_t i = 0; i < length; i++)
	{
		l = std::min(l, data[i]);
		u = std::max(u, data[i]);
	}

	if (fabs(l - u) < 1e-6)
	{
		return;
	}

	for (size_t i = 0; i < length; i++)
		data[i] = (data[i] - l) / (u - l);
}

void CRWCRSolver::calculateWeight()
{
	const float beta = 80, epsilon = 1e-5;
	float* weight = new float[numPixels_];
	memset(weight,0,numPixels_*sizeof(float));
	
	for (int y = 0; y < height_; y++)
	{
		for (int x = 0; x < width_ - 1; x++)
		{
			int index = y * width_ + x;
			weight[index] = (fabs((image_[index] - image_[index + 1])));
		}
	}

	normalize(weight, numPixels_);

	for (size_t i = 0; i < numPixels_; i++)
	{
		weight[i] = exp(-beta * weight[i]) + epsilon;
	}

	cudaMemcpy(d_wx, weight, numPixels_*sizeof(float),cudaMemcpyHostToDevice);

	memset(weight, 0, numPixels_ * sizeof(float));
	for (int x = 0; x < width_; x++)
	{
		for (int y = 0; y < height_ - 1; y++)
		{
			// a better store order
			weight[x * height_ + y] = fabs((image_[y * width_ + x] - image_[(y + 1) * width_ + x]));
		}
	}

	normalize(weight, numPixels_);
	for (size_t i = 0; i < numPixels_; i++)
	{
		weight[i] = exp(-beta * weight[i]) + epsilon;
	}

	cudaMemcpy(d_wy, weight, numPixels_*sizeof(float),cudaMemcpyHostToDevice);

	delete[] weight;
	weight=nullptr;
}

void CRWCRSolver::calculateGradient()
{
	float *grad = new float[numPixels_];
	memset(grad, 0, numPixels_ * sizeof(float));

	for (size_t x = 1; x < width_ - 1; x++)
	{
		for (size_t y = 1; y < height_ - 1; y++)
		{
			float gx = image_[x - 1 + y * width_] - image_[x + 1 + y * width_];
			float gy = image_[x + (y - 1) * width_] - image_[x + (y + 1) * width_];

			grad[x + y * width_] = fabs(gx) + fabs(gy);

		}
	}
	normalize(grad, numPixels_);

	cudaMemcpy(d_grad, grad, numPixels_*sizeof(float),cudaMemcpyHostToDevice);

	delete[] grad;
	grad=nullptr;
}
