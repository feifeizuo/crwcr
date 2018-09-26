#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ImageCanvas;
class ToolPanel;
class CRWCRAlgorithm;


/**
 * \brief main window of this application.
 */
class MainWindow : public QMainWindow
{
Q_OBJECT


public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

public slots:

	void load();

private:

	void createDockWidget();
	void createConnect();

	ImageCanvas* imageCanvas_;
	ToolPanel* toolWidget_;

	CRWCRAlgorithm* algorithm_;

	QString lastDir_;
};

#endif // MAINWINDOW_H
