#include "pointlistgeometry.h"


PointListGeometry::PointListGeometry()
{
}


PointListGeometry::~PointListGeometry()
{
}

void PointListGeometry::addSegment(const PointSegment& seg)
{
	pointList_.push_back(seg);
}

size_t PointListGeometry::getSegmentNums() const
{
	return pointList_.size();
}

PointSegment PointListGeometry::getSegment(int index) const
{
	return pointList_.at(index);
}

void PointListGeometry::clear()
{
	pointList_.clear();
}

void PointListGeometry::render()
{
	for each (auto segment in pointList_)
	{
		glBegin(GL_LINE_STRIP);
		for each (auto p in segment)
		{
			glVertex2f(p.x(), p.y());
		}
		glEnd();
	}
}
