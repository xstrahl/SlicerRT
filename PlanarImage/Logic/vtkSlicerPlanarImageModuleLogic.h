/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// .NAME vtkSlicerPlanarImageModuleLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerPlanarImageModuleLogic_h
#define __vtkSlicerPlanarImageModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerPlanarImageModuleLogicExport.h"

class vtkMRMLPlanarImageNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLModelNode;
class vtkPoints;

/// \ingroup SlicerRt_QtModules_PlanarImage
class VTK_SLICER_PLANARIMAGE_LOGIC_EXPORT vtkSlicerPlanarImageModuleLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerPlanarImageModuleLogic *New();
  vtkTypeMacro(vtkSlicerPlanarImageModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

public:
  /// Show planar image as a rectangular model with the input volume as the texture
  void CreateModelForPlanarImage(vtkMRMLPlanarImageNode* planarImageNode);

protected:
  /// Compute image plane corners in the world coordinate system based on the transforms related to the planar image
  void ComputeImagePlaneCorners(vtkMRMLScalarVolumeNode* planarImageVolume, vtkPoints* sliceCornerPoints);

  /// Set texture to the planar image model node according to the current display state of the volume to display.
  /// Creates the texture if does not exist, update otherwise
  void SetTextureForPlanarImage(vtkMRMLScalarVolumeNode* planarImageVolumeNode, vtkMRMLModelNode* displayedModelNode, vtkMRMLScalarVolumeNode* textureVolumeNode);

protected:
  vtkSlicerPlanarImageModuleLogic();
  virtual ~vtkSlicerPlanarImageModuleLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  virtual void RegisterNodes();
  virtual void OnMRMLSceneEndImport();

  /// Update texture if display properties change in an observed planar image volume node
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

private:
  vtkSlicerPlanarImageModuleLogic(const vtkSlicerPlanarImageModuleLogic&); // Not implemented
  void operator=(const vtkSlicerPlanarImageModuleLogic&);               // Not implemented
};

#endif
