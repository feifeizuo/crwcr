#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include"pointlistgeometry.h"

/**
 * \brief seed mode enum.
 */
enum SeedMode
{
	None,
	Foreground,
	Background
};


/**
 * \brief use OpenGL to rendering image and segmentation result.
 */
class ImageCanvas : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
Q_OBJECT

public:
	ImageCanvas(QWidget* parent = nullptr);
	~ImageCanvas();

	void setImage(const QImage& img);

public slots:
	void setProbability(float* p);
	void setThreshold(double t);
	void clearSeeds();
	void setSeedMode(int mode);

signals:
	void seedChanged(const PointListGeometry& foregroundSeed, const PointListGeometry& background);

protected:

	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int width, int height) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	void wheelEvent(QWheelEvent* event) override;

private:

	void initializeShader();

	//transform mouse position to pixel coordinate
	QPoint Window2Pixel(QPoint p);

	std::vector<QPoint> pointList_;

	QOpenGLVertexArrayObject vao_;
	QOpenGLBuffer vertexBuffer_, texcoordBuffer_;
	GLuint imageTex, probabilityTex;
	QOpenGLShaderProgram program;

	PointListGeometry *foregroundSeed_, *backgroundSeed_;

	bool hasImage;
	float scale;
	QSize imageDim;
	QVector2D viewportScale;

	QSize canvasSize_;
	QMatrix4x4 mvpMat_;

	SeedMode seedMode_;
};

#endif // IMAGECANVAS_H
