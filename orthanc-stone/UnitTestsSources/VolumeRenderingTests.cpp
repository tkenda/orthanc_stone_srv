/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#include "../OrthancStone/Sources/Scene2D/CairoCompositor.h"
#include "../OrthancStone/Sources/Scene2D/ColorTextureSceneLayer.h"
#include "../OrthancStone/Sources/Scene2D/CopyStyleConfigurator.h"
#include "../OrthancStone/Sources/Scene2D/MacroSceneLayer.h"
#include "../OrthancStone/Sources/Scene2D/PolylineSceneLayer.h"
#include "../OrthancStone/Sources/Toolbox/SubvoxelReader.h"
#include "../OrthancStone/Sources/Volumes/DicomVolumeImageMPRSlicer.h"
#include "../OrthancStone/Sources/Volumes/DicomVolumeImageReslicer.h"

#include <Images/ImageProcessing.h>
#include <Images/ImageTraits.h>
#include <OrthancException.h>

#include <gtest/gtest.h>



static float GetPixelValue(const Orthanc::ImageAccessor& image,
                           unsigned int x,
                           unsigned int y)
{
  switch (image.GetFormat())
  {
    case Orthanc::PixelFormat_Grayscale8:
      return Orthanc::ImageTraits<Orthanc::PixelFormat_Grayscale8>::GetFloatPixel(image, x, y);
    
    case Orthanc::PixelFormat_Float32:
      return Orthanc::ImageTraits<Orthanc::PixelFormat_Float32>::GetFloatPixel(image, x, y);
    
    case Orthanc::PixelFormat_RGB24:
    {
      Orthanc::PixelTraits<Orthanc::PixelFormat_RGB24>::PixelType pixel;
      Orthanc::ImageTraits<Orthanc::PixelFormat_RGB24>::GetPixel(pixel, image, x, y);
      return pixel.red_;
    }
    
    case Orthanc::PixelFormat_BGRA32:
    {
      Orthanc::PixelTraits<Orthanc::PixelFormat_BGRA32>::PixelType pixel;
      Orthanc::ImageTraits<Orthanc::PixelFormat_BGRA32>::GetPixel(pixel, image, x, y);
      return pixel.red_;
    }
    
    default:
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
  }
}


static bool IsConstImage(float value,
                         const Orthanc::ImageAccessor& image)
{
  for (unsigned int y = 0; y < image.GetHeight(); y++)
  {
    for (unsigned int x = 0; x < image.GetWidth(); x++)
    {
      if (!OrthancStone::LinearAlgebra::IsNear(value, GetPixelValue(image, x, y)))
      {
        return false;
      }
    }
  }

  return true;
}
  

static bool IsConstRegion(float value,
                          const Orthanc::ImageAccessor& image,
                          unsigned int x,
                          unsigned int y,
                          unsigned int width,
                          unsigned int height)
{
  Orthanc::ImageAccessor region;
  image.GetRegion(region, x, y, width, height);
  return IsConstImage(value, region);
}


static bool IsConstImageWithExclusion(float value,
                                      const Orthanc::ImageAccessor& image,
                                      unsigned int exclusionX,
                                      unsigned int exclusionY,
                                      unsigned int exclusionWidth,
                                      unsigned int exclusionHeight)
{
  for (unsigned int y = 0; y < image.GetHeight(); y++)
  {
    for (unsigned int x = 0; x < image.GetWidth(); x++)
    {
      if ((x < exclusionX ||
           y < exclusionY ||
           x >= exclusionX + exclusionWidth ||
           y >= exclusionY + exclusionHeight) &&
          !OrthancStone::LinearAlgebra::IsNear(value, GetPixelValue(image, x, y)))
      {
        return false;
      }
    }
  }

  return true;
}


static bool AreSameImages(const Orthanc::ImageAccessor& image1,
                          const Orthanc::ImageAccessor& image2)
{
  if (image1.GetWidth() != image2.GetWidth() ||
      image1.GetHeight() != image2.GetHeight())
  {
    return false;
  }
  
  for (unsigned int y = 0; y < image1.GetHeight(); y++)
  {
    for (unsigned int x = 0; x < image1.GetWidth(); x++)
    {
      if (!OrthancStone::LinearAlgebra::IsNear(GetPixelValue(image1, x, y),
                                               GetPixelValue(image2, x, y)))
      {
        return false;
      }
    }
  }

  return true;
}
  

