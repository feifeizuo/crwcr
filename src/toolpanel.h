#ifndef TOOLPANEL_H
#define TOOLPANEL_H

#include<QWidget>
#include "singleton.h"
#include "ui_toolForm.h"

class ToolPanel : public QWidget
{
Q_OBJECT

public:
	ToolPanel(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);


signals:

	// 0: None, 1:F, 2:B
	void seedModeChanged(int);
	void computerClicked();

	void loadImage();

	void clearSeeds();
	void importSeeds();
	void exportSeeds();

	void thresholdChanged(double);
	void preProcessCheckChanged(int);

public slots:
	void computeTimeChanged(int time) const;

private:

	void initParameters();
	void createConnect();

	Parameters& parameters_;
	Ui_ToolForm* ui_;
};

#endif // TOOLPANEL_H
