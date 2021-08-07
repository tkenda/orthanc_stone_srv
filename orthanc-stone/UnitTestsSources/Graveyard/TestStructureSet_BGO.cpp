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


/*
these tests are single-threaded... no worries for old buggy compilers
(I'm talking to YOU, cl.exe v100! And to your ancestors!)
*/
static std::string& GetTestJson()
{
  static const char* resultRaw = NULL;
  static std::string result;
  if (resultRaw == NULL)
  {
    std::stringstream sst;

    sst << k_rtStruct_json00
      << k_rtStruct_json01
      << k_rtStruct_json02
      << k_rtStruct_json03
      << k_rtStruct_json04
      << k_rtStruct_json05
      << k_rtStruct_json06
      << k_rtStruct_json07
      << k_rtStruct_json08;

    std::string wholeBody = sst.str();
    result.swap(wholeBody);
    resultRaw = result.c_str();
  }
  return result;
}


namespace
{
  void Initialize(const char* orthancApiUrl, OrthancStone::ILoadersContext& loadersContext)
  {
    Orthanc::WebServiceParameters p;

    OrthancStone::GenericLoadersContext& typedLoadersContext =
      dynamic_cast<OrthancStone::GenericLoadersContext&>(loadersContext);
    // Default is http://localhost:8042
    // Here's how you may change it
    p.SetUrl(orthancApiUrl);
    p.SetCredentials("orthanc", "orthanc");
    typedLoadersContext.SetOrthancParameters(p);

    typedLoadersContext.StartOracle();
  }

  void Exitialize(OrthancStone::ILoadersContext& loadersContext)
  {
    OrthancStone::GenericLoadersContext& typedLoadersContext =
      dynamic_cast<OrthancStone::GenericLoadersContext&>(loadersContext);

    typedLoadersContext.StopOracle();
  }


#if 0
  class TestObserver : public ObserverBase<TestObserver>
  {
  public:
    TestObserver() {};

    virtual void Handle

  };
#endif

}

TEST(StructureSet, DISABLED_StructureSetLoader_injection_feature_2020_05_10)
{
  namespace pt = boost::posix_time;

  std::unique_ptr<OrthancStone::ILoadersContext> loadersContext(new OrthancStone::GenericLoadersContext(1,4,1));
  Initialize("http://localhost:8042/", *loadersContext);

  boost::shared_ptr<DicomStructureSetLoader> loader = DicomStructureSetLoader::Create(*loadersContext);

  // replace with Orthanc ID of an uploaded RTSTRUCT instance!
  loader->LoadInstanceFullVisibility("72c773ac-5059f2c4-2e6a9120-4fd4bca1-45701661");

  bool bContinue(true);

  pt::ptime initialTime = pt::second_clock::local_time();

  while (bContinue)
  {
    bContinue = !loader->AreStructuresReady();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

    {
      pt::ptime nowTime = pt::second_clock::local_time();
      pt::time_duration diff = nowTime - initialTime;
      double seconds = static_cast<double>(diff.total_milliseconds()) * 0.001;
      std::cout << seconds << " seconds elapsed...\n";
      if (seconds > 30)
      {
        std::cout << "More than 30 seconds elapsed... Aborting test :(\n";
        //GTEST_FATAL_FAILURE_("More than 30 seconds elapsed... Aborting test :(");
        //bContinue = false;
      }
    }
  }
}