static void Assign3x3Pattern(Orthanc::ImageAccessor& image)
{
  if (image.GetFormat() == Orthanc::PixelFormat_Grayscale8 &&
      image.GetWidth() == 3 &&
      image.GetHeight() == 3)
  {
    unsigned int v = 0;
    for (unsigned int y = 0; y < image.GetHeight(); y++)
    {
      uint8_t *p = reinterpret_cast<uint8_t*>(image.GetRow(y));
      for (unsigned int x = 0; x < image.GetWidth(); x++, p++)
      {
        *p = v;
        v += 25;
      }
    }
  }
  else
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }
}


static Orthanc::ImageAccessor* Render(const OrthancStone::Scene2D& scene,
                                      unsigned int width,
                                      unsigned int height)
{
  OrthancStone::CairoCompositor compositor(width, height);
  compositor.Refresh(scene);
    
  Orthanc::ImageAccessor rendered;
  compositor.GetCanvas().GetReadOnlyAccessor(rendered);

  return Orthanc::Image::Clone(rendered);
}
  


// Render the scene using the identity viewpoint (default)
static Orthanc::ImageAccessor* Render(OrthancStone::ISceneLayer* layer,
                                      unsigned int width,
                                      unsigned int height,
                                      bool fitScene)
{
  OrthancStone::Scene2D scene;
  scene.SetLayer(0, layer);

  if (fitScene)
  {
    scene.FitContent(width, height);
  }
  
  return Render(scene, width, height);
}


enum SlicerType
{
  SlicerType_MPR = 0,
  SlicerType_Reslicer = 1
};


static OrthancStone::TextureBaseSceneLayer* SliceVolume(boost::shared_ptr<OrthancStone::DicomVolumeImage> volume,
                                                        const OrthancStone::CoordinateSystem3D& volumeCoordinates,
                                                        const OrthancStone::CoordinateSystem3D& cuttingPlane,
                                                        SlicerType type)
{
  Orthanc::DicomMap dicom;
  dicom.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, "study", false);
  dicom.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, "series", false);
  dicom.SetValue(Orthanc::DICOM_TAG_SOP_INSTANCE_UID, "sop", false);
  
  volume->SetDicomParameters(OrthancStone::DicomInstanceParameters(dicom));

  std::unique_ptr<OrthancStone::IVolumeSlicer> slicer;

  switch (type)
  {
    case SlicerType_MPR:
      slicer.reset(new OrthancStone::DicomVolumeImageMPRSlicer(volume));
      break;
      
    case SlicerType_Reslicer:
      slicer.reset(new OrthancStone::DicomVolumeImageReslicer(volume));
      break;

    default:
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
  }

  std::unique_ptr<OrthancStone::IVolumeSlicer::IExtractedSlice> slice(slicer->ExtractSlice(cuttingPlane));
  if (slice->IsValid())
  {
    OrthancStone::CopyStyleConfigurator configurator;
    return dynamic_cast<OrthancStone::TextureBaseSceneLayer*>(slice->CreateSceneLayer(&configurator, cuttingPlane));
  }
  else
  {
    return NULL;
  }
}


