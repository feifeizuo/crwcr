#include "initialization1d.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <chrono>
#include <algorithm>

Initialization1D::Initialization1D():
	image_(nullptr),
	seedBuffer_(nullptr),
	rowWeight_(nullptr),
	colWeight_(nullptr),
	rowGrad_(nullptr),
	colGrad_(nullptr)
{
	width_ = 0;
	height_ = 0;
}

Initialization1D::~Initialization1D()
{
	releaseMemory();
}

void Initialization1D::setImage(const float* data, int width, int height)
{
	width_ = width;
	height_ = height;

	releaseMemory();

	const size_t numPixels = width * height;
	image_ = new float[numPixels];
	memcpy(image_, data, numPixels * sizeof(float));

	seedBuffer_ = new unsigned char[numPixels];

	rowWeight_ = new float[numPixels];
	memset(rowWeight_, 0, numPixels * sizeof(float));
	colWeight_ = new float[numPixels];
	memset(colWeight_, 0, numPixels * sizeof(float));

	rowGrad_ = new float[numPixels];
	memset(rowGrad_, 0, numPixels * sizeof(float));
	colGrad_ = new float[numPixels];
	memset(colGrad_, 0, numPixels * sizeof(float));
}

void Initialization1D::setSeedBuffer(unsigned char* buffer)
{
	memcpy(seedBuffer_, buffer, width_ * height_);

	calculateWeight();
	calculateGradient();
}

