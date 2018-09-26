#include "crwcrsolver.h"
#include<cmath>
#include <iostream>
#include <chrono>
#include <algorithm>
#include<fstream>

CRWCRSolver::CRWCRSolver(const float* image, int width, int height) :
	parameters_(Singleton<Parameters>::GetInstance()),
	seeds_(nullptr),
	image_(image),
	width_(width),
	height_(height)
{
	numPixels_ = width_ * height_;
	solution_ = new float[numPixels_];
	wx_ = new float[numPixels_];
	wy_ = new float[numPixels_];
	grad_ = new float[numPixels_];

	calculateWeight();
	calculateGradient();
}

CRWCRSolver::~CRWCRSolver()
{
	delete[] solution_;
	delete[] wx_;
	delete[] wy_;
	delete[] grad_;
}

void CRWCRSolver::setSeed(TwoLabelSeed* seed)
{
	seeds_ = seed;
}

void CRWCRSolver::solve()
{
	auto start = std::chrono::system_clock::now();

	initialization();
	prcorrection();

	auto stop = std::chrono::system_clock::now();
	auto accurateTime = start - stop;
	std::chrono::duration<double> diff = stop - start;
	time_ = diff.count() * 1000;
}

float* CRWCRSolver::generateProbabilityImage() const
{
	return solution_;
}

float CRWCRSolver::getUseTime() const
{
	return time_;
}

void CRWCRSolver::initialization()
{
	std::cout << parameters_.maxIterations1D << std::endl;
	int maxSize = width_ >= height_ ? width_ : height_;

	double* a = new double[maxSize];
	double* b = new double[maxSize];
	double* c = new double[maxSize];
	double* d = new double[maxSize];
	double* solution = new double[maxSize];

	for (size_t i = 0; i < parameters_.maxIterations1D; i++)
	{
		//scan each row
		for (size_t y = 1; y < height_ - 1; y++)
		{
			for (int x = 1; x < width_ - 1; x++)
			{
				if (seeds_->isForegroundSeed(x + y * width_))
				{
					for (int r = 1; r < width_ - 1; r++)
					{
						a[r] = -wx_[r - 1 + y * width_];
						c[r] = -wx_[r + y * width_];
						b[r] = -(a[r] + c[r]) + (seeds_->isSeedPoint(r + y * width_) ? parameters_.lambda1D : 0) +
							parameters_.gamma1D * grad_[r + y * width_];
						d[r] = seeds_->isForegroundSeed(r + y * width_) ? parameters_.lambda1D : 0.f;
					}

					a[0] = -1;
					c[0] = a[1];
					b[0] = -(a[0] + c[0]);

					a[width_ - 1] = -wx_[width_ - 2 + y * width_];
					c[width_ - 1] = -1;
					b[width_ - 1] = -(a[width_ - 1] + c[width_ - 1]);


					d[0] = d[width_ - 1] = 0;
					// solve equation
					TDMA(a, b, c, d, solution, width_);

					for (int j = 0; j < width_; j++)
					{
						if (solution[j] >= parameters_.foreThreshold)
						{
							seeds_->setToForegroundSeed(j + y * width_);
						}
					}

					break;
				}
			}
		}

		// scan each column
		for (size_t x = 1; x < width_ - 1; x++)
		{
			for (size_t y = 1; y < height_ - 1; y++)
			{
				if (seeds_->isForegroundSeed(x + y * width_))
				{
					for (size_t col = 1; col < height_ - 1; col++)
					{
						a[col] = -wy_[col - 1 + x * height_];
						c[col] = -wy_[col + x * height_];
						b[col] = -(a[col] + c[col]) + (seeds_->isSeedPoint(x + col * width_) ? parameters_.lambda1D : 0)
							+ parameters_.gamma1D * grad_[x + col * width_];
						d[col] = seeds_->isForegroundSeed(x + col * width_) ? parameters_.lambda1D : 0.f;
					}

					a[0] = -1;
					c[0] = a[1];
					b[0] = -(a[0] + c[0]);

					a[height_ - 1] = -wy_[height_ - 2 + y * width_];
					c[height_ - 1] = -1;
					b[height_ - 1] = -(a[height_ - 1] + c[height_ - 1]);

					d[0] = d[height_ - 1] = 0;

					// solve equation
					TDMA(a, b, c, d, solution, height_);

					for (int j = 0; j < height_; j++)
					{
						if (solution[j] >= parameters_.foreThreshold)
						{
							seeds_->setToForegroundSeed(x + j * width_);
						}
					}
					break;
				}
			}
		}
	}

	delete[]a;
	delete[]b;
	delete[]c;
	delete[]d;
	delete[]solution;
}