static OrthancStone::TextureBaseSceneLayer* Slice3x3x1Pattern(OrthancStone::VolumeProjection projection,
                                                              const OrthancStone::CoordinateSystem3D& volumeCoordinates,
                                                              const OrthancStone::CoordinateSystem3D& cuttingPlane,
                                                              SlicerType type)
{
  OrthancStone::VolumeImageGeometry geometry;

  switch (projection)
  {
    case OrthancStone::VolumeProjection_Axial:
      geometry.SetSizeInVoxels(3, 3, 1);
      break;

    case OrthancStone::VolumeProjection_Sagittal:
      geometry.SetSizeInVoxels(1, 3, 3);
      break;

    case OrthancStone::VolumeProjection_Coronal:
      geometry.SetSizeInVoxels(3, 1, 3);
      break;

    default:
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }
  
  geometry.SetAxialGeometry(volumeCoordinates);

  boost::shared_ptr<OrthancStone::DicomVolumeImage> volume(new OrthancStone::DicomVolumeImage);
  volume->Initialize(geometry, Orthanc::PixelFormat_Grayscale8, false);

  {
    OrthancStone::ImageBuffer3D::SliceWriter writer(volume->GetPixelData(), projection, 0);
    Assign3x3Pattern(writer.GetAccessor());
  }

  OrthancStone::Vector v = volume->GetGeometry().GetVoxelDimensions(OrthancStone::VolumeProjection_Axial);
  if (!OrthancStone::LinearAlgebra::IsNear(1, v[0]) ||
      !OrthancStone::LinearAlgebra::IsNear(1, v[1]) ||
      !OrthancStone::LinearAlgebra::IsNear(1, v[2]))
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }

  return SliceVolume(volume, volumeCoordinates, cuttingPlane, type);
}