void Initialization1D::process()
{
	auto start = std::chrono::system_clock::now();

	const int maxIteratorNum = 10;
	const float foregroundThreshold = 0.6f;
	const float lambda = 100, gamma=0.2;

	int maxSize = width_ >= height_ ? width_ : height_;

	float* a = new float[maxSize];
	float* b = new float[maxSize];
	float* c = new float[maxSize];
	float* d = new float[maxSize];
	float* solution = new float[maxSize];

	const float beta = 90;

	for (size_t i = 0; i < maxIteratorNum; i++)
	{
		//scan each row
		for (size_t y = 1; y < height_ - 1; y++)
		{
			for (int x = 1; x < width_ - 1; x++)
			{
				if (isFroegroundSeed(x, y))
				{
					// build equation: (L + lambda*R'*R + gamma*|Grad|)v = lambda*R'*b
					for (int r = 1; r < width_ - 1; r++)
					{
						a[r] = -rowWeight_[r - 1 + y * width_];
						c[r] = -rowWeight_[r + y * width_];
						b[r] = -(a[r] + c[r]) + (isSeed(r, y) ? lambda : 0) + gamma * rowGrad_[r + y * width_];
						d[r] = isFroegroundSeed(r, y) ? lambda : 0.f;
					}

					a[0] = -1;
					c[0] = a[1];
					b[0] = -(a[0]+c[0]);

					a[width_ - 1] = -rowWeight_[width_ - 2 + y * width_];
					c[width_ - 1] = -1;
					b[width_ - 1] = -(a[width_-1] + c[width_ - 1]);
					

					d[0] = d[width_ - 1] = 0;
					// solve equation
					TDMA(a, b, c, d, solution, width_);

					for (int j = 0; j < width_; j++)
					{
						if (solution[j] >= foregroundThreshold)
						{
							setForeground(j, y);
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
				if (isFroegroundSeed(x, y))
				{
					// build equation: (L + lambda*R'*R + gamma*|Grad|)v = lambda*R'*b
					for (size_t col = 1; col < height_ - 1; col++)
					{
						a[col] = -colWeight_[col - 1 + x * height_];
						c[col] = -colWeight_[col + x * height_];
						b[col] = -(a[col] + c[col]) + (isSeed(x, col) ? lambda : 0) + gamma * colGrad_[col + x * height_];
						d[col] = isFroegroundSeed(x, col) ? lambda: 0.f;
					}

					a[0] = -1;
					c[0] = a[1];
					b[0] = -(a[0]+c[0]);

					a[height_ - 1] = -colWeight_[height_ - 2 + x * height_];
					c[height_ - 1] = -1;
					b[height_ - 1] = -(a[height_ - 1]+c[height_ - 1]);

					d[0] = d[height_ - 1] = 0;

					// solve equation
					TDMA(a, b, c, d, solution, height_);

					for (int j = 0; j < height_; j++)
					{
						if (solution[j] >= foregroundThreshold)
						{
							setForeground(x, j);
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

	auto stop = std::chrono::system_clock::now();
	auto accurateTime = start - stop;
	std::chrono::duration<double> diff = stop - start;
	std::cout << "1D use time " << diff.count()*1000 <<" ms." <<std::endl;
}

float * Initialization1D::getWx()
{
	return rowWeight_;
}

float * Initialization1D::getWy()
{
	return colWeight_;
}

unsigned char* Initialization1D::getInitialization()
{
	return seedBuffer_;
}

void Initialization1D::TDMA(float* a, float* b, float* c, float* d, float* x, int numRow)
{
	double m;
	for (int i = 1; i < numRow; i++)
	{
		m = a[i] / b[i - 1];
		b[i] = b[i] - m * c[i - 1];
		d[i] = d[i] - m * d[i - 1];
	}

	x[numRow - 1] = d[numRow - 1] / b[numRow - 1];

	for (int i = numRow - 2; i >= 0; i--)
	{
		x[i] = (d[i] - c[i] * x[i + 1]) / b[i];
	}
}

void Initialization1D::calculateWeight()
{
	float beta = 90;

	for (int y = 0; y < height_; y++)
	{
		for (int x = 0; x < width_ - 1; x++)
		{
			int index = y * width_ + x;
			rowWeight_[index] = (fabs((image_[index] - image_[index + 1])));
		}

		normalize(rowWeight_ + y*width_, width_);

		for (size_t i = 0; i < width_ - 1; i++)
		{
			rowWeight_[i + y * width_] = exp(-beta * rowWeight_[i + y * width_]);
		}
	}

	for (int x = 0; x < width_; x++)
	{
		for (int y = 0; y < height_ - 1; y++)
		{
			// note: store column weight in column order
			colWeight_[x * height_ + y] = fabs((image_[y * width_ + x] - image_[(y + 1) * width_ + x]));
		}
		normalize(colWeight_ + x * height_, height_);
		for (size_t y = 0; y < height_ - 1; y++)
		{
			colWeight_[x * height_ + y] = exp(-beta*colWeight_[x * height_ + y]);
		}
	}
}

void Initialization1D::calculateGradient()
{
	//central different for gradient
	for (int y = 1; y < height_ - 1; y++)
	{
		for (int x = 1; x < width_ - 1; x++)
		{
			rowGrad_[x + y * width_] = fabs(image_[x + 1 + y * width_] - image_[x - 1 + y * width_]);
		}
	}

	for (int x = 1; x < width_ - 1; x++)
	{
		for (int y = 1; y < height_ - 1; y++)
		{
			// note: store column gradient in row order
			colGrad_[x * height_ + y] = fabs(image_[x + (y + 1) * width_] - image_[x + (y - 1) * width_]);
		}
	}
}

void Initialization1D::releaseMemory()
{
	delete[] image_; image_ = nullptr;
	delete[] seedBuffer_; seedBuffer_ = nullptr;
	delete[] rowWeight_; rowWeight_ = nullptr;
	delete[] colWeight_; colWeight_ = nullptr;
	delete[] rowGrad_; rowGrad_ = nullptr;
	delete[] colGrad_; colGrad_ = nullptr;
}

void Initialization1D::normalize(float* data, size_t numPixel)
{
	float l = 1, u = 0;
	for (size_t i=0;i<numPixel;i++)
	{
		l = std::min(l, data[i]);
		u = std::max(u, data[i]);
	}

	for (size_t i = 0; i < numPixel; i++)
		data[i] = (data[i] - l) / (u - l);
}

bool Initialization1D::isSeed(int x, int y) const
{
	return seedBuffer_[x + y * width_] > 0;
}

inline bool Initialization1D::isFroegroundSeed(int x, int y)
{
	return seedBuffer_[x + y * width_] == static_cast<unsigned char>(1);
}

inline void Initialization1D::setForeground(int x, int y)
{
	seedBuffer_[x + y * width_] = 1;
}
