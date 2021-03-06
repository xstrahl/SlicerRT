/*==============================================================================

  Copyright (c) Radiation Medicine Program, University Health Network,
  Princess Margaret Hospital, Toronto, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kevin Wang, Princess Margaret Cancer Centre 
  and was supported by Cancer Care Ontario (CCO)'s ACRU program 
  with funds provided by the Ontario Ministry of Health and Long-Term Care
  and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).

==============================================================================*/

// SlicerQt includes
#include "qSlicerExternalBeamPlanningModuleWidget.h"
#include "ui_qSlicerExternalBeamPlanningModule.h"

// SlicerRtCommon includes
#include "SlicerRtCommon.h"

// QSlicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h> 
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// ExtensionTemplate Logic includes
#include <vtkSlicerCLIModuleLogic.h>

// SlicerRt includes
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLExternalBeamPlanningNode.h"
#include "vtkMRMLRTBeamNode.h"
#include "vtkMRMLRTPlanNode.h"
#include "vtkSlicerExternalBeamPlanningModuleLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLLayoutNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_ExternalBeamPlanning
class qSlicerExternalBeamPlanningModuleWidgetPrivate: public Ui_qSlicerExternalBeamPlanningModule
{
  Q_DECLARE_PUBLIC(qSlicerExternalBeamPlanningModuleWidget);
protected:
  qSlicerExternalBeamPlanningModuleWidget* const q_ptr;
public:
  qSlicerExternalBeamPlanningModuleWidgetPrivate(qSlicerExternalBeamPlanningModuleWidget& object);
  ~qSlicerExternalBeamPlanningModuleWidgetPrivate();
  vtkSlicerExternalBeamPlanningModuleLogic* logic() const;
public:
  /// Using this flag prevents overriding the parameter set node contents when the
  ///   QMRMLCombobox selects the first instance of the specified node type when initializing
  bool ModuleWindowInitialized;

  int currentBeamRow;
  int totalBeamRows;

  QString BeamName;
  vtkMRMLRTBeamNode::RTBeamType BeamType;
  vtkMRMLRTBeamNode::RTRadiationType RadiationType;
  double GantryAngle;
  QString CollimatorType;
};

//-----------------------------------------------------------------------------
// qSlicerExternalBeamPlanningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerExternalBeamPlanningModuleWidgetPrivate::qSlicerExternalBeamPlanningModuleWidgetPrivate(qSlicerExternalBeamPlanningModuleWidget& object)
  : q_ptr(&object)
  , ModuleWindowInitialized(false)
{
  currentBeamRow = -1;
  totalBeamRows = 0;
}

//-----------------------------------------------------------------------------
qSlicerExternalBeamPlanningModuleWidgetPrivate::~qSlicerExternalBeamPlanningModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerExternalBeamPlanningModuleLogic*
qSlicerExternalBeamPlanningModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerExternalBeamPlanningModuleWidget);
  return vtkSlicerExternalBeamPlanningModuleLogic::SafeDownCast(q->logic());
} 

//-----------------------------------------------------------------------------
// qSlicerExternalBeamPlanningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerExternalBeamPlanningModuleWidget::qSlicerExternalBeamPlanningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerExternalBeamPlanningModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerExternalBeamPlanningModuleWidget::~qSlicerExternalBeamPlanningModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  this->Superclass::setMRMLScene(scene);

  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()) );

  // Find parameters node or create it if there is no one in the scene
  if (scene &&  d->logic()->GetExternalBeamPlanningNode() == 0)
  {
    vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLExternalBeamPlanningNode");
    if (node)
    {
      this->setExternalBeamPlanningNode( vtkMRMLExternalBeamPlanningNode::SafeDownCast(node) );
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::onSceneImportedEvent()
{
  this->onEnter();
}
//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::enter()
{
  this->onEnter();
  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::onEnter()
{
  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::onEnter: Invalid scene!";
    return;
  }

  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  // First check the logic if it has a parameter node
  if (d->logic() == NULL)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::onEnter: Invalid logic!";
    return;
  }
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();

  // If we have a parameter node select it
  if (paramNode == NULL)
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLExternalBeamPlanningNode");
    if (node)
    {
      paramNode = vtkMRMLExternalBeamPlanningNode::SafeDownCast(node);
      d->logic()->SetAndObserveExternalBeamPlanningNode(paramNode);
      return;
    }
    else 
    {
      vtkSmartPointer<vtkMRMLExternalBeamPlanningNode> newNode = vtkSmartPointer<vtkMRMLExternalBeamPlanningNode>::New();
      this->mrmlScene()->AddNode(newNode);
      d->logic()->SetAndObserveExternalBeamPlanningNode(newNode);
    }
  }

  this->updateWidgetFromMRML();
  d->ModuleWindowInitialized = true;
}

//-----------------------------------------------------------------------------
vtkMRMLExternalBeamPlanningNode* qSlicerExternalBeamPlanningModuleWidget::getExternalBeamPlanningNode ()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  
  return d->logic()->GetExternalBeamPlanningNode();
}

//-----------------------------------------------------------------------------
vtkMRMLRTBeamNode* qSlicerExternalBeamPlanningModuleWidget::getCurrentBeamNode(vtkMRMLExternalBeamPlanningNode* paramNode)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  QTableWidgetItem *item = NULL;
  item = d->tableWidget_Beams->item(d->currentBeamRow, 1);
  if (!item)
  {
    return NULL;
  }
  std::string beamName = item->text().toStdString();

  vtkMRMLRTPlanNode* rtPlanNode = paramNode->GetRtPlanNode();
  if (!rtPlanNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::isocenterFiducialNodeChanged: Invalid rtplan node!";
    return NULL;
  }

  vtkMRMLRTBeamNode* beamNode = rtPlanNode->GetRTBeamNode (beamName);
  printf ("Found beam node (%p)\n", beamNode);

  return beamNode;
}