TEST(VolumeRendering, Pattern)
{
  {
    // Axial
    OrthancStone::ImageBuffer3D image(Orthanc::PixelFormat_Grayscale8, 3, 3, 1, true);

    {
      OrthancStone::ImageBuffer3D::SliceWriter writer(image, OrthancStone::VolumeProjection_Axial, 0);
      Assign3x3Pattern(writer.GetAccessor());
    }

    float a, b;
    ASSERT_TRUE(image.GetRange(a, b));
    ASSERT_FLOAT_EQ(0, a);
    ASSERT_FLOAT_EQ(200, b);

    ASSERT_EQ(0, image.GetVoxelGrayscale8(0, 0, 0));
    ASSERT_EQ(25, image.GetVoxelGrayscale8(1, 0, 0));
    ASSERT_EQ(50, image.GetVoxelGrayscale8(2, 0, 0));
    ASSERT_EQ(75, image.GetVoxelGrayscale8(0, 1, 0));
    ASSERT_EQ(100, image.GetVoxelGrayscale8(1, 1, 0));
    ASSERT_EQ(125, image.GetVoxelGrayscale8(2, 1, 0));
    ASSERT_EQ(150, image.GetVoxelGrayscale8(0, 2, 0));
    ASSERT_EQ(175, image.GetVoxelGrayscale8(1, 2, 0));
    ASSERT_EQ(200, image.GetVoxelGrayscale8(2, 2, 0));

    float v;
    OrthancStone::SubvoxelReader<Orthanc::PixelFormat_Grayscale8,
                                 OrthancStone::ImageInterpolation_Nearest> reader(image);
    
    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 1.01, 0.01));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 1.01, 0.01));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 1.01, 0.01));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 2.01, 0.01));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 2.01, 0.01));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 2.01, 0.01));  ASSERT_FLOAT_EQ(200, v);
    
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 1.99, 0.99));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 1.99, 0.99));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 1.99, 0.99));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 2.99, 0.99));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 2.99, 0.99));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 2.99, 0.99));  ASSERT_FLOAT_EQ(200, v);
  }

  {
    // Coronal
    OrthancStone::ImageBuffer3D image(Orthanc::PixelFormat_Grayscale8, 3, 1, 3, true);

    {
      OrthancStone::ImageBuffer3D::SliceWriter writer(image, OrthancStone::VolumeProjection_Coronal, 0);
      Assign3x3Pattern(writer.GetAccessor());
    }

    float a, b;
    ASSERT_TRUE(image.GetRange(a, b));
    ASSERT_FLOAT_EQ(0, a);
    ASSERT_FLOAT_EQ(200, b);

    // "Z" is in reverse order in "Assign3x3Pattern()", because important note in "ImageBuffer3D"
    ASSERT_EQ(0, image.GetVoxelGrayscale8(0, 0, 2));
    ASSERT_EQ(25, image.GetVoxelGrayscale8(1, 0, 2));
    ASSERT_EQ(50, image.GetVoxelGrayscale8(2, 0, 2));
    ASSERT_EQ(75, image.GetVoxelGrayscale8(0, 0, 1));
    ASSERT_EQ(100, image.GetVoxelGrayscale8(1, 0, 1));
    ASSERT_EQ(125, image.GetVoxelGrayscale8(2, 0, 1));
    ASSERT_EQ(150, image.GetVoxelGrayscale8(0, 0, 0));
    ASSERT_EQ(175, image.GetVoxelGrayscale8(1, 0, 0));
    ASSERT_EQ(200, image.GetVoxelGrayscale8(2, 0, 0));

    // Ensure that "SubvoxelReader" is consistent with "image.GetVoxelGrayscale8()"
    float v;
    OrthancStone::SubvoxelReader<Orthanc::PixelFormat_Grayscale8,
                                 OrthancStone::ImageInterpolation_Nearest> reader(image);

    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 0.01, 2.01));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 0.01, 2.01));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 0.01, 2.01));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 0.01, 1.01));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 0.01, 1.01));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 0.01, 1.01));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.01, 0.01, 0.01));  ASSERT_FLOAT_EQ(200, v);
    
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 2.99));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 0.99, 2.99));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 0.99, 2.99));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 1.99));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 0.99, 1.99));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 0.99, 1.99));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 1.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 2.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(200, v);
  }

  {
    // Sagittal
    OrthancStone::ImageBuffer3D image(Orthanc::PixelFormat_Grayscale8, 1, 3, 3, true);

    {
      OrthancStone::ImageBuffer3D::SliceWriter writer(image, OrthancStone::VolumeProjection_Sagittal, 0);
      Assign3x3Pattern(writer.GetAccessor());
    }

    float a, b;
    ASSERT_TRUE(image.GetRange(a, b));
    ASSERT_FLOAT_EQ(0, a);
    ASSERT_FLOAT_EQ(200, b);

    // "Z" is in reverse order in "Assign3x3Pattern()", because important note in "ImageBuffer3D"
    ASSERT_EQ(0, image.GetVoxelGrayscale8(0, 0, 2));
    ASSERT_EQ(25, image.GetVoxelGrayscale8(0, 1, 2));
    ASSERT_EQ(50, image.GetVoxelGrayscale8(0, 2, 2));
    ASSERT_EQ(75, image.GetVoxelGrayscale8(0, 0, 1));
    ASSERT_EQ(100, image.GetVoxelGrayscale8(0, 1, 1));
    ASSERT_EQ(125, image.GetVoxelGrayscale8(0, 2, 1));
    ASSERT_EQ(150, image.GetVoxelGrayscale8(0, 0, 0));
    ASSERT_EQ(175, image.GetVoxelGrayscale8(0, 1, 0));
    ASSERT_EQ(200, image.GetVoxelGrayscale8(0, 2, 0));

    // Ensure that "SubvoxelReader" is consistent with "image.GetVoxelGrayscale8()"
    float v;
    OrthancStone::SubvoxelReader<Orthanc::PixelFormat_Grayscale8,
                                 OrthancStone::ImageInterpolation_Nearest> reader(image);

    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 0.01, 2.01));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 1.01, 2.01));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 2.01, 2.01));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 0.01, 1.01));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 1.01, 1.01));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 2.01, 1.01));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 0.01, 0.01));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 1.01, 0.01));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.1, 2.01, 0.01));  ASSERT_FLOAT_EQ(200, v);
    
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 2.99));  ASSERT_FLOAT_EQ(0, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 1.99, 2.99));  ASSERT_FLOAT_EQ(25, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 2.99, 2.99));  ASSERT_FLOAT_EQ(50, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 1.99));  ASSERT_FLOAT_EQ(75, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 1.99, 1.99));  ASSERT_FLOAT_EQ(100, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 2.99, 1.99));  ASSERT_FLOAT_EQ(125, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 0.99, 0.99));  ASSERT_FLOAT_EQ(150, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 1.99, 0.99));  ASSERT_FLOAT_EQ(175, v);
    ASSERT_TRUE(reader.GetFloatValue(v, 0.99, 2.99, 0.99));  ASSERT_FLOAT_EQ(200, v);
  }        
}