class SliceProcessor :
  public OrthancStone::OrthancSeriesVolumeProgressiveLoader::ISlicePostProcessor,
  public OrthancStone::DicomStructureSetLoader::IInstanceLookupHandler
{
public:
  SliceProcessor(OrthancStone::DicomStructureSetLoader& structLoader) : structLoader_(structLoader)
  {
  }

  virtual void ProcessCTDicomSlice(const Orthanc::DicomMap& instance) ORTHANC_OVERRIDE
  {
    std::string sopInstanceUid;
    if (!instance.LookupStringValue(sopInstanceUid, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Missing SOPInstanceUID in a DICOM instance");
    }
    slicesDicom_[sopInstanceUid] = boost::shared_ptr<DicomMap>(instance.Clone());
  }

  virtual void RetrieveReferencedSlices(const std::set<std::string>& nonEmptyInstances) ORTHANC_OVERRIDE
  {
    for (std::set<std::string>::const_iterator it = nonEmptyInstances.begin(); 
         it != nonEmptyInstances.end(); 
         ++it)
    {
      const std::string nonEmptyInstance = *it;
      if (slicesDicom_.find(nonEmptyInstance) == slicesDicom_.end())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat, "Referenced SOPInstanceUID not found in CT");
      }
      boost::shared_ptr<Orthanc::DicomMap> instance = slicesDicom_[nonEmptyInstance];
      structLoader_.AddReferencedSlice(*instance);
    }
  }

  OrthancStone::DicomStructureSetLoader& structLoader_;
  std::map<std::string, boost::shared_ptr<Orthanc::DicomMap> > slicesDicom_;
};

void LoadCtSeriesBlocking(boost::shared_ptr<OrthancStone::OrthancSeriesVolumeProgressiveLoader> ctLoader, std::string seriesId)
{
  namespace pt = boost::posix_time;
  
  // Load the CT 
  ctLoader->LoadSeries(seriesId);

  // Wait for CT to be loaded
  pt::ptime initialTime = pt::second_clock::local_time();
  {
    bool bContinue(true);
    while (bContinue)
    {
      bContinue = !ctLoader->IsVolumeImageReadyInHighQuality();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

      {
        pt::ptime nowTime = pt::second_clock::local_time();
        pt::time_duration diff = nowTime - initialTime;
        double seconds = static_cast<double>(diff.total_milliseconds()) * 0.001;
        std::cout << seconds << " seconds elapsed...\n";
        if (seconds > 30)
        {
          const char* msg = "More than 30 seconds elapsed when waiting for CT... Aborting test :(\n";
          GTEST_FATAL_FAILURE_(msg);
          bContinue = false;
        }
      }
    }
  }
}


/**
Will fill planes
*/
void GetCTPlanes(std::vector<OrthancStone::CoordinateSystem3D>& planes, 
                 OrthancStone::VolumeProjection projection,
                 boost::shared_ptr<OrthancStone::OrthancSeriesVolumeProgressiveLoader> ctLoader)
{
    planes.clear(); // inefficient : we don't care

    const VolumeImageGeometry& geometry = ctLoader->GetImageGeometry();
    const unsigned int depth = geometry.GetProjectionDepth(projection);

    planes.resize(depth);

    for (unsigned int z = 0; z < depth; z++)
    {
      planes[z] = geometry.GetProjectionSlice(projection, z);
    }
}

void LoadRtStructBlocking(boost::shared_ptr<OrthancStone::DicomStructureSetLoader> structLoader, std::string instanceId)
{
  namespace pt = boost::posix_time;

  // Load RTSTRUCT
  structLoader->LoadInstanceFullVisibility(instanceId);

  pt::ptime initialTime = pt::second_clock::local_time();

  // Wait for the loading process to complete
  {
    bool bContinue(true);
    while (bContinue)
    {
      bContinue = !structLoader->AreStructuresReady();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

      {
        pt::ptime nowTime = pt::second_clock::local_time();
        pt::time_duration diff = nowTime - initialTime;
        double seconds = static_cast<double>(diff.total_milliseconds()) * 0.001;
        std::cout << seconds << " seconds elapsed...\n";
        if (seconds > 30)
        {
          const char* msg = "More than 30 seconds elapsed when waiting for RTSTRUCT... Aborting test :(\n";
          GTEST_FATAL_FAILURE_(msg);
          bContinue = false;
        }
      }
    }
  }
}