vtkMRMLRTBeamNode* qSlicerExternalBeamPlanningModuleWidget::getCurrentBeamNode()
{
  return this->getCurrentBeamNode (this->getExternalBeamPlanningNode());
}

//-----------------------------------------------------------------------------
std::string qSlicerExternalBeamPlanningModuleWidget::getCurrentBeamName ()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode();
  return beamNode->GetBeamName ();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::updateWidgetFromRTBeam (const vtkMRMLRTBeamNode* beamNode)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  // If node is empty, remove all tabs
  if (!beamNode) {
    d->tabWidget->clear ();
    return;
  }

  // Enable appropriate tabs and widgets for this beam type and set 
  // widget values from MRML node
  vtkMRMLRTBeamNode::RTRadiationType radType = beamNode->GetRadiationType();
  if (radType == vtkMRMLRTBeamNode::Photon)
  {
    // Enable or disable widgets on the prescription tab 
    d->label_NominalEnergy->setEnabled (true);
    d->comboBox_NominalEnergy->setEnabled (true);
    d->label_BeamOnTime->setEnabled (true);
    d->lineEdit_BeamOnTime->setEnabled (true);
    d->label_NominalmA->setEnabled (true);
    d->lineEdit_NominalmA->setEnabled (true);

    // Disable unneeded tabs
    int index;
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonBeamModel);
    if (index >= 0)
    {
      d->tabWidget->removeTab(index);
    }

    // Enable needed tabs and set widget values
    index = d->tabWidget->indexOf(d->tabWidgetPageGeometry);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageGeometry, "Geometry");
    }

    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonBeamModel);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPagePhotonBeamModel, "Beam Model");
    }
  }
  else if (radType == vtkMRMLRTBeamNode::Proton)
  {
    // Enable or disable widgets on the prescription tab 
    d->label_NominalEnergy->setEnabled (false);
    d->comboBox_NominalEnergy->setEnabled (false);
    d->label_BeamOnTime->setEnabled (false);
    d->lineEdit_BeamOnTime->setEnabled (false);
    d->label_NominalmA->setEnabled (false);
    d->lineEdit_NominalmA->setEnabled (false);

    // Disable unneeded tabs
    int index;
    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonBeamModel);
    if (index >= 0)
    {
      d->tabWidget->removeTab(index);
    }

    // Enable needed tabs and set widget values
    // N.b. Proton beam type uses photon geometry
    index = d->tabWidget->indexOf(d->tabWidgetPageRx);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageRx, "Prescription");
    }
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonEnergy);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageProtonEnergy, "Energy");
    }
    index = d->tabWidget->indexOf(d->tabWidgetPageGeometry);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageGeometry, "Geometry");
    }
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonBeamModel);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageProtonBeamModel, "Beam Model");
    }
  }
  else if (radType == vtkMRMLRTBeamNode::Electron)
  {
    /* Not implemented */
    d->tabWidget->clear ();
  }

  /* GCS TODO: Here I need to set the beam tabs from the RT Beam */

#if defined (commentout)
  d->RangeWidget_PhotonXJawsPosition->setValues(-paramNode->GetX1Jaw(), paramNode->GetX2Jaw());
  d->RangeWidget_PhotonYJawsPosition->setValues(-paramNode->GetY1Jaw(), paramNode->GetY2Jaw());
  d->SliderWidget_PhotonGantryAngle->setValue(paramNode->GetGantryAngle());
  d->SliderWidget_PhotonCollimatorAngle->setValue(paramNode->GetCollimatorAngle());
  d->SliderWidget_PhotonCouchAngle->setValue(paramNode->GetCouchAngle());

  d->SliderWidget_ProtonGantryAngle->setValue(paramNode->GetGantryAngle());
  d->SliderWidget_ProtonCollimatorAngle->setValue(paramNode->GetCollimatorAngle());
  d->SliderWidget_ProtonCouchAngle->setValue(paramNode->GetCouchAngle());