TEST(VolumeRendering, Axial)
{
  OrthancStone::CoordinateSystem3D axial(OrthancStone::LinearAlgebra::CreateVector(-0.5, -0.5, 0),
                                         OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                         OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));
  
  for (unsigned int mode = 0; mode < 2; mode++)
  {
    OrthancStone::CoordinateSystem3D cuttingPlane;
    
    std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
      Slice3x3x1Pattern(OrthancStone::VolumeProjection_Axial, axial, cuttingPlane, static_cast<SlicerType>(mode)));

    ASSERT_TRUE(layer.get() != NULL);
    ASSERT_EQ(OrthancStone::ISceneLayer::Type_FloatTexture, layer->GetType());

    OrthancStone::Extent2D box;
    layer->GetBoundingBox(box);
    ASSERT_FLOAT_EQ(-1.0f, box.GetX1());
    ASSERT_FLOAT_EQ(-1.0f, box.GetY1());
    ASSERT_FLOAT_EQ(2.0f, box.GetX2());
    ASSERT_FLOAT_EQ(2.0f, box.GetY2());
    
    {
      const Orthanc::ImageAccessor& texture = dynamic_cast<OrthancStone::TextureBaseSceneLayer&>(*layer).GetTexture();
      ASSERT_EQ(3u, texture.GetWidth());
      ASSERT_EQ(3u, texture.GetHeight());
      ASSERT_FLOAT_EQ(0, GetPixelValue(texture, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(texture, 1, 0));
      ASSERT_FLOAT_EQ(50, GetPixelValue(texture, 2, 0));
      ASSERT_FLOAT_EQ(75, GetPixelValue(texture, 0, 1));
      ASSERT_FLOAT_EQ(100, GetPixelValue(texture, 1, 1));
      ASSERT_FLOAT_EQ(125, GetPixelValue(texture, 2, 1));
      ASSERT_FLOAT_EQ(150, GetPixelValue(texture, 0, 2));
      ASSERT_FLOAT_EQ(175, GetPixelValue(texture, 1, 2));
      ASSERT_FLOAT_EQ(200, GetPixelValue(texture, 2, 2));
    }

    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 5, 5, false));
    ASSERT_EQ(5u, rendered->GetWidth());
    ASSERT_EQ(5u, rendered->GetHeight());
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 2, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 3, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 4, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 2, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 3, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 4, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 2));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 2));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 2, 2));
    ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 3, 2));
    ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 4, 2));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 3));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 3));
    ASSERT_FLOAT_EQ(75, GetPixelValue(*rendered, 2, 3));
    ASSERT_FLOAT_EQ(100, GetPixelValue(*rendered, 3, 3));
    ASSERT_FLOAT_EQ(125, GetPixelValue(*rendered, 4, 3));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 4));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 4));
    ASSERT_FLOAT_EQ(150, GetPixelValue(*rendered, 2, 4));
    ASSERT_FLOAT_EQ(175, GetPixelValue(*rendered, 3, 4));
    ASSERT_FLOAT_EQ(200, GetPixelValue(*rendered, 4, 4));
  }
}


TEST(VolumeRendering, TextureCorners)
{
  // The origin of a 2D texture is the coordinate of the BORDER of the
  // top-left pixel, *not* the center of the top-left pixel (as in
  // DICOM 3D convention)
  
  Orthanc::Image pixel(Orthanc::PixelFormat_RGB24, 1, 1, false);
  Orthanc::ImageProcessing::Set(pixel, 255, 0, 0, 255);
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(0, 0);

    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 2, 2, false));
    ASSERT_EQ(2u, rendered->GetWidth());
    ASSERT_EQ(2u, rendered->GetHeight());  
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 1));
    ASSERT_FLOAT_EQ(255, GetPixelValue(*rendered, 1, 1));
  }
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(-0.01, 0);

    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 2, 2, false));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 0));
    ASSERT_FLOAT_EQ(255, GetPixelValue(*rendered, 0, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 1));
  }
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(-0.01, -0.01);

    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 2, 2, false));
    ASSERT_FLOAT_EQ(255, GetPixelValue(*rendered, 0, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 1));
  }
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(0, -0.01);

    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 2, 2, false));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
    ASSERT_FLOAT_EQ(255, GetPixelValue(*rendered, 1, 0));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 1));
    ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 1, 1));
  }
}



