//
//  EmitterLayerWidget.cpp
//  ResourceEditorQt
//
//  Created by adebt on 11/26/12.
//
//

#include "EmitterLayerWidget.h"
#include "Commands/ParticleEditorCommands.h"
#include "Commands/CommandsManager.h"
#include "TextureBrowser/TextureConvertor.h"
#include "SceneEditor/EditorSettings.h"

#include <QHBoxLayout>
#include <QGraphicsWidget>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

#define SPRITE_SIZE 60

EmitterLayerWidget::EmitterLayerWidget(QWidget *parent) :
	QWidget(parent)
{
	mainBox = new QVBoxLayout;
	this->setLayout(mainBox);
	
	layerNameLineEdit = new QLineEdit();
	mainBox->addWidget(layerNameLineEdit);
	connect(layerNameLineEdit,
			SIGNAL(editingFinished()),
			this,
			SLOT(OnValueChanged()));

	enableCheckBox = new QCheckBox("Enable layer");
	mainBox->addWidget(enableCheckBox);
	connect(enableCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	
	additiveCheckBox = new QCheckBox("Additive");
	mainBox->addWidget(additiveCheckBox);
	connect(additiveCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	
	QHBoxLayout* spriteHBox = new QHBoxLayout;
	spriteLabel = new QLabel(this);
	spriteLabel->setMinimumSize(SPRITE_SIZE, SPRITE_SIZE);
	spriteHBox->addWidget(spriteLabel);
	mainBox->addLayout(spriteHBox);
	QVBoxLayout* spriteVBox = new QVBoxLayout;
	spriteHBox->addLayout(spriteVBox);
	spriteBtn = new QPushButton("Set sprite", this);
	spriteBtn->setMinimumHeight(30);
	spritePathLabel = new QLineEdit(this);
	spritePathLabel->setReadOnly(true);
	spriteVBox->addWidget(spriteBtn);
	spriteVBox->addWidget(spritePathLabel);
	connect(spriteBtn,
			SIGNAL(clicked(bool)),
			this,
			SLOT(OnSpriteBtn()));
	connect(spritePathLabel, SIGNAL(textChanged(const QString&)), this, SLOT(OnSpritePathChanged(const QString&)));

	lifeTimeLine = new TimeLineWidget(this);
	InitWidget(lifeTimeLine);
	numberTimeLine = new TimeLineWidget(this);
	InitWidget(numberTimeLine);
	sizeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeTimeLine);
	sizeVariationTimeLine = new TimeLineWidget(this);
	InitWidget(sizeVariationTimeLine);
	sizeOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(sizeOverLifeTimeLine);
	velocityTimeLine = new TimeLineWidget(this);
	InitWidget(velocityTimeLine);
	spinTimeLine = new TimeLineWidget(this);
	InitWidget(spinTimeLine);
	motionTimeLine = new TimeLineWidget(this);
	InitWidget(motionTimeLine);
	bounceTimeLine = new TimeLineWidget(this);
	InitWidget(bounceTimeLine);
	
	colorRandomGradient = new GradientPickerWidget(this);
	InitWidget(colorRandomGradient);
	colorOverLifeGradient = new GradientPickerWidget(this);
	InitWidget(colorOverLifeGradient);
	alphaOverLifeTimeLine = new TimeLineWidget(this);
	InitWidget(alphaOverLifeTimeLine);
	
	QHBoxLayout* frameOverlifeLayout = new QHBoxLayout();
	frameOverlifeCheckBox = new QCheckBox("frame over life", this);
	connect(frameOverlifeCheckBox, SIGNAL(stateChanged(int)),
				  this, SLOT(OnValueChanged()));

	frameOverlifeFPSSpin = new QSpinBox(this);
	frameOverlifeFPSSpin->setMinimum(0);
	frameOverlifeFPSSpin->setMaximum(1000);
	connect(frameOverlifeFPSSpin, SIGNAL(valueChanged(int)),
			this, SLOT(OnValueChanged()));

	frameOverlifeLayout->addWidget(frameOverlifeCheckBox);
	frameOverlifeLayout->addWidget(frameOverlifeFPSSpin);
	frameOverlifeLayout->addWidget(new QLabel("FPS", this));
	mainBox->addLayout(frameOverlifeLayout);
	
	angleTimeLine = new TimeLineWidget(this);
	InitWidget(angleTimeLine);
	
	QHBoxLayout* alignToMotionLayout = new QHBoxLayout;
	mainBox->addLayout(alignToMotionLayout);
	alignToMotionLayout->addWidget(new QLabel("alignToMotion", this));
	alignToMotionSpin = new QDoubleSpinBox();
	alignToMotionLayout->addWidget(alignToMotionSpin);
	connect(alignToMotionSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	
	QHBoxLayout* startTimeHBox = new QHBoxLayout;
	startTimeHBox->addWidget(new QLabel("startTime", this));
	startTimeSpin = new QDoubleSpinBox(this);
	startTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	startTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	startTimeHBox->addWidget(startTimeSpin);
	mainBox->addLayout(startTimeHBox);
	connect(startTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));

	QHBoxLayout* endTimeHBox = new QHBoxLayout;
	endTimeHBox->addWidget(new QLabel("endTime", this));
	endTimeSpin = new QDoubleSpinBox(this);
	endTimeSpin->setMinimum(-std::numeric_limits<double>::infinity());
	endTimeSpin->setMaximum(std::numeric_limits<double>::infinity());
	endTimeHBox->addWidget(endTimeSpin);
	mainBox->addLayout(endTimeHBox);
	connect(endTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	
	Q_FOREACH( QAbstractSpinBox * sp, findChildren<QAbstractSpinBox*>() ) {
        sp->installEventFilter( this );
    }
	spritePathLabel->installEventFilter(this);

	sprite = NULL;
	blockSignals = false;
}

EmitterLayerWidget::~EmitterLayerWidget()
{
	disconnect(layerNameLineEdit,
			SIGNAL(editingFinished()),
			this,
			SLOT(OnValueChanged()));
	disconnect(enableCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(additiveCheckBox,
			SIGNAL(stateChanged(int)),
			this,
			SLOT(OnValueChanged()));
	disconnect(spriteBtn,
			SIGNAL(clicked(bool)),
			this,
			SLOT(OnSpriteBtn()));
	disconnect(spritePathLabel,
			SIGNAL(textChanged(const QString&)),
			this,
			SLOT(OnSpritePathChanged(const QString&)));
	disconnect(alignToMotionSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(startTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(endTimeSpin,
			SIGNAL(valueChanged(double)),
			this,
			SLOT(OnValueChanged()));
	disconnect(frameOverlifeCheckBox,
		   SIGNAL(stateChanged(int)),
		   this,
		   SLOT(OnValueChanged()));
	disconnect(frameOverlifeFPSSpin,
		   SIGNAL(valueChanged(int)),
		   this,
		   SLOT(OnValueChanged()));
}

void EmitterLayerWidget::InitWidget(QWidget* widget)
{
	mainBox->addWidget(widget);
	connect(widget,
			SIGNAL(ValueChanged()),
			this,
			SLOT(OnValueChanged()));
}

void EmitterLayerWidget::Init(ParticleEmitter* emitter, DAVA::ParticleLayer *layer, bool updateMinimized)
{
	if (!emitter || !layer)
		return;
	
	blockSignals = true;
	
	this->emitter = emitter;
	this->layer = layer;
	
	float32 emitterLifeTime = emitter->GetLifeTime();
	float32 lifeTime = Min(emitterLifeTime, layer->endTime);

	layerNameLineEdit->setText(QString::fromStdString(layer->layerName));
	enableCheckBox->setChecked(!layer->isDisabled);
	additiveCheckBox->setChecked(layer->GetAdditive());

	//LAYER_SPRITE = 0,
	sprite = layer->GetSprite();
	Sprite* renderSprite = Sprite::CreateAsRenderTarget(SPRITE_SIZE, SPRITE_SIZE, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(renderSprite);
	if (sprite)
	{
		sprite->SetScaleSize(SPRITE_SIZE, SPRITE_SIZE);
		sprite->Draw();
	}

	RenderManager::Instance()->RestoreRenderTarget();
	Texture* texture = renderSprite->GetTexture();
	Image* image = texture->CreateImageFromMemory();
	spriteLabel->setPixmap(QPixmap::fromImage(TextureConvertor::fromDavaImage(image)));
	SafeRelease(image);
	SafeRelease(renderSprite);

	QString spriteName = "<none>";
	if (sprite)
	{
		String spritePath = FileSystem::Instance()->GetCanonicalPath(sprite->GetName());
		spriteName = QString::fromStdString(spritePath);
	}
	spritePathLabel->setText(spriteName);

	//LAYER_LIFE, LAYER_LIFE_VARIATION,
	lifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	lifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->life).GetProps(), Qt::blue, "life");
	lifeTimeLine->AddLine(1, PropLineWrapper<float32>(layer->lifeVariation).GetProps(), Qt::darkGreen, "life variation");

	//LAYER_NUMBER, LAYER_NUMBER_VARIATION,
	numberTimeLine->Init(layer->startTime, lifeTime, updateMinimized, false, true, true);
//		void Init(float32 minT, float32 maxT, bool updateSizeState, bool aliasLinePoint = false, bool allowDeleteLine = true, bool integer = false);
	numberTimeLine->SetMinLimits(0);
	numberTimeLine->AddLine(0, PropLineWrapper<float32>(layer->number).GetProps(), Qt::blue, "number");
	numberTimeLine->AddLine(1, PropLineWrapper<float32>(layer->numberVariation).GetProps(), Qt::darkGreen, "number variation");

	//LAYER_SIZE, LAYER_SIZE_VARIATION, LAYER_SIZE_OVER_LIFE,
	Vector<QColor> colors;
	colors.push_back(Qt::blue); colors.push_back(Qt::darkGreen);
	Vector<QString> legends;
	legends.push_back("size X"); legends.push_back("size Y");
	sizeTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeTimeLine->SetMinLimits(0);
	sizeTimeLine->AddLines(PropLineWrapper<Vector2>(layer->size).GetProps(), colors, legends);
	sizeTimeLine->EnableLock(true);
	
	legends.clear();
	legends.push_back("size variation X"); legends.push_back("size variation Y");
	sizeVariationTimeLine->Init(layer->startTime, lifeTime, updateMinimized, true);
	sizeVariationTimeLine->SetMinLimits(0);
	sizeVariationTimeLine->AddLines(PropLineWrapper<Vector2>(layer->sizeVariation).GetProps(), colors, legends);
	sizeVariationTimeLine->EnableLock(true);
	
	sizeOverLifeTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	sizeOverLifeTimeLine->SetMinLimits(0);
	sizeOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->sizeOverLife).GetProps(), Qt::blue, "size over life");


	//LAYER_VELOCITY, LAYER_VELOCITY_VARIATION, LAYER_VELOCITY_OVER_LIFE,
	velocityTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	velocityTimeLine->AddLine(0, PropLineWrapper<float32>(layer->velocity).GetProps(), Qt::blue, "velocity");
	velocityTimeLine->AddLine(1, PropLineWrapper<float32>(layer->velocityVariation).GetProps(), Qt::darkGreen, "velocity variation");
	velocityTimeLine->AddLine(2, PropLineWrapper<float32>(layer->velocityOverLife).GetProps(), Qt::red, "velocity over life");

	//LAYER_FORCES, LAYER_FORCES_VARIATION, LAYER_FORCES_OVER_LIFE,

	//LAYER_SPIN, LAYER_SPIN_VARIATION, LAYER_SPIN_OVER_LIFE,
	spinTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	spinTimeLine->AddLine(0, PropLineWrapper<float32>(layer->spin).GetProps(), Qt::blue, "spin");
	spinTimeLine->AddLine(1, PropLineWrapper<float32>(layer->spinVariation).GetProps(), Qt::darkGreen, "spin variation");
	spinTimeLine->AddLine(2, PropLineWrapper<float32>(layer->spinOverLife).GetProps(), Qt::red, "spin over life");
	
	//LAYER_MOTION_RANDOM, LAYER_MOTION_RANDOM_VARIATION, LAYER_MOTION_RANDOM_OVER_LIFE,
	motionTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	motionTimeLine->AddLine(0, PropLineWrapper<float32>(layer->motionRandom).GetProps(), Qt::blue, "motion random");
	motionTimeLine->AddLine(1, PropLineWrapper<float32>(layer->motionRandomVariation).GetProps(), Qt::darkGreen, "motion random variation");
	motionTimeLine->AddLine(2, PropLineWrapper<float32>(layer->motionRandomOverLife).GetProps(), Qt::red, "motion random over life");

	//LAYER_BOUNCE, LAYER_BOUNCE_VARIATION,	LAYER_BOUNCE_OVER_LIFE,
	bounceTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	bounceTimeLine->AddLine(0, PropLineWrapper<float32>(layer->bounce).GetProps(), Qt::blue, "bounce");
	bounceTimeLine->AddLine(1, PropLineWrapper<float32>(layer->bounceVariation).GetProps(), Qt::darkGreen, "bounce Variation");
	bounceTimeLine->AddLine(2, PropLineWrapper<float32>(layer->bounceOverLife).GetProps(), Qt::red, "bounce over life");
	
	//LAYER_COLOR_RANDOM, LAYER_ALPHA_OVER_LIFE, LAYER_COLOR_OVER_LIFE,
	colorRandomGradient->Init(0, 1, "random color");
	colorRandomGradient->SetValues(PropLineWrapper<Color>(layer->colorRandom).GetProps());
	
	colorOverLifeGradient->Init(0, 1, "color over life");
	colorOverLifeGradient->SetValues(PropLineWrapper<Color>(layer->colorOverLife).GetProps());

	alphaOverLifeTimeLine->Init(0, 1, updateMinimized);
	alphaOverLifeTimeLine->SetMinLimits(0);
	alphaOverLifeTimeLine->SetMaxLimits(1.f);
	alphaOverLifeTimeLine->AddLine(0, PropLineWrapper<float32>(layer->alphaOverLife).GetProps(), Qt::blue, "alpha over life");
	
	frameOverlifeCheckBox->setChecked(layer->frameOverLifeEnabled);
	frameOverlifeFPSSpin->setValue(layer->frameOverLifeFPS);
	frameOverlifeFPSSpin->setEnabled(layer->frameOverLifeEnabled);
	
	angleTimeLine->Init(layer->startTime, lifeTime, updateMinimized);
	angleTimeLine->AddLine(0, PropLineWrapper<float32>(layer->angle).GetProps(), Qt::blue, "angle");
	angleTimeLine->AddLine(1, PropLineWrapper<float32>(layer->angleVariation).GetProps(), Qt::darkGreen, "angle variation");
	
	//LAYER_ALIGN_TO_MOTION,
	alignToMotionSpin->setValue(layer->alignToMotion);
	
	//LAYER_START_TIME, LAYER_END_TIME
	startTimeSpin->setMinimum(0);
	startTimeSpin->setValue(layer->startTime);
	startTimeSpin->setMaximum(layer->endTime);
	endTimeSpin->setMinimum(0);
	endTimeSpin->setValue(layer->endTime);
	endTimeSpin->setMaximum(emitter->GetLifeTime());
	
	//, LAYER_IS_LONG
	
	blockSignals = false;
}

void EmitterLayerWidget::RestoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	lifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_LIFE_PROPS"));
	numberTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_NUMBER_PROPS"));
	sizeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_PROPS"));
	sizeVariationTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_VARIATION_PROPS"));
	sizeOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SIZE_OVER_LIFE_PROPS"));
	velocityTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_VELOCITY_PROPS"));
	spinTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_SPIN_PROPS"));
	motionTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_MOTION_RANDOM_PROPS"));
	bounceTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_BOUNCE_PROPS"));
	alphaOverLifeTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_ALPHA_OVER_LIFE_PROPS"));
	angleTimeLine->SetVisualState(visualStateProps->GetArchive("LAYER_ANGLE"));	
}

void EmitterLayerWidget::StoreVisualState(KeyedArchive* visualStateProps)
{
	if (!visualStateProps)
		return;

	KeyedArchive* props = new KeyedArchive();

	lifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	numberTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_NUMBER_PROPS", props);

	props->DeleteAllKeys();
	sizeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_PROPS", props);

	props->DeleteAllKeys();
	sizeVariationTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_VARIATION_PROPS", props);

	props->DeleteAllKeys();
	sizeOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SIZE_OVER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	velocityTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_VELOCITY_PROPS", props);

	props->DeleteAllKeys();
	spinTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_SPIN_PROPS", props);

	props->DeleteAllKeys();
	motionTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_MOTION_RANDOM_PROPS", props);

	props->DeleteAllKeys();
	bounceTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_BOUNCE_PROPS", props);

	props->DeleteAllKeys();
	alphaOverLifeTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_ALPHA_OVER_LIFE_PROPS", props);

	props->DeleteAllKeys();
	angleTimeLine->GetVisualState(props);
	visualStateProps->SetArchive("LAYER_ANGLE", props);

	SafeRelease(props);
}

void EmitterLayerWidget::OnSpriteBtn()
{
	String projectPath = EditorSettings::Instance()->GetProjectPath();
	
	projectPath += "Data/Gfx/Particles/";
	QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open particle sprite"), QString::fromStdString(projectPath), QString("Effect File (*.txt)"));
	if (filePath.isEmpty())
		return;
	
	// Yuri Coder. Verify that the path of the file opened is correct (i.e. inside the Project Path),
	// this is according to the DF-551 issue.
	String filePathToBeOpened;
	String fileNameToBeOpened;
	FileSystem::SplitPath(filePath.toStdString(), filePathToBeOpened, fileNameToBeOpened);

#ifdef __DAVAENGINE_WIN32__
	// Remove the drive name, if any.
	String::size_type driveNamePos = filePathToBeOpened.find(":/");
	if (driveNamePos != String::npos && filePathToBeOpened.length() > 2)
	{
		filePathToBeOpened = filePathToBeOpened.substr(2, filePathToBeOpened.length() - 2);
	}
#endif

	if (filePathToBeOpened != projectPath)
	{
		QString message = QString("You've opened Particle Sprite from incorrect path (%1).\n Correct one is %2.").
			arg(QString::fromStdString(filePathToBeOpened)).
			arg(QString::fromStdString(projectPath));

		QMessageBox msgBox(QMessageBox::Warning, "Warning", message);
		msgBox.exec();

		// TODO: return here in case we'll decide to not allow opening sprite from incorrect path.
	}
	
	filePath.remove(filePath.size() - 4, 4);
	Sprite* sprite = Sprite::Create(filePath.toStdString());
	if (!sprite)
		return;
	
	this->sprite = sprite;
	OnValueChanged();
}

void EmitterLayerWidget::OnValueChanged()
{
	if (blockSignals)
		return;
	
	PropLineWrapper<float32> propLife;
	PropLineWrapper<float32> propLifeVariation;
	lifeTimeLine->GetValue(0, propLife.GetPropsPtr());
	lifeTimeLine->GetValue(1, propLifeVariation.GetPropsPtr());
	
	PropLineWrapper<float32> propNumber;
	PropLineWrapper<float32> propNumberVariation;
	numberTimeLine->GetValue(0, propNumber.GetPropsPtr());
	numberTimeLine->GetValue(1, propNumberVariation.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSize;
	sizeTimeLine->GetValues(propSize.GetPropsPtr());
	
	PropLineWrapper<Vector2> propSizeVariation;
	sizeVariationTimeLine->GetValues(propSizeVariation.GetPropsPtr());

	PropLineWrapper<float32> propsizeOverLife;
	sizeOverLifeTimeLine->GetValue(0, propsizeOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propVelocity;
	PropLineWrapper<float32> propVelocityVariation;
	PropLineWrapper<float32> propVelocityOverLife;
	velocityTimeLine->GetValue(0, propVelocity.GetPropsPtr());
	velocityTimeLine->GetValue(1, propVelocityVariation.GetPropsPtr());
	velocityTimeLine->GetValue(2, propVelocityOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propSpin;
	PropLineWrapper<float32> propSpinVariation;
	PropLineWrapper<float32> propSpinOverLife;
	spinTimeLine->GetValue(0, propSpin.GetPropsPtr());
	spinTimeLine->GetValue(1, propSpinVariation.GetPropsPtr());
	spinTimeLine->GetValue(2, propSpinOverLife.GetPropsPtr());

	PropLineWrapper<float32> propMotion;
	PropLineWrapper<float32> propMotionVariation;
	PropLineWrapper<float32> propMotionOverLife;
	motionTimeLine->GetValue(0, propMotion.GetPropsPtr());
	motionTimeLine->GetValue(1, propMotionVariation.GetPropsPtr());
	motionTimeLine->GetValue(2, propMotionOverLife.GetPropsPtr());

	PropLineWrapper<float32> propBounce;
	PropLineWrapper<float32> propBounceVariation;
	PropLineWrapper<float32> propBounceOverLife;
	bounceTimeLine->GetValue(0, propBounce.GetPropsPtr());
	bounceTimeLine->GetValue(1, propBounceVariation.GetPropsPtr());
	bounceTimeLine->GetValue(2, propBounceOverLife.GetPropsPtr());

	PropLineWrapper<Color> propColorRandom;
	colorRandomGradient->GetValues(propColorRandom.GetPropsPtr());

	PropLineWrapper<Color> propColorOverLife;
	colorOverLifeGradient->GetValues(propColorOverLife.GetPropsPtr());

	PropLineWrapper<float32> propAlphaOverLife;
	alphaOverLifeTimeLine->GetValue(0, propAlphaOverLife.GetPropsPtr());
	
	PropLineWrapper<float32> propAngle;
	PropLineWrapper<float32> propAngleVariation;
	angleTimeLine->GetValue(0, propAngle.GetPropsPtr());
	angleTimeLine->GetValue(1, propAngleVariation.GetPropsPtr());
	
	CommandUpdateParticleLayer* updateLayerCmd = new CommandUpdateParticleLayer(emitter, layer);
	updateLayerCmd->Init(layerNameLineEdit->text(),
						 !enableCheckBox->isChecked(),
						 additiveCheckBox->isChecked(),
						 sprite,
						 propLife.GetPropLine(),
						 propLifeVariation.GetPropLine(),
						 propNumber.GetPropLine(),
						 propNumberVariation.GetPropLine(),
						 propSize.GetPropLine(),
						 propSizeVariation.GetPropLine(),
						 propsizeOverLife.GetPropLine(),
						 propVelocity.GetPropLine(),
						 propVelocityVariation.GetPropLine(),
						 propVelocityOverLife.GetPropLine(),
						 propSpin.GetPropLine(),
						 propSpinVariation.GetPropLine(),
						 propSpinOverLife.GetPropLine(),
						 propMotion.GetPropLine(),
						 propMotionVariation.GetPropLine(),
						 propMotionOverLife.GetPropLine(),
						 propBounce.GetPropLine(),
						 propBounceVariation.GetPropLine(),
						 propBounceOverLife.GetPropLine(),
						 propColorRandom.GetPropLine(),
						 propAlphaOverLife.GetPropLine(),
						 propColorOverLife.GetPropLine(),
						 propAngle.GetPropLine(),
						 propAngleVariation.GetPropLine(),
						 (float32)alignToMotionSpin->value(),
						 (float32)startTimeSpin->value(),
						 (float32)endTimeSpin->value(),
						 frameOverlifeCheckBox->isChecked(),
						 (float32)frameOverlifeFPSSpin->value());
	
	if (CommandsManager::Instance())
	CommandsManager::Instance()->Execute(updateLayerCmd);
	SafeRelease(updateLayerCmd);
	
	Init(this->emitter, this->layer, false);
	emit ValueChanged();
}

void EmitterLayerWidget::Update()
{
	Init(this->emitter, this->layer, false);
}

void EmitterLayerWidget::UpdateTooltip()
{
	QFontMetrics fm = spritePathLabel->fontMetrics();
	if (fm.width(spritePathLabel->text()) >= spritePathLabel->width())
	{
		spritePathLabel->setToolTip(spritePathLabel->text());
	}
	else
	{
		spritePathLabel->setToolTip("");
	}
}

bool EmitterLayerWidget::eventFilter( QObject * o, QEvent * e )
{
    if ( e->type() == QEvent::Wheel &&
		qobject_cast<QAbstractSpinBox*>( o ) )
    {
        e->ignore();
        return true;
    }

	if (e->type() == QEvent::Resize && qobject_cast<QLineEdit*>(o))
	{
		UpdateTooltip();
		return true;
	}

    return QWidget::eventFilter( o, e );
}

void EmitterLayerWidget::OnSpritePathChanged(const QString& text)
{
	UpdateTooltip();
}