#endif
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (paramNode && this->mrmlScene())
  {
    d->MRMLNodeComboBox_ParameterSet->setCurrentNode(paramNode);
    if (paramNode->GetReferenceVolumeNode())
    {
      d->MRMLNodeComboBox_ReferenceVolume->setCurrentNode(paramNode->GetReferenceVolumeNode());
    }
    else
    {
      this->referenceVolumeNodeChanged(d->MRMLNodeComboBox_ReferenceVolume->currentNode());
    }

    if (paramNode->GetRtPlanNode())
    {
      d->MRMLNodeComboBox_RtPlan->setCurrentNode(paramNode->GetRtPlanNode());
    }
    else
    {
      this->rtPlanNodeChanged(d->MRMLNodeComboBox_RtPlan->currentNode());
    }

    if (paramNode->GetMLCPositionDoubleArrayNode())
    {
      d->MRMLNodeComboBox_PhotonMLCPositionDoubleArray->setCurrentNode(paramNode->GetMLCPositionDoubleArrayNode());
    }
    else
    {
      this->MLCPositionDoubleArrayNodeChanged(d->MRMLNodeComboBox_PhotonMLCPositionDoubleArray->currentNode());
    }

    if (paramNode->GetTargetSegmentationNode())
    {
      d->SegmentSelectorWidget_TargetSegment->setCurrentNode(paramNode->GetTargetSegmentationNode());
    }
    else
    {
      this->targetContourNodeChanged(d->SegmentSelectorWidget_TargetSegment->currentNode());
    }

    if (paramNode->GetIsocenterFiducialNode())
    {
      d->MRMLNodeComboBox_IsocenterFiducial->setCurrentNode(paramNode->GetIsocenterFiducialNode());
    }
    else
    {
      this->isocenterFiducialNodeChanged(d->MRMLNodeComboBox_IsocenterFiducial->currentNode());
    }

#if defined (commentout)
    if (paramNode->GetBeamName())
    {
      d->lineEdit_BeamName->setText(paramNode->GetBeamName());
    }
#endif
    /* GCS TODO: Here I still need to set the beam tabs */

  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::setup()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  vtkSlicerExternalBeamPlanningModuleLogic* externalBeamPlanningModuleLogic =
    vtkSlicerExternalBeamPlanningModuleLogic::SafeDownCast(this->logic());

  qSlicerAbstractCoreModule* matlabDoseCalculationModule =
    qSlicerCoreApplication::application()->moduleManager()->module("MatlabDoseCalculation");
  if (matlabDoseCalculationModule)
    {
    vtkSlicerCLIModuleLogic* matlabDoseCalculationModuleLogic =
      vtkSlicerCLIModuleLogic::SafeDownCast(matlabDoseCalculationModule->logic());
    externalBeamPlanningModuleLogic->SetMatlabDoseCalculationModuleLogic(matlabDoseCalculationModuleLogic);
    }
  else
    {
    qWarning() << "MatlabDoseCalculation module is not found!";
    }

  // Make connections
  this->connect( d->MRMLNodeComboBox_ParameterSet, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setExternalBeamPlanningNode(vtkMRMLNode*)) );

  // RT plan page
  this->connect( d->MRMLNodeComboBox_ReferenceVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(referenceVolumeNodeChanged(vtkMRMLNode*)) );
  this->connect( d->SegmentSelectorWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(planContourSetNodeChanged(vtkMRMLNode*)) );
  this->connect( d->MRMLNodeComboBox_RtPlan, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(rtPlanNodeChanged(vtkMRMLNode*)) );
  this->connect( d->MRMLNodeComboBox_DoseVolume, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(rtDoseVolumeNodeChanged(vtkMRMLNode*)) );
  this->connect( d->MRMLNodeComboBox_DoseROI, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(rtDoseROINodeChanged(vtkMRMLNode*)) );
  this->connect( d->lineEdit_DoseGridSpacing, SIGNAL(textChanged(const QString &)), this, SLOT(doseGridSpacingChanged(const QString &)) );
  this->connect( d->comboBox_DoseEngineType, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(doseEngineTypeChanged(const QString &)) );

  // RT Beams page
  this->connect( d->tableWidget_Beams, SIGNAL(cellClicked(int, int)), this, SLOT(tableWidgetCellClicked(int, int)) );
  this->connect( d->pushButton_AddBeam, SIGNAL(clicked()), this, SLOT(addBeamClicked()) );
  this->connect( d->pushButton_RemoveBeam, SIGNAL(clicked()), this, SLOT(removeBeamClicked()) );

  /* Beam global parameters */
  this->connect( d->lineEdit_BeamName, SIGNAL(textChanged(const QString &)), this, SLOT(beamNameChanged(const QString &)) );
  this->connect( d->comboBox_RadiationType, SIGNAL(currentIndexChanged(int)), this, SLOT(radiationTypeChanged(int)) );

  /* Prescription page */
  this->connect( d->comboBox_BeamType, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(beamTypeChanged(const QString &)) );
  this->connect( d->SegmentSelectorWidget_TargetSegment, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(targetContourNodeChanged(vtkMRMLNode*)) );
  this->connect( d->SegmentSelectorWidget_TargetSegment, SIGNAL(currentSegmentChanged(QString)), this, SLOT(targetContourSegmentChanged(const QString&)) );
  this->connect( d->doubleSpinBox_RxDose, SIGNAL(valueChanged(double)), this, SLOT(RxDoseChanged(double)) );
  this->connect( d->MRMLNodeComboBox_IsocenterFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(isocenterFiducialNodeChanged(vtkMRMLNode*)) );
  this->connect( d->MRMLNodeComboBox_DosePointFiducial, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(dosePointFiducialNodeChanged(vtkMRMLNode*)) );
  this->connect( d->comboBox_NominalEnergy, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(nominalEnergyChanged(const QString&)) );
  this->connect( d->lineEdit_NominalmA, SIGNAL(textChanged(const QString &)), this, SLOT(nominalmAChanged(const QString &)) );
  this->connect( d->lineEdit_BeamOnTime, SIGNAL(textChanged(const QString &)), this, SLOT(beamOnTimeChanged(const QString &)) );

  /* Energy page */
  this->connect( d->doubleSpinBox_ProtonSmearing, SIGNAL(valueChanged(double)), this, SLOT(protonSmearingChanged(double)) );
  this->connect( d->doubleSpinBox_ProtonProximalMargins, SIGNAL(valueChanged(double)), this, SLOT(protonProximalMarginChanged(double)) );
  this->connect( d->doubleSpinBox_ProtonDistalMargins, SIGNAL(valueChanged(double)), this, SLOT(protonDistalMarginChanged(double)) );

  /* Geometry page */
  this->connect( d->MRMLNodeComboBox_PhotonMLCPositionDoubleArray, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(MLCPositionDoubleArrayNodeChanged(vtkMRMLNode*)) );
  this->connect( d->RangeWidget_PhotonXJawsPosition, SIGNAL(valuesChanged(double, double)), this, SLOT(XJawsPositionValuesChanged(double, double)) );
  this->connect( d->RangeWidget_PhotonYJawsPosition, SIGNAL(valuesChanged(double, double)), this, SLOT(YJawsPositionValuesChanged(double, double)) );
  this->connect( d->SliderWidget_PhotonGantryAngle, SIGNAL(valueChanged(double)), this, SLOT(gantryAngleChanged(double)) );
  this->connect( d->SliderWidget_PhotonCollimatorAngle, SIGNAL(valueChanged(double)), this, SLOT(collimatorAngleChanged(double)) );
  this->connect( d->SliderWidget_PhotonCouchAngle, SIGNAL(valueChanged(double)), this, SLOT(couchAngleChanged(double)) );
  this->connect( d->doubleSpinBox_beamWeight, SIGNAL(valueChanged(double)), this, SLOT(beamWeightChanged(double)) );

  /* Proton beam model */
  this->connect( d->doubleSpinBox_SAD, SIGNAL(valueChanged(double)), this, SLOT(protonSourceDistanceChanged(double)) );
  this->connect( d->doubleSpinBox_ProtonSourceSize, SIGNAL(valueChanged(double)), this, SLOT(protonSourceSizeChanged(double)) );
  this->connect( d->doubleSpinBox_ApertureDistance, SIGNAL(valueChanged(double)), this, SLOT(protonApertureOffsetChanged(double)) );
  this->connect( d->doubleSpinBox_EnergyResolution, SIGNAL(valueChanged(double)), this, SLOT(protonEnergyResolutionChanged(double)) );
  this->connect( d->comboBox_beamFlavor, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(protonBeamFlavorChanged(const QString &)) );
  this->connect( d->doubleSpinBox_DoseResolution, SIGNAL(valueChanged(double)), this, SLOT(protonDoseResolutionChanged(double)) );

  /* Beam visualization */
  this->connect( d->pushButton_UpdateDRR, SIGNAL(clicked()), this, SLOT(updateDRRClicked()) );
  this->connect( d->checkBox_BeamEyesView, SIGNAL(clicked(bool)), this, SLOT(beamEyesViewClicked(bool)) );
  this->connect( d->checkBox_ContoursInBEW, SIGNAL(clicked(bool)), this, SLOT(contoursInBEWClicked(bool)) );

  /* Calculation buttons */
  this->connect( d->pushButton_CalculateDose, SIGNAL(clicked()), this, SLOT(calculateDoseClicked()) );
  this->connect( d->pushButton_CalculateWED, SIGNAL(clicked()), this, SLOT(calculateWEDClicked()) );
  this->connect( d->pushButton_ClearDose, SIGNAL(clicked()), this, SLOT(clearDoseClicked()) );

  /* Disable unused buttons in prescription task */
  //this->radiationTypeChanged(0);

  /* Remove all tabs in Beam TabWidget */
  d->tabWidget->clear ();

  // Handle scene change event if occurs
  qvtkConnect( d->logic(), vtkCommand::ModifiedEvent, this, SLOT( onLogicModified() ) );
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::setExternalBeamPlanningNode(vtkMRMLNode *node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  vtkMRMLExternalBeamPlanningNode* paramNode = vtkMRMLExternalBeamPlanningNode::SafeDownCast(node);

  qDebug() << "qSlicerExternalBeamPlanningModuleWidget::setExternalBeamPlanningNode" << " " << node << " +\n";

  // Each time the node is modified, the qt widgets are updated
  qvtkReconnect( d->logic()->GetExternalBeamPlanningNode(), paramNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()) );

  d->logic()->SetAndObserveExternalBeamPlanningNode(paramNode);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::onLogicModified()
{
  this->updateRTBeamTableWidget();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget: Invalid parameter node!";
    return;
  }

  printf ("*** Updating RT BEAM TABLE ***\n");

  // Clear the beam table
  d->tableWidget_Beams->setRowCount(0);
  d->tableWidget_Beams->setColumnCount(0);
  d->tableWidget_Beams->clearContents();

  // Get rt plan node for ExternalBeamPlanning node
  vtkMRMLRTPlanNode* rtPlanNode = d->logic()->GetExternalBeamPlanningNode()->GetRtPlanNode();
  if (!rtPlanNode)
  { 
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget: Invalid rtplan node!";
    return;
  }
  vtkSmartPointer<vtkCollection> beams = vtkSmartPointer<vtkCollection>::New();
  rtPlanNode->GetRTBeamNodes(beams);
  
  // Go through each RT beam node
  beams->InitTraversal();
  if (beams->GetNumberOfItems() < 1)
  {
    qWarning() << "qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget: Selected rtplan node has no children rtbeam nodes!";
    return;
  }

  // Set up the table
  d->tableWidget_Beams->setColumnCount(4);
  QStringList headerLabels;
  headerLabels << "Beam#" << "Name" << "Type" << "Radiation";

  d->tableWidget_Beams->setColumnWidth(0, 24);
  d->tableWidget_Beams->setHorizontalHeaderLabels(headerLabels);
  d->tableWidget_Beams->setRowCount(beams->GetNumberOfItems());

  // Fill the table
  for (int i=0; i<beams->GetNumberOfItems(); ++i)
  {
    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(beams->GetItemAsObject(i));
    if (beamNode)
    {
      d->tableWidget_Beams->setItem(i, 1, new QTableWidgetItem( QString(beamNode->GetBeamName()) ) );
    }
  }
  if (d->currentBeamRow >=0)
  {
    d->tableWidget_Beams->selectRow(d->currentBeamRow);
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::referenceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::referenceVolumeNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveReferenceVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::planContourSetNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::planContoursNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObservePlanSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::rtPlanNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::rtPlanNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveRtPlanNode(vtkMRMLRTPlanNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::rtDoseVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::planContoursNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  vtkMRMLRTPlanNode* rtPlanNode = paramNode->GetRtPlanNode();
  if (!rtPlanNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::isocenterFiducialNodeChanged: Invalid rtplan node!";
    return;
  }

  rtPlanNode->DisableModifiedEventOn();
  rtPlanNode->SetAndObserveRTPlanDoseVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
  rtPlanNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::rtDoseROINodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::planContoursNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  // paramNode->DisableModifiedEventOn();
  // paramNode->SetAndObservePlanContourSetNode(node);
  // paramNode->DisableModifiedEventOff();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::doseGridSpacingChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::doseGridSpacingChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::doseEngineTypeChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  vtkMRMLRTPlanNode* rtPlanNode = paramNode->GetRtPlanNode();
  if (!rtPlanNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::isocenterFiducialNodeChanged: Invalid rtplan node!";
    return;
  }

  if (text.compare("Plastimatch") == 0)
  {
    rtPlanNode->SetRTPlanDoseEngine(vtkMRMLRTPlanNode::Plastimatch);
  }
  else if (text.compare("PMH") == 0)
  {
    rtPlanNode->SetRTPlanDoseEngine(vtkMRMLRTPlanNode::PMH);
  }
  else if (text.compare("Matlab") == 0)
  {
    rtPlanNode->SetRTPlanDoseEngine(vtkMRMLRTPlanNode::Matlab);
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::addBeamClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::addBeamClicked Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    qWarning() << "qSlicerExternalBeamPlanningModuleWidget::addBeamClicked without parameter node";
    return;
  }

  // GCS FIX

  // --------- Future algorithm ----------------
  // Step 1. Find out which beam is currently highlighted
  // Step 2. Replicate beam node (or create from default if none highlighted)
  // Step 3. Update UI
  // Step 4. Add node to scene

  // --------- Current algorithm ---------------
  // Step 1. Create node and add to scene
  // Step 2. Update UI

  // Create node and add to scene
  vtkMRMLRTBeamNode *beamNode = d->logic()->AddBeam();

  // Update UI
  this->updateWidgetFromRTBeam (beamNode);

  // Make new beam current in the table
  d->currentBeamRow = d->totalBeamRows;
  d->totalBeamRows ++;
  d->tableWidget_Beams->selectRow(d->currentBeamRow);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::removeBeamClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  QTableWidgetItem *item = NULL;
  char beamName[100];
  item = d->tableWidget_Beams->item(d->currentBeamRow, 1);
  if (!item)
  {
    return;
  }
  else
  {
    strcpy(beamName, item->text().toStdString().c_str());
    d->logic()->RemoveBeam(beamName);
    d->totalBeamRows--;
    d->currentBeamRow = d->totalBeamRows-1;
    d->tableWidget_Beams->selectRow(d->currentBeamRow);
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::tableWidgetCellClicked(int row, int column)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(column);

  /* Don't do anything when clicking on selected beam */
  if (row == d->currentBeamRow) {
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }

  // Make sure inputs are initialized
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode)
  {
    qCritical() << "tableWidgetItemClicked: Inputs are not initialized!";
    return;
  }

  // Copy data from beam into tabs UI
  this->updateWidgetFromRTBeam (beamNode);

#if defined (commentout)
  /* GCS FIX TODO -- use vtkRTBeamData::Copy() */
  paramNode->SetBeamName(beamNode->GetBeamData()->GetBeamName());

  paramNode->SetX1Jaw(beamNode->GetX1Jaw());
  paramNode->SetX2Jaw(beamNode->GetX2Jaw());
  paramNode->SetY1Jaw(beamNode->GetY1Jaw());
  paramNode->SetY2Jaw(beamNode->GetY2Jaw());
  paramNode->SetAndObserveMLCPositionDoubleArrayNode(beamNode->GetMLCPositionDoubleArrayNode());
  paramNode->SetAndObserveIsocenterFiducialNode(beamNode->GetIsocenterFiducialNode());
  paramNode->SetAndObserveTargetSegmentationNode(beamNode->GetTargetSegmentationNode());

  paramNode->SetGantryAngle(beamNode->GetGantryAngle());
  paramNode->SetCollimatorAngle(beamNode->GetCollimatorAngle());
  paramNode->SetCouchAngle(beamNode->GetCouchAngle());

  paramNode->SetNominalEnergy(beamNode->GetNominalEnergy());
  paramNode->SetNominalmA(beamNode->GetNominalmA());
  paramNode->SetBeamOnTime(beamNode->GetBeamOnTime());
  paramNode->SetBeamFlavor(beamNode->GetBeamFlavor());
  paramNode->SetBeamWeight(beamNode->GetBeamWeight());
  paramNode->SetEnergyResolution(beamNode->GetEnergyResolution());

  paramNode->SetApertureOffset(beamNode->GetApertureOffset());
  paramNode->SetApertureSpacingAtIso(beamNode->GetApertureSpacingAtIso());
  paramNode->SetSourceSize(beamNode->GetSourceSize());
  paramNode->SetSAD(beamNode->GetSAD());

  paramNode->SetSmearing(beamNode->GetSmearing());
  paramNode->SetProximalMargin(beamNode->GetProximalMargin());
  paramNode->SetDistalMargin(beamNode->GetDistalMargin());
#endif

  /* To be implemented */
  //paramNode->SetRadiationType
  //paramNode->SetBeamType

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::beamNameChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::beamNameChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetBeamName(text.toStdString().c_str());

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetBeamName(text.toStdString().c_str());

  // Update in table
  this->updateRTBeamTableWidget();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::radiationTypeChanged(int index)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (index == -1)
  {
    return;
  }


  // GCS FIX: This needs to make changes to the node, then set 
  // values from node into UI

#if defined (commentout)
  QString text = d->comboBox_RadiationType->currentText();

  printf ("qSEBP::radiationTypeChanged (%s)\n",
    text.toUtf8().data());

  if (text == "Photon")
  {
    d->label_NominalEnergy->setEnabled (true);
    d->comboBox_NominalEnergy->setEnabled (true);
    d->label_BeamOnTime->setEnabled (true);
    d->lineEdit_BeamOnTime->setEnabled (true);
    d->label_NominalmA->setEnabled (true);
    d->lineEdit_NominalmA->setEnabled (true);
    //d->label_CollimatorType->setEnabled (true);
    //d->comboBox_CollimatorType->setEnabled (true);

    // Make Photon pages visible and others invisible
    int index =-1;
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonBeamModel);
    if (index >= 0)
    {
      d->tabWidget->removeTab(index);
    }
    index = 0;
    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonGeometry);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPagePhotonGeometry, "Geometry");
    }
    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonBeamModel);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPagePhotonBeamModel, "Beam Model");
    }
  }
  else if (text == "Proton")
  {
    d->label_NominalEnergy->setEnabled (false);
    d->comboBox_NominalEnergy->setEnabled (false);
    d->label_BeamOnTime->setEnabled (false);
    d->lineEdit_BeamOnTime->setEnabled (false);
    d->label_NominalmA->setEnabled (false);
    d->lineEdit_NominalmA->setEnabled (false);
    //d->label_CollimatorType->setEnabled (false);
    //d->comboBox_CollimatorType->setEnabled (false);

    // Make Photon pages visible and others invisible
    int index =-1;
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonGeometry);
    if (index >=0)
    {
      d->tabWidget->removeTab(index);
    }
    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonBeamModel);
    if (index >= 0)
    {
      d->tabWidget->removeTab(index);
    }
    index = 0;
    index = d->tabWidget->indexOf(d->tabWidgetPagePhotonGeometry);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPagePhotonGeometry, "Geometry");
    }
    index = d->tabWidget->indexOf(d->tabWidgetPageProtonBeamModel);
    if (index == -1)
    {
      d->tabWidget->addTab(d->tabWidgetPageProtonBeamModel, "Beam Model");
    }

  }
  else if (text == "Electron")
  {
    d->label_NominalEnergy->setEnabled (false);
    d->comboBox_NominalEnergy->setEnabled (false);
    d->label_BeamOnTime->setEnabled (false);
    d->lineEdit_BeamOnTime->setEnabled (false);
    d->label_NominalmA->setEnabled (false);
    d->lineEdit_NominalmA->setEnabled (false);
    //d->label_CollimatorType->setEnabled (false);
    //d->comboBox_CollimatorType->setEnabled (false);
  }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::beamTypeChanged(const QString &text)
{
  //Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  //TODO:
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::targetContourNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::targetContourNodeChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveTargetSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetAndObserveTargetSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::targetContourSegmentChanged(const QString& segment)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::targetContourSegmentChanged: Invalid scene!";
    return;
  }

  // Convert QString to std::string
  std::string s = segment.toUtf8().constData();
  
  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetTargetSegmentID (s.c_str());

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetTargetSegmentID (s.c_str());
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::RxDoseChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonRxDoseChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetRxDose(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::isocenterFiducialNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::IsocenterFiducialNodeChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node) {
    return;
  }
  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetAndObserveIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::dosePointFiducialNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::IsocenterFiducialNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::nominalEnergyChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::nominalEnergyChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::nominalmAChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::nominalmAChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::beamOnTimeChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::beamOnTimeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonSmearingChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonSmearingChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetSmearing(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetSmearing(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonProximalMarginChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonProximalMarginChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetProximalMargin(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetProximalMargin(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonDistalMarginChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonDistalMarginChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetDistalMargin(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetDistalMargin(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::gantryAngleChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::gantryAngleChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetGantryAngle(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetGantryAngle(value);

  // Update beam visualization
  this->UpdateBeamTransform();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::collimatorAngleChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::collimatorAngleChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetCollimatorAngle(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetCollimatorAngle(value);

  // Update beam visualization
  this->UpdateBeamTransform();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::couchAngleChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::couchAngleChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetCouchAngle(value);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetCouchAngle(value);

  // Update beam visualization
  this->UpdateBeamTransform();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::XJawsPositionValuesChanged(double minVal, double maxVal)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::XJawPositionsValueChanged: Invalid scene!";
    return;
  }

  // Update in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::XJawPositionsValueChanged: Invalid module parameter!";
    return;
  }
  paramNode->SetX1Jaw(-minVal);
  paramNode->SetX2Jaw( maxVal);

  // Update in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetX1Jaw(-minVal);
  beamNode->SetX2Jaw( maxVal);

  // Update beam visualization
  this->UpdateBeamGeometryModel();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::YJawsPositionValuesChanged(double minVal, double maxVal)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::YJawPositionsValueChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }
  paramNode->SetY1Jaw(-minVal);
  paramNode->SetY2Jaw( maxVal);

  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);

  // Make sure inputs are initialized
  if (!beamNode)
  {
    qCritical() << "YJawsPositionValuesChanged: Inputs are not initialized!";
    return;
  }

  beamNode->SetY1Jaw(-minVal);
  beamNode->SetY2Jaw( maxVal);

  // Update beam visualization
  this->UpdateBeamGeometryModel();

  return;
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::MLCPositionDoubleArrayNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::MLCPositionDoubleArrayNodeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode || !node)
  {
    return;
  }

  // Get rt plan node for ExternalBeamPlanning node
  vtkMRMLRTPlanNode* rtPlanNode = d->logic()->GetExternalBeamPlanningNode()->GetRtPlanNode();
  if (!rtPlanNode)
  { 
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::updateRTBeamTableWidget: Invalid rtplan node!";
    return;
  }

  paramNode->DisableModifiedEventOn();
  paramNode->SetAndObserveMLCPositionDoubleArrayNode(vtkMRMLDoubleArrayNode::SafeDownCast(node));
  paramNode->DisableModifiedEventOff();

  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);

  // Make sure inputs are initialized
  if (!beamNode)
  {
    qCritical() << "couchAngleChanged: Inputs are not initialized!";
    return;
  }

  beamNode->SetAndObserveMLCPositionDoubleArrayNode(vtkMRMLDoubleArrayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::beamWeightChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::beamWeightChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }
  paramNode->SetBeamWeight(value);

  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);

  // Make sure inputs are initialized
  if (!beamNode)
  {
    qCritical() << "beamWeightChanged: Inputs are not initialized!";
    return;
  }

  beamNode->SetBeamWeight(value);

  return;
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonSourceDistanceChanged(double value)
{
   Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonSourceDistanceChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetSAD(value);

  // Set in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode)
  {
    qCritical() << "protonSourceDistanceChanged: Inputs are not initialized!";
    return;
  }
  beamNode->SetSAD(value);

  // Update beam visualization
  this->UpdateBeamGeometryModel();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonSourceSizeChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonSourceSizeChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetSourceSize(value);

  // Set in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode)
  {
    qCritical() << "protonSourceSizeChanged: Inputs are not initialized!";
    return;
  }
  beamNode->SetSourceSize(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonApertureOffsetChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonApertureOffsetChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetApertureOffset(value);

  // Make sure inputs are initialized
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetApertureOffset(value);

  // Update beam visualization
  this->UpdateBeamGeometryModel();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonEnergyResolutionChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonEnergyResolutionChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }
  paramNode->SetEnergyResolution(value);


  // Set in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    qCritical() << "protonEnergyResolutionChanged: Inputs are not initialized!";
    return;
  }
  beamNode->SetEnergyResolution(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonDoseResolutionChanged(double value)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonSpacingAtIsoChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) 
  {
    return;
  }
  paramNode->SetApertureSpacingAtIso(value);

  // Set in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) 
  {
    return;
  }
  beamNode->SetApertureSpacingAtIso(value);
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonEnergySpreadChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonEnergySpreadChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) 
  {
    return;
  }

  // TODO: to be implemented
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::protonBeamFlavorChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::protonBeamFlavorChanged: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) {
    return;
  }

  if(text == "Ray Tracer Algorithm")
  {
    paramNode->SetBeamFlavor('a');
    d->label_CalculateDoseStatus->setText("Ray Tracer Algorithm");
  }
  else if(text == "Cartesian Geometry Algorithm")
  {
    paramNode->SetBeamFlavor('f');
    d->label_CalculateDoseStatus->setText("Cartesian Geometry Algorithm");
  }
  else if(text == "Divergent Geometry Algorithm")
  {
    paramNode->SetBeamFlavor('g');
    d->label_CalculateDoseStatus->setText("Divergent Geometry Algorithm");
  }
  else if(text == "Modified Hong Algorithm")
  {
    paramNode->SetBeamFlavor('h');
    d->label_CalculateDoseStatus->setText("Modified Hong Algorithm");
  }
  else
  {
    paramNode->SetBeamFlavor('a');
    d->label_CalculateDoseStatus->setText("Algorithm error, Ray Tracer chosen by default");
  }

  // Set in beam node
  vtkMRMLRTBeamNode* beamNode = this->getCurrentBeamNode(paramNode);
  if (!beamNode) {
    return;
  }
  beamNode->SetBeamFlavor(paramNode->GetBeamFlavor());
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::beamEyesViewClicked(bool checked)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::beamEyesViewClicked: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) 
  {
    return;
  }

  if (checked)
  {
    qSlicerApplication::application()->layoutManager()->setLayout(vtkMRMLLayoutNode::SlicerLayoutTwoOverTwoView);
  }
  else
  {
    qSlicerApplication::application()->layoutManager()->setLayout(vtkMRMLLayoutNode::SlicerLayoutFourUpView);
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::contoursInBEWClicked(bool checked)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::contoursInBEWClicked: Invalid scene!";
    return;
  }

  // Set in parameter node
  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode) 
  {
    return;
  }

  // Todo: add the logic to check if contours should be included in the DRR view
  // right now the contours are included always. 
  if (checked)
  {
  }
  else
  {
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::updateDRRClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::updateDRRClicked: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  QTableWidgetItem *item = NULL;
  char beamName[100];
  item = d->tableWidget_Beams->item(d->currentBeamRow, 1);

  if (item)
  {
    strcpy(beamName, item->text().toStdString().c_str());
    d->logic()->UpdateDRR(beamName);
  }
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::calculateWEDClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  d->label_CalculateDoseStatus->setText("Starting WED calculation...");

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::calculateWEDClicked: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }

  /* Make sure inputs were specified - only CT needed*/
  vtkMRMLScalarVolumeNode* referenceVolume = paramNode->GetReferenceVolumeNode();
  if (!referenceVolume)
  {
    d->label_CalculateDoseStatus->setText("No reference image");
    return;
  }

  /* Copy pertinent variable values from GUI to logic */
  /* Is this the right place for this? */
  d->logic()->GetExternalBeamPlanningNode()->SetGantryAngle(d->SliderWidget_PhotonGantryAngle->value());

  /* OK, we're good to go (well, not really, but let's pretend). 
     Do the actual computation in the logic object */
  d->logic()->ComputeWED();

  d->label_CalculateDoseStatus->setText("WED calculation done.");
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::calculateDoseClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  d->label_CalculateDoseStatus->setText("Starting dose calculation...");

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::calculateDoseClicked: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::calculateDoseClicked: Invalid paramNode!";
    return;
  }

  /* Make sure inputs were specified */
  vtkMRMLScalarVolumeNode* referenceVolume = paramNode->GetReferenceVolumeNode();
  if (!referenceVolume)
  {
    d->label_CalculateDoseStatus->setText("No reference image");
    return;
  }

  vtkMRMLSegmentationNode* segmentationNode = paramNode->GetTargetSegmentationNode();
  if (!segmentationNode)
  {
    d->label_CalculateDoseStatus->setText("No proton target volume");
    return;
  }

  /* Make sure segmentation has a labelmap form */
  /* GCS FIX: Maybe this explicit conversion is not even needed */
