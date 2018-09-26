#include "imagecanvas.h"
#include <QtWidgets>
#include <iostream>

ImageCanvas::ImageCanvas(QWidget* parent /*= 0*/): QOpenGLWidget(parent)
{
	hasImage = false;
	scale = 1.f;
	imageDim = QSize(0, 0);
	seedMode_ = None;

	foregroundSeed_ = new PointListGeometry();
	backgroundSeed_ = new PointListGeometry();

	setMinimumSize(800, 600);
}

ImageCanvas::~ImageCanvas()
{
	makeCurrent();
	vertexBuffer_.destroy();
	texcoordBuffer_.destroy();
	vao_.destroy();

	delete foregroundSeed_;
	delete backgroundSeed_;

	doneCurrent();
}

void ImageCanvas::setImage(const QImage& img)
{
	imageDim = img.size();
	makeCurrent();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, imageTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageDim.width(), imageDim.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
	             img.bits());

	clearSeeds();

	hasImage = true;
	update();
}

void ImageCanvas::setProbability(float* p)
{
	makeCurrent();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, probabilityTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, imageDim.width(), imageDim.height(), 0, GL_LUMINANCE, GL_FLOAT, p);

	if (program.isLinked())
	{
		program.bind();
		program.setUniformValue(program.uniformLocation("hasSegment"), 1);
		program.setUniformValue(program.uniformLocation("dh"), 1.0f / imageDim.width());
		program.setUniformValue(program.uniformLocation("dv"), 1.0f / imageDim.height());
		program.release();
	}

	update();
}


void ImageCanvas::setThreshold(double t)
{
	if (program.isLinked())
	{
		program.bind();
		program.setUniformValue(program.uniformLocation("t"), (float)t);
		program.release();
		update();
	}
}

void ImageCanvas::clearSeeds()
{
	foregroundSeed_->clear();
	backgroundSeed_->clear();

	if (program.isLinked())
	{
		program.bind();
		program.setUniformValue(program.uniformLocation("hasSegment"), 0);
		program.release();
	}

	update();
}

void ImageCanvas::setSeedMode(int mode)
{
	seedMode_ = static_cast<SeedMode>(mode);
}

void ImageCanvas::initializeGL()
{
	initializeOpenGLFunctions();

	initializeShader();

	glGenTextures(1, &imageTex);
	glBindTexture(GL_TEXTURE_2D, imageTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &probabilityTex);
	glBindTexture(GL_TEXTURE_2D, probabilityTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void ImageCanvas::paintGL()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (hasImage && program.isLinked())
	{
		mvpMat_.setToIdentity();
		mvpMat_.scale(scale, scale, 1);
		mvpMat_.ortho(-2.0f * viewportScale.y(), 2.0f * viewportScale.y(), -2.0f * viewportScale.x(),
		              2.0f * viewportScale.x(), -1, 1);
		mvpMat_.scale(float(imageDim.width()) / std::max(imageDim.width(), imageDim.height()),
		              float(imageDim.height()) / std::max(imageDim.width(), imageDim.height()), 1);

		program.bind();
		program.setUniformValue(program.uniformLocation("mvpMat"), mvpMat_);
		vao_.bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, probabilityTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		program.release();
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2.0f * viewportScale.y(), 2.0f * viewportScale.y(), -2.0f * viewportScale.x(), 2.0f * viewportScale.x(),
	        -4,
	        4);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	glScalef(scale * float(imageDim.width()) / std::max(imageDim.width(), imageDim.height()),
	         scale * float(imageDim.height()) / std::max(imageDim.width(), imageDim.height()), 1);

	glTranslatef(-1, 1, 0);
	glScalef(2.f / (imageDim.width() - 1), -2.f / (imageDim.height() - 1), 1);

	if (seedMode_ == Foreground)
	{
		glColor3f(1, 0, 0);
	}
	else if (seedMode_ == Background)
	{
		glColor3f(0, 0, 1);
	}

	glBegin(GL_LINE_STRIP);

	for each (auto point in pointList_)
	{
		glVertex2f(point.x(), point.y());
	}
	glEnd();

	glLineWidth(3);

	glColor3f(1, 0, 0);
	foregroundSeed_->render();

	glColor3f(0, 0, 1);
	backgroundSeed_->render();

	glPopMatrix();
}

void ImageCanvas::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);
	canvasSize_ = QSize(width, height);
	viewportScale.setX(std::min(width, height) / (float)width);
	viewportScale.setY(std::min(width, height) / (float)height);
}

void ImageCanvas::mousePressEvent(QMouseEvent* event)
{
	if (seedMode_ != None)
	{
		pointList_.push_back(Window2Pixel(QPoint(event->x(), event->y())));
		update();
	}
}

