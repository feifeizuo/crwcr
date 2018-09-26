#ifndef SEEDGEOMETRY_H
#define SEEDGEOMETRY_H

#include <QOpenGLFunctions_3_3_Core>
#include <QPoint>
#include <vector>

typedef std::vector<QPoint> PointSegment;

class PointListGeometry : public QOpenGLFunctions_3_3_Core
{
public:
	PointListGeometry();
	~PointListGeometry();

	void addSegment(const PointSegment& seg);

	size_t getSegmentNums() const;

	PointSegment getSegment(int index) const;

	void clear();

	void render();

private:

	std::vector<PointSegment> pointList_;
};

#endif // SEEDGEOMETRY_H
