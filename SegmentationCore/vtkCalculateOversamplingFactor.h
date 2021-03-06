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

// .NAME vtkCalculateOversamplingFactor - Calculate oversampling factor based on model properties
// .SECTION Description

#ifndef __vtkCalculateOversamplingFactor_h
#define __vtkCalculateOversamplingFactor_h

// VTK includes
#include <vtkObject.h>
#include <vtkMassProperties.h>
#include <vtkPolyData.h>

// SegmentationCore includes
#include "vtkOrientedImageData.h"

#include "vtkSegmentationCoreConfigure.h"

class vtkPiecewiseFunction;

/// \ingroup SegmentationCore
/// \brief Calculate oversampling factor based on model properties using fuzzy logics
class vtkSegmentationCore_EXPORT vtkCalculateOversamplingFactor : public vtkObject
{
public:
  static vtkCalculateOversamplingFactor *New();
  vtkTypeMacro(vtkCalculateOversamplingFactor, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

public:
  /// Calculate oversampling factor for the input model and its rasterization reference volume
  /// based on model properties using fuzzy logics.
  bool CalculateOversamplingFactor();

  /// Apply oversampling factor on image data geometry.
  /// Changes spacing and extent of oversampling factor is not 1 (and sensible)
  static void ApplyOversamplingOnImageGeometry(vtkOrientedImageData* imageData, double oversamplingFactor);

protected:
  /// Calculate relative structure size from input model and rasterization reference volume
  double CalculateRelativeStructureSize();

  /// Calculate complexity measure based on surface poly data in input model
  double CalculateComplexityMeasure();

  /// Use fuzzy rules to determine oversampling factor based on relative structure size and complexity measure
  /// \param relativeStructureSize Relative structure size calculated by \sa CalculateRelativeStructureSize
  /// \param complexityMeasure Complexity measure calculated by \sa CalculateComplexityMeasure
  /// \return Automatically calculated oversampling factor
  double DetermineOversamplingFactor(double relativeStructureSize, double complexityMeasure);

  /// Clip a membership function with the clip value
  /// This means that the values of the membership function will be maximized at the clip value,
  /// while the function remains the same otherwise (0 values, slopes).
  /// \param membershipFunction Membership function to clip
  /// \param clipValue Clip value
  void ClipMembershipFunction(vtkPiecewiseFunction* membershipFunction, double clipValue);

public:
  vtkGetObjectMacro(InputPolyData, vtkPolyData);
  vtkSetObjectMacro(InputPolyData, vtkPolyData);

  vtkGetObjectMacro(ReferenceGeometryImageData, vtkOrientedImageData);
  vtkSetObjectMacro(ReferenceGeometryImageData, vtkOrientedImageData);

  vtkGetMacro(OutputOversamplingFactor, double);
  vtkSetMacro(OutputOversamplingFactor, double);

  vtkGetMacro(LogSpeedMeasurements, bool);
  vtkSetMacro(LogSpeedMeasurements, bool);
  vtkBooleanMacro(LogSpeedMeasurements, bool);

protected:
  vtkGetObjectMacro(MassPropertiesAlgorithm, vtkMassProperties);
  vtkSetObjectMacro(MassPropertiesAlgorithm, vtkMassProperties);

protected:
  /// Input poly data to rasterize
  vtkPolyData* InputPolyData;

  /// Image containing the rasterization reference geometry
  vtkOrientedImageData* ReferenceGeometryImageData;

  /// Calculated oversampling factor for the segmentation node and its reference volume
  double OutputOversamplingFactor;

  /// Flag telling whether the speed measurements are logged on standard output
  bool LogSpeedMeasurements;

  /// Temporary storage for mass properties algorithm that is used in both sub-calculations
  /// \sa CalculateRelativeStructureSize and CalculateComplexityMeasure
  vtkMassProperties* MassPropertiesAlgorithm;
  
protected:
  vtkCalculateOversamplingFactor();
  virtual ~vtkCalculateOversamplingFactor();

private:
  vtkCalculateOversamplingFactor(const vtkCalculateOversamplingFactor&); // Not implemented
  void operator=(const vtkCalculateOversamplingFactor&);               // Not implemented
  //ETX
};

#endif 