void CRWCRSolver::prcorrection()
{
	int maxSize = width_ >= height_ ? width_ : height_;

	for (size_t i = 0; i < numPixels_; i++)
	{
		solution_[i] = seeds_->isForegroundSeed(i);
	}

	double* a = new double[maxSize];
	double* b = new double[maxSize];
	double* c = new double[maxSize];
	double* d = new double[maxSize];
	double* solution = new double[maxSize];

	float* u_n = new float[numPixels_];
	memcpy(u_n, solution_, numPixels_ * sizeof(float));

	for (size_t i = 0; i < parameters_.maxIterations2D; i++)
	{
		// row sweeping
		for (size_t y = 0; y < height_; y++)
		{
			for (size_t x = 0; x < width_; x++)
			{
				const int index = x + y * width_;

				if (x == 0)
				{
					a[x] = -1;
					c[x] = -wx_[y * width_];
					b[x] = -(c[x] + a[x]);
				}
				else if (x == width_ - 1)
				{
					a[x] = -wx_[index - 1];
					c[x] = -1;
					b[x] = -(a[x] + c[x]);
				}
				else
				{
					a[x] = -wx_[index - 1];
					c[x] = -wx_[index];
					b[x] = -(a[x] + c[x]) + parameters_.gamma2D * grad_[index] +
						(seeds_->isSeedPoint(x + y * width_) ? parameters_.lambda2D : 0) + parameters_.dt;
				}

				float a_, b_, c_;

				if (y == 0)
				{
					a_ = -1;
					c_ = -wy_[x * height_];
					b_ = -(c_ + a_);
					d[x] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[index] * b_ +
						u_n[x + width_] * c_) + u_n[index] * parameters_.dt;
				}
				else if (y == height_ - 1)
				{
					a_ = -wy_[height_ - 2 + x * height_];
					c_ = -1;
					b_ = -(a_ + c_);
					d[x] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[x + (y - 1) *
						width_] * a_ + u_n[index] * b_) + u_n[index] * parameters_.dt;
				}
				else
				{
					a_ = -wy_[y - 1 + x * height_];
					c_ = -wy_[y + x * height_];
					b_ = -(a_ + c_);
					d[x] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[x + (y - 1) *
						width_] * a_ + u_n[index] * b_ + u_n[x + (y + 1) * width_] * c_) + u_n[index] * parameters_.dt;
				}
			}

			// TDMA
			TDMA(a, b, c, d, solution, width_);

			for (size_t x = 0; x < width_; x++)
			{
				solution_[x + y * width_] = solution[x];
			}
		}

		memcpy(u_n, solution_, numPixels_ * sizeof(float));

		// column sweeping
		for (size_t x = 0; x < width_; x++)
		{
			for (size_t y = 0; y < height_; y++)
			{
				int index = x + y * width_;

				if (y == 0)
				{
					a[y] = -1;
					c[y] = -wy_[x * height_];
					b[y] = -(c[y] + a[y]);
				}
				else if (y == height_ - 1)
				{
					a[y] = -wy_[height_ - 2 + x * height_];
					c[y] = -1;
					b[y] = -(a[y] + c[y]);
				}
				else
				{
					a[y] = -wy_[y - 1 + x * height_];
					c[y] = -wy_[y + x * height_];
					b[y] = -(a[y] + c[y]) + parameters_.gamma2D * grad_[index] + (seeds_->isSeedPoint(x + y * width_)
						                                                              ? parameters_.lambda2D
						                                                              : 0) + parameters_.dt;
				}

				float a_, b_, c_;

				if (x == 0)
				{
					a_ = -1;
					c_ = -wx_[index];
					b_ = -(c_ + a_);
					d[y] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[y * width_] * b_
						+ u_n[1 + y * width_] * c_) + u_n[index] * parameters_.dt;
				}
				else if (x == width_ - 1)
				{
					a_ = -wx_[index - 1];
					c_ = -1;
					b_ = -(a_ + c_);
					d[y] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[x - 1 + y *
						width_] * a_ + u_n[x + y * width_] * b_) + u_n[index] * parameters_.dt;
				}
				else
				{
					a_ = -wx_[index - 1];
					c_ = -wx_[index];
					b_ = -(a_ + c_);
					d[y] = (seeds_->isForegroundSeed(x + y * width_) ? parameters_.lambda2D : 0) - (u_n[x - 1 + y *
							width_] * a_ + u_n[x + y * width_] * b_ + u_n[x + 1 + y * width_] * c_) + u_n[index] *
						parameters_.dt;
				}
			}
			// TDMA
			TDMA(a, b, c, d, solution, height_);

			for (int y = 0; y < height_; y++)
			{
				solution_[x + y * width_] = solution[y];
			}
		}

		memcpy(u_n, solution_, numPixels_ * sizeof(float));
	}

	delete[] a;
	delete[] c;
	delete[] b;
	delete[] d;
	delete[] solution;
	delete[] u_n;
}

