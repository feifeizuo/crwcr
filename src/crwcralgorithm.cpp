#include "crwcralgorithm.h"
#include <QImage>
#include <array>


/**
 * \brief Simple 2D median filter with 3X3 window
 * \tparam T 
 * \param img 
 * \param w 
 * \param h 
 */
template <typename T>
void medianFilter(T* img, int w, int h)
{
	size_t numPixels = w * h;
	int kernelSize = 3;

	T* imgOut = new T[numPixels];
	memcpy(imgOut, img, numPixels * sizeof(T));

	int halfSize = kernelSize / 2;

#pragma omp parallel for
	for (int x = halfSize; x < w - halfSize; x++)
	{
		for (int y = halfSize; y < h - halfSize; y++)
		{
			std::array<T, 9> elements;
			int subIdx = 0;
			for (int i = -halfSize; i <= halfSize; i++)
			{
				for (int j = -halfSize; j <= halfSize; j++)
				{
					size_t idx = x + i + (y + j) * w;
					elements[subIdx++] = img[idx];
				}
			}
			std::sort(elements.begin(), elements.end());
			imgOut[x + y * w] = elements[kernelSize * kernelSize / 2 + 1];
		}
	}
	memcpy(img, imgOut, numPixels * sizeof(T));
	delete[] imgOut;
	imgOut = nullptr;
}


CRWCRAlgorithm::CRWCRAlgorithm(QObject* parent)
	: QObject(parent), solver_(nullptr), isPreProcess_(false), image_(nullptr)
{
	twoLabelSeed_ = new TwoLabelSeed();
}

void CRWCRAlgorithm::process()
{
	// initialize seed buffer
	twoLabelSeed_->initialize(dim_);
	solver_->setSeed(twoLabelSeed_);
	solver_->solve();

	emit segmentationTime(solver_->getUseTime());
	emit segmentationDone(solver_->generateProbabilityImage());
}


CRWCRAlgorithm::~CRWCRAlgorithm()
{
	delete[] image_;
	image_ = nullptr;
	delete twoLabelSeed_;
	twoLabelSeed_ = nullptr;
	delete solver_;
	solver_ = nullptr;
}

/**
 * \brief  Set image data
 * \param data 
 */
void CRWCRAlgorithm::setImage(const QImage& data)
{
	dim_ = QSize(data.width(), data.height());

	delete[] image_;

	image_ = new float[dim_.width() * dim_.height()];

	for (int x = 0; x < dim_.width(); x++)
	{
		for (int y = 0; y < dim_.height(); y++)
		{
			QColor col = data.pixelColor(x, y);

			//rgb2gray: 0.2989 * R + 0.5870 * G + 0.1140 * B 
			// normalize to [0,1]
			image_[x + dim_.width() * y] = (0.2989 * col.red() + 0.5870 * col.green() + 0.1140 * col.blue()) / 255;
		}
	}

	if (isPreProcess_)
	{
		medianFilter(image_, dim_.width(), dim_.height());
	}

	delete solver_;
	solver_ = nullptr;
	solver_ = new CRWCRSolver(image_, dim_.width(), dim_.height());
}

void CRWCRAlgorithm::setSeeds(const PointListGeometry& foregroundseed, const PointListGeometry& backgroundseed)
{
	twoLabelSeed_->setSeeds(foregroundseed, backgroundseed);
}
