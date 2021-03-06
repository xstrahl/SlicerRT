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

#ifndef __qSlicerExternalBeamPlanningModuleWidget_h
#define __qSlicerExternalBeamPlanningModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerExternalBeamPlanningModuleExport.h"

class qSlicerExternalBeamPlanningModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLExternalBeamPlanningNode;
class vtkMRMLRTBeamNode;
class QString;
class QTableWidgetItem;

/// \ingroup SlicerRt_QtModules_ExternalBeamPlanning
class Q_SLICER_QTMODULES_EXTERNALBEAMPLANNING_EXPORT qSlicerExternalBeamPlanningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerExternalBeamPlanningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerExternalBeamPlanningModuleWidget();

  virtual void enter();

public slots:
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

  /// Process loaded scene
  void onSceneImportedEvent();

  /// Set current parameter node
  void setExternalBeamPlanningNode(vtkMRMLNode*);

  /// Update widget GUI from parameter node
  void updateWidgetFromMRML();

protected slots:
  ///
  void onLogicModified();

  // RT plan page
  void referenceVolumeNodeChanged(vtkMRMLNode*);
  void planContourSetNodeChanged(vtkMRMLNode*);
  void rtPlanNodeChanged(vtkMRMLNode*);
  void rtDoseVolumeNodeChanged(vtkMRMLNode*);
  void rtDoseROINodeChanged(vtkMRMLNode*);
  void doseGridSpacingChanged(const QString &);
  void doseEngineTypeChanged(const QString &);
  
  // RT Beams page
  void tableWidgetCellClicked(int, int);
  void addBeamClicked();
  void removeBeamClicked();

  /* Beam global parameters */
  void beamNameChanged(const QString &);
  void radiationTypeChanged(int);

  /* Prescription page */
  void beamTypeChanged(const QString &);
  void targetContourNodeChanged(vtkMRMLNode* node);
  void targetContourSegmentChanged(const QString& segment);
  void RxDoseChanged(double);
  void isocenterFiducialNodeChanged(vtkMRMLNode*);
  void dosePointFiducialNodeChanged(vtkMRMLNode*);
  void nominalEnergyChanged(const QString&);
  void nominalmAChanged(const QString &);
  void beamOnTimeChanged(const QString &);

  /* Energy page */
  void protonSmearingChanged(double);
  void protonProximalMarginChanged(double);
  void protonDistalMarginChanged(double);

  /* Proton Geometry page */
  // void gantryAngleChanged(double);
  // void collimatorAngleChanged(double);
  // void couchAngleChanged(double);
  void beamWeightChanged(double);

  /* Photon Geometry page */
  void gantryAngleChanged(double);
  void collimatorAngleChanged(double);
  void couchAngleChanged(double);
  void MLCPositionDoubleArrayNodeChanged(vtkMRMLNode* node);
  void XJawsPositionValuesChanged(double, double);
  void YJawsPositionValuesChanged(double, double);

  /* Proton beam model */
  void protonSourceDistanceChanged(double);
  void protonSourceSizeChanged(double);
  void protonApertureOffsetChanged(double);
  void protonEnergyResolutionChanged(double);
  void protonDoseResolutionChanged(double);
  void protonEnergySpreadChanged(const QString &);
  void protonBeamFlavorChanged(const QString &);
  //void protonSpacingAtIsoChanged(const QString &);

  /* Photon beam model */

  /* Beam visualization */
  void updateDRRClicked();
  void beamEyesViewClicked(bool);
  void contoursInBEWClicked(bool);
  
  /* Calculation buttons */
  void calculateDoseClicked();
  void calculateWEDClicked();
  void clearDoseClicked();

  void collimatorTypeChanged(const QString &);

protected:
  ///
  virtual void setup();

  ///
  void onEnter();

  /// Utility functions
  vtkMRMLExternalBeamPlanningNode* getExternalBeamPlanningNode ();
  vtkMRMLRTBeamNode* getCurrentBeamNode (vtkMRMLExternalBeamPlanningNode*);
  vtkMRMLRTBeamNode* getCurrentBeamNode();
  std::string getCurrentBeamName ();

  ///
  void updateRTBeamTableWidget();

  /// Update widget GUI from a beam node
  void updateWidgetFromRTBeam (const vtkMRMLRTBeamNode* beamNode);

  ///
  void updateBeamParameters();

  ///
  void UpdateBeamTransform();

  ///
  void UpdateBeamGeometryModel();

protected:
  QScopedPointer<qSlicerExternalBeamPlanningModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExternalBeamPlanningModuleWidget);
  Q_DISABLE_COPY(qSlicerExternalBeamPlanningModuleWidget);
};

#endif