TEST(VolumeRendering, FitTexture)
{
  Orthanc::Image pixel(Orthanc::PixelFormat_RGB24, 1, 1, false);
  Orthanc::ImageProcessing::Set(pixel, 255, 0, 0, 255);
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(-42.0f, 35.0f);
    layer->SetPixelSpacing(2, 3);

    OrthancStone::Scene2D scene;
    scene.SetLayer(0, layer.release());
    scene.FitContent(30, 30);
    
    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(scene, 30, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 5, 30));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 5, 0, 20, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 25, 0, 5, 30));

    rendered.reset(Render(scene, 40, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 10, 30));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 10, 0, 20, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 30, 0, 5, 30));

    scene.FitContent(40, 30);
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 10, 30));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 10, 0, 20, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 30, 0, 5, 30));
    
    rendered.reset(Render(scene, 30, 36));  // The scene has not been fitted
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 30, 3));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 3, 36));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 5, 3, 20, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 25, 0, 5, 36));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 33, 30, 3));

    scene.FitContent(30, 36);  // Refit
    rendered.reset(Render(scene, 30, 36));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 3, 36));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 3, 0, 24, 36));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 27, 0, 3, 36));
  }
  
  {    
    std::unique_ptr<OrthancStone::ColorTextureSceneLayer> layer(new OrthancStone::ColorTextureSceneLayer(pixel));
    layer->SetOrigin(42.0f, -35.0f);
    layer->SetPixelSpacing(3, 2);

    OrthancStone::Scene2D scene;
    scene.SetLayer(0, layer.release());
    scene.FitContent(30, 30);
    
    std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(scene, 30, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 30, 5));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 0, 5, 30, 20));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 25, 30, 5));

    rendered.reset(Render(scene, 30, 40));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 30, 10));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 0, 10, 30, 20));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 30, 30, 5));

    scene.FitContent(30, 40);
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 30, 10));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 0, 10, 30, 20));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 30, 30, 5));
    
    rendered.reset(Render(scene, 36, 30));  // The scene has not been fitted
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 3, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 36, 3));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 3, 5, 30, 20));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 25, 36, 5));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 33, 0, 3, 30));

    scene.FitContent(36, 30);  // Refit
    rendered.reset(Render(scene, 36, 30));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 0, 36, 3));
    ASSERT_TRUE(IsConstRegion(255.0f, *rendered, 0, 3, 36, 24));
    ASSERT_TRUE(IsConstRegion(0.0f, *rendered, 0, 27, 36, 3));
  }
}



