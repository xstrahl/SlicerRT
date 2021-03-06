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

// MRMLDoseAccumulation includes
#include "vtkMRMLDoseAccumulationNode.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
static const char* REFERENCE_DOSE_VOLUME_REFERENCE_ROLE = "referenceDoseVolumeRef";
static const char* ACCUMULATED_DOSE_VOLUME_REFERENCE_ROLE = "accumulatedDoseVolumeRef";
static const char* SELECTED_INPUT_VOLUME_REFERENCE_ROLE = "selectedInputVolumeRef";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLDoseAccumulationNode);

//----------------------------------------------------------------------------
vtkMRMLDoseAccumulationNode::vtkMRMLDoseAccumulationNode()
{
  this->ShowDoseVolumesOnly = true;
  this->VolumeNodeIdsToWeightsMap.clear();

  this->HideFromEditors = false;
}

//----------------------------------------------------------------------------
vtkMRMLDoseAccumulationNode::~vtkMRMLDoseAccumulationNode()
{
  this->VolumeNodeIdsToWeightsMap.clear();
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  of << indent << " ShowDoseVolumesOnly=\"" << (this->ShowDoseVolumesOnly ? "true" : "false") << "\"";

  {
    of << indent << " VolumeNodeIdsToWeightsMap=\"";
    for (std::map<std::string,double>::iterator it = this->VolumeNodeIdsToWeightsMap.begin(); it != this->VolumeNodeIdsToWeightsMap.end(); ++it)
      {
      of << it->first << ":" << it->second << "|";
      }
    of << "\"";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "ShowDoseVolumesOnly")) 
      {
      this->ShowDoseVolumesOnly = 
        (strcmp(attValue,"true") ? false : true);
      }
    else if (!strcmp(attName, "VolumeNodeIdsToWeightsMap")) 
      {
      std::stringstream ss;
      ss << attValue;
      std::string valueStr = ss.str();
      std::string separatorCharacter("|");

      this->VolumeNodeIdsToWeightsMap.clear();
      size_t separatorPosition = valueStr.find( separatorCharacter );
      while (separatorPosition != std::string::npos)
        {
        std::string mapPairStr = valueStr.substr(0, separatorPosition);
        size_t colonPosition = mapPairStr.find( ":" );
        if (colonPosition == std::string::npos)
          {
          continue;
          }
        std::string volumeNodeId = mapPairStr.substr(0, colonPosition);

        double weight;
        std::stringstream vss;
        vss << mapPairStr.substr( colonPosition+1 );
        vss >> weight;

        this->VolumeNodeIdsToWeightsMap[volumeNodeId] = weight;
        valueStr = valueStr.substr( separatorPosition+1 );
        separatorPosition = valueStr.find( separatorCharacter );
        }
      if (! valueStr.empty() )
        {
        std::string mapPairStr = valueStr.substr(0, separatorPosition);
        size_t colonPosition = mapPairStr.find( ":" );
        if (colonPosition != std::string::npos)
          {
          std::string volumeNodeId = mapPairStr.substr(0, colonPosition);

          double weight;
          std::stringstream vss;
          vss << mapPairStr.substr( colonPosition+1 );
          vss >> weight;

          this->VolumeNodeIdsToWeightsMap[volumeNodeId] = weight;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLDoseAccumulationNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  this->DisableModifiedEventOn();

  vtkMRMLDoseAccumulationNode *node = (vtkMRMLDoseAccumulationNode *) anode;

  this->SetShowDoseVolumesOnly(node->ShowDoseVolumesOnly);

  this->VolumeNodeIdsToWeightsMap = node->VolumeNodeIdsToWeightsMap;

  this->DisableModifiedEventOff();
  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ShowDoseVolumesOnly:   " << (this->ShowDoseVolumesOnly ? "true" : "false") << "\n";

  {
    os << indent << "VolumeNodeIdsToWeightsMap:   ";
    for (std::map<std::string,double>::iterator it = this->VolumeNodeIdsToWeightsMap.begin(); it != this->VolumeNodeIdsToWeightsMap.end(); ++it)
      {
      os << it->first << ":" << it->second << "|";
      }
    os << "\n";
  }
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLDoseAccumulationNode::GetReferenceDoseVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(REFERENCE_DOSE_VOLUME_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::SetAndObserveReferenceDoseVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(REFERENCE_DOSE_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLDoseAccumulationNode::GetAccumulatedDoseVolumeNode()
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNodeReference(ACCUMULATED_DOSE_VOLUME_REFERENCE_ROLE) );
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::SetAndObserveAccumulatedDoseVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->SetNodeReferenceID(ACCUMULATED_DOSE_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkMRMLDoseAccumulationNode::GetNthSelectedInputVolumeNode(unsigned int index)
{
  return vtkMRMLScalarVolumeNode::SafeDownCast( this->GetNthNodeReference(SELECTED_INPUT_VOLUME_REFERENCE_ROLE, index) );
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLDoseAccumulationNode::GetNumberOfSelectedInputVolumeNodes()
{
  return this->GetNumberOfNodeReferences(SELECTED_INPUT_VOLUME_REFERENCE_ROLE);
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::AddSelectedInputVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  this->AddNodeReferenceID(SELECTED_INPUT_VOLUME_REFERENCE_ROLE, (node ? node->GetID() : NULL));
}

//----------------------------------------------------------------------------
void vtkMRMLDoseAccumulationNode::RemoveSelectedInputVolumeNode(vtkMRMLScalarVolumeNode* node)
{
  for (unsigned int referenceIndex=0; referenceIndex<this->GetNumberOfSelectedInputVolumeNodes(); ++referenceIndex)
  {
    if (this->GetNthNodeReference(SELECTED_INPUT_VOLUME_REFERENCE_ROLE, referenceIndex) == node)
    {
      this->RemoveNthNodeReferenceID(SELECTED_INPUT_VOLUME_REFERENCE_ROLE, referenceIndex);
      break;
    }
  }
}