TEST(StructureSet, DISABLED_Integration_Compound_CT_Struct_Loading)
{
  const double TOLERANCE = 0.0000001;

  // create loaders context
  std::unique_ptr<OrthancStone::ILoadersContext> loadersContext(new OrthancStone::GenericLoadersContext(1,4,1));
  Initialize("http://localhost:8042/", *loadersContext);

  const char* ctSeriesId = "a04ecf01-79b2fc33-58239f7e-ad9db983-28e81afa";
  const char* rtStructInstanceId = "54460695-ba3885ee-ddf61ac0-f028e31d-a6e474d9";

  // we'll compare normal loading and optimized loading with SliceProcessor to store the dicom

  boost::shared_ptr<OrthancStone::DicomStructureSetLoader> normalStructLoader;
  boost::shared_ptr<OrthancStone::DicomStructureSetLoader> optimizedStructLoader;
  
  {
    // Create the CT volume
    boost::shared_ptr<OrthancStone::DicomVolumeImage> volume = boost::make_shared<OrthancStone::DicomVolumeImage>();

    // Create CT loader
    boost::shared_ptr<OrthancStone::OrthancSeriesVolumeProgressiveLoader> ctLoader =
      OrthancStone::OrthancSeriesVolumeProgressiveLoader::Create(*loadersContext, volume);

    // Create struct loader
    normalStructLoader = OrthancStone::DicomStructureSetLoader::Create(*loadersContext);

    // Load the CT
    LoadCtSeriesBlocking(ctLoader, ctSeriesId);
    
    const OrthancStone::VolumeImageGeometry& imageGeometry = ctLoader->GetImageGeometry();
    unsigned int width = imageGeometry.GetWidth();
    EXPECT_EQ(512u, width);
    unsigned int height = imageGeometry.GetHeight();
    EXPECT_EQ(512u, height);
    unsigned int depth = imageGeometry.GetDepth();
    EXPECT_EQ(109u, depth);

    // Load the RTStruct
    LoadRtStructBlocking(normalStructLoader, rtStructInstanceId);
  }

  std::vector<OrthancStone::CoordinateSystem3D> axialPlanes;
  std::vector<OrthancStone::CoordinateSystem3D> coronalPlanes;
  std::vector<OrthancStone::CoordinateSystem3D> sagittalPlanes;

  {
    // Create the CT volume
    boost::shared_ptr<OrthancStone::DicomVolumeImage> volume = boost::make_shared<OrthancStone::DicomVolumeImage>();

    // Create CT loader
    boost::shared_ptr<OrthancStone::OrthancSeriesVolumeProgressiveLoader> ctLoader =
      OrthancStone::OrthancSeriesVolumeProgressiveLoader::Create(*loadersContext, volume);

    // Create struct loader
    optimizedStructLoader = OrthancStone::DicomStructureSetLoader::Create(*loadersContext);

    // create the slice processor / instance lookup
    boost::shared_ptr<SliceProcessor> sliceProcessor(new SliceProcessor(*optimizedStructLoader));

    // Inject it into CT loader
    ctLoader->SetDicomSlicePostProcessor(sliceProcessor);

    // Inject it into RTSTRUCT loader
    optimizedStructLoader->SetInstanceLookupHandler(sliceProcessor);

    // Load the CT
    LoadCtSeriesBlocking(ctLoader, ctSeriesId);
  
    // now, the slices are collected. let's do some checks
    EXPECT_EQ(109u, sliceProcessor->slicesDicom_.size());

    // Load the RTStruct
    LoadRtStructBlocking(optimizedStructLoader, rtStructInstanceId);

    GetCTPlanes(axialPlanes, VolumeProjection_Axial, ctLoader);
    GetCTPlanes(coronalPlanes, VolumeProjection_Coronal, ctLoader);
    GetCTPlanes(sagittalPlanes, VolumeProjection_Sagittal, ctLoader);
  }

  // DO NOT DELETE THOSE!
  OrthancStone::DicomStructureSet* normalContent = normalStructLoader->GetContent();
  OrthancStone::DicomStructureSet* optimizedContent = optimizedStructLoader->GetContent();

  EXPECT_EQ(normalContent->GetStructuresCount(), optimizedContent->GetStructuresCount());

  /*void GetCTPlanes(std::vector<OrthancStone::CoordinateSystem3D>& planes, 
                 OrthancStone::VolumeProjection projection,
                 boost::shared_ptr<OrthancStone::OrthancSeriesVolumeProgressiveLoader> ctLoader)*/


  std::vector<OrthancStone::CoordinateSystem3D> allPlanes;

  // let's gather all the possible cutting planes in a single struct
  for (size_t i = 0; i < axialPlanes.size(); ++i)
    allPlanes.push_back(axialPlanes[i]);

  for (size_t i = 0; i < coronalPlanes.size(); ++i)
    allPlanes.push_back(coronalPlanes[i]);

  for (size_t i = 0; i < sagittalPlanes.size(); ++i)
    allPlanes.push_back(sagittalPlanes[i]);

  for (size_t i = 0; i < normalContent->GetStructuresCount(); ++i)
  {
    std::cout << "Testing structure (" << i << "/" << normalContent->GetStructuresCount() << ")\n";
    Vector structureCenter1                     = normalContent->GetStructureCenter(i);
    const std::string& structureName1           = normalContent->GetStructureName(i);
    const std::string& structureInterpretation1 = normalContent->GetStructureInterpretation(i);
    Color structureColor1                       = normalContent->GetStructureColor(i);

    Vector structureCenter2                     = optimizedContent->GetStructureCenter(i);
    const std::string& structureName2           = optimizedContent->GetStructureName(i);
    const std::string& structureInterpretation2 = optimizedContent->GetStructureInterpretation(i);
    Color structureColor2                       = optimizedContent->GetStructureColor(i);

    EXPECT_NEAR(structureCenter1[0], structureCenter2[0], TOLERANCE);
    EXPECT_NEAR(structureCenter1[1], structureCenter2[1], TOLERANCE);
    EXPECT_NEAR(structureCenter1[2], structureCenter2[2], TOLERANCE);
    
    EXPECT_EQ(structureName1, structureName2);
    EXPECT_EQ(structureInterpretation1, structureInterpretation2);
    EXPECT_EQ(structureColor1.GetRed(), structureColor2.GetRed());
    EXPECT_EQ(structureColor1.GetGreen(), structureColor2.GetGreen());
    EXPECT_EQ(structureColor1.GetBlue(), structureColor2.GetBlue());

    // "random" walk through the planes. Processing them all takes too long (~ 1 min)
    for (size_t j = 0; j < allPlanes.size(); j += 37)
    {
      const OrthancStone::CoordinateSystem3D& plane = allPlanes[j];

      std::vector< std::pair<Point2D, Point2D> > segments1;
      std::vector< std::pair<Point2D, Point2D> > segments2;
      
      bool ok1 = normalContent->ProjectStructure(segments1, i, plane);
      bool ok2 = optimizedContent->ProjectStructure(segments2, i, plane);

      // checks here
      EXPECT_EQ(ok1, ok2);
      EXPECT_EQ(segments1.size(), segments2.size());

      for (size_t k = 0; k < segments1.size(); ++k)
      {
        EXPECT_NEAR(segments1[k].first.x, segments2[k].first.x, TOLERANCE);
        EXPECT_NEAR(segments1[k].first.y, segments2[k].first.y, TOLERANCE);
        EXPECT_NEAR(segments1[k].second.x, segments2[k].second.x, TOLERANCE);
        EXPECT_NEAR(segments1[k].second.y, segments2[k].second.y, TOLERANCE);
      }
    }
  }

  Exitialize(*loadersContext);
}