TEST(VolumeRendering, MPR)
{
  double x = 2;
  double y = 1;
  OrthancStone::CoordinateSystem3D axial(OrthancStone::LinearAlgebra::CreateVector(x, y, 0),
                                         OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                         OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

  Orthanc::Image pattern(Orthanc::PixelFormat_Grayscale8, 3, 3, false);
  Assign3x3Pattern(pattern);
  
  Orthanc::Image patternX(Orthanc::PixelFormat_Grayscale8, 3, 3, false);
  Assign3x3Pattern(patternX);
  Orthanc::ImageProcessing::FlipX(patternX);
  
  Orthanc::Image patternY(Orthanc::PixelFormat_Grayscale8, 3, 3, false);
  Assign3x3Pattern(patternY);
  Orthanc::ImageProcessing::FlipY(patternY);
  
  Orthanc::Image patternXY(Orthanc::PixelFormat_Grayscale8, 3, 3, false);
  Assign3x3Pattern(patternXY);
  Orthanc::ImageProcessing::FlipX(patternXY);
  Orthanc::ImageProcessing::FlipY(patternXY);
    
  for (unsigned int mode = 0; mode < 2; mode++)
  {
    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Axial, axial, cuttingPlane, static_cast<SlicerType>(mode)));
      ASSERT_TRUE(AreSameImages(layer->GetTexture(), pattern));

      OrthancStone::Extent2D extent;
      layer->GetBoundingBox(extent);
      ASSERT_FLOAT_EQ(x - 0.5, extent.GetX1());
      ASSERT_FLOAT_EQ(y - 0.5, extent.GetY1());
      ASSERT_FLOAT_EQ(x + 2.5, extent.GetX2());
      ASSERT_FLOAT_EQ(y + 2.5, extent.GetY2());

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 15, 15, false));
      ASSERT_TRUE(IsConstImageWithExclusion(0.0f, *rendered, 9, 8, 3, 3));

      Orthanc::ImageAccessor p;
      rendered->GetRegion(p, 9, 8, 3, 3);
      ASSERT_TRUE(AreSameImages(p, pattern));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Axial, axial, cuttingPlane, static_cast<SlicerType>(mode)));
      if (mode == 1)
      {
        // Reslicer directly flips the pixels of the texture
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), patternX));
      }
      else
      {
        // MPR slicer uses "TextureBaseSceneLayer::SetTransform()" to flip
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), pattern));
      }

      OrthancStone::Extent2D extent;
      layer->GetBoundingBox(extent);
      ASSERT_FLOAT_EQ(-(x + 2.5), extent.GetX1());
      ASSERT_FLOAT_EQ(y - 0.5, extent.GetY1());
      ASSERT_FLOAT_EQ(-(x - 0.5), extent.GetX2());
      ASSERT_FLOAT_EQ(y + 2.5, extent.GetY2());

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 15, 15, false));
      ASSERT_TRUE(IsConstImageWithExclusion(0.0f, *rendered, 3, 8, 3, 3));

      Orthanc::ImageAccessor p;
      rendered->GetRegion(p, 3, 8, 3, 3);
      ASSERT_TRUE(AreSameImages(p, patternX));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Axial, axial, cuttingPlane, static_cast<SlicerType>(mode)));
      if (mode == 1)
      {
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), patternY));
      }
      else
      {
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), pattern));
      }

      OrthancStone::Extent2D extent;
      layer->GetBoundingBox(extent);
      ASSERT_FLOAT_EQ(x - 0.5, extent.GetX1());
      ASSERT_FLOAT_EQ(-(y + 2.5), extent.GetY1());
      ASSERT_FLOAT_EQ(x + 2.5, extent.GetX2());
      ASSERT_FLOAT_EQ(-(y - 0.5), extent.GetY2());

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 15, 15, false));
      ASSERT_TRUE(IsConstImageWithExclusion(0.0f, *rendered, 9, 4, 3, 3));

      Orthanc::ImageAccessor p;
      rendered->GetRegion(p, 9, 4, 3, 3);
      ASSERT_TRUE(AreSameImages(p, patternY));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Axial, axial, cuttingPlane, static_cast<SlicerType>(mode)));
      if (mode == 1)
      {
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), patternXY));
      }
      else
      {
        ASSERT_TRUE(AreSameImages(layer->GetTexture(), pattern));
      }

      OrthancStone::Extent2D extent;
      layer->GetBoundingBox(extent);
      ASSERT_FLOAT_EQ(-(x + 2.5), extent.GetX1());
      ASSERT_FLOAT_EQ(-(y + 2.5), extent.GetY1());
      ASSERT_FLOAT_EQ(-(x - 0.5), extent.GetX2());
      ASSERT_FLOAT_EQ(-(y - 0.5), extent.GetY2());

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 15, 15, false));
      ASSERT_TRUE(IsConstImageWithExclusion(0.0f, *rendered, 3, 4, 3, 3));

      Orthanc::ImageAccessor p;
      rendered->GetRegion(p, 3, 4, 3, 3);
      ASSERT_TRUE(AreSameImages(p, patternXY));
    }

    // Tests along the sagittal and coronal axis: cf.
    // "TEST(VolumeRendering, Pattern)" to understand why Z is flipped

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(150, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(175, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(200, GetPixelValue(*rendered, 0, 2));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 1),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(75, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(100, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(125, GetPixelValue(*rendered, 0, 2));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 2));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 2));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 2));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 1, 3, true));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 0, 1));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 2));
    }

    for (double z = -1; z < 4; z += 4)  // z in { -1, 3 }, out of volume
    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, z),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Sagittal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      ASSERT_TRUE(layer.get() == NULL ||  // This is for DicomVolumeImageMPRSlicer
                  (layer->GetTexture().GetWidth() == 0 &&  // This is for DicomVolumeImageReslicer
                   layer->GetTexture().GetHeight() == 0));
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(150, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(175, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(200, GetPixelValue(*rendered, 2, 0));      
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 1),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(75, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(100, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(125, GetPixelValue(*rendered, 2, 0));      
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 2, 0));      
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 2, 0));      
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 2, 0));      
    }

    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, 2),
                                                    OrthancStone::LinearAlgebra::CreateVector(-1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, -1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      std::unique_ptr<Orthanc::ImageAccessor> rendered(Render(layer.release(), 3, 1, true));
      ASSERT_FLOAT_EQ(50, GetPixelValue(*rendered, 0, 0));
      ASSERT_FLOAT_EQ(25, GetPixelValue(*rendered, 1, 0));
      ASSERT_FLOAT_EQ(0, GetPixelValue(*rendered, 2, 0));      
    }

    for (double z = -1; z < 4; z += 4)  // z in { -1, 3 }, out of volume
    {
      OrthancStone::CoordinateSystem3D cuttingPlane(OrthancStone::LinearAlgebra::CreateVector(0, 0, z),
                                                    OrthancStone::LinearAlgebra::CreateVector(1, 0, 0),
                                                    OrthancStone::LinearAlgebra::CreateVector(0, 1, 0));

      std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer(
        Slice3x3x1Pattern(OrthancStone::VolumeProjection_Coronal, axial, cuttingPlane, static_cast<SlicerType>(mode)));

      ASSERT_TRUE(layer.get() == NULL ||  // This is for DicomVolumeImageMPRSlicer
                  (layer->GetTexture().GetWidth() == 0 &&  // This is for DicomVolumeImageReslicer
                   layer->GetTexture().GetHeight() == 0));
    }
  }
}