void ImageCanvas::mouseMoveEvent(QMouseEvent* event)
{
	if (seedMode_ != None)
	{
		pointList_.push_back(Window2Pixel(QPoint(event->x(), event->y())));
		update();
	}
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent* event)
{
	switch (seedMode_)
	{
	case None:
		break;
	case Foreground:
		pointList_.push_back(Window2Pixel(QPoint(event->x(), event->y())));
		foregroundSeed_->addSegment(pointList_);
		pointList_.clear();
		emit seedChanged(*foregroundSeed_, *backgroundSeed_);
		break;
	case Background:
		pointList_.push_back(Window2Pixel(QPoint(event->x(), event->y())));
		backgroundSeed_->addSegment(pointList_);
		pointList_.clear();
		emit seedChanged(*foregroundSeed_, *backgroundSeed_);
		break;
	default:
		break;
	}

	update();
}

void ImageCanvas::wheelEvent(QWheelEvent* event)
{
	if (event->delta() > 0)
	{
		scale = scale + 0.1;
	}
	else
	{
		scale = scale - 0.1;
	}

	update();
}

void ImageCanvas::initializeShader()
{
	char* vsrc =
		"#version 450\n"
		"layout (location = 0) in vec4 vertex_;\n"
		"layout (location = 1) in vec2 texCoord_;\n"
		"uniform mat4 mvpMat;\n"
		"out vec2 texCoord;\n"
		"void main()\n"
		"{\n"
		"	texCoord = texCoord_;\n"
		"	gl_Position = mvpMat*vertex_;\n"
		"}\n";
	char* fscr =
		"#version 450\n"
		"uniform sampler2D imageTex;\n"
		"uniform sampler2D probabilityTex;\n"
		"in vec2 texCoord;\n"
		"uniform float dh;\n"
		"uniform float dv;\n"
		"uniform float t;\n"
		"uniform int hasSegment;\n" // whether segmentation done
		"out vec4 color;\n"

		"bool isBound()\n"
		"{\n"
		"	float curCol = texture(probabilityTex, texCoord).x;\n"

		"																						 \n"
		"	if (curCol > t)																	 \n"
		"	{																					 \n"
		"		float leftCol = texture(probabilityTex, vec2(texCoord.x-dh,texCoord.y)).x;\n"
		"		float rightCol = texture(probabilityTex, vec2(texCoord.x+dh,texCoord.y)).x;\n"
		"		float upCol = texture(probabilityTex, vec2(texCoord.x,texCoord.y-dv)).x;	 \n"
		"		float downCol = texture(probabilityTex, vec2(texCoord.x,texCoord.y+dv)).x; \n"

		"		if (leftCol < t || rightCol < t || upCol < t || downCol < t)			 \n"
		"		{																				 \n"
		"			return true;																 \n"
		"		}																				 \n"
		"	}																					 \n"
		"	return false;																		 \n"
		"}																						 \n"

		"void main()														\n"
		"{																	\n"
		"	color = vec4(texture(imageTex, texCoord).xyz,1.0);		\n"
		"	if(hasSegment==1 && isBound())\n"
		"	{\n"
		"		color = vec4(1,0,0,1.0);\n"
		"	}\n"
		"}\n";

	QOpenGLShader* vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	vshader->compileSourceCode(vsrc);

	QOpenGLShader* fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	fshader->compileSourceCode(fscr);
	program.addShader(vshader);
	program.addShader(fshader);

	program.link();
	program.bind();

	GLfloat points[] =
	{
		-1, -1, 0.0, 1.0, //p1
		1, -1, 0.0, 1.0, //p2
		1, 1, 0.0, 1.0, //p3
		-1, -1, 0.0, 1.0, //p1
		1, 1, 0.0, 1.0, //p3
		-1, 1, 0.0, 1.0 //p4
	};

	GLfloat texPos[] =
	{
		0, 1,
		1, 1,
		1, 0,
		0, 1,
		1, 0,
		0, 0
	};

	// create vertex array object and bind
	vao_.create();
	vao_.bind();

	// create cube vertex buffer object and bind
	vertexBuffer_.create();
	vertexBuffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vertexBuffer_.bind();
	vertexBuffer_.allocate(points, 24 * sizeof(GLfloat));
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	// create texture coordinate buffer object and bind
	texcoordBuffer_.create();
	texcoordBuffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);
	texcoordBuffer_.bind();
	texcoordBuffer_.allocate(texPos, 12 * sizeof(GLfloat));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	program.enableAttributeArray(0);
	program.enableAttributeArray(1);

	program.setUniformValue("imageTex", 0);
	program.setUniformValue("probabilityTex", 1);
	program.setUniformValue(program.uniformLocation("t"), 0.5f);
	program.setUniformValue(program.uniformLocation("hasSegment"), 0);
	program.release();
}

QPoint ImageCanvas::Window2Pixel(QPoint p)
{
	QVector4D q(2.0 * p.x() / canvasSize_.width() - 1, 1 - 2.0 / canvasSize_.height() * p.y(), 0, 1);

	QVector4D q_ = mvpMat_.inverted() * q;
	return QPoint(0.5 * (q_.x() + 1) * (imageDim.width() - 1), 0.5 * (1 - q_.y()) * (imageDim.height() - 1));
}
