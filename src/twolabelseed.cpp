#include "twolabelseed.h"
#include <fstream>
#include<cassert>
#include<QVector2D>


TwoLabelSeed::TwoLabelSeed():
	seedBuffer_(nullptr)
{
}

TwoLabelSeed::~TwoLabelSeed()
{
	delete[] seedBuffer_;
	seedBuffer_ = nullptr;
}

void TwoLabelSeed::setSeeds(const PointListGeometry& foregroundSeed, const PointListGeometry& backgroundSeed)
{
	foregroundSeed_ = foregroundSeed;
	backgroundSeed_ = backgroundSeed;
}

bool TwoLabelSeed::isSeedPoint(size_t index) const
{
	assert(seedBuffer_ != nullptr);

	return seedBuffer_[index] > 0;
}

bool TwoLabelSeed::isForegroundSeed(size_t index) const
{
	assert(seedBuffer_ != nullptr);

	return seedBuffer_[index] == 1;
}

void TwoLabelSeed::setToForegroundSeed(size_t index)
{
	assert(seedBuffer_ != nullptr);

	seedBuffer_[index] = 1;
}

void TwoLabelSeed::initialize(QSize dim)
{
	delete[] seedBuffer_;
	seedBuffer_ = new unsigned char[dim.width() * dim.height()];
	memset(seedBuffer_, 0, dim.width() * dim.height());

	for (size_t i = 0; i < foregroundSeed_.getSegmentNums(); i++)
	{
		auto pointList = foregroundSeed_.getSegment(i);

		if (pointList.empty())
			continue;

		for (size_t j = 0; j < pointList.size() - 1; j++)
		{
			QVector2D left = QVector2D(pointList.at(j));
			QVector2D right = QVector2D(pointList.at(j + 1));
			QVector2D dir = right - left;
			QVector2D d = dir.normalized();

			for (float t = 0; t < dir.length(); t += 1.f)
			{
				auto p = left + d * t;
				size_t index = int(p.x()) + dim.width() * (int(p.y()));
				seedBuffer_[index] = 1;
			}
		}
	}

	for (size_t i = 0; i < backgroundSeed_.getSegmentNums(); i++)
	{
		auto pointList = backgroundSeed_.getSegment(i);

		if (pointList.empty())
			continue;

		for (size_t j = 0; j < pointList.size() - 1; j++)
		{
			QVector2D left = QVector2D(pointList.at(j));
			QVector2D right = QVector2D(pointList.at(j + 1));
			QVector2D dir = right - left;
			QVector2D d = dir.normalized();

			for (float t = 0; t < dir.length(); t += 1.f)
			{
				auto p = left + d * t;
				size_t index = int(p.x()) + dim.width() * (int(p.y()));
				seedBuffer_[index] = 2;
			}
		}
	}
}

unsigned char* TwoLabelSeed::getSeedBuffer()
{
	return seedBuffer_;
}