TEST(VolumeRendering, MacroLayer)
{
  OrthancStone::MacroSceneLayer layer;
  ASSERT_THROW(layer.AddLayer(NULL), Orthanc::OrthancException);

  ASSERT_EQ(0u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(1u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(2u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(3u, layer.GetSize());
  ASSERT_TRUE(layer.HasLayer(0));
  ASSERT_TRUE(layer.HasLayer(1));
  ASSERT_TRUE(layer.HasLayer(2));

  layer.DeleteLayer(1);
  ASSERT_EQ(3u, layer.GetSize());
  ASSERT_TRUE(layer.HasLayer(0));
  ASSERT_FALSE(layer.HasLayer(1));
  ASSERT_TRUE(layer.HasLayer(2));

  ASSERT_THROW(layer.UpdateLayer(1, NULL), Orthanc::OrthancException);
  layer.UpdateLayer(1, new OrthancStone::PolylineSceneLayer);
  ASSERT_TRUE(layer.HasLayer(1));

  ASSERT_EQ(3u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(4u, layer.GetSize());

  layer.DeleteLayer(1);
  layer.DeleteLayer(2);
  ASSERT_EQ(1u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));

  std::unique_ptr<OrthancStone::MacroSceneLayer> clone(dynamic_cast<OrthancStone::MacroSceneLayer*>(layer.Clone()));
  
  layer.UpdateLayer(2, new OrthancStone::PolylineSceneLayer);
  ASSERT_EQ(4u, layer.AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(5u, layer.GetSize());
  ASSERT_TRUE(layer.HasLayer(0));
  ASSERT_TRUE(layer.HasLayer(1));
  ASSERT_TRUE(layer.HasLayer(2));
  ASSERT_TRUE(layer.HasLayer(3));
  ASSERT_TRUE(layer.HasLayer(4));

  ASSERT_EQ(2u, clone->AddLayer(new OrthancStone::PolylineSceneLayer));
  ASSERT_EQ(4u, clone->GetSize());
  ASSERT_TRUE(clone->HasLayer(0));
  ASSERT_TRUE(clone->HasLayer(1));
  ASSERT_TRUE(clone->HasLayer(2));
  ASSERT_TRUE(clone->HasLayer(3));
  ASSERT_THROW(clone->HasLayer(4), Orthanc::OrthancException);
}
