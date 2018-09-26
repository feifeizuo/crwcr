#include "mainwindow.h"
#include"imagecanvas.h"
#include"toolpanel.h"

#include "crwcralgorithm.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
{
	imageCanvas_ = new ImageCanvas(this);

	algorithm_ = new CRWCRAlgorithm();

	lastDir_ = "";

	createDockWidget();
	createConnect();

	setCentralWidget(imageCanvas_);

	qRegisterMetaType<PointListGeometry>("PointListGeometry");
}

MainWindow::~MainWindow()
{
	delete algorithm_;
	algorithm_ = nullptr;
}

void MainWindow::createDockWidget()
{
	QDockWidget* dock = new QDockWidget(QStringLiteral("CRWCR"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	toolWidget_ = new ToolPanel();

	dock->setWidget(toolWidget_);
	addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::createConnect()
{
	connect(toolWidget_, &ToolPanel::loadImage, this, &MainWindow::load);
	connect(toolWidget_, &ToolPanel::seedModeChanged, imageCanvas_, &ImageCanvas::setSeedMode);
	connect(toolWidget_, &ToolPanel::computerClicked, algorithm_, &CRWCRAlgorithm::process);
	connect(toolWidget_, &ToolPanel::clearSeeds, imageCanvas_, &ImageCanvas::clearSeeds);
	connect(toolWidget_, &ToolPanel::thresholdChanged, imageCanvas_, &ImageCanvas::setThreshold);
	connect(toolWidget_, &ToolPanel::preProcessCheckChanged, algorithm_, &CRWCRAlgorithm::setPreProcessState);
	connect(algorithm_, &CRWCRAlgorithm::segmentationDone, imageCanvas_, &ImageCanvas::setProbability);
	connect(algorithm_, &CRWCRAlgorithm::segmentationTime, toolWidget_, &ToolPanel::computeTimeChanged);
	connect(imageCanvas_, &ImageCanvas::seedChanged, algorithm_, &CRWCRAlgorithm::setSeeds);
}

void MainWindow::load()
{
	//QString selfilter = tr("Image (*.bmp *.jpg *.jpeg)");
	QString filename = QFileDialog::getOpenFileName(
		this,
		"Open Image",
		lastDir_,
		tr("Image (*.bmp *.jpg *.png);;Binary Data (*.raw);;All files (*.*)")
		//&selfilter
	);

	if (filename.size() == 0)
	{
		return;
	}

	if (filename.isEmpty())
		return;

	lastDir_ = filename;

	QImage image(filename);
	image = image.convertToFormat(QImage::Format_RGBA8888);
	imageCanvas_->setImage(image);
	algorithm_->setImage(image);
}