void CRWCRSolver::TDMA(double* a, double* b, double* c, double* d, double* x, int numRow)
{
	c[0] = c[0] / b[0];
	d[0] = d[0] / b[0];

	// forward sweep
	for (int i = 1; i < numRow; ++i)
	{
		double id = 1.0 / (b[i] - c[i - 1] * a[i]);
		c[i] = c[i] * id;
		d[i] = (d[i] - a[i] * d[i - 1]) * id;
	}

	// backward sweep
	x[numRow - 1] = d[numRow - 1];
	for (int i = numRow - 2; i > -1; i--)
	{
		x[i] = d[i] - c[i] * x[i + 1];
	}
}

void CRWCRSolver::normalize(float* data, size_t length)
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

	memset(wx_, 0, numPixels_ * sizeof(float));
	for (int y = 0; y < height_; y++)
	{
		for (int x = 0; x < width_ - 1; x++)
		{
			int index = y * width_ + x;
			wx_[index] = (fabs((image_[index] - image_[index + 1])));
		}
	}

	normalize(wx_, numPixels_);

	for (size_t i = 0; i < numPixels_; i++)
	{
		wx_[i] = exp(-beta * wx_[i]) + epsilon;
	}

	memset(wy_, 0, numPixels_ * sizeof(float));
	for (int x = 0; x < width_; x++)
	{
		for (int y = 0; y < height_ - 1; y++)
		{
			wy_[x * height_ + y] = fabs((image_[y * width_ + x] - image_[(y + 1) * width_ + x]));
		}
	}

	normalize(wy_, numPixels_);
	for (size_t i = 0; i < numPixels_; i++)
	{
		wy_[i] = exp(-beta * wy_[i]) + epsilon;
	}
}

void CRWCRSolver::calculateGradient()
{
	memset(grad_, 0, numPixels_ * sizeof(float));

	for (size_t x = 1; x < width_ - 1; x++)
	{
		for (size_t y = 1; y < height_ - 1; y++)
		{
			float gx = image_[x - 1 + y * width_] - image_[x + 1 + y * width_];
			float gy = image_[x + (y - 1) * width_] - image_[x + (y + 1) * width_];

			grad_[x + y * width_] = fabs(gx) + fabs(gy);
		}
	}
	normalize(grad_, numPixels_);
}