#if defined (commentout)
  if ( !segmentationNode->HasMergedLabelmap() )
  {
    d->label_CalculateDoseStatus->setText("Target volume must be labelmap");
    return;
  }
#endif
  
  /* The last verifications were fine so we can compute the dose */
  /* Dose Calculation - loop on all the beam and sum in a global dose matrix */

  // Get rt plan node for ExternalBeamPlanning node
  vtkMRMLRTPlanNode* rtPlanNode = d->logic()->GetExternalBeamPlanningNode()->GetRtPlanNode();
  if (!rtPlanNode)
  { 
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::calculateDoseClicked: Invalid rtplan node!";
    return;
  }

  vtkSmartPointer<vtkCollection> beams = vtkSmartPointer<vtkCollection>::New();
  rtPlanNode->GetRTBeamNodes(beams);
  if (!beams) 
  {
    d->label_CalculateDoseStatus->setText("no beam found in the plan");
    return;
  }
  vtkMRMLRTBeamNode* beamNode = NULL;

  printf ("Top of loop\n");
  d->logic()->InitializeAccumulatedDose();
  for (int i=0; i<beams->GetNumberOfItems(); ++i)
  {
    beamNode = vtkMRMLRTBeamNode::SafeDownCast(beams->GetItemAsObject(i));
    if (beamNode)
    {
      beamNode->UpdateApertureParameters(); // Just to be sure before to send the beam to be calculated
      d->logic()->ComputeDose(beamNode);
      d->label_CalculateDoseStatus->setText("Dose calculation in progress"); //+ beamNode->GetBeamName();
      // sum in the final dose matrix with weights externalbeam for RxDose, and beamNode for beam wieght
    }
    else
    {
      d->label_CalculateDoseStatus->setText("beam not found"); // + beamNode->GetBeamName();
    }
  }
  printf ("Bottom of loop\n");
  d->logic()->RegisterAccumulatedDose();
  d->label_CalculateDoseStatus->setText("Dose calculation done.");
  return;
  
  /* just check which beam is selected for calculation
     QTableWidgetItem *item = NULL;
     char beamName[100];
     item = d->tableWidget_Beams->item(d->currentBeamRow, 1);
     if (!item)
     {
     return;
     }
     else 
     {
     strcpy(beamName, item->text().toStdString().c_str());
     // OK, we're good to go (well, not really, but let's pretend). 
     Do the actual computation in the logic object
     d->logic()->ComputeDose(beamName);
     d->label_CalculateDoseStatus->setText("Dose calculation done.");
     } */
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::clearDoseClicked()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::clearDoseClicked: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::clearDoseClicked: Invalid paramNode!";
    return;
  }

  d->logic()->RemoveDoseNodes();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::updateBeamParameters()
{
  //Q_D(qSlicerExternalBeamPlanningModuleWidget);

  // TODO:
  std::string errorMessage;
  //d->logic()->UpdateBeam();
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::UpdateBeamTransform()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  QTableWidgetItem *item = NULL;
  char beamName[100];
  item = d->tableWidget_Beams->item(d->currentBeamRow, 1);
  if (!item)
  {
    return;
  }
  else 
  {
    strcpy(beamName, item->text().toStdString().c_str());
    d->logic()->UpdateBeamTransform(beamName);
  }
  return;
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::UpdateBeamGeometryModel()
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);

  QTableWidgetItem *item = NULL;
  char beamName[100];
  item = d->tableWidget_Beams->item(d->currentBeamRow, 1);
  if (!item)
  {
    return;
  }
  else 
  {
    strcpy(beamName, item->text().toStdString().c_str());
    d->logic()->UpdateBeamGeometryModel(beamName);
  }
  return;
}

//-----------------------------------------------------------------------------
void qSlicerExternalBeamPlanningModuleWidget::collimatorTypeChanged(const QString &text)
{
  Q_D(qSlicerExternalBeamPlanningModuleWidget);
  UNUSED_VARIABLE(text);

  if (!this->mrmlScene())
  {
    qCritical() << "qSlicerExternalBeamPlanningModuleWidget::collimatorTypeChanged: Invalid scene!";
    return;
  }

  vtkMRMLExternalBeamPlanningNode* paramNode = d->logic()->GetExternalBeamPlanningNode();
  if (!paramNode)
  {
    return;
  }
  
  //TODO:
}
