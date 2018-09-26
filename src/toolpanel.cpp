#include<QtWidgets>
#include "toolpanel.h"

ToolPanel::ToolPanel(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/):
	parameters_(Singleton<Parameters>::GetInstance()),
	ui_(new Ui_ToolForm)
{
	ui_->setupUi(this);

	initParameters();
	createConnect();
}

void ToolPanel::computeTimeChanged(int time) const
{
	std::string str = std::to_string(time) + " ms";
	ui_->computeTime->setText(QString(str.c_str()));
}

void ToolPanel::initParameters()
{
	ui_->iteration1D->setValue(parameters_.maxIterations1D);
	ui_->foreThreshold1D->setValue(parameters_.foreThreshold);
	ui_->gamma1D->setValue(parameters_.gamma1D);
	ui_->lambda1D->setValue(parameters_.lambda1D);

	ui_->iteration2D->setValue(parameters_.maxIterations2D);
	ui_->dt2D->setValue(parameters_.dt);
	ui_->gamma2D->setValue(parameters_.gamma2D);
	ui_->labmda2D->setValue(parameters_.lambda2D);

	ui_->computeTime->setText(QString(" "));
}

void ToolPanel::createConnect()
{
	connect(ui_->preProcessCbox, &QCheckBox::stateChanged, [=](int state) { emit preProcessCheckChanged(state); });
	connect(ui_->loadBtn, &QPushButton::clicked, [=]() { emit loadImage(); });
	connect(ui_->computerBtn, &QPushButton::clicked, [=]() { emit computerClicked(); });
	connect(ui_->fRadioButton, &QRadioButton::clicked, [=]() { emit seedModeChanged(1); });
	connect(ui_->bRadioButton, &QRadioButton::clicked, [=]() { emit seedModeChanged(2); });
	connect(ui_->noneRadioButton, &QRadioButton::clicked, [=]() { emit seedModeChanged(0); });
	connect(ui_->clearSeedBtn, &QPushButton::clicked, [=]() { emit clearSeeds(); });

	connect(ui_->iteration1D, qOverload<int>(&QSpinBox::valueChanged),
	        [=](int value) { parameters_.maxIterations1D = value; });
	connect(ui_->foreThreshold1D, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value)
	{
		parameters_.foreThreshold = value;
	});
	connect(ui_->gamma1D, qOverload<double>(&QDoubleSpinBox::valueChanged),
	        [=](double value) { parameters_.gamma1D = value; });
	connect(ui_->lambda1D, qOverload<double>(&QDoubleSpinBox::valueChanged),
	        [=](double value) { parameters_.lambda1D = value; });

	connect(ui_->iteration2D, qOverload<int>(&QSpinBox::valueChanged),
	        [=](int value) { parameters_.maxIterations2D = value; });
	connect(ui_->dt2D, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double value) { parameters_.dt = value; });
	connect(ui_->gamma2D, qOverload<double>(&QDoubleSpinBox::valueChanged),
	        [=](double value) { parameters_.gamma2D = value; });
	connect(ui_->labmda2D, qOverload<double>(&QDoubleSpinBox::valueChanged),
	        [=](double value) { parameters_.lambda2D = value; });

	connect(ui_->renderContourThreshold, qOverload<double>(&QDoubleSpinBox::valueChanged),
	        [=](double value) { emit thresholdChanged(value); });
}
