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

// MRMLPlanarImage includes
#include "vtkMRMLPlanarImageNode.h"

// SlicerRt includes
#include "SlicerRtCommon.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLModelNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlanarImageNode);

//----------------------------------------------------------------------------
vtkMRMLPlanarImageNode::vtkMRMLPlanarImageNode()
{
  this->HideFromEditors = false;
}

//----------------------------------------------------------------------------
vtkMRMLPlanarImageNode::~vtkMRMLPlanarImageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLPlanarImageNode::GetRtImageVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    this->GetNodeReference(SlicerRtCommon::PLANARIMAGE_RT_IMAGE_VOLUME_REFERENCE_ROLE.c_str()) );
}

//----------------------------------------------------------------------------
void vtkMRMLPlanarImageNode::SetAndObserveRtImageVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(SlicerRtCommon::PLANARIMAGE_RT_IMAGE_VOLUME_REFERENCE_ROLE.c_str(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLPlanarImageNode::GetDisplayedModelNode()
{
  return vtkMRMLModelNode::SafeDownCast(
    this->GetNodeReference(SlicerRtCommon::PLANARIMAGE_DISPLAYED_MODEL_REFERENCE_ROLE.c_str()) );
}

//----------------------------------------------------------------------------
void vtkMRMLPlanarImageNode::SetAndObserveDisplayedModelNode(vtkMRMLModelNode* node)
{
  this->SetNodeReferenceID(SlicerRtCommon::PLANARIMAGE_DISPLAYED_MODEL_REFERENCE_ROLE.c_str(), (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLPlanarImageNode::GetTextureVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    this->GetNodeReference(SlicerRtCommon::PLANARIMAGE_TEXTURE_VOLUME_REFERENCE_ROLE.c_str()) );
}

//----------------------------------------------------------------------------
void vtkMRMLPlanarImageNode::SetAndObserveTextureVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(SlicerRtCommon::PLANARIMAGE_TEXTURE_VOLUME_REFERENCE_ROLE.c_str(), (node ? node->GetID() : NULL));
}
