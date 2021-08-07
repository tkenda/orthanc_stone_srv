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

// working around a bug where the Visual C++ compiler would get 
// stuck trying to compile this cpp file in release mode
// (versions: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B)
#ifdef _MSC_VER
#  pragma optimize("", off)
// warning C4748: /GS can not protect parameters and local variables from
// local buffer overrun because optimizations are disabled in function
#  pragma warning(disable: 4748)
#endif

#include "../OrthancStone/Sources/Loaders/DicomStructureSetLoader.h"
#include "../OrthancStone/Sources/Loaders/OrthancSeriesVolumeProgressiveLoader.h"
#include "../OrthancStone/Sources/Toolbox/DicomStructureSet2.h"
#include "../OrthancStone/Sources/Toolbox/DicomStructureSetUtils.h"
#include "../OrthancStone/Sources/Toolbox/DisjointDataSet.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace OrthancStone;

/*
The following string is the reply to the following Orthanc request:

http://localhost:8042/instances/1aa5f84b-c32a03b4-3c1857da-da2e69f3-3ef6e2b3/tags?ignore-length=3006-0050

The tag hierarchy can be found here: https://dicom.innolitics.com/ciods/rt-dose
*/

const double DELTA_MAX = 10.0 * std::numeric_limits<float>::epsilon();

#ifdef _MSC_VER
#pragma region BigJsonString
#endif
// _MSC_VER
const char* k_rtStruct_json00 =
"{\n"
"   \"0008,0005\" : {\n"
"      \"Name\" : \"SpecificCharacterSet\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"ISO_IR 100\"\n"
"   },\n"
"   \"0008,0012\" : {\n"
"      \"Name\" : \"InstanceCreationDate\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"20190318\"\n"
"   },\n"
"   \"0008,0013\" : {\n"
"      \"Name\" : \"InstanceCreationTime\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"182529\"\n"
"   },\n"
"   \"0008,0016\" : {\n"
"      \"Name\" : \"SOPClassUID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1.2.840.10008.5.1.4.1.1.481.3\"\n"
"   },\n"
"   \"0008,0018\" : {\n"
"      \"Name\" : \"SOPInstanceUID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1.2.752.243.1.1.20190318182529549.3500.4285482120751\"\n"
"   },\n"
"   \"0008,0020\" : {\n"
"      \"Name\" : \"StudyDate\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"20190225\"\n"
"   },\n"
"   \"0008,0030\" : {\n"
"      \"Name\" : \"StudyTime\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"135152\"\n"
"   },\n"
"   \"0008,0050\" : {\n"
"      \"Name\" : \"AccessionNumber\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"897154\"\n"
"   },\n"
"   \"0008,0060\" : {\n"
"      \"Name\" : \"Modality\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"RTSTRUCT\"\n"
"   },\n"
"   \"0008,0070\" : {\n"
"      \"Name\" : \"Manufacturer\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"RaySearch Laboratories\"\n"
"   },\n"
"   \"0008,0090\" : {\n"
"      \"Name\" : \"ReferringPhysicianName\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"\"\n"
"   },\n"
"   \"0008,1030\" : {\n"
"      \"Name\" : \"StudyDescription\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"CT ohne KM\"\n"
"   },\n"
"   \"0008,103e\" : {\n"
"      \"Name\" : \"SeriesDescription\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"RS: Approved Structure Set\"\n"
"   },\n"
"   \"0008,1070\" : {\n"
"      \"Name\" : \"OperatorsName\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"\"\n"
"   },\n"
"   \"0008,1090\" : {\n"
"      \"Name\" : \"ManufacturerModelName\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"RayStation\"\n"
"   },\n"
"   \"0010,0010\" : {\n"
"      \"Name\" : \"PatientName\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"Karamazov^Serge\"\n"
"   },\n"
"   \"0010,0020\" : {\n"
"      \"Name\" : \"PatientID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"66498\"\n"
"   },\n"
"   \"0010,0030\" : {\n"
"      \"Name\" : \"PatientBirthDate\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"19630511\"\n"
"   },\n"
"   \"0010,0040\" : {\n"
"      \"Name\" : \"PatientSex\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"F\"\n"
"   },\n"
"   \"0018,1020\" : {\n"
"      \"Name\" : \"SoftwareVersions\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"7.0.0.19 (Dicom Export)\"\n"
"   },\n"
"   \"0020,000d\" : {\n"
"      \"Name\" : \"StudyInstanceUID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1.2.840.113854.1977802846882851650617130240362685394\"\n"
"   },\n"
"   \"0020,000e\" : {\n"
"      \"Name\" : \"SeriesInstanceUID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1.2.752.243.1.1.20190318182529549.3582165\"\n"
"   },\n"
"   \"0020,0010\" : {\n"
"      \"Name\" : \"StudyID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"467\"\n"
"   },\n"
"   \"0020,0011\" : {\n"
"      \"Name\" : \"SeriesNumber\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1\"\n"
"   },\n"
"   \"0020,0052\" : {\n"
"      \"Name\" : \"FrameOfReferenceUID\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965689415626\"\n"
"   },\n"
"   \"0020,1040\" : {\n"
"      \"Name\" : \"PositionReferenceIndicator\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"\"\n"
"   },\n"
"   \"3006,0002\" : {\n"
"      \"Name\" : \"StructureSetLabel\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"RS: Approved\"\n"
"   },\n"
"   \"3006,0008\" : {\n"
"      \"Name\" : \"StructureSetDate\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"20190318\"\n"
"   },\n"
"   \"3006,0009\" : {\n"
"      \"Name\" : \"StructureSetTime\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"182529\"\n"
"   },\n"
"   \"3006,0010\" : {\n"
"      \"Name\" : \"ReferencedFrameOfReferenceSequence\",\n"
"      \"Type\" : \"Sequence\",\n"
"      \"Value\" : [\n"
"         {\n"
"            \"0020,0052\" : {\n"
"               \"Name\" : \"FrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.47019656894156\"\n"
"            },\n"
"            \"3006,0012\" : {\n"
"               \"Name\" : \"RTReferencedStudySequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"0008,1150\" : {\n"
"                        \"Name\" : \"ReferencedSOPClassUID\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1.2.840.10008.3.1.2.3.1\"\n"
"                     },\n"
"                     \"0008,1155\" : {\n"
"                        \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1.2.840.113854.1977802846882851650617130240362685\"\n"
"                     },\n"
"                     \"3006,0014\" : {\n"
"                        \"Name\" : \"RTReferencedSeriesSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0020,000e\" : {\n"
"                                 \"Name\" : \"SeriesInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.752.243.1.1.20190318182529549.3582165\"\n"
"                              },\n"
"                              \"3006,0016\" : {\n"
"                                 \"Name\" : \"ContourImageSequence\",\n"
"                                 \"Type\" : \"Sequence\",\n"
"                                 \"Value\" : [\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699846442461900001.533642576430\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699846477463900001.467218939844\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699846504465400001.571321740640\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699846528466800001.465226999207\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848817597700001.509440095881\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848849599600001.538291679804\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848866600500001.541849142008\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848890601900001.533619501923\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848911603100001.549938975401\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848933604400001.474954755728\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848957605700001.539833993627\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699848978606900001.507522632828\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851016723500001.512636853846\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851035724600001.533642609670\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851068726500001.546012551665\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851092727900001.507332422180\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851113729100001.470312254320\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851133730200001.543478973601\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851156731500001.532752123275\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851179732800001.515857303411\"\n"
"                                       }\n"
"                                    },\n"
"                                    {\n"
"                                       \"0008,1150\" : {\n"
"                                          \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                                       },\n"
"                                       \"0008,1155\" : {\n"
"                                          \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                          \"Type\" : \"String\",\n"
"                                          \"Value\" : \"1.3.46.670589.33.1.63686699851194733700001.468081152243\"\n"
"                                       }\n"
"                                    }\n"
"                                 ]\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            }\n"
"         }\n"
"      ]\n"
"   },\n"
"   \"3006,0020\" : {\n"
"      \"Name\" : \"StructureSetROISequence\",\n"
"      \"Type\" : \"Sequence\",\n"
"      \"Value\" : [\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"LN300\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"2\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Cortical Bone\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"3\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Adipose\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"4\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"CB2-50%\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"5\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Water\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"10\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"External\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0022\" : {\n"
"               \"Name\" : \"ROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"11\"\n"
"            },\n"
"            \"3006,0024\" : {\n"
"               \"Name\" : \"ReferencedFrameOfReferenceUID\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1.3.46.670589.33.1.63686699677866819900002.4701965682522348054\"\n"
"            },\n"
"            \"3006,0026\" : {\n"
"               \"Name\" : \"ROIName\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"PTV\"\n"
"            },\n"
"            \"3006,0036\" : {\n"
"               \"Name\" : \"ROIGenerationAlgorithm\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"SEMIAUTOMATIC\"\n"
"            }\n"
"         }\n"
"      ]\n"
"   },\n"
"   \"3006,0039\" : {\n"
"      \"Name\" : \"ROIContourSequence\",\n"
"      \"Type\" : \"Sequence\",\n"
"      \"Value\" : [\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"255\\\\0\\\\0\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"7.657838\\\\108.2725\\\\304.01\\\\6.826687\\\\107.4413\\\\304.01\\\\6.152492\\\\106.4785\\\\304.01\\\\5.655735\\\\105.4132\\\\304.01\\\\5.351513\\\\104.2778\\\\304.01\\\\5.249068\\\\103.1069\\\\304.01\\\\5.351513\\\\101.9359\\\\304.01\\\\5.655735\\\\100.8005\\\\304.01\\\\6.152492\\\\99.73524\\\\304.01\\\\6.826687\\\\98.77239\\\\304.01\\\\7.657838\\\\97.94124\\\\304.01\\\\8.620689\\\\97.26704\\\\304.01\\\\9.685987\\\\96.77029\\\\304.01\\\\10.82136\\\\96.46606\\\\304.01\\\\11.99231\\\\96.36362\\\\304.01\\\\13.16326\\\\96.46606\\\\304.01\\\\14.29864\\\\96.77029\\\\304.01\\\\15.36393\\\\97.26704\\\\304.01\\\\16.32678\\\\97.94124\\\\304.01\\\\17.15794\\\\98.77239\\\\304.01\\\\17.83213\\\\99.73524\\\\304.01\\\\18.32889\\\\100.8005\\\\304.01\\\\18.63311\\\\101.9359\\\\304.01\\\\18.73555\\\\103.1069\\\\304.01\\\\18.63311\\\\104.2778\\\\304.01\\\\18.32889\\\\105.4132\\\\304.01\\\\17.83213\\\\106.4785\\\\304.01\\\\17.15794\\\\107.4413\\\\304.01\\\\16.32678\\\\108.2725\\\\304.01\\\\15.36393\\\\108.9467\\\\304.01\\\\14.29864\\\\109.4434\\\\304.01\\\\13.16326\\\\109.7477\\\\304.01\\\\11.99231\\\\109.8501\\\\304.01\\\\10.82136\\\\109.7477\\\\304.01\\\\9.685987\\\\109.4434\\\\304.01\\\\8.620689\\\\108.9467\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"7.657838\\\\108.2725\\\\307.01\\\\6.826687\\\\107.4413\\\\307.01\\\\6.152492\\\\106.4785\\\\307.01\\\\5.655735\\\\105.4132\\\\307.01\\\\5.351513\\\\104.2778\\\\307.01\\\\5.249068\\\\103.1069\\\\307.01\\\\5.351513\\\\101.9359\\\\307.01\\\\5.655735\\\\100.8005\\\\307.01\\\\6.152492\\\\99.73524\\\\307.01\\\\6.826687\\\\98.77239\\\\307.01\\\\7.657838\\\\97.94124\\\\307.01\\\\8.620689\\\\97.26704\\\\307.01\\\\9.685987\\\\96.77029\\\\307.01\\\\10.82136\\\\96.46606\\\\307.01\\\\11.99231\\\\96.36362\\\\307.01\\\\13.16326\\\\96.46606\\\\307.01\\\\14.29864\\\\96.77029\\\\307.01\\\\15.36393\\\\97.26704\\\\307.01\\\\16.32678\\\\97.94124\\\\307.01\\\\17.15794\\\\98.77239\\\\307.01\\\\17.83213\\\\99.73524\\\\307.01\\\\18.32889\\\\100.8005\\\\307.01\\\\18.63311\\\\101.9359\\\\307.01\\\\18.73555\\\\103.1069\\\\307.01\\\\18.63311\\\\104.2778\\\\307.01\\\\18.32889\\\\105.4132\\\\307.01\\\\17.83213\\\\106.4785\\\\307.01\\\\17.15794\\\\107.4413\\\\307.01\\\\16.32678\\\\108.2725\\\\307.01\\\\15.36393\\\\108.9467\\\\307.01\\\\14.29864\\\\109.4434\\\\307.01\\\\13.16326\\\\109.7477\\\\307.01\\\\11.99231\\\\109.8501\\\\307.01\\\\10.82136\\\\109.7477\\\\307.01\\\\9.685987\\\\109.4434\\\\307.01\\\\8.620689\\\\108.9467\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"7.657838\\\\108.2725\\\\310.01\\\\6.826687\\\\107.4413\\\\310.01\\\\6.152492\\\\106.4785\\\\310.01\\\\5.655735\\\\105.4132\\\\310.01\\\\5.351513\\\\104.2778\\\\310.01\\\\5.249068\\\\103.1069\\\\310.01\\\\5.351513\\\\101.9359\\\\310.01\\\\5.655735\\\\100.8005\\\\310.01\\\\6.152492\\\\99.73524\\\\310.01\\\\6.826687\\\\98.77239\\\\310.01\\\\7.657838\\\\97.94124\\\\310.01\\\\8.620689\\\\97.26704\\\\310.01\\\\9.685987\\\\96.77029\\\\310.01\\\\10.82136\\\\96.46606\\\\310.01\\\\11.99231\\\\96.36362\\\\310.01\\\\13.16326\\\\96.46606\\\\310.01\\\\14.29864\\\\96.77029\\\\310.01\\\\15.36393\\\\97.26704\\\\310.01\\\\16.32678\\\\97.94124\\\\310.01\\\\17.15794\\\\98.77239\\\\310.01\\\\17.83213\\\\99.73524\\\\310.01\\\\18.32889\\\\100.8005\\\\310.01\\\\18.63311\\\\101.9359\\\\310.01\\\\18.73555\\\\103.1069\\\\310.01\\\\18.63311\\\\104.2778\\\\310.01\\\\18.32889\\\\105.4132\\\\310.01\\\\17.83213\\\\106.4785\\\\310.01\\\\17.15794\\\\107.4413\\\\310.01\\\\16.32678\\\\108.2725\\\\310.01\\\\15.36393\\\\108.9467\\\\310.01\\\\14.29864\\\\109.4434\\\\310.01\\\\13.16326\\\\109.7477\\\\310.01\\\\11.99231\\\\109.8501\\\\310.01\\\\10.82136\\\\109.7477\\\\310.01\\\\9.685987\\\\109.4434\\\\310.01\\\\8.620689\\\\108.9467\\\\310.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"0\\\\255\\\\255\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-37.967\\\\161.9664\\\\304.01\\\\-39.10237\\\\161.6622\\\\304.01\\\\-40.16767\\\\161.1655\\\\304.01\\\\-41.13052\\\\160.4913\\\\304.01\\\\-41.96167\\\\159.6601\\\\304.01\\\\-42.63587\\\\158.6973\\\\304.01\\\\-43.13263\\\\157.632\\\\304.01\\\\-43.43685\\\\156.4966\\\\304.01\\\\-43.53929\\\\155.3257\\\\304.01\\\\-43.43685\\\\154.1547\\\\304.01\\\\-43.13263\\\\153.0193\\\\304.01\\\\-42.63587\\\\151.954\\\\304.01\\\\-41.96167\\\\150.9912\\\\304.01\\\\-41.13052\\\\150.16\\\\304.01\\\\-40.16767\\\\149.4858\\\\304.01\\\\-39.10237\\\\148.9891\\\\304.01\\\\-37.967\\\\148.6849\\\\304.01\\\\-36.79605\\\\148.5824\\\\304.01\\\\-35.6251\\\\148.6849\\\\304.01\\\\-34.48972\\\\148.9891\\\\304.01\\\\-33.42443\\\\149.4858\\\\304.01\\\\-32.46157\\\\150.16\\\\304.01\\\\-31.63042\\\\150.9912\\\\304.01\\\\-30.95623\\\\151.954\\\\304.01\\\\-30.45947\\\\153.0193\\\\304.01\\\\-30.15525\\\\154.1547\\\\304.01\\\\-30.0528\\\\155.3257\\\\304.01\\\\-30.15525\\\\156.4966\\\\304.01\\\\-30.45947\\\\157.632\\\\304.01\\\\-30.95623\\\\158.6973\\\\304.01\\\\-31.63042\\\\159.6601\\\\304.01\\\\-32.46157\\\\160.4913\\\\304.01\\\\-33.42443\\\\161.1655\\\\304.01\\\\-34.48972\\\\161.6622\\\\304.01\\\\-35.6251\\\\161.9664\\\\304.01\\\\-36.79605\\\\162.0689\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-37.967\\\\161.9664\\\\307.01\\\\-39.10237\\\\161.6622\\\\307.01\\\\-40.16767\\\\161.1655\\\\307.01\\\\-41.13052\\\\160.4913\\\\307.01\\\\-41.96167\\\\159.6601\\\\307.01\\\\-42.63587\\\\158.6973\\\\307.01\\\\-43.13263\\\\157.632\\\\307.01\\\\-43.43685\\\\156.4966\\\\307.01\\\\-43.53929\\\\155.3257\\\\307.01\\\\-43.43685\\\\154.1547\\\\307.01\\\\-43.13263\\\\153.0193\\\\307.01\\\\-42.63587\\\\151.954\\\\307.01\\\\-41.96167\\\\150.9912\\\\307.01\\\\-41.13052\\\\150.16\\\\307.01\\\\-40.16767\\\\149.4858\\\\307.01\\\\-39.10237\\\\148.9891\\\\307.01\\\\-37.967\\\\148.6849\\\\307.01\\\\-36.79605\\\\148.5824\\\\307.01\\\\-35.6251\\\\148.6849\\\\307.01\\\\-34.48972\\\\148.9891\\\\307.01\\\\-33.42443\\\\149.4858\\\\307.01\\\\-32.46157\\\\150.16\\\\307.01\\\\-31.63042\\\\150.9912\\\\307.01\\\\-30.95623\\\\151.954\\\\307.01\\\\-30.45947\\\\153.0193\\\\307.01\\\\-30.15525\\\\154.1547\\\\307.01\\\\-30.0528\\\\155.3257\\\\307.01\\\\-30.15525\\\\156.4966\\\\307.01\\\\-30.45947\\\\157.632\\\\307.01\\\\-30.95623\\\\158.6973\\\\307.01\\\\-31.63042\\\\159.6601\\\\307.01\\\\-32.46157\\\\160.4913\\\\307.01\\\\-33.42443\\\\161.1655\\\\307.01\\\\-34.48972\\\\161.6622\\\\307.01\\\\-35.6251\\\\161.9664\\\\307.01\\\\-36.79605\\\\162.0689\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-37.967\\\\161.9664\\\\310.01\\\\-39.10237\\\\161.6622\\\\310.01\\\\-40.16767\\\\161.1655\\\\310.01\\\\-41.13052\\\\160.4913\\\\310.01\\\\-41.96167\\\\159.6601\\\\310.01\\\\-42.63587\\\\158.6973\\\\310.01\\\\-43.13263\\\\157.632\\\\310.01\\\\-43.43685\\\\156.4966\\\\310.01\\\\-43.53929\\\\155.3257\\\\310.01\\\\-43.43685\\\\154.1547\\\\310.01\\\\-43.13263\\\\153.0193\\\\310.01\\\\-42.63587\\\\151.954\\\\310.01\\\\-41.96167\\\\150.9912\\\\310.01\\\\-41.13052\\\\150.16\\\\310.01\\\\-40.16767\\\\149.4858\\\\310.01\\\\-39.10237\\\\148.9891\\\\310.01\\\\-37.967\\\\148.6849\\\\310.01\\\\-36.79605\\\\148.5824\\\\310.01\\\\-35.6251\\\\148.6849\\\\310.01\\\\-34.48972\\\\148.9891\\\\310.01\\\\-33.42443\\\\149.4858\\\\310.01\\\\-32.46157\\\\150.16\\\\310.01\\\\-31.63042\\\\150.9912\\\\310.01\\\\-30.95623\\\\151.954\\\\310.01\\\\-30.45947\\\\153.0193\\\\310.01\\\\-30.15525\\\\154.1547\\\\310.01\\\\-30.0528\\\\155.3257\\\\310.01\\\\-30.15525\\\\156.4966\\\\310.01\\\\-30.45947\\\\157.632\\\\310.01\\\\-30.95623\\\\158.6973\\\\310.01\\\\-31.63042\\\\159.6601\\\\310.01\\\\-32.46157\\\\160.4913\\\\310.01\\\\-33.42443\\\\161.1655\\\\310.01\\\\-34.48972\\\\161.6622\\\\310.01\\\\-35.6251\\\\161.9664\\\\310.01\\\\-36.79605\\\\162.0689\\\\310.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"2\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"255\\\\0\\\\255\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"69.4042\\\\150.7324\\\\304.01\\\\69.70842\\\\151.8678\\\\304.01\\\\69.81087\\\\153.0387\\\\304.01\\\\69.70842\\\\154.2097\\\\304.01\\\\69.4042\\\\155.345\\\\304.01\\\\68.90745\\\\156.4103\\\\304.01\\\\68.23325\\\\157.3732\\\\304.01\\\\67.4021\\\\158.2043\\\\304.01\\\\66.43925\\\\158.8785\\\\304.01\\\\65.37395\\\\159.3753\\\\304.01\\\\64.23858\\\\159.6795\\\\304.01\\\\63.06762\\\\159.7819\\\\304.01\\\\61.89667\\\\159.6795\\\\304.01\\\\60.7613\\\\159.3753\\\\304.01\\\\59.696\\\\158.8785\\\\304.01\\\\58.73315\\\\158.2043\\\\304.01\\\\57.902\\\\157.3732\\\\304.01\\\\57.22781\\\\156.4103\\\\304.01\\\\56.73105\\\\155.345\\\\304.01\\\\56.42683\\\\154.2097\\\\304.01\\\\56.32438\\\\153.0387\\\\304.01\\\\56.42683\\\\151.8678\\\\304.01\\\\56.73105\\\\150.7324\\\\304.01\\\\57.22781\\\\149.6671\\\\304.01\\\\57.902\\\\148.7042\\\\304.01\\\\58.73315\\\\147.8731\\\\304.01\\\\59.696\\\\147.1989\\\\304.01\\\\60.7613\\\\146.7021\\\\304.01\\\\61.89667\\\\146.3979\\\\304.01\\\\63.06762\\\\146.2955\\\\304.01\\\\64.23858\\\\146.3979\\\\304.01\\\\65.37395\\\\146.7021\\\\304.01\\\\66.43925\\\\147.1989\\\\304.01\\\\67.4021\\\\147.8731\\\\304.01\\\\68.23325\\\\148.7042\\\\304.01\\\\68.90745\\\\149.6671\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"69.4042\\\\150.7324\\\\307.01\\\\69.70842\\\\151.8678\\\\307.01\\\\69.81087\\\\153.0387\\\\307.01\\\\69.70842\\\\154.2097\\\\307.01\\\\69.4042\\\\155.345\\\\307.01\\\\68.90745\\\\156.4103\\\\307.01\\\\68.23325\\\\157.3732\\\\307.01\\\\67.4021\\\\158.2043\\\\307.01\\\\66.43925\\\\158.8785\\\\307.01\\\\65.37395\\\\159.3753\\\\307.01\\\\64.23858\\\\159.6795\\\\307.01\\\\63.06762\\\\159.7819\\\\307.01\\\\61.89667\\\\159.6795\\\\307.01\\\\60.7613\\\\159.3753\\\\307.01\\\\59.696\\\\158.8785\\\\307.01\\\\58.73315\\\\158.2043\\\\307.01\\\\57.902\\\\157.3732\\\\307.01\\\\57.22781\\\\156.4103\\\\307.01\\\\56.73105\\\\155.345\\\\307.01\\\\56.42683\\\\154.2097\\\\307.01\\\\56.32438\\\\153.0387\\\\307.01\\\\56.42683\\\\151.8678\\\\307.01\\\\56.73105\\\\150.7324\\\\307.01\\\\57.22781\\\\149.6671\\\\307.01\\\\57.902\\\\148.7042\\\\307.01\\\\58.73315\\\\147.8731\\\\307.01\\\\59.696\\\\147.1989\\\\307.01\\\\60.7613\\\\146.7021\\\\307.01\\\\61.89667\\\\146.3979\\\\307.01\\\\63.06762\\\\146.2955\\\\307.01\\\\64.23858\\\\146.3979\\\\307.01\\\\65.37395\\\\146.7021\\\\307.01\\\\66.43925\\\\147.1989\\\\307.01\\\\67.4021\\\\147.8731\\\\307.01\\\\68.23325\\\\148.7042\\\\307.01\\\\68.90745\\\\149.6671\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"69.4042\\\\150.7324\\\\310.01\\\\69.70842\\\\151.8678\\\\310.01\\\\69.81087\\\\153.0387\\\\310.01\\\\69.70842\\\\154.2097\\\\310.01\\\\69.4042\\\\155.345\\\\310.01\\\\68.90745\\\\156.4103\\\\310.01\\\\68.23325\\\\157.3732\\\\310.01\\\\67.4021\\\\158.2043\\\\310.01\\\\66.43925\\\\158.8785\\\\310.01\\\\65.37395\\\\159.3753\\\\310.01\\\\64.23858\\\\159.6795\\\\310.01\\\\63.06762\\\\159.7819\\\\310.01\\\\61.89667\\\\159.6795\\\\310.01\\\\60.7613\\\\159.3753\\\\310.01\\\\59.696\\\\158.8785\\\\310.01\\\\58.73315\\\\158.2043\\\\310.01\\\\57.902\\\\157.3732\\\\310.01\\\\57.22781\\\\156.4103\\\\310.01\\\\56.73105\\\\155.345\\\\310.01\\\\56.42683\\\\154.2097\\\\310.01\\\\56.32438\\\\153.0387\\\\310.01\\\\56.42683\\\\151.8678\\\\310.01\\\\56.73105\\\\150.7324\\\\310.01\\\\57.22781\\\\149.6671\\\\310.01\\\\57.902\\\\148.7042\\\\310.01\\\\58.73315\\\\147.8731\\\\310.01\\\\59.696\\\\147.1989\\\\310.01\\\\60.7613\\\\146.7021\\\\310.01\\\\61.89667\\\\146.3979\\\\310.01\\\\63.06762\\\\146.2955\\\\310.01\\\\64.23858\\\\146.3979\\\\310.01\\\\65.37395\\\\146.7021\\\\310.01\\\\66.43925\\\\147.1989\\\\310.01\\\\67.4021\\\\147.8731\\\\310.01\\\\68.23325\\\\148.7042\\\\310.01\\\\68.90745\\\\149.6671\\\\310.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"3\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"0\\\\0\\\\255\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"15.45022\\\\210.3737\\\\304.01\\\\14.27927\\\\210.4761\\\\304.01\\\\13.10831\\\\210.3737\\\\304.01\\\\11.97294\\\\210.0694\\\\304.01\\\\10.90764\\\\209.5727\\\\304.01\\\\9.944793\\\\208.8985\\\\304.01\\\\9.113642\\\\208.0673\\\\304.01\\\\8.439445\\\\207.1045\\\\304.01\\\\7.94269\\\\206.0392\\\\304.01\\\\7.638467\\\\204.9038\\\\304.01\\\\7.536023\\\\203.7328\\\\304.01\\\\7.638467\\\\202.5619\\\\304.01\\\\7.94269\\\\201.4265\\\\304.01\\\\8.439445\\\\200.3612\\\\304.01\\\\9.113642\\\\199.3984\\\\304.01\\\\9.944793\\\\198.5672\\\\304.01\\\\10.90764\\\\197.893\\\\304.01\\\\11.97294\\\\197.3963\\\\304.01\\\\13.10831\\\\197.0921\\\\304.01\\\\14.27927\\\\196.9896\\\\304.01\\\\15.45022\\\\197.0921\\\\304.01\\\\16.58559\\\\197.3963\\\\304.01\\\\17.65089\\\\197.893\\\\304.01\\\\18.61374\\\\198.5672\\\\304.01\\\\19.44489\\\\199.3984\\\\304.01\\\\20.11909\\\\200.3612\\\\304.01\\\\20.61584\\\\201.4265\\\\304.01\\\\20.92006\\\\202.5619\\\\304.01\\\\21.02251\\\\203.7328\\\\304.01\\\\20.92006\\\\204.9038\\\\304.01\\\\20.61584\\\\206.0392\\\\304.01\\\\20.11909\\\\207.1045\\\\304.01\\\\19.44489\\\\208.0673\\\\304.01\\\\18.61374\\\\208.8985\\\\304.01\\\\17.65089\\\\209.5727\\\\304.01\\\\16.58559\\\\210.0694\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"15.45022\\\\210.3737\\\\307.01\\\\14.27927\\\\210.4761\\\\307.01\\\\13.10831\\\\210.3737\\\\307.01\\\\11.97294\\\\210.0694\\\\307.01\\\\10.90764\\\\209.5727\\\\307.01\\\\9.944793\\\\208.8985\\\\307.01\\\\9.113642\\\\208.0673\\\\307.01\\\\8.439445\\\\207.1045\\\\307.01\\\\7.94269\\\\206.0392\\\\307.01\\\\7.638467\\\\204.9038\\\\307.01\\\\7.536023\\\\203.7328\\\\307.01\\\\7.638467\\\\202.5619\\\\307.01\\\\7.94269\\\\201.4265\\\\307.01\\\\8.439445\\\\200.3612\\\\307.01\\\\9.113642\\\\199.3984\\\\307.01\\\\9.944793\\\\198.5672\\\\307.01\\\\10.90764\\\\197.893\\\\307.01\\\\11.97294\\\\197.3963\\\\307.01\\\\13.10831\\\\197.0921\\\\307.01\\\\14.27927\\\\196.9896\\\\307.01\\\\15.45022\\\\197.0921\\\\307.01\\\\16.58559\\\\197.3963\\\\307.01\\\\17.65089\\\\197.893\\\\307.01\\\\18.61374\\\\198.5672\\\\307.01\\\\19.44489\\\\199.3984\\\\307.01\\\\20.11909\\\\200.3612\\\\307.01\\\\20.61584\\\\201.4265\\\\307.01\\\\20.92006\\\\202.5619\\\\307.01\\\\21.02251\\\\203.7328\\\\307.01\\\\20.92006\\\\204.9038\\\\307.01\\\\20.61584\\\\206.0392\\\\307.01\\\\20.11909\\\\207.1045\\\\307.01\\\\19.44489\\\\208.0673\\\\307.01\\\\18.61374\\\\208.8985\\\\307.01\\\\17.65089\\\\209.5727\\\\307.01\\\\16.58559\\\\210.0694\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"15.45022\\\\210.3737\\\\310.01\\\\14.27927\\\\210.4761\\\\310.01\\\\13.10831\\\\210.3737\\\\310.01\\\\11.97294\\\\210.0694\\\\310.01\\\\10.90764\\\\209.5727\\\\310.01\\\\9.944793\\\\208.8985\\\\310.01\\\\9.113642\\\\208.0673\\\\310.01\\\\8.439445\\\\207.1045\\\\310.01\\\\7.94269\\\\206.0392\\\\310.01\\\\7.638467\\\\204.9038\\\\310.01\\\\7.536023\\\\203.7328\\\\310.01\\\\7.638467\\\\202.5619\\\\310.01\\\\7.94269\\\\201.4265\\\\310.01\\\\8.439445\\\\200.3612\\\\310.01\\\\9.113642\\\\199.3984\\\\310.01\\\\9.944793\\\\198.5672\\\\310.01\\\\10.90764\\\\197.893\\\\310.01\\\\11.97294\\\\197.3963\\\\310.01\\\\13.10831\\\\197.0921\\\\310.01\\\\14.27927\\\\196.9896\\\\310.01\\\\15.45022\\\\197.0921\\\\310.01\\\\16.58559\\\\197.3963\\\\310.01\\\\17.65089\\\\197.893\\\\310.01\\\\18.61374\\\\198.5672\\\\310.01\\\\19.44489\\\\199.3984\\\\310.01\\\\20.11909\\\\200.3612\\\\310.01\\\\20.61584\\\\201.4265\\\\310.01\\\\20.92006\\\\202.5619\\\\310.01\\\\21.02251\\\\203.7328\\\\310.01\\\\20.92006\\\\204.9038\\\\310.01\\\\20.61584\\\\206.0392\\\\310.01\\\\20.11909\\\\207.1045\\\\310.01\\\\19.44489\\\\208.0673\\\\310.01\\\\18.61374\\\\208.8985\\\\310.01\\\\17.65089\\\\209.5727\\\\310.01\\\\16.58559\\\\210.0694\\\\310.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"4\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"0\\\\128\\\\255\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.97014\\\\164.0225\\\\295.01\\\\13.79919\\\\164.1249\\\\295.01\\\\12.62824\\\\164.0225\\\\295.01\\\\11.49286\\\\163.7183\\\\295.01\\\\10.42757\\\\163.2215\\\\295.01\\\\9.464716\\\\162.5473\\\\295.01\\\\8.633565\\\\161.7162\\\\295.01\\\\7.95937\\\\160.7533\\\\295.01\\\\7.462614\\\\159.688\\\\295.01\\\\7.158391\\\\158.5526\\\\295.01\\\\7.055946\\\\157.3817\\\\295.01\\\\7.158391\\\\156.2107\\\\295.01\\\\7.462614\\\\155.0753\\\\295.01\\\\7.95937\\\\154.0101\\\\295.01\\\\8.633565\\\\153.0472\\\\295.01\\\\9.464716\\\\152.216\\\\295.01\\\\10.42757\\\\151.5419\\\\295.01\\\\11.49286\\\\151.0451\\\\295.01\\\\12.62824\\\\150.7409\\\\295.01\\\\13.79919\\\\150.6384\\\\295.01\\\\14.97014\\\\150.7409\\\\295.01\\\\16.10551\\\\151.0451\\\\295.01\\\\17.17081\\\\151.5419\\\\295.01\\\\18.13366\\\\152.216\\\\295.01\\\\18.96481\\\\153.0472\\\\295.01\\\\19.63901\\\\154.0101\\\\295.01\\\\20.13577\\\\155.0753\\\\295.01\\\\20.43999\\\\156.2107\\\\295.01\\\\20.54243\\\\157.3817\\\\295.01\\\\20.43999\\\\158.5526\\\\295.01\\\\20.13577\\\\159.688\\\\295.01\\\\19.63901\\\\160.7533\\\\295.01\\\\18.96481\\\\161.7162\\\\295.01\\\\18.13366\\\\162.5473\\\\295.01\\\\17.17081\\\\163.2215\\\\295.01\\\\16.10551\\\\163.7183\\\\295.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n";
const char* k_rtStruct_json01 =
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.97014\\\\164.0225\\\\298.01\\\\13.79919\\\\164.1249\\\\298.01\\\\12.62824\\\\164.0225\\\\298.01\\\\11.49286\\\\163.7183\\\\298.01\\\\10.42757\\\\163.2215\\\\298.01\\\\9.464716\\\\162.5473\\\\298.01\\\\8.633565\\\\161.7162\\\\298.01\\\\7.95937\\\\160.7533\\\\298.01\\\\7.462614\\\\159.688\\\\298.01\\\\7.158391\\\\158.5526\\\\298.01\\\\7.055946\\\\157.3817\\\\298.01\\\\7.158391\\\\156.2107\\\\298.01\\\\7.462614\\\\155.0753\\\\298.01\\\\7.95937\\\\154.0101\\\\298.01\\\\8.633565\\\\153.0472\\\\298.01\\\\9.464716\\\\152.216\\\\298.01\\\\10.42757\\\\151.5419\\\\298.01\\\\11.49286\\\\151.0451\\\\298.01\\\\12.62824\\\\150.7409\\\\298.01\\\\13.79919\\\\150.6384\\\\298.01\\\\14.97014\\\\150.7409\\\\298.01\\\\16.10551\\\\151.0451\\\\298.01\\\\17.17081\\\\151.5419\\\\298.01\\\\18.13366\\\\152.216\\\\298.01\\\\18.96481\\\\153.0472\\\\298.01\\\\19.63901\\\\154.0101\\\\298.01\\\\20.13577\\\\155.0753\\\\298.01\\\\20.43999\\\\156.2107\\\\298.01\\\\20.54243\\\\157.3817\\\\298.01\\\\20.43999\\\\158.5526\\\\298.01\\\\20.13577\\\\159.688\\\\298.01\\\\19.63901\\\\160.7533\\\\298.01\\\\18.96481\\\\161.7162\\\\298.01\\\\18.13366\\\\162.5473\\\\298.01\\\\17.17081\\\\163.2215\\\\298.01\\\\16.10551\\\\163.7183\\\\298.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.97014\\\\164.0225\\\\301.01\\\\13.79919\\\\164.1249\\\\301.01\\\\12.62824\\\\164.0225\\\\301.01\\\\11.49286\\\\163.7183\\\\301.01\\\\10.42757\\\\163.2215\\\\301.01\\\\9.464716\\\\162.5473\\\\301.01\\\\8.633565\\\\161.7162\\\\301.01\\\\7.95937\\\\160.7533\\\\301.01\\\\7.462614\\\\159.688\\\\301.01\\\\7.158391\\\\158.5526\\\\301.01\\\\7.055946\\\\157.3817\\\\301.01\\\\7.158391\\\\156.2107\\\\301.01\\\\7.462614\\\\155.0753\\\\301.01\\\\7.95937\\\\154.0101\\\\301.01\\\\8.633565\\\\153.0472\\\\301.01\\\\9.464716\\\\152.216\\\\301.01\\\\10.42757\\\\151.5419\\\\301.01\\\\11.49286\\\\151.0451\\\\301.01\\\\12.62824\\\\150.7409\\\\301.01\\\\13.79919\\\\150.6384\\\\301.01\\\\14.97014\\\\150.7409\\\\301.01\\\\16.10551\\\\151.0451\\\\301.01\\\\17.17081\\\\151.5419\\\\301.01\\\\18.13366\\\\152.216\\\\301.01\\\\18.96481\\\\153.0472\\\\301.01\\\\19.63901\\\\154.0101\\\\301.01\\\\20.13577\\\\155.0753\\\\301.01\\\\20.43999\\\\156.2107\\\\301.01\\\\20.54243\\\\157.3817\\\\301.01\\\\20.43999\\\\158.5526\\\\301.01\\\\20.13577\\\\159.688\\\\301.01\\\\19.63901\\\\160.7533\\\\301.01\\\\18.96481\\\\161.7162\\\\301.01\\\\18.13366\\\\162.5473\\\\301.01\\\\17.17081\\\\163.2215\\\\301.01\\\\16.10551\\\\163.7183\\\\301.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"5\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"0\\\\128\\\\0\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"340\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"108.3984\\\\232.7406\\\\274.01\\\\106.0547\\\\231.7948\\\\274.01\\\\103.7109\\\\232.8407\\\\274.01\\\\96.67969\\\\232.8757\\\\274.01\\\\77.92969\\\\232.887\\\\274.01\\\\47.46094\\\\232.8902\\\\274.01\\\\38.08594\\\\232.7537\\\\274.01\\\\37.6668\\\\232.3734\\\\274.01\\\\38.08594\\\\231.9774\\\\274.01\\\\40.42969\\\\231.8475\\\\274.01\\\\41.76413\\\\230.0297\\\\274.01\\\\42.77344\\\\229.1388\\\\274.01\\\\45.11719\\\\228.5069\\\\274.01\\\\47.46094\\\\227.1533\\\\274.01\\\\49.80469\\\\226.3505\\\\274.01\\\\52.14844\\\\224.6564\\\\274.01\\\\54.49219\\\\223.923\\\\274.01\\\\56.83594\\\\222.0692\\\\274.01\\\\59.17969\\\\220.3438\\\\274.01\\\\61.52344\\\\219.3888\\\\274.01\\\\63.86719\\\\217.1287\\\\274.01\\\\65.83488\\\\215.9672\\\\274.01\\\\68.55469\\\\213.2383\\\\274.01\\\\70.89844\\\\211.2328\\\\274.01\\\\72.8125\\\\208.9359\\\\274.01\\\\75.58594\\\\206.3615\\\\274.01\\\\76.91445\\\\204.2484\\\\274.01\\\\78.89509\\\\201.9047\\\\274.01\\\\80.51276\\\\199.5609\\\\274.01\\\\81.51955\\\\197.2172\\\\274.01\\\\83.67448\\\\194.8734\\\\274.01\\\\84.60938\\\\192.5297\\\\274.01\\\\85.86986\\\\190.1859\\\\274.01\\\\86.57623\\\\187.8422\\\\274.01\\\\88.30051\\\\185.4984\\\\274.01\\\\88.94002\\\\183.1547\\\\274.01\\\\89.23261\\\\180.8109\\\\274.01\\\\89.64844\\\\180.3263\\\\274.01\\\\90.71885\\\\178.4672\\\\274.01\\\\90.97656\\\\176.1234\\\\274.01\\\\91.99219\\\\174.4794\\\\274.01\\\\92.56773\\\\173.7797\\\\274.01\\\\92.80016\\\\171.4359\\\\274.01\\\\93.23473\\\\169.0922\\\\274.01\\\\93.37606\\\\166.7484\\\\274.01\\\\93.60748\\\\157.3734\\\\274.01\\\\93.6341\\\\152.6859\\\\274.01\\\\93.35742\\\\140.9672\\\\274.01\\\\92.89317\\\\138.6234\\\\274.01\\\\92.7069\\\\136.2797\\\\274.01\\\\92.03726\\\\133.9359\\\\274.01\\\\90.84009\\\\131.5922\\\\274.01\\\\90.3769\\\\129.2484\\\\274.01\\\\89.09074\\\\126.9047\\\\274.01\\\\88.13225\\\\122.2172\\\\274.01\\\\86.17828\\\\119.8734\\\\274.01\\\\84.96094\\\\117.4163\\\\274.01\\\\83.99619\\\\115.1859\\\\274.01\\\\83.13079\\\\112.8422\\\\274.01\\\\82.61719\\\\112.2984\\\\274.01\\\\80.27344\\\\108.8454\\\\274.01\\\\79.64514\\\\108.1547\\\\274.01\\\\77.21497\\\\105.8109\\\\274.01\\\\76.47787\\\\103.4672\\\\274.01\\\\75.58594\\\\102.6177\\\\274.01\\\\73.24219\\\\100.0077\\\\274.01\\\\69.54492\\\\96.43594\\\\274.01\\\\67.34096\\\\94.09219\\\\274.01\\\\64.66306\\\\91.74844\\\\274.01\\\\63.86719\\\\90.92619\\\\274.01\\\\61.52344\\\\90.20454\\\\274.01\\\\59.17969\\\\87.78574\\\\274.01\\\\56.83594\\\\86.48566\\\\274.01\\\\54.49219\\\\84.31388\\\\274.01\\\\52.14844\\\\83.44438\\\\274.01\\\\49.80469\\\\82.75121\\\\274.01\\\\49.37617\\\\82.37344\\\\274.01\\\\47.46094\\\\81.26244\\\\274.01\\\\45.71391\\\\80.02969\\\\274.01\\\\45.11719\\\\79.45415\\\\274.01\\\\42.77344\\\\79.08185\\\\274.01\\\\40.42969\\\\78.51941\\\\274.01\\\\38.08594\\\\78.27534\\\\274.01\\\\37.36932\\\\77.68594\\\\274.01\\\\35.74219\\\\76.67624\\\\274.01\\\\33.39844\\\\76.49941\\\\274.01\\\\31.05469\\\\76.03495\\\\274.01\\\\28.71094\\\\74.83174\\\\274.01\\\\26.36719\\\\74.62859\\\\274.01\\\\24.02344\\\\74.55463\\\\274.01\\\\21.67969\\\\74.22861\\\\274.01\\\\19.33594\\\\74.05312\\\\274.01\\\\12.30469\\\\73.99397\\\\274.01\\\\5.273438\\\\74.0736\\\\274.01\\\\2.929688\\\\74.55463\\\\274.01\\\\0.5859375\\\\74.68513\\\\274.01\\\\-1.757813\\\\74.914\\\\274.01\\\\-2.319131\\\\75.34219\\\\274.01\\\\-4.101563\\\\76.31516\\\\274.01\\\\-8.789063\\\\76.74514\\\\274.01\\\\-11.13281\\\\78.39038\\\\274.01\\\\-13.47656\\\\78.6124\\\\274.01\\\\-15.82031\\\\79.19784\\\\274.01\\\\-18.16406\\\\81.11024\\\\274.01\\\\-20.50781\\\\82.03296\\\\274.01\\\\-22.85156\\\\83.13991\\\\274.01\\\\-25.19531\\\\83.70732\\\\274.01\\\\-27.53906\\\\85.85863\\\\274.01\\\\-29.88281\\\\87.03368\\\\274.01\\\\-32.22656\\\\88.3274\\\\274.01\\\\-34.57031\\\\90.53674\\\\274.01\\\\-36.91406\\\\92.5602\\\\274.01\\\\-39.25781\\\\93.55952\\\\274.01\\\\-41.60156\\\\95.74537\\\\274.01\\\\-43.94531\\\\98.26609\\\\274.01\\\\-46.28906\\\\100.3701\\\\274.01\\\\-47.02621\\\\101.1234\\\\274.01\\\\-47.86611\\\\103.4672\\\\274.01\\\\-49.83594\\\\105.8109\\\\274.01\\\\-51.98182\\\\108.1547\\\\274.01\\\\-53.06448\\\\110.4984\\\\274.01\\\\-53.32031\\\\110.7675\\\\274.01\\\\-54.53804\\\\112.8422\\\\274.01\\\\-55.66406\\\\114.273\\\\274.01\\\\-56.55722\\\\115.1859\\\\274.01\\\\-57.13953\\\\117.5297\\\\274.01\\\\-58.29264\\\\119.8734\\\\274.01\\\\-59.26869\\\\122.2172\\\\274.01\\\\-60.35156\\\\124.0119\\\\274.01\\\\-60.84229\\\\124.5609\\\\274.01\\\\-61.54484\\\\126.9047\\\\274.01\\\\-61.71691\\\\129.2484\\\\274.01\\\\-63.62281\\\\131.5922\\\\274.01\\\\-63.81256\\\\133.9359\\\\274.01\\\\-64.12511\\\\136.2797\\\\274.01\\\\-64.84515\\\\138.6234\\\\274.01\\\\-65.13599\\\\140.9672\\\\274.01\\\\-65.33604\\\\143.3109\\\\274.01\\\\-65.87358\\\\145.6547\\\\274.01\\\\-66.10577\\\\147.9984\\\\274.01\\\\-66.17618\\\\155.0297\\\\274.01\\\\-66.09933\\\\162.0609\\\\274.01\\\\-65.40382\\\\164.4047\\\\274.01\\\\-65.24833\\\\166.7484\\\\274.01\\\\-64.71442\\\\171.4359\\\\274.01\\\\-63.88171\\\\173.7797\\\\274.01\\\\-63.69299\\\\176.1234\\\\274.01\\\\-61.79081\\\\178.4672\\\\274.01\\\\-61.59269\\\\180.8109\\\\274.01\\\\-61.19405\\\\183.1547\\\\274.01\\\\-60.35156\\\\185.2055\\\\274.01\\\\-59.08288\\\\187.8422\\\\274.01\\\\-58.00781\\\\189.3499\\\\274.01\\\\-57.25858\\\\190.1859\\\\274.01\\\\-56.64558\\\\192.5297\\\\274.01\\\\-55.29191\\\\194.8734\\\\274.01\\\\-54.28698\\\\197.2172\\\\274.01\\\\-52.28595\\\\199.5609\\\\274.01\\\\-51.47569\\\\201.9047\\\\274.01\\\\-48.63281\\\\204.6417\\\\274.01\\\\-47.10181\\\\206.5922\\\\274.01\\\\-44.64154\\\\208.9359\\\\274.01\\\\-42.38504\\\\211.2797\\\\274.01\\\\-39.25781\\\\214.4025\\\\274.01\\\\-37.42723\\\\215.9672\\\\274.01\\\\-34.57031\\\\218.9107\\\\274.01\\\\-32.22656\\\\219.7277\\\\274.01\\\\-29.88281\\\\221.6934\\\\274.01\\\\-27.53906\\\\222.852\\\\274.01\\\\-25.19531\\\\224.5168\\\\274.01\\\\-22.85156\\\\225.9419\\\\274.01\\\\-20.50781\\\\226.7359\\\\274.01\\\\-18.16406\\\\228.3332\\\\274.01\\\\-15.82031\\\\229.065\\\\274.01\\\\-13.47656\\\\229.267\\\\274.01\\\\-12.63854\\\\230.0297\\\\274.01\\\\-11.13281\\\\231.9201\\\\274.01\\\\-10.65505\\\\232.3734\\\\274.01\\\\-11.13281\\\\232.7794\\\\274.01\\\\-15.82031\\\\232.792\\\\274.01\\\\-18.16406\\\\232.8902\\\\274.01\\\\-36.91406\\\\232.9015\\\\274.01\\\\-39.25781\\\\232.8902\\\\274.01\\\\-50.97656\\\\232.9236\\\\274.01\\\\-60.35156\\\\232.9126\\\\274.01\\\\-67.38281\\\\232.8407\\\\274.01\\\\-72.07031\\\\232.8642\\\\274.01\\\\-79.10156\\\\232.8555\\\\274.01\\\\-83.78906\\\\232.8788\\\\274.01\\\\-95.50781\\\\232.8902\\\\274.01\\\\-97.85156\\\\233.4886\\\\274.01\\\\-100.1953\\\\233.647\\\\274.01\\\\-102.5391\\\\232.9858\\\\274.01\\\\-104.8828\\\\233.6969\\\\274.01\\\\-109.5703\\\\233.722\\\\274.01\\\\-125.9766\\\\233.7086\\\\274.01\\\\-128.3203\\\\233.2849\\\\274.01\\\\-130.6641\\\\233.702\\\\274.01\\\\-135.3516\\\\233.727\\\\274.01\\\\-149.4141\\\\233.7135\\\\274.01\\\\-156.4453\\\\233.727\\\\274.01\\\\-163.4766\\\\233.7119\\\\274.01\\\\-168.1641\\\\233.7643\\\\274.01\\\\-191.6016\\\\233.7809\\\\274.01\\\\-210.3516\\\\233.7716\\\\274.01\\\\-224.4141\\\\233.7998\\\\274.01\\\\-233.7891\\\\233.7647\\\\274.01\\\\-243.1641\\\\233.7785\\\\274.01\\\\-247.8516\\\\233.7378\\\\274.01\\\\-254.8828\\\\233.8578\\\\274.01\\\\-257.2266\\\\235.2519\\\\274.01\\\\-259.5703\\\\236.0817\\\\274.01\\\\-260.7617\\\\237.0609\\\\274.01\\\\-261.9141\\\\238.2262\\\\274.01\\\\-262.8989\\\\239.4047\\\\274.01\\\\-262.9743\\\\241.7484\\\\274.01\\\\-262.5977\\\\244.0922\\\\274.01\\\\-260.6675\\\\246.4359\\\\274.01\\\\-259.6161\\\\248.7797\\\\274.01\\\\-257.2266\\\\251.0035\\\\274.01\\\\-255.0361\\\\253.4672\\\\274.01\\\\-252.5391\\\\256.0995\\\\274.01\\\\-251.2277\\\\258.1547\\\\274.01\\\\-246.7444\\\\262.8422\\\\274.01\\\\-243.1641\\\\266.3515\\\\274.01\\\\-239.7411\\\\269.8734\\\\274.01\\\\-238.4766\\\\270.9495\\\\274.01\\\\-237.2269\\\\272.2172\\\\274.01\\\\-236.1328\\\\273.5215\\\\274.01\\\\-235.0934\\\\274.5609\\\\274.01\\\\-233.7891\\\\275.6655\\\\274.01\\\\-232.5319\\\\276.9047\\\\274.01\\\\-231.4453\\\\278.1693\\\\274.01\\\\-227.917\\\\281.5922\\\\274.01\\\\-224.4141\\\\285.1802\\\\274.01\\\\-222.0703\\\\287.4025\\\\274.01\\\\-218.6841\\\\290.9672\\\\274.01\\\\-217.3828\\\\291.9709\\\\274.01\\\\-215.0391\\\\293.1788\\\\274.01\\\\-212.6953\\\\294.5138\\\\274.01\\\\-210.3516\\\\295.2614\\\\274.01\\\\-209.8994\\\\295.6547\\\\274.01\\\\-208.0078\\\\296.7083\\\\274.01\\\\-203.3203\\\\296.9372\\\\274.01\\\\-196.2891\\\\296.9317\\\\274.01\\\\-193.9453\\\\296.8988\\\\274.01\\\\-172.8516\\\\296.8482\\\\274.01\\\\-161.1328\\\\296.843\\\\274.01\\\\-137.6953\\\\296.8542\\\\274.01\\\\-130.6641\\\\296.8378\\\\274.01\\\\-107.2266\\\\296.8379\\\\274.01\\\\-93.16406\\\\296.8208\\\\274.01\\\\-74.41406\\\\296.838\\\\274.01\\\\-65.03906\\\\296.8609\\\\274.01\\\\-50.97656\\\\296.8556\\\\274.01\\\\-46.28906\\\\296.9051\\\\274.01\\\\-41.60156\\\\298.5331\\\\274.01\\\\-39.25781\\\\298.5624\\\\274.01\\\\-36.91406\\\\297.1455\\\\274.01\\\\-34.57031\\\\297.0498\\\\274.01\\\\-32.22656\\\\298.5589\\\\274.01\\\\-25.19531\\\\298.5624\\\\274.01\\\\-22.85156\\\\297.2842\\\\274.01\\\\-20.50781\\\\298.5624\\\\274.01\\\\-1.757813\\\\298.5624\\\\274.01\\\\0.5859375\\\\297.2104\\\\274.01\\\\2.929688\\\\298.5624\\\\274.01\\\\5.273438\\\\297.6946\\\\274.01\\\\7.617188\\\\298.5168\\\\274.01\\\\9.960938\\\\298.5512\\\\274.01\\\\12.30469\\\\296.937\\\\274.01\\\\14.64844\\\\298.5478\\\\274.01\\\\16.99219\\\\298.5478\\\\274.01\\\\19.33594\\\\297.0782\\\\274.01\\\\21.67969\\\\296.844\\\\274.01\\\\23.54531\\\\297.9984\\\\274.01\\\\24.02344\\\\298.4023\\\\274.01\\\\24.50156\\\\297.9984\\\\274.01\\\\26.36719\\\\296.844\\\\274.01\\\\38.08594\\\\296.8381\\\\274.01\\\\52.14844\\\\296.8033\\\\274.01\\\\59.17969\\\\296.8033\\\\274.01\\\\73.24219\\\\296.7682\\\\274.01\\\\99.02344\\\\296.7566\\\\274.01\\\\117.7734\\\\296.7216\\\\274.01\\\\129.4922\\\\296.7152\\\\274.01\\\\131.8359\\\\295.9083\\\\274.01\\\\134.1797\\\\295.5245\\\\274.01\\\\138.8672\\\\295.4763\\\\274.01\\\\155.2734\\\\295.4763\\\\274.01\\\\176.3672\\\\295.3861\\\\274.01\\\\190.4297\\\\295.3718\\\\274.01\\\\197.4609\\\\295.4763\\\\274.01\\\\202.1484\\\\295.4454\\\\274.01\\\\204.4922\\\\295.3438\\\\274.01\\\\206.8359\\\\295.0757\\\\274.01\\\\209.1797\\\\294.4124\\\\274.01\\\\211.5234\\\\292.3133\\\\274.01\\\\213.8672\\\\291.0809\\\\274.01\\\\216.2109\\\\289.6743\\\\274.01\\\\217.3081\\\\288.6234\\\\274.01\\\\219.3558\\\\286.2797\\\\274.01\\\\221.8608\\\\283.9359\\\\274.01\\\\225.5859\\\\280.045\\\\274.01\\\\227.9297\\\\277.8885\\\\274.01\\\\230.2734\\\\275.2857\\\\274.01\\\\232.6172\\\\273.2225\\\\274.01\\\\233.6225\\\\272.2172\\\\274.01\\\\234.9609\\\\270.5822\\\\274.01\\\\238.2254\\\\267.5297\\\\274.01\\\\240.3691\\\\265.1859\\\\274.01\\\\244.3359\\\\261.3326\\\\274.01\\\\246.6797\\\\258.8034\\\\274.01\\\\249.0234\\\\256.7196\\\\274.01\\\\251.3672\\\\254.0746\\\\274.01\\\\254.5313\\\\251.1234\\\\274.01\\\\255.333\\\\248.7797\\\\274.01\\\\257.3723\\\\246.4359\\\\274.01\\\\259.7201\\\\244.0922\\\\274.01\\\\260.106\\\\241.7484\\\\274.01\\\\261.6423\\\\239.4047\\\\274.01\\\\261.0804\\\\237.0609\\\\274.01\\\\259.3552\\\\234.7172\\\\274.01\\\\258.3984\\\\233.7696\\\\274.01\\\\256.0547\\\\232.8757\\\\274.01\\\\253.7109\\\\232.792\\\\274.01\\\\251.3672\\\\232.8161\\\\274.01\\\\246.6797\\\\232.6981\\\\274.01\\\\244.3359\\\\232.725\\\\274.01\\\\239.6484\\\\232.9137\\\\274.01\\\\234.9609\\\\232.8525\\\\274.01\\\\225.5859\\\\232.8757\\\\274.01\\\\209.1797\\\\232.8757\\\\274.01\\\\204.4922\\\\232.7537\\\\274.01\\\\195.1172\\\\232.7794\\\\274.01\\\\171.6797\\\\232.792\\\\274.01\\\\164.6484\\\\232.7666\\\\274.01\\\\152.9297\\\\232.7666\\\\274.01\\\\148.2422\\\\232.792\\\\274.01\\\\138.8672\\\\232.7406\\\\274.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"380\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0671\\\\277.01\\\\-32.22656\\\\233.1221\\\\277.01\\\\-41.60156\\\\233.1397\\\\277.01\\\\-43.94531\\\\233.1221\\\\277.01\\\\-50.97656\\\\233.1818\\\\277.01\\\\-60.35156\\\\233.1568\\\\277.01\\\\-67.38281\\\\232.9942\\\\277.01\\\\-74.41406\\\\233.0537\\\\277.01\\\\-79.10156\\\\233.0283\\\\277.01\\\\-88.47656\\\\233.0859\\\\277.01\\\\-95.50781\\\\233.1042\\\\277.01\\\\-97.85156\\\\234.0105\\\\277.01\\\\-100.1953\\\\234.062\\\\277.01\\\\-102.5391\\\\233.987\\\\277.01\\\\-104.8828\\\\234.0941\\\\277.01\\\\-109.5703\\\\234.1996\\\\277.01\\\\-114.2578\\\\234.1551\\\\277.01\\\\-123.6328\\\\234.1127\\\\277.01\\\\-125.9766\\\\234.1266\\\\277.01\\\\-128.3203\\\\234.0177\\\\277.01\\\\-135.3516\\\\234.1741\\\\277.01\\\\-140.0391\\\\234.1551\\\\277.01\\\\-142.3828\\\\234.1996\\\\277.01\\\\-147.0703\\\\234.1174\\\\277.01\\\\-156.4453\\\\234.1828\\\\277.01\\\\-163.4766\\\\234.1357\\\\277.01\\\\-165.8203\\\\234.1871\\\\277.01\\\\-168.1641\\\\234.3922\\\\277.01\\\\-177.5391\\\\234.4145\\\\277.01\\\\-179.8828\\\\234.3618\\\\277.01\\\\-184.5703\\\\234.3864\\\\277.01\\\\-186.9141\\\\234.4535\\\\277.01\\\\-189.2578\\\\234.4194\\\\277.01\\\\-191.6016\\\\234.471\\\\277.01\\\\-198.6328\\\\234.3891\\\\277.01\\\\-200.9766\\\\234.4386\\\\277.01\\\\-205.6641\\\\234.3731\\\\277.01\\\\-210.3516\\\\234.3731\\\\277.01\\\\-217.3828\\\\234.4242\\\\277.01\\\\-219.7266\\\\234.5104\\\\277.01\\\\-224.4141\\\\234.4944\\\\277.01\\\\-229.1016\\\\234.4218\\\\277.01\\\\-233.7891\\\\234.2678\\\\277.01\\\\-240.8203\\\\234.3296\\\\277.01\\\\-243.1641\\\\234.3917\\\\277.01\\\\-247.8516\\\\234.2029\\\\277.01\\\\-250.1953\\\\234.3235\\\\277.01\\\\-251.4551\\\\234.7172\\\\277.01\\\\-252.5391\\\\236.6024\\\\277.01\\\\-254.8828\\\\235.4008\\\\277.01\\\\-257.2266\\\\235.5063\\\\277.01\\\\-259.5703\\\\236.2655\\\\277.01\\\\-260.5737\\\\237.0609\\\\277.01\\\\-262.332\\\\239.4047\\\\277.01\\\\-262.689\\\\241.7484\\\\277.01\\\\-261.9141\\\\243.7504\\\\277.01\\\\-260.6889\\\\246.4359\\\\277.01\\\\-259.5703\\\\248.4867\\\\277.01\\\\-255.2138\\\\253.4672\\\\277.01\\\\-253.2667\\\\255.8109\\\\277.01\\\\-250.1953\\\\258.8662\\\\277.01\\\\-247.8516\\\\261.4125\\\\277.01\\\\-246.2862\\\\262.8422\\\\277.01\\\\-244.1278\\\\265.1859\\\\277.01\\\\-240.8203\\\\268.3467\\\\277.01\\\\-239.375\\\\269.8734\\\\277.01\\\\-236.9429\\\\272.2172\\\\277.01\\\\-233.7891\\\\275.382\\\\277.01\\\\-231.4453\\\\277.8091\\\\277.01\\\\-229.8568\\\\279.2484\\\\277.01\\\\-227.6842\\\\281.5922\\\\277.01\\\\-225.1387\\\\283.9359\\\\277.01\\\\-222.0703\\\\287.1957\\\\277.01\\\\-220.5108\\\\288.6234\\\\277.01\\\\-218.2596\\\\290.9672\\\\277.01\\\\-217.3828\\\\291.6321\\\\277.01\\\\-215.0391\\\\292.8542\\\\277.01\\\\-214.579\\\\293.3109\\\\277.01\\\\-212.6953\\\\294.5394\\\\277.01\\\\-210.3516\\\\295.0909\\\\277.01\\\\-209.68\\\\295.6547\\\\277.01\\\\-208.0078\\\\296.4548\\\\277.01\\\\-205.6641\\\\296.8658\\\\277.01\\\\-203.3203\\\\297.1639\\\\277.01\\\\-196.2891\\\\297.1488\\\\277.01\\\\-193.9453\\\\297.0495\\\\277.01\\\\-179.8828\\\\296.9703\\\\277.01\\\\-172.8516\\\\296.9012\\\\277.01\\\\-168.1641\\\\296.9407\\\\277.01\\\\-161.1328\\\\296.9063\\\\277.01\\\\-156.4453\\\\296.9453\\\\277.01\\\\-151.7578\\\\296.9222\\\\277.01\\\\-147.0703\\\\296.9559\\\\277.01\\\\-144.7266\\\\296.9085\\\\277.01\\\\-135.3516\\\\296.92\\\\277.01\\\\-121.2891\\\\296.8532\\\\277.01\\\\-107.2266\\\\296.8819\\\\277.01\\\\-93.16406\\\\296.7973\\\\277.01\\\\-90.82031\\\\296.8363\\\\277.01\\\\-83.78906\\\\296.8459\\\\277.01\\\\-81.44531\\\\296.8837\\\\277.01\\\\-74.41406\\\\296.9028\\\\277.01\\\\-65.03906\\\\296.96\\\\277.01\\\\-60.35156\\\\296.9494\\\\277.01\\\\-58.00781\\\\296.9872\\\\277.01\\\\-53.32031\\\\296.9377\\\\277.01\\\\-50.97656\\\\296.9579\\\\277.01\\\\-48.63281\\\\297.1438\\\\277.01\\\\-43.94531\\\\297.2253\\\\277.01\\\\-41.60156\\\\297.048\\\\277.01\\\\-39.25781\\\\297.3759\\\\277.01\\\\-36.91406\\\\296.9644\\\\277.01\\\\-34.57031\\\\297.3341\\\\277.01\\\\-32.22656\\\\297.1747\\\\277.01\\\\-29.88281\\\\297.4229\\\\277.01\\\\-27.53906\\\\297.3652\\\\277.01\\\\-25.19531\\\\297.4334\\\\277.01\\\\-22.85156\\\\297.3652\\\\277.01\\\\-18.16406\\\\297.4229\\\\277.01\\\\-11.13281\\\\297.4334\\\\277.01\\\\-4.101563\\\\297.3235\\\\277.01\\\\-1.757813\\\\297.4229\\\\277.01\\\\2.929688\\\\297.3531\\\\277.01\\\\5.273438\\\\297.2199\\\\277.01\\\\7.617188\\\\297.4055\\\\277.01\\\\9.960938\\\\297.3842\\\\277.01\\\\12.30469\\\\296.9557\\\\277.01\\\\14.64844\\\\297.2621\\\\277.01\\\\16.99219\\\\297.2089\\\\277.01\\\\21.67969\\\\296.916\\\\277.01\\\\24.02344\\\\297.3701\\\\277.01\\\\26.36719\\\\296.9067\\\\277.01\\\\28.71094\\\\296.9467\\\\277.01\\\\33.39844\\\\296.8867\\\\277.01\\\\40.42969\\\\296.8564\\\\277.01\\\\45.11719\\\\296.7865\\\\277.01\\\\56.83594\\\\296.6788\\\\277.01\\\\61.52344\\\\296.6724\\\\277.01\\\\73.24219\\\\296.5521\\\\277.01\\\\87.30469\\\\296.5362\\\\277.01\\\\91.99219\\\\296.4801\\\\277.01\\\\96.67969\\\\296.4873\\\\277.01\\\\101.3672\\\\296.4462\\\\277.01\\\\115.4297\\\\296.3703\\\\277.01\\\\117.7734\\\\296.3846\\\\277.01\\\\134.1797\\\\296.2709\\\\277.01\\\\136.5234\\\\296.2305\\\\277.01\\\\138.8672\\\\295.864\\\\277.01\\\\141.2109\\\\295.9064\\\\277.01\\\\145.8984\\\\295.8486\\\\277.01\\\\148.2422\\\\295.9027\\\\277.01\\\\150.5859\\\\295.833\\\\277.01\\\\155.2734\\\\295.864\\\\277.01\\\\157.6172\\\\295.8012\\\\277.01\\\\164.6484\\\\295.7516\\\\277.01\\\\169.3359\\\\295.6819\\\\277.01\\\\174.0234\\\\295.6819\\\\277.01\\\\176.3672\\\\295.5916\\\\277.01\\\\181.0547\\\\295.557\\\\277.01\\\\185.7422\\\\295.5742\\\\277.01\\\\192.7734\\\\295.5401\\\\277.01\\\\197.4609\\\\295.833\\\\277.01\\\\202.1484\\\\295.7684\\\\277.01\\\\204.4922\\\\295.4594\\\\277.01\\\\206.8359\\\\294.8185\\\\277.01\\\\209.1797\\\\294.347\\\\277.01\\\\211.5234\\\\292.4135\\\\277.01\\\\213.8672\\\\291.0298\\\\277.01\\\\216.2109\\\\289.437\\\\277.01\\\\217.074\\\\288.6234\\\\277.01\\\\218.5547\\\\286.9247\\\\277.01\\\\221.5416\\\\283.9359\\\\277.01\\\\225.5859\\\\279.7662\\\\277.01\\\\227.9297\\\\277.4546\\\\277.01\\\\230.2734\\\\274.9609\\\\277.01\\\\233.0324\\\\272.2172\\\\277.01\\\\234.9609\\\\270.159\\\\277.01\\\\241.9922\\\\262.9747\\\\277.01\\\\244.3264\\\\260.4984\\\\277.01\\\\246.6903\\\\258.1547\\\\277.01\\\\249.0234\\\\255.711\\\\277.01\\\\251.3672\\\\253.4211\\\\277.01\\\\255.433\\\\248.7797\\\\277.01\\\\257.1424\\\\246.4359\\\\277.01\\\\259.4697\\\\244.0922\\\\277.01\\\\261.12\\\\239.4047\\\\277.01\\\\260.5029\\\\237.0609\\\\277.01\\\\259.4318\\\\234.7172\\\\277.01\\\\258.3984\\\\233.723\\\\277.01\\\\256.0547\\\\233.0686\\\\277.01\\\\253.7109\\\\232.8224\\\\277.01\\\\251.3672\\\\233.3211\\\\277.01\\\\249.0234\\\\232.7988\\\\277.01\\\\246.6797\\\\232.5842\\\\277.01\\\\244.3359\\\\232.6886\\\\277.01\\\\239.6484\\\\233.2349\\\\277.01\\\\237.3047\\\\233.3486\\\\277.01\\\\234.9609\\\\233.0207\\\\277.01\\\\227.9297\\\\233.0108\\\\277.01\\\\225.5859\\\\233.0402\\\\277.01\\\\209.1797\\\\233.0726\\\\277.01\\\\206.8359\\\\232.9699\\\\277.01\\\\204.4922\\\\232.7719\\\\277.01\\\\199.8047\\\\232.7719\\\\277.01\\\\195.1172\\\\232.8224\\\\277.01\\\\183.3984\\\\232.8852\\\\277.01\\\\176.3672\\\\232.8253\\\\277.01\\\\171.6797\\\\232.8497\\\\277.01\\\\166.9922\\\\232.81\\\\277.01\\\\157.6172\\\\232.8253\\\\277.01\\\\152.9297\\\\232.8003\\\\277.01\\\\145.8984\\\\232.8497\\\\277.01\\\\138.8672\\\\232.7614\\\\277.01\\\\136.5234\\\\232.7746\\\\277.01\\\\129.4922\\\\232.7186\\\\277.01\\\\127.1484\\\\232.7614\\\\277.01\\\\120.1172\\\\232.7346\\\\277.01\\\\108.3984\\\\232.7481\\\\277.01\\\\106.0547\\\\232.7048\\\\277.01\\\\103.7109\\\\232.9735\\\\277.01\\\\101.3672\\\\232.9629\\\\277.01\\\\96.67969\\\\233.0726\\\\277.01\\\\87.30469\\\\233.091\\\\277.01\\\\75.58594\\\\233.0819\\\\277.01\\\\54.49219\\\\233.1221\\\\277.01\\\\47.46094\\\\233.1132\\\\277.01\\\\45.11719\\\\233.0766\\\\277.01\\\\43.65234\\\\232.3734\\\\277.01\\\\42.77344\\\\231.5049\\\\277.01\\\\42.03585\\\\230.0297\\\\277.01\\\\42.77344\\\\229.2833\\\\277.01\\\\45.11719\\\\228.6392\\\\277.01\\\\47.46094\\\\227.2099\\\\277.01\\\\49.80469\\\\226.1658\\\\277.01\\\\52.14844\\\\224.6817\\\\277.01\\\\54.49219\\\\223.8452\\\\277.01\\\\56.83594\\\\222.0004\\\\277.01\\\\59.17969\\\\220.4439\\\\277.01\\\\61.52344\\\\219.1271\\\\277.01\\\\63.86719\\\\217.0811\\\\277.01\\\\66.21094\\\\215.1756\\\\277.01\\\\67.88793\\\\213.6234\\\\277.01\\\\68.55469\\\\212.8797\\\\277.01\\\\70.89844\\\\210.7867\\\\277.01\\\\72.54482\\\\208.9359\\\\277.01\\\\75.58594\\\\205.6807\\\\277.01\\\\78.63421\\\\201.9047\\\\277.01\\\\80.27344\\\\199.5051\\\\277.01\\\\81.52466\\\\197.2172\\\\277.01\\\\83.37788\\\\194.8734\\\\277.01\\\\84.31507\\\\192.5297\\\\277.01\\\\85.73579\\\\190.1859\\\\277.01\\\\86.63975\\\\187.8422\\\\277.01\\\\88.17874\\\\185.4984\\\\277.01\\\\88.71944\\\\183.1547\\\\277.01\\\\89.50195\\\\180.8109\\\\277.01\\\\90.46938\\\\178.4672\\\\277.01\\\\91.07824\\\\176.1234\\\\277.01\\\\91.79828\\\\173.7797\\\\277.01\\\\92.70757\\\\171.4359\\\\277.01\\\\93.03977\\\\169.0922\\\\277.01\\\\93.52213\\\\162.0609\\\\277.01\\\\93.79673\\\\157.3734\\\\277.01\\\\93.88062\\\\152.6859\\\\277.01\\\\93.4634\\\\145.6547\\\\277.01\\\\92.9856\\\\138.6234\\\\277.01\\\\92.41903\\\\136.2797\\\\277.01\\\\91.40625\\\\133.9359\\\\277.01\\\\90.85396\\\\131.5922\\\\277.01\\\\90.15889\\\\129.2484\\\\277.01\\\\89.05895\\\\126.9047\\\\277.01\\\\88.55738\\\\124.5609\\\\277.01\\\\87.68194\\\\122.2172\\\\277.01\\\\86.25967\\\\119.8734\\\\277.01\\\\85.33871\\\\117.5297\\\\277.01\\\\84.04059\\\\115.1859\\\\277.01\\\\82.57069\\\\112.8422\\\\277.01\\\\81.18974\\\\110.4984\\\\277.01\\\\80.27344\\\\109.1834\\\\277.01\\\\77.70041\\\\105.8109\\\\277.01\\\\76.15247\\\\103.4672\\\\277.01\\\\71.9184\\\\98.77969\\\\277.01\\\\68.55469\\\\95.4427\\\\277.01\\\\63.86719\\\\91.32878\\\\277.01\\\\61.52344\\\\89.91453\\\\277.01\\\\59.17969\\\\88.0407\\\\277.01\\\\56.83594\\\\86.33517\\\\277.01\\\\54.49219\\\\85.22703\\\\277.01\\\\53.90625\\\\84.71719\\\\277.01\\\\52.14844\\\\83.53945\\\\277.01\\\\49.80469\\\\82.68647\\\\277.01\\\\47.46094\\\\81.38185\\\\277.01\\\\45.11719\\\\80.48212\\\\277.01\\\\42.77344\\\\79.08845\\\\277.01\\\\40.42969\\\\78.67566\\\\277.01\\\\38.08594\\\\78.01502\\\\277.01\\\\35.74219\\\\77.02299\\\\277.01\\\\33.39844\\\\76.44418\\\\277.01\\\\31.05469\\\\75.99764\\\\277.01\\\\26.36719\\\\74.59824\\\\277.01\\\\24.02344\\\\74.41319\\\\277.01\\\\16.99219\\\\74.11594\\\\277.01\\\\12.30469\\\\74.05871\\\\277.01\\\\7.617188\\\\74.18881\\\\277.01\\\\2.929688\\\\74.428\\\\277.01\\\\0.5859375\\\\74.75625\\\\277.01\\\\-1.757813\\\\75.58151\\\\277.01\\\\-4.101563\\\\76.20659\\\\277.01\\\\-6.445313\\\\76.63125\\\\277.01\\\\-8.789063\\\\77.37503\\\\277.01\\\\-11.13281\\\\78.3486\\\\277.01\\\\-13.47656\\\\78.8168\\\\277.01\\\\-15.82031\\\\79.47342\\\\277.01\\\\-18.16406\\\\80.87569\\\\277.01\\\\-20.50781\\\\81.76657\\\\277.01\\\\-22.85156\\\\83.13133\\\\277.01\\\\-25.19531\\\\83.96092\\\\277.01\\\\-27.53906\\\\85.77746\\\\277.01\\\\-29.88281\\\\87.03368\\\\277.01\\\\-32.22656\\\\88.52746\\\\277.01\\\\-34.57031\\\\90.49887\\\\277.01\\\\-36.2212\\\\91.74844\\\\277.01\\\\-39.02853\\\\94.09219\\\\277.01\\\\-41.60156\\\\96.38751\\\\277.01\\\\-43.955\\\\98.77969\\\\277.01\\\\-46.28906\\\\101.3187\\\\277.01\\\\-48.63281\\\\104.1332\\\\277.01\\\\-49.87989\\\\105.8109\\\\277.01\\\\-51.93118\\\\108.1547\\\\277.01\\\\-53.12213\\\\110.4984\\\\277.01\\\\-55.66406\\\\114.2724\\\\277.01\\\\-56.35877\\\\115.1859\\\\277.01\\\\-57.13709\\\\117.5297\\\\277.01\\\\-58.55372\\\\119.8734\\\\277.01\\\\-59.49023\\\\122.2172\\\\277.01\\\\-60.81275\\\\124.5609\\\\277.01\\\\-61.92934\\\\129.2484\\\\277.01\\\\-63.00622\\\\131.5922\\\\277.01\\\\-63.77141\\\\133.9359\\\\277.01\\\\-64.50639\\\\138.6234\\\\277.01\\\\-65.16927\\\\140.9672\\\\277.01\\\\-65.6146\\\\143.3109\\\\277.01\\\\-65.7933\\\\145.6547\\\\277.01\\\\-66.06516\\\\150.3422\\\\277.01\\\\-66.13699\\\\155.0297\\\\277.01\\\\-66.09879\\\\157.3734\\\\277.01\\\\-65.89844\\\\162.0609\\\\277.01\\\\-65.71338\\\\164.4047\\\\277.01\\\\-65.41684\\\\166.7484\\\\277.01\\\\-64.78502\\\\169.0922\\\\277.01\\\\-64.27641\\\\171.4359\\\\277.01\\\\-63.9314\\\\173.7797\\\\277.01\\\\-63.32528\\\\176.1234\\\\277.01\\\\-62.18676\\\\178.4672\\\\277.01\\\\-61.52936\\\\180.8109\\\\277.01\\\\-61.08647\\\\183.1547\\\\277.01\\\\-59.75184\\\\185.4984\\\\277.01\\\\-58.89447\\\\187.8422\\\\277.01\\\\-57.47786\\\\190.1859\\\\277.01\\\\-56.72234\\\\192.5297\\\\277.01\\\\-55.0424\\\\194.8734\\\\277.01\\\\-53.77855\\\\197.2172\\\\277.01\\\\-52.24194\\\\199.5609\\\\277.01\\\\-50.4126\\\\201.9047\\\\277.01\\\\-48.63281\\\\204.3003\\\\277.01\\\\-47.03415\\\\206.5922\\\\277.01\\\\-42.56292\\\\211.2797\\\\277.01\\\\-39.25781\\\\214.4452\\\\277.01\\\\-36.91406\\\\216.3386\\\\277.01\\\\-34.26873\\\\218.3109\\\\277.01\\\\-29.88281\\\\221.4518\\\\277.01\\\\-27.53906\\\\223.027\\\\277.01\\\\-25.19531\\\\224.3439\\\\277.01\\\\-23.14453\\\\225.3422\\\\277.01\\\\-20.50781\\\\226.7399\\\\277.01\\\\-18.16406\\\\228.2227\\\\277.01\\\\-15.82031\\\\229.069\\\\277.01\\\\-14.3044\\\\230.0297\\\\277.01\\\\-15.82031\\\\231.8656\\\\277.01\\\\-16.46402\\\\232.3734\\\\277.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"374\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-95.50781\\\\233.1042\\\\280.01\\\\-97.85156\\\\234.0105\\\\280.01\\\\-100.1953\\\\233.4044\\\\280.01\\\\-102.5391\\\\234.062\\\\280.01\\\\-104.8828\\\\233.3561\\\\280.01\\\\-109.5703\\\\234.1651\\\\280.01\\\\-111.9141\\\\234.0722\\\\280.01\\\\-116.6016\\\\234.0411\\\\280.01\\\\-121.2891\\\\234.0855\\\\280.01\\\\-128.3203\\\\234.0672\\\\280.01\\\\-135.3516\\\\234.1266\\\\280.01\\\\-140.0391\\\\234.1078\\\\280.01\\\\-142.3828\\\\234.1741\\\\280.01\\\\-149.4141\\\\234.0642\\\\280.01\\\\-156.4453\\\\234.1132\\\\280.01\\\\-163.4766\\\\234.0613\\\\280.01\\\\-165.8203\\\\234.1313\\\\280.01\\\\-168.1641\\\\234.3108\\\\280.01\\\\-177.5391\\\\234.3173\\\\280.01\\\\-182.2266\\\\234.2721\\\\280.01\\\\-186.9141\\\\234.3703\\\\280.01\\\\-191.6016\\\\234.3837\\\\280.01\\\\-198.6328\\\\234.3084\\\\280.01\\\\-200.9766\\\\234.3389\\\\280.01\\\\-208.0078\\\\234.2822\\\\280.01\\\\-217.3828\\\\234.3296\\\\280.01\\\\-219.7266\\\\234.3758\\\\280.01\\\\-229.1016\\\\234.3389\\\\280.01\\\\-233.7891\\\\234.2125\\\\280.01\\\\-240.8203\\\\234.2537\\\\280.01\\\\-243.1641\\\\234.2967\\\\280.01\\\\-247.8516\\\\234.0375\\\\280.01\\\\-250.1953\\\\234.1916\\\\280.01\\\\-252.181\\\\234.7172\\\\280.01\\\\-252.5391\\\\235.6379\\\\280.01\\\\-254.1504\\\\234.7172\\\\280.01\\\\-254.8828\\\\234.668\\\\280.01\\\\-259.5703\\\\236.2655\\\\280.01\\\\-260.5737\\\\237.0609\\\\280.01\\\\-262.3019\\\\239.4047\\\\280.01\\\\-262.5477\\\\241.7484\\\\280.01\\\\-261.5577\\\\244.0922\\\\280.01\\\\-260.8438\\\\246.4359\\\\280.01\\\\-259.5991\\\\248.7797\\\\280.01\\\\-254.8828\\\\253.9193\\\\280.01\\\\-253.2919\\\\255.8109\\\\280.01\\\\-250.1953\\\\258.8925\\\\280.01\\\\-247.8516\\\\261.4312\\\\280.01\\\\-246.2802\\\\262.8422\\\\280.01\\\\-244.138\\\\265.1859\\\\280.01\\\\-240.8203\\\\268.3534\\\\280.01\\\\-239.3737\\\\269.8734\\\\280.01\\\\-236.938\\\\272.2172\\\\280.01\\\\-233.7891\\\\275.3765\\\\280.01\\\\-231.4453\\\\277.7994\\\\280.01\\\\-229.1016\\\\279.9918\\\\280.01\\\\-226.7578\\\\282.4992\\\\280.01\\\\-225.119\\\\283.9359\\\\280.01\\\\-222.0703\\\\287.169\\\\280.01\\\\-220.4806\\\\288.6234\\\\280.01\\\\-218.2311\\\\290.9672\\\\280.01\\\\-217.3828\\\\291.613\\\\280.01\\\\-215.0391\\\\292.8596\\\\280.01\\\\-214.5047\\\\293.3109\\\\280.01\\\\-212.6953\\\\294.459\\\\280.01\\\\-210.3516\\\\295.1842\\\\280.01\\\\-208.0078\\\\296.4338\\\\280.01\\\\-205.6641\\\\296.8595\\\\280.01\\\\-203.3203\\\\297.1639\\\\280.01\\\\-196.2891\\\\297.1563\\\\280.01\\\\-193.9453\\\\297.0622\\\\280.01\\\\-184.5703\\\\296.9731\\\\280.01\\\\-172.8516\\\\296.8959\\\\280.01\\\\-165.8203\\\\296.9125\\\\280.01\\\\-161.1328\\\\296.8904\\\\280.01\\\\-156.4453\\\\296.9295\\\\280.01\\\\-144.7266\\\\296.9097\\\\280.01\\\\-137.6953\\\\296.9173\\\\280.01\\\\-125.9766\\\\296.8528\\\\280.01\\\\-118.9453\\\\296.8629\\\\280.01\\\\-116.6016\\\\296.8357\\\\280.01\\\\-109.5703\\\\296.8723\\\\280.01\\\\-102.5391\\\\296.8266\\\\280.01\\\\-93.16406\\\\296.7973\\\\280.01\\\\-76.75781\\\\296.8647\\\\280.01\\\\-72.07031\\\\296.9145\\\\280.01\\\\-67.38281\\\\296.9028\\\\280.01\\\\-65.03906\\\\296.9514\\\\280.01\\\\-60.35156\\\\296.9322\\\\280.01\\\\-55.66406\\\\296.9622\\\\280.01\\\\-53.32031\\\\296.925\\\\280.01\\\\-48.63281\\\\296.9622\\\\280.01\\\\-46.28906\\\\296.8973\\\\280.01\\\\-41.60156\\\\296.9067\\\\280.01\\\\-39.25781\\\\297.0439\\\\280.01\\\\-36.91406\\\\296.9267\\\\280.01\\\\-34.57031\\\\297.1128\\\\280.01\\\\-32.22656\\\\296.9926\\\\280.01\\\\-29.88281\\\\297.3952\\\\280.01\\\\-27.53906\\\\297.0591\\\\280.01\\\\-25.19531\\\\297.3984\\\\280.01\\\\-22.85156\\\\297.0809\\\\280.01\\\\-20.50781\\\\297.0809\\\\280.01\\\\-18.16406\\\\297.3798\\\\280.01\\\\-13.47656\\\\297.4022\\\\280.01\\\\-11.13281\\\\297.2922\\\\280.01\\\\-8.789063\\\\297.4125\\\\280.01\\\\-4.101563\\\\297.3691\\\\280.01\\\\-1.757813\\\\297.4334\\\\280.01\\\\0.5859375\\\\297.3911\\\\280.01\\\\2.929688\\\\297.106\\\\280.01\\\\5.273438\\\\297.1626\\\\280.01\\\\7.617188\\\\297.409\\\\280.01\\\\9.960938\\\\297.3879\\\\280.01\\\\12.30469\\\\296.9377\\\\280.01\\\\14.64844\\\\297.0091\\\\280.01\\\\16.99219\\\\296.9358\\\\280.01\\\\19.33594\\\\297.0362\\\\280.01\\\\21.67969\\\\296.8867\\\\280.01\\\\24.02344\\\\297.1195\\\\280.01\\\\26.36719\\\\296.8762\\\\280.01\\\\33.39844\\\\296.8961\\\\280.01\\\\42.77344\\\\296.8163\\\\280.01\\\\56.83594\\\\296.6724\\\\280.01\\\\63.86719\\\\296.6577\\\\280.01\\\\68.55469\\\\296.5859\\\\280.01\\\\70.89844\\\\296.6028\\\\280.01\\\\77.92969\\\\296.5441\\\\280.01\\\\84.96094\\\\296.549\\\\280.01\\\\96.67969\\\\296.5208\\\\280.01\\\\101.3672\\\\296.4462\\\\280.01\\\\103.7109\\\\296.4668\\\\280.01\\\\113.0859\\\\296.4189\\\\280.01\\\\122.4609\\\\296.3348\\\\280.01\\\\136.5234\\\\296.2873\\\\280.01\\\\138.8672\\\\295.8791\\\\280.01\\\\141.2109\\\\296.1963\\\\280.01\\\\145.8984\\\\295.8486\\\\280.01\\\\148.2422\\\\295.9009\\\\280.01\\\\150.5859\\\\295.8172\\\\280.01\\\\155.2734\\\\295.8791\\\\280.01\\\\162.3047\\\\295.7684\\\\280.01\\\\164.6484\\\\295.7684\\\\280.01\\\\183.3984\\\\295.557\\\\280.01\\\\192.7734\\\\295.557\\\\280.01\\\\197.4609\\\\295.8172\\\\280.01\\\\202.1484\\\\295.7516\\\\280.01\\\\204.4922\\\\295.4439\\\\280.01\\\\206.8359\\\\294.8185\\\\280.01\\\\209.1797\\\\294.3583\\\\280.01\\\\211.5234\\\\292.4108\\\\280.01\\\\213.8672\\\\291.0471\\\\280.01\\\\216.2109\\\\289.452\\\\280.01\\\\217.0839\\\\288.6234\\\\280.01\\\\218.5547\\\\286.9506\\\\280.01\\\\221.5668\\\\283.9359\\\\280.01\\\\225.5859\\\\279.7481\\\\280.01\\\\227.9297\\\\277.4546\\\\280.01\\\\230.2734\\\\274.9423\\\\280.01\\\\233.0475\\\\272.2172\\\\280.01\\\\234.9609\\\\270.1762\\\\280.01\\\\237.5903\\\\267.5297\\\\280.01\\\\242.1654\\\\262.8422\\\\280.01\\\\244.3673\\\\260.4984\\\\280.01\\\\246.773\\\\258.1547\\\\280.01\\\\249.0142\\\\255.8109\\\\280.01\\\\251.3672\\\\253.4397\\\\280.01\\\\253.7109\\\\250.8472\\\\280.01\\\\255.4546\\\\248.7797\\\\280.01\\\\257.1065\\\\246.4359\\\\280.01\\\\259.3784\\\\244.0922\\\\280.01\\\\260.9815\\\\239.4047\\\\280.01\\\\260.4452\\\\237.0609\\\\280.01\\\\259.4202\\\\234.7172\\\\280.01\\\\258.3984\\\\233.7341\\\\280.01\\\\256.0547\\\\233.1091\\\\280.01\\\\253.7109\\\\232.7821\\\\280.01\\\\251.3672\\\\232.8781\\\\280.01\\\\249.0234\\\\232.6162\\\\280.01\\\\244.3359\\\\232.7025\\\\280.01\\\\241.9922\\\\232.9379\\\\280.01\\\\237.3047\\\\233.0766\\\\280.01\\\\234.9609\\\\233.0108\\\\280.01\\\\232.6172\\\\233.0402\\\\280.01\\\\227.9297\\\\233.0108\\\\280.01\\\\225.5859\\\\233.0498\\\\280.01\\\\211.5234\\\\233.0819\\\\280.01\\\\206.8359\\\\232.9594\\\\280.01\\\\204.4922\\\\232.7588\\\\280.01\\\\202.1484\\\\232.7848\\\\280.01\\\\183.3984\\\\232.8852\\\\280.01\\\\178.7109\\\\232.8467\\\\280.01\\\\162.3047\\\\232.8003\\\\280.01\\\\159.9609\\\\232.8253\\\\280.01\\\\143.5547\\\\232.8376\\\\280.01\\\\141.2109\\\\232.7746\\\\280.01\\\\136.5234\\\\232.7975\\\\280.01\\\\131.8359\\\\232.7322\\\\280.01\\\\127.1484\\\\232.7614\\\\280.01\\\\124.8047\\\\232.721\\\\280.01\\\\120.1172\\\\232.7481\\\\280.01\\\\113.0859\\\\232.7071\\\\280.01\\\\106.0547\\\\232.721\\\\280.01\\\\103.7109\\\\232.9629\\\\280.01\\\\101.3672\\\\232.9735\\\\280.01\\\\96.67969\\\\233.0819\\\\280.01\\\\82.61719\\\\233.1001\\\\280.01\\\\77.92969\\\\233.0726\\\\280.01\\\\73.24219\\\\233.1132\\\\280.01\\\\63.86719\\\\233.1221\\\\280.01\\\\56.83594\\\\233.0951\\\\280.01\\\\47.46094\\\\233.1221\\\\280.01\\\\45.11719\\\\233.0726\\\\280.01\\\\44.1779\\\\232.3734\\\\280.01\\\\42.77344\\\\230.9901\\\\280.01\\\\42.0716\\\\230.0297\\\\280.01\\\\42.77344\\\\229.2739\\\\280.01\\\\45.11719\\\\228.632\\\\280.01\\\\47.46094\\\\227.1643\\\\280.01\\\\49.80469\\\\226.1565\\\\280.01\\\\52.14844\\\\224.6574\\\\280.01\\\\54.49219\\\\223.8228\\\\280.01\\\\56.83594\\\\221.9803\\\\280.01\\\\59.17969\\\\220.4287\\\\280.01\\\\61.52344\\\\219.0851\\\\280.01\\\\62.3638\\\\218.3109\\\\280.01\\\\66.21094\\\\215.1389\\\\280.01\\\\67.85556\\\\213.6234\\\\280.01\\\\68.55469\\\\212.8447\\\\280.01\\\\70.89844\\\\210.758\\\\280.01\\\\72.51065\\\\208.9359\\\\280.01\\\\75.58594\\\\205.632\\\\280.01\\\\78.58622\\\\201.9047\\\\280.01\\\\80.27344\\\\199.4393\\\\280.01\\\\81.50667\\\\197.2172\\\\280.01\\\\83.34528\\\\194.8734\\\\280.01\\\\84.28662\\\\192.5297\\\\280.01\\\\85.68763\\\\190.1859\\\\280.01\\\\86.60807\\\\187.8422\\\\280.01\\\\88.12763\\\\185.4984\\\\280.01\\\\88.70493\\\\183.1547\\\\280.01\\\\89.40742\\\\180.8109\\\\280.01\\\\90.42329\\\\178.4672\\\\280.01\\\\91.73814\\\\173.7797\\\\280.01\\\\92.66084\\\\171.4359\\\\280.01\\\\93.0161\\\\169.0922\\\\280.01\\\\93.30299\\\\164.4047\\\\280.01\\\\93.50073\\\\162.0609\\\\280.01\\\\93.76065\\\\157.3734\\\\280.01\\\\93.84212\\\\155.0297\\\\280.01\\\\93.83057\\\\152.6859\\\\280.01\\\\93.44278\\\\145.6547\\\\280.01\\\\92.97371\\\\138.6234\\\\280.01\\\\92.3933\\\\136.2797\\\\280.01\\\\91.39597\\\\133.9359\\\\280.01\\\\90.83705\\\\131.5922\\\\280.01\\\\90.15889\\\\129.2484\\\\280.01\\\\89.05895\\\\126.9047\\\\280.01\\\\88.55198\\\\124.5609\\\\280.01\\\\87.66837\\\\122.2172\\\\280.01\\\\86.27174\\\\119.8734\\\\280.01\\\\85.36424\\\\117.5297\\\\280.01\\\\84.03852\\\\115.1859\\\\280.01\\\\81.2221\\\\110.4984\\\\280.01\\\\80.27344\\\\109.1401\\\\280.01\\\\77.76597\\\\105.8109\\\\280.01\\\\76.17959\\\\103.4672\\\\280.01\\\\74.11412\\\\101.1234\\\\280.01\\\\71.95528\\\\98.77969\\\\280.01\\\\68.55469\\\\95.41232\\\\280.01\\\\66.21094\\\\93.31791\\\\280.01\\\\63.86719\\\\91.31586\\\\280.01\\\\61.52344\\\\89.8781\\\\280.01\\\\61.00544\\\\89.40469\\\\280.01\\\\56.83594\\\\86.32603\\\\280.01\\\\54.49219\\\\85.22374\\\\280.01\\\\53.91059\\\\84.71719\\\\280.01\\\\52.14844\\\\83.52773\\\\280.01\\\\49.80469\\\\82.67247\\\\280.01\\\\47.46094\\\\81.38185\\\\280.01\\\\45.11719\\\\80.45787\\\\280.01\\\\42.77344\\\\79.08102\\\\280.01\\\\40.42969\\\\78.68263\\\\280.01\\\\38.08594\\\\78.00113\\\\280.01\\\\35.74219\\\\76.98543\\\\280.01\\\\33.39844\\\\76.44418\\\\280.01\\\\31.05469\\\\75.99764\\\\280.01\\\\28.71094\\\\75.21002\\\\280.01\\\\26.36719\\\\74.59824\\\\280.01\\\\24.02344\\\\74.40105\\\\280.01\\\\14.64844\\\\74.0463\\\\280.01\\\\9.960938\\\\74.09591\\\\280.01\\\\5.273438\\\\74.27516\\\\280.01\\\\2.929688\\\\74.42056\\\\280.01\\\\0.5859375\\\\74.75625\\\\280.01\\\\-1.757813\\\\75.52052\\\\280.01\\\\-4.101563\\\\76.1918\\\\280.01\\\\-6.445313\\\\76.61909\\\\280.01\\\\-8.789063\\\\77.32118\\\\280.01\\\\-11.13281\\\\78.33466\\\\280.01\\\\-13.47656\\\\78.80508\\\\280.01\\\\-15.82031\\\\79.44011\\\\280.01\\\\-18.16406\\\\80.84683\\\\280.01\\\\-20.50781\\\\81.74243\\\\280.01\\\\-22.85156\\\\83.11391\\\\280.01\\\\-25.19531\\\\83.92464\\\\280.01\\\\-27.53906\\\\85.7457\\\\280.01\\\\-29.88281\\\\86.96401\\\\280.01\\\\-32.22656\\\\88.49249\\\\280.01\\\\-34.57031\\\\90.47998\\\\280.01\\\\-36.28038\\\\91.74844\\\\280.01\\\\-39.07948\\\\94.09219\\\\280.01\\\\-41.60156\\\\96.27657\\\\280.01\\\\-44.03106\\\\98.77969\\\\280.01\\\\-46.28906\\\\101.2371\\\\280.01\\\\-48.63281\\\\104.0658\\\\280.01\\\\-49.94405\\\\105.8109\\\\280.01\\\\-51.96513\\\\108.1547\\\\280.01\\\\-53.18715\\\\110.4984\\\\280.01\\\\-54.74057\\\\112.8422\\\\280.01\\\\-56.42476\\\\115.1859\\\\280.01\\\\-57.19754\\\\117.5297\\\\280.01\\\\-58.65653\\\\119.8734\\\\280.01\\\\-59.544\\\\122.2172\\\\280.01\\\\-60.8896\\\\124.5609\\\\280.01\\\\-62.0208\\\\129.2484\\\\280.01\\\\-63.12626\\\\131.5922\\\\280.01\\\\-63.84518\\\\133.9359\\\\280.01\\\\-64.19696\\\\136.2797\\\\280.01\\\\-64.66129\\\\138.6234\\\\280.01\\\\-65.33604\\\\140.9672\\\\280.01\\\\-65.68493\\\\143.3109\\\\280.01\\\\-66.13839\\\\150.3422\\\\280.01\\\\-66.21657\\\\155.0297\\\\280.01\\\\-66.17729\\\\157.3734\\\\280.01\\\\-65.96361\\\\162.0609\\\\280.01\\\\-65.54951\\\\166.7484\\\\280.01\\\\-64.3463\\\\171.4359\\\\280.01\\\\-63.998\\\\173.7797\\\\280.01\\\\-63.46595\\\\176.1234\\\\280.01\\\\-62.32813\\\\178.4672\\\\280.01\\\\-61.61405\\\\180.8109\\\\280.01\\\\-61.15642\\\\183.1547\\\\280.01\\\\-59.86389\\\\185.4984\\\\280.01\\\\-58.95148\\\\187.8422\\\\280.01\\\\-57.52841\\\\190.1859\\\\280.01\\\\-56.74625\\\\192.5297\\\\280.01\\\\-55.1036\\\\194.8734\\\\280.01\\\\-53.84033\\\\197.2172\\\\280.01\\\\-52.26687\\\\199.5609\\\\280.01\\\\-48.63281\\\\204.3615\\\\280.01\\\\-47.1009\\\\206.5922\\\\280.01\\\\-42.59905\\\\211.2797\\\\280.01\\\\-39.25781\\\\214.5093\\\\280.01\\\\-36.91406\\\\216.3794\\\\280.01\\\\-34.31858\\\\218.3109\\\\280.01\\\\-29.88281\\\\221.4737\\\\280.01\\\\-27.53906\\\\223.0821\\\\280.01\\\\-25.19531\\\\224.3511\\\\280.01\\\\-20.50781\\\\226.7643\\\\280.01\\\\-18.16406\\\\228.2682\\\\280.01\\\\-15.82031\\\\229.0908\\\\280.01\\\\-14.92839\\\\230.0297\\\\280.01\\\\-15.82031\\\\231.4137\\\\280.01\\\\-16.95631\\\\232.3734\\\\280.01\\\\-18.16406\\\\233.0537\\\\280.01\\\\-20.50781\\\\233.0951\\\\280.01\\\\-39.25781\\\\233.1397\\\\280.01\\\\-43.94531\\\\233.1221\\\\280.01\\\\-53.32031\\\\233.1736\\\\280.01\\\\-60.35156\\\\233.1736\\\\280.01\\\\-65.03906\\\\232.9735\\\\280.01\\\\-74.41406\\\\233.0441\\\\280.01\\\\-79.10156\\\\233.0145\\\\280.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846442461900001.533642576430\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"354\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"3\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0766\\\\283.01\\\\-27.53906\\\\233.1221\\\\283.01\\\\-32.22656\\\\233.1042\\\\283.01\\\\-39.25781\\\\233.1397\\\\283.01\\\\-48.63281\\\\233.131\\\\283.01\\\\-53.32031\\\\233.1652\\\\283.01\\\\-60.35156\\\\233.1652\\\\283.01\\\\-62.69531\\\\233.0576\\\\283.01\\\\-65.03906\\\\232.8497\\\\283.01\\\\-67.38281\\\\232.9304\\\\283.01\\\\-74.41406\\\\233.0245\\\\283.01\\\\-86.13281\\\\232.9942\\\\283.01\\\\-88.47656\\\\233.0479\\\\283.01\\\\-97.85156\\\\233.0992\\\\283.01\\\\-107.2266\\\\233.1132\\\\283.01\\\\-109.5703\\\\233.2337\\\\283.01\\\\-111.9141\\\\233.9859\\\\283.01\\\\-116.6016\\\\233.9503\\\\283.01\\\\-121.2891\\\\234.0035\\\\283.01\\\\-130.6641\\\\234.0088\\\\283.01\\\\-135.3516\\\\234.0855\\\\283.01\\\\-140.0391\\\\234.0722\\\\283.01\\\\-142.3828\\\\234.1266\\\\283.01\\\\-149.4141\\\\234.0211\\\\283.01\\\\-151.7578\\\\234.0591\\\\283.01\\\\-161.1328\\\\234.0243\\\\283.01\\\\-165.8203\\\\234.0691\\\\283.01\\\\-168.1641\\\\234.2175\\\\283.01\\\\-172.8516\\\\234.2252\\\\283.01\\\\-175.1953\\\\234.1797\\\\283.01\\\\-179.8828\\\\234.2112\\\\283.01\\\\-184.5703\\\\234.2869\\\\283.01\\\\-191.6016\\\\234.2721\\\\283.01\\\\-198.6328\\\\234.2187\\\\283.01\\\\-200.9766\\\\234.2902\\\\283.01\\\\-205.6641\\\\234.205\\\\283.01\\\\-210.3516\\\\234.1861\\\\283.01\\\\-212.6953\\\\234.2398\\\\283.01\\\\-219.7266\\\\234.3115\\\\283.01\\\\-229.1016\\\\234.3235\\\\283.01\\\\-233.7891\\\\234.2362\\\\283.01\\\\-236.1328\\\\234.2789\\\\283.01\\\\-243.1641\\\\234.2502\\\\283.01\\\\-247.8516\\\\233.9138\\\\283.01\\\\-250.1953\\\\234.1229\\\\283.01\\\\-254.8828\\\\234.6658\\\\283.01\\\\-257.2266\\\\235.4369\\\\283.01\\\\-259.5703\\\\236.3773\\\\283.01\\\\-260.3961\\\\237.0609\\\\283.01\\\\-262.014\\\\239.4047\\\\283.01\\\\-262.2271\\\\241.7484\\\\283.01\\\\-261.3423\\\\244.0922\\\\283.01\\\\-260.8191\\\\246.4359\\\\283.01\\\\-259.7416\\\\248.7797\\\\283.01\\\\-257.5902\\\\251.1234\\\\283.01\\\\-255.5257\\\\253.4672\\\\283.01\\\\-252.5391\\\\256.6338\\\\283.01\\\\-250.1953\\\\258.856\\\\283.01\\\\-247.8516\\\\261.3733\\\\283.01\\\\-246.2019\\\\262.8422\\\\283.01\\\\-243.1641\\\\266.1315\\\\283.01\\\\-241.6046\\\\267.5297\\\\283.01\\\\-239.419\\\\269.8734\\\\283.01\\\\-236.9491\\\\272.2172\\\\283.01\\\\-233.7891\\\\275.3661\\\\283.01\\\\-231.4453\\\\277.8091\\\\283.01\\\\-229.1016\\\\280.0036\\\\283.01\\\\-226.7578\\\\282.509\\\\283.01\\\\-225.1317\\\\283.9359\\\\283.01\\\\-222.0703\\\\287.1792\\\\283.01\\\\-220.4806\\\\288.6234\\\\283.01\\\\-218.2311\\\\290.9672\\\\283.01\\\\-217.3828\\\\291.613\\\\283.01\\\\-215.0391\\\\292.905\\\\283.01\\\\-212.6953\\\\294.4828\\\\283.01\\\\-210.3516\\\\295.2615\\\\283.01\\\\-208.0078\\\\296.4508\\\\283.01\\\\-205.6641\\\\296.8789\\\\283.01\\\\-203.3203\\\\297.1639\\\\283.01\\\\-196.2891\\\\297.1563\\\\283.01\\\\-191.6016\\\\297.023\\\\283.01\\\\-189.2578\\\\297.0306\\\\283.01\\\\-182.2266\\\\296.948\\\\283.01\\\\-172.8516\\\\296.9022\\\\283.01\\\\-158.7891\\\\296.8988\\\\283.01\\\\-147.0703\\\\296.9323\\\\283.01\\\\-142.3828\\\\296.9074\\\\283.01\\\\-133.0078\\\\296.92\\\\283.01\\\\-123.6328\\\\296.8528\\\\283.01\\\\-116.6016\\\\296.8356\\\\283.01\\\\-111.9141\\\\296.8723\\\\283.01\\\\-100.1953\\\\296.8075\\\\283.01\\\\-74.41406\\\\296.8938\\\\283.01\\\\-67.38281\\\\296.9028\\\\283.01\\\\-62.69531\\\\296.96\\\\283.01\\\\-58.00781\\\\296.9234\\\\283.01\\\\-41.60156\\\\296.9081\\\\283.01\\\\-36.91406\\\\297.0328\\\\283.01\\\\-34.57031\\\\297.3842\\\\283.01\\\\-32.22656\\\\297.1128\\\\283.01\\\\-27.53906\\\\297.3192\\\\283.01\\\\-25.19531\\\\296.9926\\\\283.01\\\\-20.50781\\\\297.1747\\\\283.01\\\\-18.16406\\\\297.3531\\\\283.01\\\\-15.82031\\\\297.4125\\\\283.01\\\\-6.445313\\\\297.4125\\\\283.01\\\\-4.101563\\\\297.257\\\\283.01\\\\-1.757813\\\\297.4229\\\\283.01\\\\9.960938\\\\297.4125\\\\283.01\\\\12.30469\\\\296.9377\\\\283.01\\\\16.99219\\\\296.9467\\\\283.01\\\\21.67969\\\\296.8877\\\\283.01\\\\33.39844\\\\296.8771\\\\283.01\\\\40.42969\\\\296.8369\\\\283.01\\\\45.11719\\\\296.7643\\\\283.01\\\\52.14844\\\\296.7021\\\\283.01\\\\56.83594\\\\296.6906\\\\283.01\\\\68.55469\\\\296.5732\\\\283.01\\\\75.58594\\\\296.5859\\\\283.01\\\\77.92969\\\\296.5441\\\\283.01\\\\84.96094\\\\296.5441\\\\283.01\\\\87.30469\\\\296.4813\\\\283.01\\\\91.99219\\\\296.5154\\\\283.01\\\\110.7422\\\\296.4051\\\\283.01\\\\117.7734\\\\296.3495\\\\283.01\\\\129.4922\\\\296.2929\\\\283.01\\\\131.8359\\\\296.3023\\\\283.01\\\\136.5234\\\\295.8486\\\\283.01\\\\143.5547\\\\295.864\\\\283.01\\\\152.9297\\\\295.8012\\\\283.01\\\\155.2734\\\\295.8486\\\\283.01\\\\164.6484\\\\295.6998\\\\283.01\\\\169.3359\\\\295.7173\\\\283.01\\\\176.3672\\\\295.5916\\\\283.01\\\\188.0859\\\\295.5235\\\\283.01\\\\192.7734\\\\295.5401\\\\283.01\\\\197.4609\\\\295.833\\\\283.01\\\\202.1484\\\\295.7516\\\\283.01\\\\204.4922\\\\295.4439\\\\283.01\\\\206.8359\\\\294.8185\\\\283.01\\\\209.1797\\\\294.3532\\\\283.01\\\\211.5234\\\\292.4178\\\\283.01\\\\213.8672\\\\291.0471\\\\283.01\\\\216.2109\\\\289.437\\\\283.01\\\\217.074\\\\288.6234\\\\283.01\\\\218.5547\\\\286.9377\\\\283.01\\\\221.5668\\\\283.9359\\\\283.01\\\\225.5859\\\\279.7481\\\\283.01\\\\227.9297\\\\277.4225\\\\283.01\\\\230.2734\\\\274.9423\\\\283.01\\\\233.014\\\\272.2172\\\\283.01\\\\234.9609\\\\270.1737\\\\283.01\\\\237.6074\\\\267.5297\\\\283.01\\\\244.4276\\\\260.4984\\\\283.01\\\\246.7938\\\\258.1547\\\\283.01\\\\251.3978\\\\253.4672\\\\283.01\\\\253.7109\\\\250.8942\\\\283.01\\\\255.4174\\\\248.7797\\\\283.01\\\\257.0561\\\\246.4359\\\\283.01\\\\259.226\\\\244.0922\\\\283.01\\\\260.0378\\\\241.7484\\\\283.01\\\\260.5957\\\\239.4047\\\\283.01\\\\260.2749\\\\237.0609\\\\283.01\\\\259.2681\\\\234.7172\\\\283.01\\\\258.3984\\\\233.8742\\\\283.01\\\\256.0547\\\\233.2096\\\\283.01\\\\253.7109\\\\232.7947\\\\283.01\\\\251.3672\\\\232.6745\\\\283.01\\\\246.6797\\\\232.7162\\\\283.01\\\\244.3359\\\\232.6725\\\\283.01\\\\241.9922\\\\232.927\\\\283.01\\\\239.6484\\\\233.0007\\\\283.01\\\\230.2734\\\\233.0007\\\\283.01\\\\216.2109\\\\233.0819\\\\283.01\\\\209.1797\\\\233.0726\\\\283.01\\\\206.8359\\\\232.9839\\\\283.01\\\\204.4922\\\\232.7588\\\\283.01\\\\199.8047\\\\232.7719\\\\283.01\\\\195.1172\\\\232.8346\\\\283.01\\\\185.7422\\\\232.8346\\\\283.01\\\\183.3984\\\\232.8617\\\\283.01\\\\176.3672\\\\232.8224\\\\283.01\\\\174.0234\\\\232.8497\\\\283.01\\\\162.3047\\\\232.8003\\\\283.01\\\\159.9609\\\\232.8376\\\\283.01\\\\155.2734\\\\232.8253\\\\283.01\\\\145.8984\\\\232.8497\\\\283.01\\\\138.8672\\\\232.7588\\\\283.01\\\\136.5234\\\\232.7975\\\\283.01\\\\131.8359\\\\232.7456\\\\283.01\\\\122.4609\\\\232.7746\\\\283.01\\\\117.7734\\\\232.7186\\\\283.01\\\\106.0547\\\\232.721\\\\283.01\\\\103.7109\\\\232.9735\\\\283.01\\\\101.3672\\\\232.9735\\\\283.01\\\\99.02344\\\\233.0766\\\\283.01\\\\89.64844\\\\233.0726\\\\283.01\\\\82.61719\\\\233.1091\\\\283.01\\\\77.92969\\\\233.0766\\\\283.01\\\\75.58594\\\\233.1042\\\\283.01\\\\63.86719\\\\233.131\\\\283.01\\\\56.83594\\\\233.1042\\\\283.01\\\\54.49219\\\\233.131\\\\283.01\\\\45.11719\\\\233.0726\\\\283.01\\\\44.18501\\\\232.3734\\\\283.01\\\\42.77344\\\\230.9932\\\\283.01\\\\42.06263\\\\230.0297\\\\283.01\\\\42.77344\\\\229.2645\\\\283.01\\\\45.11719\\\\228.6247\\\\283.01\\\\47.46094\\\\227.1533\\\\283.01\\\\49.80469\\\\226.1347\\\\283.01\\\\52.14844\\\\224.6675\\\\283.01\\\\54.49219\\\\223.8092\\\\283.01\\\\56.83594\\\\221.9546\\\\283.01\\\\59.17969\\\\220.3699\\\\283.01\\\\61.52344\\\\219.056\\\\283.01\\\\62.3228\\\\218.3109\\\\283.01\\\\66.21094\\\\215.1232\\\\283.01\\\\67.84627\\\\213.6234\\\\283.01\\\\68.55469\\\\212.8298\\\\283.01\\\\70.89844\\\\210.6976\\\\283.01\\\\74.68501\\\\206.5922\\\\283.01\\\\76.71939\\\\204.2484\\\\283.01\\\\78.54079\\\\201.9047\\\\283.01\\\\80.27344\\\\199.3159\\\\283.01\\\\81.47583\\\\197.2172\\\\283.01\\\\83.28172\\\\194.8734\\\\283.01\\\\84.25013\\\\192.5297\\\\283.01\\\\85.65463\\\\190.1859\\\\283.01\\\\86.55134\\\\187.8422\\\\283.01\\\\88.09267\\\\185.4984\\\\283.01\\\\89.37801\\\\180.8109\\\\283.01\\\\90.42329\\\\178.4672\\\\283.01\\\\91.70932\\\\173.7797\\\\283.01\\\\92.6346\\\\171.4359\\\\283.01\\\\93.02129\\\\169.0922\\\\283.01\\\\93.30979\\\\164.4047\\\\283.01\\\\93.6478\\\\159.7172\\\\283.01\\\\93.86867\\\\155.0297\\\\283.01\\\\93.75715\\\\150.3422\\\\283.01\\\\93.46669\\\\145.6547\\\\283.01\\\\93.27223\\\\143.3109\\\\283.01\\\\92.9856\\\\138.6234\\\\283.01\\\\92.43455\\\\136.2797\\\\283.01\\\\91.41665\\\\133.9359\\\\283.01\\\\90.85938\\\\131.5922\\\\283.01\\\\90.19202\\\\129.2484\\\\283.01\\\\89.08047\\\\126.9047\\\\283.01\\\\88.56892\\\\124.5609\\\\283.01\\\\87.69531\\\\122.2172\\\\283.01\\\\86.27174\\\\119.8734\\\\283.01\\\\85.33871\\\\117.5297\\\\283.01\\\\84.03852\\\\115.1859\\\\283.01\\\\81.2221\\\\110.4984\\\\283.01\\\\80.27344\\\\109.1604\\\\283.01\\\\77.73295\\\\105.8109\\\\283.01\\\\76.16416\\\\103.4672\\\\283.01\\\\74.09638\\\\101.1234\\\\283.01\\\\71.9184\\\\98.77969\\\\283.01\\\\68.55469\\\\95.4427\\\\283.01\\\\66.21094\\\\93.35094\\\\283.01\\\\63.86719\\\\91.35781\\\\283.01\\\\61.52344\\\\89.90582\\\\283.01\\\\59.17969\\\\88.0407\\\\283.01\\\\56.83594\\\\86.3728\\\\283.01\\\\54.49219\\\\85.23886\\\\283.01\\\\52.14844\\\\83.56911\\\\283.01\\\\49.80469\\\\82.70029\\\\283.01\\\\47.46094\\\\81.4009\\\\283.01\\\\45.11719\\\\80.45787\\\\283.01\\\\42.77344\\\\79.09343\\\\283.01\\\\40.42969\\\\78.69472\\\\283.01\\\\38.08594\\\\78.00113\\\\283.01\\\\35.74219\\\\77.01344\\\\283.01\\\\33.39844\\\\76.44418\\\\283.01\\\\31.05469\\\\76.01651\\\\283.01\\\\28.71094\\\\75.22678\\\\283.01\\\\26.36719\\\\74.61642\\\\283.01\\\\24.02344\\\\74.40105\\\\283.01\\\\14.64844\\\\74.05312\\\\283.01\\\\9.960938\\\\74.09591\\\\283.01\\\\2.929688\\\\74.41573\\\\283.01\\\\0.5859375\\\\74.74547\\\\283.01\\\\-4.101563\\\\76.1767\\\\283.01\\\\-6.445313\\\\76.62514\\\\283.01\\\\-8.789063\\\\77.27011\\\\283.01\\\\-11.13281\\\\78.3245\\\\283.01\\\\-13.47656\\\\78.80508\\\\283.01\\\\-15.82031\\\\79.42928\\\\283.01\\\\-18.16406\\\\80.84683\\\\283.01\\\\-20.50781\\\\81.73235\\\\283.01\\\\-22.85156\\\\83.10505\\\\283.01\\\\-25.19531\\\\83.8896\\\\283.01\\\\-27.53906\\\\85.73323\\\\283.01\\\\-29.88281\\\\86.93073\\\\283.01\\\\-32.22656\\\\88.45541\\\\283.01\\\\-34.57031\\\\90.46002\\\\283.01\\\\-36.32372\\\\91.74844\\\\283.01\\\\-41.60156\\\\96.22536\\\\283.01\\\\-44.08482\\\\98.77969\\\\283.01\\\\-46.28906\\\\101.1685\\\\283.01\\\\-48.63281\\\\104.0022\\\\283.01\\\\-49.98205\\\\105.8109\\\\283.01\\\\-51.99305\\\\108.1547\\\\283.01\\\\-53.27454\\\\110.4984\\\\283.01\\\\-54.77706\\\\112.8422\\\\283.01\\\\-56.46552\\\\115.1859\\\\283.01\\\\-57.23792\\\\117.5297\\\\283.01\\\\-58.69997\\\\119.8734\\\\283.01\\\\-59.59329\\\\122.2172\\\\283.01\\\\-60.93391\\\\124.5609\\\\283.01\\\\-61.45277\\\\126.9047\\\\283.01\\\\-62.0745\\\\129.2484\\\\283.01\\\\-63.20891\\\\131.5922\\\\283.01\\\\-63.89457\\\\133.9359\\\\283.01\\\\-64.2122\\\\136.2797\\\\283.01\\\\-64.74208\\\\138.6234\\\\283.01\\\\-65.41684\\\\140.9672\\\\283.01\\\\-65.71338\\\\143.3109\\\\283.01\\\\-66.04911\\\\147.9984\\\\283.01\\\\-66.25537\\\\152.6859\\\\283.01\\\\-66.28944\\\\155.0297\\\\283.01\\\\-66.13839\\\\159.7172\\\\283.01\\\\-65.84704\\\\164.4047\\\\283.01\\\\-65.6146\\\\166.7484\\\\283.01\\\\-65.16927\\\\169.0922\\\\283.01\\\\-64.43269\\\\171.4359\\\\283.01\\\\-64.04324\\\\173.7797\\\\283.01\\\\-63.56812\\\\176.1234\\\\283.01\\\\-62.48605\\\\178.4672\\\\283.01\\\\-61.66916\\\\180.8109\\\\283.01\\\\-61.23355\\\\183.1547\\\\283.01\\\\-60\\\\185.4984\\\\283.01\\\\-59.02929\\\\187.8422\\\\283.01\\\\-57.57813\\\\190.1859\\\\283.01\\\\-56.76455\\\\192.5297\\\\283.01\\\\-55.16396\\\\194.8734\\\\283.01\\\\-53.87801\\\\197.2172\\\\283.01\\\\-52.31674\\\\199.5609\\\\283.01\\\\-48.63281\\\\204.4387\\\\283.01\\\\-47.14196\\\\206.5922\\\\283.01\\\\-42.63475\\\\211.2797\\\\283.01\\\\-39.25781\\\\214.554\\\\283.01\\\\-36.91406\\\\216.4344\\\\283.01\\\\-32.22656\\\\219.8671\\\\283.01\\\\-27.53906\\\\223.0999\\\\283.01\\\\-23.31792\\\\225.3422\\\\283.01\\\\-22.85156\\\\225.6686\\\\283.01\\\\-20.50781\\\\226.7934\\\\283.01\\\\-18.16406\\\\228.3114\\\\283.01\\\\-15.82031\\\\229.1045\\\\283.01\\\\-14.9365\\\\230.0297\\\\283.01\\\\-15.82031\\\\231.4054\\\\283.01\\\\-16.93359\\\\232.3734\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846477463900001.467218939844\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"351\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"4\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0766\\\\286.01\\\\-60.35156\\\\233.1818\\\\286.01\\\\-62.69531\\\\233.0766\\\\286.01\\\\-65.03906\\\\232.8253\\\\286.01\\\\-69.72656\\\\232.9942\\\\286.01\\\\-83.78906\\\\233.0441\\\\286.01\\\\-95.50781\\\\233.0671\\\\286.01\\\\-97.85156\\\\233.3067\\\\286.01\\\\-100.1953\\\\233.6324\\\\286.01\\\\-102.5391\\\\233.4044\\\\286.01\\\\-107.2266\\\\233.1842\\\\286.01\\\\-109.5703\\\\233.2251\\\\286.01\\\\-111.9141\\\\233.9738\\\\286.01\\\\-114.2578\\\\233.8765\\\\286.01\\\\-116.6016\\\\233.6229\\\\286.01\\\\-118.9453\\\\233.9675\\\\286.01\\\\-130.6641\\\\233.9967\\\\286.01\\\\-133.0078\\\\234.0855\\\\286.01\\\\-137.6953\\\\234.0855\\\\286.01\\\\-142.3828\\\\234.1551\\\\286.01\\\\-144.7266\\\\234.0941\\\\286.01\\\\-154.1016\\\\234.0642\\\\286.01\\\\-158.7891\\\\234.0999\\\\286.01\\\\-165.8203\\\\234.0642\\\\286.01\\\\-168.1641\\\\234.2429\\\\286.01\\\\-175.1953\\\\234.2112\\\\286.01\\\\-182.2266\\\\234.2686\\\\286.01\\\\-184.5703\\\\234.3486\\\\286.01\\\\-196.2891\\\\234.2902\\\\286.01\\\\-198.6328\\\\234.2467\\\\286.01\\\\-203.3203\\\\234.2935\\\\286.01\\\\-210.3516\\\\234.2224\\\\286.01\\\\-217.3828\\\\234.3115\\\\286.01\\\\-226.7578\\\\234.3891\\\\286.01\\\\-233.7891\\\\234.2362\\\\286.01\\\\-236.1328\\\\234.3235\\\\286.01\\\\-238.4766\\\\234.2502\\\\286.01\\\\-243.1641\\\\234.2088\\\\286.01\\\\-247.8516\\\\233.9638\\\\286.01\\\\-252.5391\\\\234.2967\\\\286.01\\\\-254.8828\\\\234.7444\\\\286.01\\\\-257.2266\\\\235.601\\\\286.01\\\\-259.5802\\\\237.0609\\\\286.01\\\\-261.0486\\\\239.4047\\\\286.01\\\\-261.1572\\\\241.7484\\\\286.01\\\\-260.2748\\\\246.4359\\\\286.01\\\\-257.2359\\\\251.1234\\\\286.01\\\\-255.457\\\\253.4672\\\\286.01\\\\-250.6794\\\\258.1547\\\\286.01\\\\-248.6664\\\\260.4984\\\\286.01\\\\-245.5078\\\\263.4952\\\\286.01\\\\-244.041\\\\265.1859\\\\286.01\\\\-241.534\\\\267.5297\\\\286.01\\\\-239.4093\\\\269.8734\\\\286.01\\\\-236.9663\\\\272.2172\\\\286.01\\\\-233.7891\\\\275.371\\\\286.01\\\\-231.4453\\\\277.8091\\\\286.01\\\\-229.8568\\\\279.2484\\\\286.01\\\\-226.7578\\\\282.5186\\\\286.01\\\\-225.1317\\\\283.9359\\\\286.01\\\\-222.0703\\\\287.1792\\\\286.01\\\\-220.4928\\\\288.6234\\\\286.01\\\\-218.2422\\\\290.9672\\\\286.01\\\\-217.3828\\\\291.6227\\\\286.01\\\\-215.0391\\\\292.9462\\\\286.01\\\\-212.6953\\\\294.5394\\\\286.01\\\\-210.3516\\\\295.2875\\\\286.01\\\\-208.0078\\\\296.4592\\\\286.01\\\\-205.6641\\\\296.8852\\\\286.01\\\\-203.3203\\\\297.1639\\\\286.01\\\\-196.2891\\\\297.1563\\\\286.01\\\\-193.9453\\\\297.0926\\\\286.01\\\\-179.8828\\\\296.943\\\\286.01\\\\-165.8203\\\\296.8959\\\\286.01\\\\-156.4453\\\\296.9137\\\\286.01\\\\-151.7578\\\\296.9559\\\\286.01\\\\-149.4141\\\\296.9249\\\\286.01\\\\-137.6953\\\\296.9338\\\\286.01\\\\-125.9766\\\\296.9228\\\\286.01\\\\-114.2578\\\\296.8545\\\\286.01\\\\-111.9141\\\\296.8917\\\\286.01\\\\-100.1953\\\\296.8456\\\\286.01\\\\-93.16406\\\\296.8549\\\\286.01\\\\-83.78906\\\\296.9041\\\\286.01\\\\-81.44531\\\\296.8837\\\\286.01\\\\-74.41406\\\\296.9322\\\\286.01\\\\-67.38281\\\\296.9234\\\\286.01\\\\-65.03906\\\\296.9706\\\\286.01\\\\-50.97656\\\\296.9358\\\\286.01\\\\-48.63281\\\\297.048\\\\286.01\\\\-46.28906\\\\296.9267\\\\286.01\\\\-43.94531\\\\297.0552\\\\286.01\\\\-41.60156\\\\296.9396\\\\286.01\\\\-39.25781\\\\296.9191\\\\286.01\\\\-36.91406\\\\297.4229\\\\286.01\\\\-34.57031\\\\297.3212\\\\286.01\\\\-29.88281\\\\297.409\\\\286.01\\\\-25.19531\\\\297.1687\\\\286.01\\\\-22.85156\\\\297.4125\\\\286.01\\\\-11.13281\\\\297.4441\\\\286.01\\\\-6.445313\\\\297.4125\\\\286.01\\\\0.5859375\\\\297.4229\\\\286.01\\\\2.929688\\\\297.3586\\\\286.01\\\\5.273438\\\\297.4334\\\\286.01\\\\9.960938\\\\297.3911\\\\286.01\\\\12.30469\\\\296.9467\\\\286.01\\\\14.64844\\\\296.9899\\\\286.01\\\\19.33594\\\\296.8973\\\\286.01\\\\33.39844\\\\296.8867\\\\286.01\\\\38.08594\\\\296.8666\\\\286.01\\\\49.80469\\\\296.7436\\\\286.01\\\\52.14844\\\\296.6929\\\\286.01\\\\59.17969\\\\296.6906\\\\286.01\\\\61.52344\\\\296.6365\\\\286.01\\\\66.21094\\\\296.6454\\\\286.01\\\\80.27344\\\\296.5362\\\\286.01\\\\82.61719\\\\296.5521\\\\286.01\\\\87.30469\\\\296.4947\\\\286.01\\\\89.64844\\\\296.5284\\\\286.01\\\\99.02344\\\\296.506\\\\286.01\\\\108.3984\\\\296.3986\\\\286.01\\\\115.4297\\\\296.4117\\\\286.01\\\\117.7734\\\\296.3557\\\\286.01\\\\129.4922\\\\296.3023\\\\286.01\\\\131.8359\\\\296.3139\\\\286.01\\\\136.5234\\\\295.833\\\\286.01\\\\141.2109\\\\295.8172\\\\286.01\\\\143.5547\\\\295.9179\\\\286.01\\\\145.8984\\\\295.864\\\\286.01\\\\152.9297\\\\295.8012\\\\286.01\\\\155.2734\\\\295.8486\\\\286.01\\\\157.6172\\\\295.7516\\\\286.01\\\\162.3047\\\\295.7346\\\\286.01\\\\164.6484\\\\295.6819\\\\286.01\\\\169.3359\\\\295.6998\\\\286.01\\\\181.0547\\\\295.5401\\\\286.01\\\\185.7422\\\\295.5071\\\\286.01\\\\192.7734\\\\295.5401\\\\286.01\\\\197.4609\\\\295.8012\\\\286.01\\\\202.1484\\\\295.7516\\\\286.01\\\\204.4922\\\\295.4287\\\\286.01\\\\206.8359\\\\294.8107\\\\286.01\\\\209.1797\\\\294.347\\\\286.01\\\\211.5234\\\\292.432\\\\286.01\\\\213.8672\\\\291.0641\\\\286.01\\\\216.2109\\\\289.4511\\\\286.01\\\\217.0997\\\\288.6234\\\\286.01\\\\219.2182\\\\286.2797\\\\286.01\\\\221.6202\\\\283.9359\\\\286.01\\\\225.5859\\\\279.7662\\\\286.01\\\\227.9297\\\\277.4504\\\\286.01\\\\230.2734\\\\274.9912\\\\286.01\\\\233.0769\\\\272.2172\\\\286.01\\\\234.9609\\\\270.226\\\\286.01\\\\237.6601\\\\267.5297\\\\286.01\\\\241.9922\\\\263.0508\\\\286.01\\\\246.6797\\\\258.3049\\\\286.01\\\\249.209\\\\255.8109\\\\286.01\\\\251.3672\\\\253.4207\\\\286.01\\\\253.1321\\\\251.1234\\\\286.01\\\\255.0749\\\\248.7797\\\\286.01\\\\256.5535\\\\246.4359\\\\286.01\\\\257.7698\\\\244.0922\\\\286.01\\\\259.3152\\\\241.7484\\\\286.01\\\\259.6588\\\\239.4047\\\\286.01\\\\259.5052\\\\237.0609\\\\286.01\\\\257.4836\\\\234.7172\\\\286.01\\\\256.0547\\\\233.5391\\\\286.01\\\\253.7109\\\\232.9048\\\\286.01\\\\251.3672\\\\232.5688\\\\286.01\\\\249.0234\\\\232.7431\\\\286.01\\\\246.6797\\\\232.7947\\\\286.01\\\\244.3359\\\\232.7162\\\\286.01\\\\239.6484\\\\233.0007\\\\286.01\\\\232.6172\\\\233.0108\\\\286.01\\\\230.2734\\\\232.9699\\\\286.01\\\\220.8984\\\\233.0726\\\\286.01\\\\211.5234\\\\233.091\\\\286.01\\\\206.8359\\\\233.0044\\\\286.01\\\\204.4922\\\\232.7719\\\\286.01\\\\199.8047\\\\232.7456\\\\286.01\\\\195.1172\\\\232.8346\\\\286.01\\\\190.4297\\\\232.8586\\\\286.01\\\\181.0547\\\\232.8224\\\\286.01\\\\171.6797\\\\232.8346\\\\286.01\\\\166.9922\\\\232.81\\\\286.01\\\\159.9609\\\\232.8376\\\\286.01\\\\155.2734\\\\232.8129\\\\286.01\\\\150.5859\\\\232.8376\\\\286.01\\\\143.5547\\\\232.8253\\\\286.01\\\\138.8672\\\\232.7456\\\\286.01\\\\131.8359\\\\232.7048\\\\286.01\\\\127.1484\\\\232.7614\\\\286.01\\\\117.7734\\\\232.7048\\\\286.01\\\\113.0859\\\\232.721\\\\286.01\\\\108.3984\\\\232.6767\\\\286.01\\\\106.0547\\\\232.5873\\\\286.01\\\\103.7109\\\\232.8967\\\\286.01\\\\101.3672\\\\232.9081\\\\286.01\\\\99.02344\\\\233.0951\\\\286.01\\\\89.64844\\\\233.0819\\\\286.01\\\\80.27344\\\\233.1132\\\\286.01\\\\77.92969\\\\233.0819\\\\286.01\\\\73.24219\\\\233.1221\\\\286.01\\\\63.86719\\\\233.1397\\\\286.01\\\\47.46094\\\\233.1042\\\\286.01\\\\45.11719\\\\233.0859\\\\286.01\\\\44.1744\\\\232.3734\\\\286.01\\\\42.77344\\\\230.9829\\\\286.01\\\\42.0716\\\\230.0297\\\\286.01\\\\42.77344\\\\229.2739\\\\286.01\\\\45.11719\\\\228.6098\\\\286.01\\\\47.46094\\\\227.1533\\\\286.01\\\\49.80469\\\\226.1119\\\\286.01\\\\52.14844\\\\224.628\\\\286.01\\\\54.49219\\\\223.8002\\\\286.01\\\\56.83594\\\\221.9418\\\\286.01\\\\59.17969\\\\220.3256\\\\286.01\\\\61.52344\\\\219.017\\\\286.01\\\\62.28633\\\\218.3109\\\\286.01\\\\66.21094\\\\215.1155\\\\286.01\\\\67.83708\\\\213.6234\\\\286.01\\\\68.55469\\\\212.8201\\\\286.01\\\\70.89844\\\\210.637\\\\286.01\\\\73.24219\\\\208.133\\\\286.01\\\\76.6862\\\\204.2484\\\\286.01\\\\78.48246\\\\201.9047\\\\286.01\\\\80.27344\\\\199.2337\\\\286.01\\\\81.43317\\\\197.2172\\\\286.01\\\\83.23588\\\\194.8734\\\\286.01\\\\84.22379\\\\192.5297\\\\286.01\\\\85.618\\\\190.1859\\\\286.01\\\\86.50858\\\\187.8422\\\\286.01\\\\88.05106\\\\185.4984\\\\286.01\\\\89.46748\\\\180.8109\\\\286.01\\\\90.44455\\\\178.4672\\\\286.01\\\\91.72363\\\\173.7797\\\\286.01\\\\92.65485\\\\171.4359\\\\286.01\\\\93.02794\\\\169.0922\\\\286.01\\\\93.30979\\\\164.4047\\\\286.01\\\\93.66728\\\\159.7172\\\\286.01\\\\93.85384\\\\155.0297\\\\286.01\\\\93.76065\\\\150.3422\\\\286.01\\\\93.25375\\\\143.3109\\\\286.01\\\\92.96676\\\\138.6234\\\\286.01\\\\92.39601\\\\136.2797\\\\286.01\\\\91.37577\\\\133.9359\\\\286.01\\\\90.82589\\\\131.5922\\\\286.01\\\\90.13611\\\\129.2484\\\\286.01\\\\89.05535\\\\126.9047\\\\286.01\\\\88.54037\\\\124.5609\\\\286.01\\\\87.64068\\\\122.2172\\\\286.01\\\\86.2421\\\\119.8734\\\\286.01\\\\85.29913\\\\117.5297\\\\286.01\\\\84.01531\\\\115.1859\\\\286.01\\\\81.1849\\\\110.4984\\\\286.01\\\\77.92969\\\\106.097\\\\286.01\\\\76.125\\\\103.4672\\\\286.01\\\\71.88107\\\\98.77969\\\\286.01\\\\68.55469\\\\95.47974\\\\286.01\\\\63.86719\\\\91.39851\\\\286.01\\\\61.52344\\\\89.95306\\\\286.01\\\\59.17969\\\\88.07359\\\\286.01\\\\56.83594\\\\86.40219\\\\286.01\\\\54.49219\\\\85.2847\\\\286.01\\\\52.14844\\\\83.56911\\\\286.01\\\\49.80469\\\\82.75372\\\\286.01\\\\47.46094\\\\81.42015\\\\286.01\\\\45.11719\\\\80.50576\\\\286.01\\\\42.77344\\\\79.10091\\\\286.01\\\\40.42969\\\\78.70156\\\\286.01\\\\38.08594\\\\78.04225\\\\286.01\\\\35.74219\\\\77.04612\\\\286.01\\\\33.39844\\\\76.45493\\\\286.01\\\\31.05469\\\\76.03495\\\\286.01\\\\28.71094\\\\75.26105\\\\286.01\\\\26.36719\\\\74.62567\\\\286.01\\\\24.02344\\\\74.40835\\\\286.01\\\\16.99219\\\\74.11594\\\\286.01\\\\14.64844\\\\74.06655\\\\286.01\\\\9.960938\\\\74.10896\\\\286.01\\\\2.929688\\\\74.40835\\\\286.01\\\\0.5859375\\\\74.73109\\\\286.01\\\\-4.101563\\\\76.15349\\\\286.01\\\\-6.445313\\\\76.61499\\\\286.01\\\\-8.789063\\\\77.28263\\\\286.01\\\\-11.13281\\\\78.30383\\\\286.01\\\\-13.47656\\\\78.80508\\\\286.01\\\\-15.82031\\\\79.40802\\\\286.01\\\\-18.16406\\\\80.83015\\\\286.01\\\\-20.50781\\\\81.71255\\\\286.01\\\\-22.85156\\\\83.07788\\\\286.01\\\\-25.19531\\\\83.8896\\\\286.01\\\\-27.53906\\\\85.7117\\\\286.01\\\\-29.88281\\\\86.86703\\\\286.01\\\\-32.22656\\\\88.43999\\\\286.01\\\\-34.57031\\\\90.43253\\\\286.01\\\\-36.39626\\\\91.74844\\\\286.01\\\\-41.60156\\\\96.15862\\\\286.01\\\\-44.13608\\\\98.77969\\\\286.01\\\\-46.29859\\\\101.1234\\\\286.01\\\\-48.63281\\\\103.9307\\\\286.01\\\\-50.00961\\\\105.8109\\\\286.01\\\\-52.01322\\\\108.1547\\\\286.01\\\\-53.32031\\\\110.4421\\\\286.01\\\\-54.80162\\\\112.8422\\\\286.01\\\\-56.5049\\\\115.1859\\\\286.01\\\\-57.25154\\\\117.5297\\\\286.01\\\\-58.74689\\\\119.8734\\\\286.01\\\\-59.61914\\\\122.2172\\\\286.01\\\\-60.97609\\\\124.5609\\\\286.01\\\\-61.4707\\\\126.9047\\\\286.01\\\\-62.12735\\\\129.2484\\\\286.01\\\\-63.28823\\\\131.5922\\\\286.01\\\\-63.93199\\\\133.9359\\\\286.01\\\\-64.24364\\\\136.2797\\\\286.01\\\\-65.46725\\\\140.9672\\\\286.01\\\\-65.92946\\\\145.6547\\\\286.01\\\\-66.21094\\\\150.3422\\\\286.01\\\\-66.32308\\\\155.0297\\\\286.01\\\\-66.18277\\\\159.7172\\\\286.01\\\\-65.88306\\\\164.4047\\\\286.01\\\\-65.65548\\\\166.7484\\\\286.01\\\\-65.26347\\\\169.0922\\\\286.01\\\\-64.4847\\\\171.4359\\\\286.01\\\\-63.62205\\\\176.1234\\\\286.01\\\\-62.61541\\\\178.4672\\\\286.01\\\\-61.71265\\\\180.8109\\\\286.01\\\\-61.26099\\\\183.1547\\\\286.01\\\\-60.12716\\\\185.4984\\\\286.01\\\\-59.11038\\\\187.8422\\\\286.01\\\\-57.65788\\\\190.1859\\\\286.01\\\\-56.79988\\\\192.5297\\\\286.01\\\\-55.26563\\\\194.8734\\\\286.01\\\\-53.9098\\\\197.2172\\\\286.01\\\\-52.35414\\\\199.5609\\\\286.01\\\\-50.58594\\\\201.9047\\\\286.01\\\\-47.18681\\\\206.5922\\\\286.01\\\\-42.69849\\\\211.2797\\\\286.01\\\\-39.25781\\\\214.6135\\\\286.01\\\\-34.48792\\\\218.3109\\\\286.01\\\\-29.88281\\\\221.5676\\\\286.01\\\\-27.53906\\\\223.1529\\\\286.01\\\\-23.37105\\\\225.3422\\\\286.01\\\\-22.85156\\\\225.6946\\\\286.01\\\\-20.50781\\\\226.8241\\\\286.01\\\\-18.16406\\\\228.3425\\\\286.01\\\\-15.82031\\\\229.1045\\\\286.01\\\\-14.9365\\\\230.0297\\\\286.01\\\\-15.82031\\\\231.4054\\\\286.01\\\\-16.93359\\\\232.3734\\\\286.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n";
const char* k_rtStruct_json02 =
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846504465400001.571321740640\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"364\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"5\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"45.11719\\\\233.0859\\\\289.01\\\\44.16001\\\\232.3734\\\\289.01\\\\42.77344\\\\231.028\\\\289.01\\\\42.04498\\\\230.0297\\\\289.01\\\\42.77344\\\\229.2462\\\\289.01\\\\45.11719\\\\228.6173\\\\289.01\\\\47.46094\\\\227.1104\\\\289.01\\\\49.80469\\\\226.098\\\\289.01\\\\52.14844\\\\224.6141\\\\289.01\\\\54.49219\\\\223.7681\\\\289.01\\\\56.83594\\\\221.9221\\\\289.01\\\\59.17969\\\\220.2719\\\\289.01\\\\61.52344\\\\218.9816\\\\289.01\\\\62.24398\\\\218.3109\\\\289.01\\\\66.21094\\\\215.1002\\\\289.01\\\\67.80134\\\\213.6234\\\\289.01\\\\68.55469\\\\212.7718\\\\289.01\\\\70.89844\\\\210.6111\\\\289.01\\\\74.61816\\\\206.5922\\\\289.01\\\\76.64589\\\\204.2484\\\\289.01\\\\78.44145\\\\201.9047\\\\289.01\\\\80.27344\\\\199.1565\\\\289.01\\\\81.39723\\\\197.2172\\\\289.01\\\\83.19193\\\\194.8734\\\\289.01\\\\84.19829\\\\192.5297\\\\289.01\\\\85.55715\\\\190.1859\\\\289.01\\\\86.47488\\\\187.8422\\\\289.01\\\\87.98352\\\\185.4984\\\\289.01\\\\89.43614\\\\180.8109\\\\289.01\\\\90.43185\\\\178.4672\\\\289.01\\\\91.66755\\\\173.7797\\\\289.01\\\\92.6346\\\\171.4359\\\\289.01\\\\93.0094\\\\169.0922\\\\289.01\\\\93.45066\\\\162.0609\\\\289.01\\\\93.64378\\\\159.7172\\\\289.01\\\\93.74645\\\\157.3734\\\\289.01\\\\93.7934\\\\152.6859\\\\289.01\\\\93.7114\\\\150.3422\\\\289.01\\\\92.93333\\\\138.6234\\\\289.01\\\\92.32584\\\\136.2797\\\\289.01\\\\91.33673\\\\133.9359\\\\289.01\\\\90.79788\\\\131.5922\\\\289.01\\\\90.07662\\\\129.2484\\\\289.01\\\\89.01367\\\\126.9047\\\\289.01\\\\88.51154\\\\124.5609\\\\289.01\\\\87.5829\\\\122.2172\\\\289.01\\\\86.21134\\\\119.8734\\\\289.01\\\\85.2438\\\\117.5297\\\\289.01\\\\83.98051\\\\115.1859\\\\289.01\\\\82.44851\\\\112.8422\\\\289.01\\\\81.15704\\\\110.4984\\\\289.01\\\\77.92969\\\\106.1652\\\\289.01\\\\77.60783\\\\105.8109\\\\289.01\\\\76.08817\\\\103.4672\\\\289.01\\\\71.85195\\\\98.77969\\\\289.01\\\\68.55469\\\\95.5261\\\\289.01\\\\63.86719\\\\91.42658\\\\289.01\\\\61.52344\\\\90.00546\\\\289.01\\\\59.17969\\\\88.09907\\\\289.01\\\\56.83594\\\\86.42617\\\\289.01\\\\54.49219\\\\85.32829\\\\289.01\\\\52.14844\\\\83.5929\\\\289.01\\\\49.80469\\\\82.79196\\\\289.01\\\\47.46094\\\\81.4396\\\\289.01\\\\45.11719\\\\80.55132\\\\289.01\\\\42.77344\\\\79.11613\\\\289.01\\\\40.42969\\\\78.70156\\\\289.01\\\\38.08594\\\\78.06875\\\\289.01\\\\35.74219\\\\77.06593\\\\289.01\\\\33.39844\\\\76.46006\\\\289.01\\\\31.05469\\\\76.05299\\\\289.01\\\\28.71094\\\\75.29677\\\\289.01\\\\26.36719\\\\74.63503\\\\289.01\\\\24.02344\\\\74.42056\\\\289.01\\\\16.99219\\\\74.12224\\\\289.01\\\\12.30469\\\\74.05988\\\\289.01\\\\7.617188\\\\74.17641\\\\289.01\\\\2.929688\\\\74.40835\\\\289.01\\\\0.5859375\\\\74.69978\\\\289.01\\\\-4.101563\\\\76.1376\\\\289.01\\\\-6.445313\\\\76.582\\\\289.01\\\\-8.789063\\\\77.28263\\\\289.01\\\\-11.13281\\\\78.30383\\\\289.01\\\\-13.47656\\\\78.79336\\\\289.01\\\\-15.82031\\\\79.36918\\\\289.01\\\\-18.16406\\\\80.80879\\\\289.01\\\\-20.50781\\\\81.69893\\\\289.01\\\\-22.85156\\\\83.05925\\\\289.01\\\\-25.19531\\\\83.88489\\\\289.01\\\\-27.53906\\\\85.67889\\\\289.01\\\\-29.88281\\\\86.82161\\\\289.01\\\\-32.22656\\\\88.42921\\\\289.01\\\\-34.57031\\\\90.3988\\\\289.01\\\\-36.43259\\\\91.74844\\\\289.01\\\\-41.60156\\\\96.09487\\\\289.01\\\\-46.40716\\\\101.1234\\\\289.01\\\\-48.63281\\\\103.8925\\\\289.01\\\\-50.05581\\\\105.8109\\\\289.01\\\\-52.03897\\\\108.1547\\\\289.01\\\\-54.84508\\\\112.8422\\\\289.01\\\\-56.54788\\\\115.1859\\\\289.01\\\\-57.31724\\\\117.5297\\\\289.01\\\\-58.77809\\\\119.8734\\\\289.01\\\\-59.66415\\\\122.2172\\\\289.01\\\\-61.01031\\\\124.5609\\\\289.01\\\\-61.49414\\\\126.9047\\\\289.01\\\\-62.17529\\\\129.2484\\\\289.01\\\\-63.36781\\\\131.5922\\\\289.01\\\\-63.95774\\\\133.9359\\\\289.01\\\\-64.28482\\\\136.2797\\\\289.01\\\\-65.52673\\\\140.9672\\\\289.01\\\\-65.77621\\\\143.3109\\\\289.01\\\\-66.25\\\\150.3422\\\\289.01\\\\-66.3455\\\\155.0297\\\\289.01\\\\-66.22784\\\\159.7172\\\\289.01\\\\-65.91351\\\\164.4047\\\\289.01\\\\-65.68493\\\\166.7484\\\\289.01\\\\-65.33604\\\\169.0922\\\\289.01\\\\-64.55139\\\\171.4359\\\\289.01\\\\-63.66697\\\\176.1234\\\\289.01\\\\-61.73706\\\\180.8109\\\\289.01\\\\-61.2854\\\\183.1547\\\\289.01\\\\-60.35156\\\\185.3682\\\\289.01\\\\-59.14885\\\\187.8422\\\\289.01\\\\-57.77686\\\\190.1859\\\\289.01\\\\-56.84834\\\\192.5297\\\\289.01\\\\-55.35103\\\\194.8734\\\\289.01\\\\-53.94861\\\\197.2172\\\\289.01\\\\-50.97656\\\\201.5585\\\\289.01\\\\-48.63281\\\\204.6038\\\\289.01\\\\-47.23869\\\\206.5922\\\\289.01\\\\-42.7328\\\\211.2797\\\\289.01\\\\-39.25781\\\\214.6555\\\\289.01\\\\-34.54219\\\\218.3109\\\\289.01\\\\-32.22656\\\\219.8927\\\\289.01\\\\-27.53906\\\\223.2363\\\\289.01\\\\-23.37646\\\\225.3422\\\\289.01\\\\-22.85156\\\\225.6921\\\\289.01\\\\-20.50781\\\\226.8489\\\\289.01\\\\-18.16406\\\\228.3625\\\\289.01\\\\-15.82031\\\\229.1235\\\\289.01\\\\-14.9447\\\\230.0297\\\\289.01\\\\-15.82031\\\\231.385\\\\289.01\\\\-16.95703\\\\232.3734\\\\289.01\\\\-18.16406\\\\233.0632\\\\289.01\\\\-20.50781\\\\233.1042\\\\289.01\\\\-27.53906\\\\233.0951\\\\289.01\\\\-36.91406\\\\233.1132\\\\289.01\\\\-50.97656\\\\233.1652\\\\289.01\\\\-60.35156\\\\233.1652\\\\289.01\\\\-62.69531\\\\233.0671\\\\289.01\\\\-65.03906\\\\232.8376\\\\289.01\\\\-69.72656\\\\232.9839\\\\289.01\\\\-76.75781\\\\233.0245\\\\289.01\\\\-90.82031\\\\233.0382\\\\289.01\\\\-97.85156\\\\233.1345\\\\289.01\\\\-100.1953\\\\233.0992\\\\289.01\\\\-102.5391\\\\233.1524\\\\289.01\\\\-104.8828\\\\233.3934\\\\289.01\\\\-107.2266\\\\233.2115\\\\289.01\\\\-109.5703\\\\233.2027\\\\289.01\\\\-111.9141\\\\233.9046\\\\289.01\\\\-114.2578\\\\233.3123\\\\289.01\\\\-116.6016\\\\233.131\\\\289.01\\\\-118.9453\\\\233.9503\\\\289.01\\\\-123.6328\\\\234.0035\\\\289.01\\\\-130.6641\\\\234.0035\\\\289.01\\\\-135.3516\\\\234.1218\\\\289.01\\\\-137.6953\\\\234.054\\\\289.01\\\\-142.3828\\\\234.1551\\\\289.01\\\\-147.0703\\\\234.0336\\\\289.01\\\\-158.7891\\\\234.0691\\\\289.01\\\\-165.8203\\\\234.0314\\\\289.01\\\\-168.1641\\\\234.2175\\\\289.01\\\\-172.8516\\\\234.2539\\\\289.01\\\\-175.1953\\\\234.1973\\\\289.01\\\\-179.8828\\\\234.2721\\\\289.01\\\\-182.2266\\\\234.2574\\\\289.01\\\\-186.9141\\\\234.3359\\\\289.01\\\\-189.2578\\\\234.2721\\\\289.01\\\\-196.2891\\\\234.2502\\\\289.01\\\\-198.6328\\\\234.205\\\\289.01\\\\-200.9766\\\\234.2644\\\\289.01\\\\-205.6641\\\\234.2088\\\\289.01\\\\-215.0391\\\\234.2398\\\\289.01\\\\-217.3828\\\\234.2822\\\\289.01\\\\-229.1016\\\\234.2967\\\\289.01\\\\-233.7891\\\\234.1692\\\\289.01\\\\-238.4766\\\\234.2088\\\\289.01\\\\-243.1641\\\\234.1652\\\\289.01\\\\-245.5078\\\\234.0396\\\\289.01\\\\-247.8516\\\\234.0172\\\\289.01\\\\-250.1953\\\\234.1313\\\\289.01\\\\-252.5391\\\\234.3629\\\\289.01\\\\-254.8828\\\\235.229\\\\289.01\\\\-257.4503\\\\237.0609\\\\289.01\\\\-258.6989\\\\239.4047\\\\289.01\\\\-258.6135\\\\241.7484\\\\289.01\\\\-258.1335\\\\244.0922\\\\289.01\\\\-257.8311\\\\246.4359\\\\289.01\\\\-256.3118\\\\251.1234\\\\289.01\\\\-254.9679\\\\253.4672\\\\289.01\\\\-252.5391\\\\256.3686\\\\289.01\\\\-250.728\\\\258.1547\\\\289.01\\\\-248.6658\\\\260.4984\\\\289.01\\\\-245.5078\\\\263.5156\\\\289.01\\\\-243.1641\\\\266.0784\\\\289.01\\\\-241.5408\\\\267.5297\\\\289.01\\\\-239.3968\\\\269.8734\\\\289.01\\\\-233.7891\\\\275.3661\\\\289.01\\\\-231.4453\\\\277.8055\\\\289.01\\\\-229.1016\\\\280.0036\\\\289.01\\\\-226.7578\\\\282.5186\\\\289.01\\\\-224.4141\\\\284.6728\\\\289.01\\\\-222.0703\\\\287.1957\\\\289.01\\\\-220.4987\\\\288.6234\\\\289.01\\\\-218.2467\\\\290.9672\\\\289.01\\\\-217.3828\\\\291.6321\\\\289.01\\\\-215.0391\\\\292.9727\\\\289.01\\\\-212.6953\\\\294.534\\\\289.01\\\\-210.3516\\\\295.2615\\\\289.01\\\\-208.0078\\\\296.4424\\\\289.01\\\\-205.6641\\\\296.8658\\\\289.01\\\\-203.3203\\\\297.1563\\\\289.01\\\\-198.6328\\\\297.1793\\\\289.01\\\\-193.9453\\\\297.0673\\\\289.01\\\\-184.5703\\\\296.9514\\\\289.01\\\\-175.1953\\\\296.923\\\\289.01\\\\-165.8203\\\\296.8651\\\\289.01\\\\-154.1016\\\\296.9236\\\\289.01\\\\-149.4141\\\\296.8921\\\\289.01\\\\-144.7266\\\\296.9173\\\\289.01\\\\-137.6953\\\\296.9008\\\\289.01\\\\-135.3516\\\\296.9292\\\\289.01\\\\-128.3203\\\\296.9121\\\\289.01\\\\-116.6016\\\\296.8357\\\\289.01\\\\-111.9141\\\\296.8635\\\\289.01\\\\-102.5391\\\\296.8731\\\\289.01\\\\-88.47656\\\\296.8549\\\\289.01\\\\-81.44531\\\\296.8647\\\\289.01\\\\-79.10156\\\\296.913\\\\289.01\\\\-65.03906\\\\296.9409\\\\289.01\\\\-62.69531\\\\296.9706\\\\289.01\\\\-50.97656\\\\296.9668\\\\289.01\\\\-48.63281\\\\297.2359\\\\289.01\\\\-46.28906\\\\296.9447\\\\289.01\\\\-43.94531\\\\297.1975\\\\289.01\\\\-41.60156\\\\296.9191\\\\289.01\\\\-39.25781\\\\296.9095\\\\289.01\\\\-36.91406\\\\297.267\\\\289.01\\\\-34.57031\\\\297.0362\\\\289.01\\\\-32.22656\\\\297.4055\\\\289.01\\\\-27.53906\\\\297.3806\\\\289.01\\\\-25.19531\\\\297.4229\\\\289.01\\\\-13.47656\\\\297.4441\\\\289.01\\\\-4.101563\\\\297.4229\\\\289.01\\\\-1.757813\\\\297.2719\\\\289.01\\\\0.5859375\\\\297.4334\\\\289.01\\\\2.929688\\\\297.1626\\\\289.01\\\\5.273438\\\\297.4266\\\\289.01\\\\7.617188\\\\297.3879\\\\289.01\\\\9.960938\\\\297.2416\\\\289.01\\\\12.30469\\\\296.9841\\\\289.01\\\\14.64844\\\\297.1018\\\\289.01\\\\16.99219\\\\296.9081\\\\289.01\\\\21.67969\\\\296.9095\\\\289.01\\\\26.36719\\\\296.8673\\\\289.01\\\\28.71094\\\\296.9175\\\\289.01\\\\31.05469\\\\296.8771\\\\289.01\\\\47.46094\\\\296.7858\\\\289.01\\\\49.80469\\\\296.7229\\\\289.01\\\\59.17969\\\\296.6999\\\\289.01\\\\61.52344\\\\296.6331\\\\289.01\\\\68.55469\\\\296.6365\\\\289.01\\\\73.24219\\\\296.5732\\\\289.01\\\\84.96094\\\\296.5362\\\\289.01\\\\91.99219\\\\296.5569\\\\289.01\\\\103.7109\\\\296.4873\\\\289.01\\\\108.3984\\\\296.3976\\\\289.01\\\\113.0859\\\\296.4325\\\\289.01\\\\120.1172\\\\296.3495\\\\289.01\\\\122.4609\\\\296.364\\\\289.01\\\\127.1484\\\\296.3023\\\\289.01\\\\131.8359\\\\296.3259\\\\289.01\\\\134.1797\\\\296.1569\\\\289.01\\\\136.5234\\\\295.8717\\\\289.01\\\\138.8672\\\\295.9832\\\\289.01\\\\141.2109\\\\295.8685\\\\289.01\\\\143.5547\\\\296.085\\\\289.01\\\\145.8984\\\\295.8791\\\\289.01\\\\152.9297\\\\295.833\\\\289.01\\\\155.2734\\\\295.864\\\\289.01\\\\157.6172\\\\295.7849\\\\289.01\\\\164.6484\\\\295.6998\\\\289.01\\\\169.3359\\\\295.7346\\\\289.01\\\\174.0234\\\\295.6454\\\\289.01\\\\178.7109\\\\295.6272\\\\289.01\\\\188.0859\\\\295.5071\\\\289.01\\\\192.7734\\\\295.557\\\\289.01\\\\197.4609\\\\295.7849\\\\289.01\\\\202.1484\\\\295.7173\\\\289.01\\\\204.4922\\\\295.4439\\\\289.01\\\\206.8359\\\\294.8185\\\\289.01\\\\209.1797\\\\294.3595\\\\289.01\\\\211.5234\\\\292.4465\\\\289.01\\\\213.8672\\\\291.0974\\\\289.01\\\\216.2109\\\\289.4896\\\\289.01\\\\217.1345\\\\288.6234\\\\289.01\\\\218.5547\\\\287.023\\\\289.01\\\\221.6488\\\\283.9359\\\\289.01\\\\225.5859\\\\279.7803\\\\289.01\\\\227.9297\\\\277.5081\\\\289.01\\\\230.2734\\\\275.0207\\\\289.01\\\\232.6172\\\\272.7547\\\\289.01\\\\234.9609\\\\270.2734\\\\289.01\\\\237.7078\\\\267.5297\\\\289.01\\\\239.9578\\\\265.1859\\\\289.01\\\\244.3359\\\\260.784\\\\289.01\\\\246.6797\\\\258.2849\\\\289.01\\\\249.093\\\\255.8109\\\\289.01\\\\251.3672\\\\252.927\\\\289.01\\\\253.7109\\\\249.593\\\\289.01\\\\254.3837\\\\248.7797\\\\289.01\\\\255.2195\\\\246.4359\\\\289.01\\\\255.899\\\\244.0922\\\\289.01\\\\256.6528\\\\239.4047\\\\289.01\\\\256.6685\\\\237.0609\\\\289.01\\\\256.0547\\\\236.1658\\\\289.01\\\\255.3861\\\\234.7172\\\\289.01\\\\253.7109\\\\233.2892\\\\289.01\\\\251.3672\\\\232.6603\\\\289.01\\\\246.6797\\\\232.927\\\\289.01\\\\244.3359\\\\232.8224\\\\289.01\\\\239.6484\\\\233.0007\\\\289.01\\\\237.3047\\\\233.0305\\\\289.01\\\\230.2734\\\\232.9699\\\\289.01\\\\220.8984\\\\233.091\\\\289.01\\\\209.1797\\\\233.0819\\\\289.01\\\\206.8359\\\\233.0245\\\\289.01\\\\204.4922\\\\232.7848\\\\289.01\\\\199.8047\\\\232.7588\\\\289.01\\\\197.4609\\\\232.81\\\\289.01\\\\190.4297\\\\232.8467\\\\289.01\\\\183.3984\\\\232.8224\\\\289.01\\\\164.6484\\\\232.81\\\\289.01\\\\155.2734\\\\232.7875\\\\289.01\\\\148.2422\\\\232.8253\\\\289.01\\\\143.5547\\\\232.8003\\\\289.01\\\\138.8672\\\\232.7186\\\\289.01\\\\129.4922\\\\232.6623\\\\289.01\\\\122.4609\\\\232.7048\\\\289.01\\\\117.7734\\\\232.6477\\\\289.01\\\\115.4297\\\\232.6908\\\\289.01\\\\110.7422\\\\232.6477\\\\289.01\\\\108.3984\\\\232.5557\\\\289.01\\\\106.0547\\\\231.9897\\\\289.01\\\\103.7109\\\\232.81\\\\289.01\\\\101.3672\\\\232.8735\\\\289.01\\\\99.02344\\\\233.0819\\\\289.01\\\\82.61719\\\\233.1091\\\\289.01\\\\75.58594\\\\233.1042\\\\289.01\\\\63.86719\\\\233.1397\\\\289.01\\\\61.52344\\\\233.1132\\\\289.01\\\\54.49219\\\\233.131\\\\289.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846528466800001.465226999207\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"352\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"6\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0632\\\\292.01\\\\-34.57031\\\\233.1221\\\\292.01\\\\-46.28906\\\\233.1221\\\\292.01\\\\-53.32031\\\\233.1652\\\\292.01\\\\-60.35156\\\\233.1736\\\\292.01\\\\-62.69531\\\\233.0951\\\\292.01\\\\-65.03906\\\\232.8376\\\\292.01\\\\-69.72656\\\\232.9735\\\\292.01\\\\-83.78906\\\\233.0245\\\\292.01\\\\-86.13281\\\\233.0082\\\\292.01\\\\-95.50781\\\\233.0766\\\\292.01\\\\-109.5703\\\\233.1083\\\\292.01\\\\-111.9141\\\\233.1524\\\\292.01\\\\-114.2578\\\\233.4062\\\\292.01\\\\-116.6016\\\\233.1221\\\\292.01\\\\-118.9453\\\\233.9388\\\\292.01\\\\-123.6328\\\\233.9793\\\\292.01\\\\-130.6641\\\\233.9793\\\\292.01\\\\-135.3516\\\\234.0805\\\\292.01\\\\-137.6953\\\\234.0105\\\\292.01\\\\-142.3828\\\\234.0855\\\\292.01\\\\-149.4141\\\\233.9967\\\\292.01\\\\-154.1016\\\\234.0263\\\\292.01\\\\-161.1328\\\\233.9784\\\\292.01\\\\-165.8203\\\\233.9954\\\\292.01\\\\-168.1641\\\\234.1756\\\\292.01\\\\-172.8516\\\\234.2357\\\\292.01\\\\-175.1953\\\\234.1837\\\\292.01\\\\-179.8828\\\\234.2394\\\\292.01\\\\-186.9141\\\\234.2574\\\\292.01\\\\-189.2578\\\\234.2012\\\\292.01\\\\-193.9453\\\\234.205\\\\292.01\\\\-198.6328\\\\234.1437\\\\292.01\\\\-200.9766\\\\234.2224\\\\292.01\\\\-205.6641\\\\234.1523\\\\292.01\\\\-212.6953\\\\234.2224\\\\292.01\\\\-215.0391\\\\234.2088\\\\292.01\\\\-222.0703\\\\234.2644\\\\292.01\\\\-229.1016\\\\234.2224\\\\292.01\\\\-233.7891\\\\234.0787\\\\292.01\\\\-238.4766\\\\234.1437\\\\292.01\\\\-243.1641\\\\234.1482\\\\292.01\\\\-245.5078\\\\234.0015\\\\292.01\\\\-247.8516\\\\233.9742\\\\292.01\\\\-250.1953\\\\234.1478\\\\292.01\\\\-252.5391\\\\234.7444\\\\292.01\\\\-254.8828\\\\236.1971\\\\292.01\\\\-255.7199\\\\237.0609\\\\292.01\\\\-256.4182\\\\239.4047\\\\292.01\\\\-255.9699\\\\244.0922\\\\292.01\\\\-255.9741\\\\246.4359\\\\292.01\\\\-255.8244\\\\248.7797\\\\292.01\\\\-255.2465\\\\251.1234\\\\292.01\\\\-254.8828\\\\251.716\\\\292.01\\\\-252.9549\\\\255.8109\\\\292.01\\\\-250.1953\\\\258.7278\\\\292.01\\\\-247.8516\\\\261.3344\\\\292.01\\\\-245.5078\\\\263.528\\\\292.01\\\\-243.1641\\\\266.0844\\\\292.01\\\\-241.5582\\\\267.5297\\\\292.01\\\\-239.4252\\\\269.8734\\\\292.01\\\\-233.7891\\\\275.382\\\\292.01\\\\-231.4453\\\\277.8153\\\\292.01\\\\-229.1016\\\\280.0153\\\\292.01\\\\-226.7578\\\\282.5249\\\\292.01\\\\-224.4141\\\\284.6784\\\\292.01\\\\-222.0703\\\\287.1957\\\\292.01\\\\-220.5108\\\\288.6234\\\\292.01\\\\-218.2575\\\\290.9672\\\\292.01\\\\-217.3828\\\\291.6415\\\\292.01\\\\-215.0391\\\\293\\\\292.01\\\\-212.6953\\\\294.5345\\\\292.01\\\\-210.3516\\\\295.3008\\\\292.01\\\\-208.0078\\\\296.4338\\\\292.01\\\\-205.6641\\\\296.8658\\\\292.01\\\\-203.3203\\\\297.1488\\\\292.01\\\\-196.2891\\\\297.1563\\\\292.01\\\\-193.9453\\\\297.0522\\\\292.01\\\\-184.5703\\\\296.9452\\\\292.01\\\\-177.5391\\\\296.9218\\\\292.01\\\\-172.8516\\\\296.8713\\\\292.01\\\\-161.1328\\\\296.8585\\\\292.01\\\\-158.7891\\\\296.882\\\\292.01\\\\-149.4141\\\\296.8672\\\\292.01\\\\-147.0703\\\\296.8921\\\\292.01\\\\-137.6953\\\\296.8852\\\\292.01\\\\-128.3203\\\\296.9041\\\\292.01\\\\-116.6016\\\\296.8174\\\\292.01\\\\-102.5391\\\\296.8641\\\\292.01\\\\-95.50781\\\\296.8456\\\\292.01\\\\-90.82031\\\\296.8738\\\\292.01\\\\-74.41406\\\\296.8938\\\\292.01\\\\-67.38281\\\\296.9218\\\\292.01\\\\-62.69531\\\\296.9731\\\\292.01\\\\-50.97656\\\\296.9447\\\\292.01\\\\-48.63281\\\\297.0809\\\\292.01\\\\-46.28906\\\\296.9447\\\\292.01\\\\-43.94531\\\\297.1128\\\\292.01\\\\-41.60156\\\\296.9191\\\\292.01\\\\-36.91406\\\\297.3879\\\\292.01\\\\-34.57031\\\\297.0362\\\\292.01\\\\-29.88281\\\\297.4229\\\\292.01\\\\-18.16406\\\\297.3836\\\\292.01\\\\-13.47656\\\\297.4229\\\\292.01\\\\-11.13281\\\\297.3192\\\\292.01\\\\-8.789063\\\\297.4125\\\\292.01\\\\-4.101563\\\\297.3947\\\\292.01\\\\-1.757813\\\\297.1564\\\\292.01\\\\0.5859375\\\\297.4125\\\\292.01\\\\5.273438\\\\297.2937\\\\292.01\\\\7.617188\\\\296.9841\\\\292.01\\\\9.960938\\\\297.1865\\\\292.01\\\\12.30469\\\\296.9081\\\\292.01\\\\14.64844\\\\296.9396\\\\292.01\\\\19.33594\\\\296.8985\\\\292.01\\\\28.71094\\\\296.8973\\\\292.01\\\\35.74219\\\\296.8466\\\\292.01\\\\40.42969\\\\296.8673\\\\292.01\\\\61.52344\\\\296.6577\\\\292.01\\\\68.55469\\\\296.6487\\\\292.01\\\\75.58594\\\\296.5943\\\\292.01\\\\87.30469\\\\296.549\\\\292.01\\\\91.99219\\\\296.5862\\\\292.01\\\\106.0547\\\\296.4597\\\\292.01\\\\113.0859\\\\296.4597\\\\292.01\\\\115.4297\\\\296.4117\\\\292.01\\\\127.1484\\\\296.3229\\\\292.01\\\\134.1797\\\\296.3288\\\\292.01\\\\136.5234\\\\296.2406\\\\292.01\\\\143.5547\\\\296.2559\\\\292.01\\\\145.8984\\\\295.894\\\\292.01\\\\148.2422\\\\295.9087\\\\292.01\\\\155.2734\\\\295.864\\\\292.01\\\\162.3047\\\\295.7516\\\\292.01\\\\166.9922\\\\295.7849\\\\292.01\\\\171.6797\\\\295.6998\\\\292.01\\\\183.3984\\\\295.557\\\\292.01\\\\188.0859\\\\295.5916\\\\292.01\\\\192.7734\\\\295.5742\\\\292.01\\\\197.4609\\\\295.8012\\\\292.01\\\\202.1484\\\\295.7173\\\\292.01\\\\204.4922\\\\295.4287\\\\292.01\\\\206.8359\\\\294.8185\\\\292.01\\\\209.1797\\\\294.3768\\\\292.01\\\\211.5234\\\\292.4613\\\\292.01\\\\213.8672\\\\291.1455\\\\292.01\\\\216.2109\\\\289.5482\\\\292.01\\\\217.195\\\\288.6234\\\\292.01\\\\218.5547\\\\287.0752\\\\292.01\\\\221.6824\\\\283.9359\\\\292.01\\\\225.5859\\\\279.8388\\\\292.01\\\\227.9297\\\\277.5633\\\\292.01\\\\230.2734\\\\275.0632\\\\292.01\\\\233.2115\\\\272.2172\\\\292.01\\\\234.9609\\\\270.3186\\\\292.01\\\\237.735\\\\267.5297\\\\292.01\\\\239.9391\\\\265.1859\\\\292.01\\\\244.3359\\\\260.7643\\\\292.01\\\\246.751\\\\258.1547\\\\292.01\\\\249.0234\\\\255.6098\\\\292.01\\\\250.567\\\\253.4672\\\\292.01\\\\252.0319\\\\251.1234\\\\292.01\\\\252.7511\\\\248.7797\\\\292.01\\\\253.3727\\\\246.4359\\\\292.01\\\\253.5807\\\\244.0922\\\\292.01\\\\253.6483\\\\241.7484\\\\292.01\\\\254.3572\\\\239.4047\\\\292.01\\\\254.5918\\\\237.0609\\\\292.01\\\\253.1041\\\\234.7172\\\\292.01\\\\251.3672\\\\232.9906\\\\292.01\\\\249.0234\\\\232.8704\\\\292.01\\\\246.6797\\\\232.9906\\\\292.01\\\\244.3359\\\\232.9594\\\\292.01\\\\239.6484\\\\233.0108\\\\292.01\\\\230.2734\\\\232.9906\\\\292.01\\\\220.8984\\\\233.0819\\\\292.01\\\\211.5234\\\\233.091\\\\292.01\\\\206.8359\\\\233.0245\\\\292.01\\\\204.4922\\\\232.7848\\\\292.01\\\\199.8047\\\\232.7719\\\\292.01\\\\190.4297\\\\232.8346\\\\292.01\\\\181.0547\\\\232.7719\\\\292.01\\\\178.7109\\\\232.7975\\\\292.01\\\\174.0234\\\\232.7588\\\\292.01\\\\166.9922\\\\232.7456\\\\292.01\\\\152.9297\\\\232.8003\\\\292.01\\\\145.8984\\\\232.7875\\\\292.01\\\\141.2109\\\\232.6908\\\\292.01\\\\136.5234\\\\232.6908\\\\292.01\\\\134.1797\\\\232.6477\\\\292.01\\\\127.1484\\\\232.6767\\\\292.01\\\\124.8047\\\\232.6329\\\\292.01\\\\122.4609\\\\232.6767\\\\292.01\\\\117.7734\\\\232.6329\\\\292.01\\\\115.4297\\\\232.6767\\\\292.01\\\\110.7422\\\\232.6329\\\\292.01\\\\108.3984\\\\232.4011\\\\292.01\\\\106.0547\\\\231.7168\\\\292.01\\\\103.7109\\\\232.7346\\\\292.01\\\\99.02344\\\\233.0819\\\\292.01\\\\96.67969\\\\233.1001\\\\292.01\\\\80.27344\\\\233.0859\\\\292.01\\\\77.92969\\\\233.131\\\\292.01\\\\68.55469\\\\233.1042\\\\292.01\\\\63.86719\\\\233.1483\\\\292.01\\\\54.49219\\\\233.1132\\\\292.01\\\\45.11719\\\\233.1042\\\\292.01\\\\44.13916\\\\232.3734\\\\292.01\\\\42.77344\\\\231.028\\\\292.01\\\\42.04498\\\\230.0297\\\\292.01\\\\42.77344\\\\229.2462\\\\292.01\\\\45.11719\\\\228.6098\\\\292.01\\\\47.46094\\\\227.1\\\\292.01\\\\49.80469\\\\226.0746\\\\292.01\\\\52.14844\\\\224.5908\\\\292.01\\\\54.49219\\\\223.7256\\\\292.01\\\\56.83594\\\\221.9043\\\\292.01\\\\59.17969\\\\220.2334\\\\292.01\\\\61.52344\\\\218.9233\\\\292.01\\\\62.17359\\\\218.3109\\\\292.01\\\\65.03114\\\\215.9672\\\\292.01\\\\66.21094\\\\215.0853\\\\292.01\\\\67.77555\\\\213.6234\\\\292.01\\\\68.55469\\\\212.7391\\\\292.01\\\\70.89844\\\\210.5899\\\\292.01\\\\74.60287\\\\206.5922\\\\292.01\\\\76.61049\\\\204.2484\\\\292.01\\\\78.37865\\\\201.9047\\\\292.01\\\\80.27344\\\\199.0659\\\\292.01\\\\81.3732\\\\197.2172\\\\292.01\\\\83.11832\\\\194.8734\\\\292.01\\\\84.16552\\\\192.5297\\\\292.01\\\\85.52934\\\\190.1859\\\\292.01\\\\86.43362\\\\187.8422\\\\292.01\\\\87.9126\\\\185.4984\\\\292.01\\\\89.3596\\\\180.8109\\\\292.01\\\\90.38334\\\\178.4672\\\\292.01\\\\91.58888\\\\173.7797\\\\292.01\\\\92.57813\\\\171.4359\\\\292.01\\\\92.99067\\\\169.0922\\\\292.01\\\\93.58354\\\\159.7172\\\\292.01\\\\93.69106\\\\157.3734\\\\292.01\\\\93.75715\\\\152.6859\\\\292.01\\\\93.66728\\\\150.3422\\\\292.01\\\\93.08162\\\\140.9672\\\\292.01\\\\92.89124\\\\138.6234\\\\292.01\\\\92.23668\\\\136.2797\\\\292.01\\\\91.29035\\\\133.9359\\\\292.01\\\\90.7637\\\\131.5922\\\\292.01\\\\90.01319\\\\129.2484\\\\292.01\\\\88.96999\\\\126.9047\\\\292.01\\\\88.48242\\\\124.5609\\\\292.01\\\\87.50434\\\\122.2172\\\\292.01\\\\86.16302\\\\119.8734\\\\292.01\\\\85.18534\\\\117.5297\\\\292.01\\\\83.93555\\\\115.1859\\\\292.01\\\\82.35394\\\\112.8422\\\\292.01\\\\81.1205\\\\110.4984\\\\292.01\\\\77.92969\\\\106.2493\\\\292.01\\\\77.53906\\\\105.8109\\\\292.01\\\\76.03666\\\\103.4672\\\\292.01\\\\71.81973\\\\98.77969\\\\292.01\\\\68.55469\\\\95.57756\\\\292.01\\\\63.86719\\\\91.47022\\\\292.01\\\\61.52344\\\\90.03793\\\\292.01\\\\59.17969\\\\88.1257\\\\292.01\\\\56.83594\\\\86.46088\\\\292.01\\\\54.49219\\\\85.37372\\\\292.01\\\\52.14844\\\\83.62384\\\\292.01\\\\49.80469\\\\82.8195\\\\292.01\\\\47.46094\\\\81.45925\\\\292.01\\\\45.11719\\\\80.5947\\\\292.01\\\\42.77344\\\\79.13168\\\\292.01\\\\40.42969\\\\78.71358\\\\292.01\\\\38.08594\\\\78.09455\\\\292.01\\\\35.74219\\\\77.09653\\\\292.01\\\\33.39844\\\\76.47086\\\\292.01\\\\31.05469\\\\76.06187\\\\292.01\\\\28.71094\\\\75.29641\\\\292.01\\\\26.36719\\\\74.65405\\\\292.01\\\\24.02344\\\\74.42056\\\\292.01\\\\14.64844\\\\74.05988\\\\292.01\\\\12.30469\\\\74.06546\\\\292.01\\\\7.617188\\\\74.16415\\\\292.01\\\\2.929688\\\\74.40835\\\\292.01\\\\0.5859375\\\\74.66957\\\\292.01\\\\-4.101563\\\\76.10484\\\\292.01\\\\-6.445313\\\\76.55428\\\\292.01\\\\-8.789063\\\\77.2619\\\\292.01\\\\-11.13281\\\\78.29331\\\\292.01\\\\-15.82031\\\\79.33912\\\\292.01\\\\-18.16406\\\\80.80879\\\\292.01\\\\-20.50781\\\\81.67974\\\\292.01\\\\-22.85156\\\\83.04977\\\\292.01\\\\-25.19531\\\\83.8679\\\\292.01\\\\-27.53906\\\\85.67369\\\\292.01\\\\-29.88281\\\\86.77807\\\\292.01\\\\-32.22656\\\\88.40859\\\\292.01\\\\-34.57031\\\\90.3647\\\\292.01\\\\-36.45433\\\\91.74844\\\\292.01\\\\-39.25781\\\\94.04677\\\\292.01\\\\-41.60156\\\\96.05102\\\\292.01\\\\-46.47692\\\\101.1234\\\\292.01\\\\-48.63281\\\\103.8257\\\\292.01\\\\-50.09444\\\\105.8109\\\\292.01\\\\-52.05878\\\\108.1547\\\\292.01\\\\-53.53416\\\\110.4984\\\\292.01\\\\-54.89417\\\\112.8422\\\\292.01\\\\-56.60029\\\\115.1859\\\\292.01\\\\-57.3473\\\\117.5297\\\\292.01\\\\-58.83843\\\\119.8734\\\\292.01\\\\-59.71175\\\\122.2172\\\\292.01\\\\-61.0397\\\\124.5609\\\\292.01\\\\-61.54119\\\\126.9047\\\\292.01\\\\-62.23412\\\\129.2484\\\\292.01\\\\-63.41292\\\\131.5922\\\\292.01\\\\-63.98491\\\\133.9359\\\\292.01\\\\-64.32825\\\\136.2797\\\\292.01\\\\-65.57173\\\\140.9672\\\\292.01\\\\-65.81827\\\\143.3109\\\\292.01\\\\-66.16071\\\\147.9984\\\\292.01\\\\-66.35813\\\\152.6859\\\\292.01\\\\-66.35179\\\\157.3734\\\\292.01\\\\-66.26755\\\\159.7172\\\\292.01\\\\-65.93555\\\\164.4047\\\\292.01\\\\-65.72266\\\\166.7484\\\\292.01\\\\-65.40382\\\\169.0922\\\\292.01\\\\-64.58663\\\\171.4359\\\\292.01\\\\-63.71209\\\\176.1234\\\\292.01\\\\-61.78386\\\\180.8109\\\\292.01\\\\-61.30256\\\\183.1547\\\\292.01\\\\-60.36086\\\\185.4984\\\\292.01\\\\-59.18586\\\\187.8422\\\\292.01\\\\-57.87363\\\\190.1859\\\\292.01\\\\-56.84834\\\\192.5297\\\\292.01\\\\-55.66406\\\\194.5832\\\\292.01\\\\-54.00845\\\\197.2172\\\\292.01\\\\-50.97656\\\\201.6594\\\\292.01\\\\-47.27555\\\\206.5922\\\\292.01\\\\-42.76654\\\\211.2797\\\\292.01\\\\-39.25781\\\\214.6977\\\\292.01\\\\-34.57031\\\\218.3578\\\\292.01\\\\-32.22656\\\\219.9231\\\\292.01\\\\-27.53906\\\\223.3272\\\\292.01\\\\-25.19531\\\\224.3983\\\\292.01\\\\-22.85156\\\\225.7515\\\\292.01\\\\-20.50781\\\\226.8662\\\\292.01\\\\-18.16406\\\\228.3724\\\\292.01\\\\-15.82031\\\\229.1319\\\\292.01\\\\-14.95299\\\\230.0297\\\\292.01\\\\-15.82031\\\\231.3763\\\\292.01\\\\-16.95703\\\\232.3734\\\\292.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848817597700001.509440095881\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"352\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"7\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0632\\\\295.01\\\\-39.25781\\\\233.1397\\\\295.01\\\\-41.60156\\\\233.1221\\\\295.01\\\\-53.32031\\\\233.1818\\\\295.01\\\\-60.35156\\\\233.1818\\\\295.01\\\\-62.69531\\\\233.1221\\\\295.01\\\\-65.03906\\\\232.8497\\\\295.01\\\\-69.72656\\\\232.9735\\\\295.01\\\\-74.41406\\\\233.0145\\\\295.01\\\\-83.78906\\\\233.0145\\\\295.01\\\\-93.16406\\\\233.0859\\\\295.01\\\\-109.5703\\\\233.1132\\\\295.01\\\\-111.9141\\\\233.5764\\\\295.01\\\\-114.2578\\\\233.9331\\\\295.01\\\\-125.9766\\\\234.0105\\\\295.01\\\\-130.6641\\\\233.9793\\\\295.01\\\\-135.3516\\\\234.0672\\\\295.01\\\\-137.6953\\\\234.0035\\\\295.01\\\\-142.3828\\\\234.0672\\\\295.01\\\\-151.7578\\\\233.9848\\\\295.01\\\\-154.1016\\\\234.0192\\\\295.01\\\\-163.4766\\\\233.9784\\\\295.01\\\\-165.8203\\\\234.0072\\\\295.01\\\\-168.1641\\\\234.1894\\\\295.01\\\\-172.8516\\\\234.2539\\\\295.01\\\\-175.1953\\\\234.2012\\\\295.01\\\\-186.9141\\\\234.2721\\\\295.01\\\\-189.2578\\\\234.2326\\\\295.01\\\\-193.9453\\\\234.261\\\\295.01\\\\-198.6328\\\\234.1692\\\\295.01\\\\-200.9766\\\\234.2362\\\\295.01\\\\-205.6641\\\\234.1564\\\\295.01\\\\-208.0078\\\\234.1992\\\\295.01\\\\-215.0391\\\\234.2125\\\\295.01\\\\-222.0703\\\\234.2967\\\\295.01\\\\-224.4141\\\\234.226\\\\295.01\\\\-229.1016\\\\234.2224\\\\295.01\\\\-233.7891\\\\234.067\\\\295.01\\\\-240.8203\\\\234.1692\\\\295.01\\\\-243.1641\\\\234.1482\\\\295.01\\\\-247.8516\\\\233.8526\\\\295.01\\\\-250.1953\\\\233.9885\\\\295.01\\\\-252.5391\\\\234.8955\\\\295.01\\\\-254.3724\\\\237.0609\\\\295.01\\\\-255.6279\\\\239.4047\\\\295.01\\\\-254.6435\\\\241.7484\\\\295.01\\\\-254.2969\\\\244.0922\\\\295.01\\\\-254.2467\\\\248.7797\\\\295.01\\\\-253.6501\\\\253.4672\\\\295.01\\\\-252.733\\\\255.8109\\\\295.01\\\\-250.7417\\\\258.1547\\\\295.01\\\\-247.8516\\\\261.34\\\\295.01\\\\-246.2058\\\\262.8422\\\\295.01\\\\-244.0969\\\\265.1859\\\\295.01\\\\-241.6217\\\\267.5297\\\\295.01\\\\-239.489\\\\269.8734\\\\295.01\\\\-233.7891\\\\275.382\\\\295.01\\\\-231.4453\\\\277.8344\\\\295.01\\\\-229.1016\\\\280.0153\\\\295.01\\\\-226.7578\\\\282.5409\\\\295.01\\\\-224.4141\\\\284.6849\\\\295.01\\\\-222.0703\\\\287.2122\\\\295.01\\\\-220.5406\\\\288.6234\\\\295.01\\\\-218.2745\\\\290.9672\\\\295.01\\\\-217.3828\\\\291.6508\\\\295.01\\\\-215.0391\\\\293.0865\\\\295.01\\\\-212.6953\\\\294.6149\\\\295.01\\\\-210.3516\\\\295.3416\\\\295.01\\\\-208.0078\\\\296.4592\\\\295.01\\\\-205.6641\\\\296.8974\\\\295.01\\\\-203.3203\\\\297.1488\\\\295.01\\\\-196.2891\\\\297.1563\\\\295.01\\\\-193.9453\\\\297.0647\\\\295.01\\\\-182.2266\\\\296.9607\\\\295.01\\\\-175.1953\\\\296.9311\\\\295.01\\\\-170.5078\\\\296.8813\\\\295.01\\\\-158.7891\\\\296.8978\\\\295.01\\\\-154.1016\\\\296.9236\\\\295.01\\\\-149.4141\\\\296.8921\\\\295.01\\\\-142.3828\\\\296.9338\\\\295.01\\\\-128.3203\\\\296.942\\\\295.01\\\\-116.6016\\\\296.854\\\\295.01\\\\-111.9141\\\\296.8927\\\\295.01\\\\-109.5703\\\\296.8635\\\\295.01\\\\-102.5391\\\\296.8927\\\\295.01\\\\-97.85156\\\\296.8641\\\\295.01\\\\-88.47656\\\\296.9234\\\\295.01\\\\-81.44531\\\\296.9428\\\\295.01\\\\-74.41406\\\\296.9322\\\\295.01\\\\-67.38281\\\\296.9622\\\\295.01\\\\-65.03906\\\\297.1085\\\\295.01\\\\-62.69531\\\\297.0091\\\\295.01\\\\-60.35156\\\\297.0949\\\\295.01\\\\-58.00781\\\\297.0091\\\\295.01\\\\-53.32031\\\\297.0091\\\\295.01\\\\-50.97656\\\\296.9644\\\\295.01\\\\-46.28906\\\\297.3691\\\\295.01\\\\-43.94531\\\\297.4229\\\\295.01\\\\-39.25781\\\\297.2468\\\\295.01\\\\-36.91406\\\\297.1104\\\\295.01\\\\-34.57031\\\\297.373\\\\295.01\\\\-32.22656\\\\297.22\\\\295.01\\\\-29.88281\\\\297.4441\\\\295.01\\\\-27.53906\\\\297.3984\\\\295.01\\\\-22.85156\\\\297.4334\\\\295.01\\\\-13.47656\\\\297.4125\\\\295.01\\\\-11.13281\\\\297.252\\\\295.01\\\\-8.789063\\\\297.4022\\\\295.01\\\\-6.445313\\\\297.231\\\\295.01\\\\-4.101563\\\\297.3798\\\\295.01\\\\-1.757813\\\\296.9644\\\\295.01\\\\0.5859375\\\\297.252\\\\295.01\\\\2.929688\\\\297.4229\\\\295.01\\\\5.273438\\\\297.1516\\\\295.01\\\\9.960938\\\\296.8998\\\\295.01\\\\12.30469\\\\296.8888\\\\295.01\\\\14.64844\\\\296.9755\\\\295.01\\\\16.99219\\\\296.8888\\\\295.01\\\\28.71094\\\\296.878\\\\295.01\\\\40.42969\\\\296.847\\\\295.01\\\\59.17969\\\\296.7115\\\\295.01\\\\61.52344\\\\296.6365\\\\295.01\\\\66.21094\\\\296.6365\\\\295.01\\\\77.92969\\\\296.5696\\\\295.01\\\\89.64844\\\\296.5616\\\\295.01\\\\101.3672\\\\296.506\\\\295.01\\\\106.0547\\\\296.4528\\\\295.01\\\\115.4297\\\\296.4117\\\\295.01\\\\124.8047\\\\296.308\\\\295.01\\\\129.4922\\\\296.3348\\\\295.01\\\\131.8359\\\\296.1569\\\\295.01\\\\134.1797\\\\296.1766\\\\295.01\\\\136.5234\\\\295.9102\\\\295.01\\\\141.2109\\\\295.8807\\\\295.01\\\\143.5547\\\\295.9983\\\\295.01\\\\145.8984\\\\295.8791\\\\295.01\\\\148.2422\\\\295.8791\\\\295.01\\\\157.6172\\\\295.7684\\\\295.01\\\\166.9922\\\\295.7173\\\\295.01\\\\169.3359\\\\295.6638\\\\295.01\\\\174.0234\\\\295.6638\\\\295.01\\\\176.3672\\\\295.5916\\\\295.01\\\\183.3984\\\\295.5235\\\\295.01\\\\188.0859\\\\295.5742\\\\295.01\\\\192.7734\\\\295.557\\\\295.01\\\\197.4609\\\\295.8012\\\\295.01\\\\202.1484\\\\295.7173\\\\295.01\\\\204.4922\\\\295.4137\\\\295.01\\\\206.8359\\\\294.8107\\\\295.01\\\\209.1797\\\\294.3656\\\\295.01\\\\211.5234\\\\292.4688\\\\295.01\\\\213.8672\\\\291.1611\\\\295.01\\\\216.2109\\\\289.5734\\\\295.01\\\\217.2207\\\\288.6234\\\\295.01\\\\218.5547\\\\287.0638\\\\295.01\\\\221.6933\\\\283.9359\\\\295.01\\\\225.5859\\\\279.8647\\\\295.01\\\\227.9297\\\\277.5633\\\\295.01\\\\230.2734\\\\275.067\\\\295.01\\\\233.2115\\\\272.2172\\\\295.01\\\\234.9609\\\\270.3037\\\\295.01\\\\237.7232\\\\267.5297\\\\295.01\\\\239.9391\\\\265.1859\\\\295.01\\\\244.3359\\\\260.7816\\\\295.01\\\\246.788\\\\258.1547\\\\295.01\\\\249.0234\\\\255.4653\\\\295.01\\\\250.3651\\\\253.4672\\\\295.01\\\\251.5764\\\\251.1234\\\\295.01\\\\252.2446\\\\248.7797\\\\295.01\\\\252.5619\\\\246.4359\\\\295.01\\\\252.6451\\\\241.7484\\\\295.01\\\\252.8992\\\\239.4047\\\\295.01\\\\253.0001\\\\237.0609\\\\295.01\\\\252.33\\\\234.7172\\\\295.01\\\\251.3672\\\\233.4192\\\\295.01\\\\249.0234\\\\232.9487\\\\295.01\\\\241.9922\\\\233.0537\\\\295.01\\\\237.3047\\\\232.9699\\\\295.01\\\\232.6172\\\\232.9487\\\\295.01\\\\227.9297\\\\233.0207\\\\295.01\\\\216.2109\\\\233.0819\\\\295.01\\\\211.5234\\\\233.0632\\\\295.01\\\\206.8359\\\\232.9699\\\\295.01\\\\204.4922\\\\232.7588\\\\295.01\\\\199.8047\\\\232.7322\\\\295.01\\\\190.4297\\\\232.7975\\\\295.01\\\\176.3672\\\\232.7186\\\\295.01\\\\162.3047\\\\232.7322\\\\295.01\\\\152.9297\\\\232.7848\\\\295.01\\\\143.5547\\\\232.7456\\\\295.01\\\\138.8672\\\\232.6623\\\\295.01\\\\136.5234\\\\232.6908\\\\295.01\\\\131.8359\\\\232.6329\\\\295.01\\\\127.1484\\\\232.6623\\\\295.01\\\\124.8047\\\\232.6329\\\\295.01\\\\110.7422\\\\232.6477\\\\295.01\\\\108.3984\\\\232.6179\\\\295.01\\\\106.0547\\\\231.7168\\\\295.01\\\\103.7109\\\\232.6027\\\\295.01\\\\101.3672\\\\232.9735\\\\295.01\\\\99.02344\\\\233.091\\\\295.01\\\\89.64844\\\\233.1091\\\\295.01\\\\80.27344\\\\233.0819\\\\295.01\\\\73.24219\\\\233.131\\\\295.01\\\\66.21094\\\\233.1132\\\\295.01\\\\56.83594\\\\233.1397\\\\295.01\\\\54.49219\\\\233.1042\\\\295.01\\\\45.11719\\\\233.0951\\\\295.01\\\\44.15678\\\\232.3734\\\\295.01\\\\42.77344\\\\231.0106\\\\295.01\\\\42.05375\\\\230.0297\\\\295.01\\\\42.77344\\\\229.2553\\\\295.01\\\\45.11719\\\\228.5743\\\\295.01\\\\47.46094\\\\227.0897\\\\295.01\\\\49.80469\\\\226.0467\\\\295.01\\\\52.14844\\\\224.5908\\\\295.01\\\\54.49219\\\\223.7059\\\\295.01\\\\56.83594\\\\221.8778\\\\295.01\\\\59.17969\\\\220.2334\\\\295.01\\\\61.52344\\\\218.9233\\\\295.01\\\\64.98363\\\\215.9672\\\\295.01\\\\66.21094\\\\215.0635\\\\295.01\\\\67.76714\\\\213.6234\\\\295.01\\\\68.55469\\\\212.7302\\\\295.01\\\\70.89844\\\\210.5392\\\\295.01\\\\74.57118\\\\206.5922\\\\295.01\\\\76.57597\\\\204.2484\\\\295.01\\\\78.3308\\\\201.9047\\\\295.01\\\\80.27344\\\\198.9949\\\\295.01\\\\81.35471\\\\197.2172\\\\295.01\\\\83.0397\\\\194.8734\\\\295.01\\\\84.14964\\\\192.5297\\\\295.01\\\\85.47139\\\\190.1859\\\\295.01\\\\86.37085\\\\187.8422\\\\295.01\\\\87.84957\\\\185.4984\\\\295.01\\\\88.62815\\\\183.1547\\\\295.01\\\\89.2244\\\\180.8109\\\\295.01\\\\90.29716\\\\178.4672\\\\295.01\\\\91.52786\\\\173.7797\\\\295.01\\\\92.50722\\\\171.4359\\\\295.01\\\\92.97176\\\\169.0922\\\\295.01\\\\93.68722\\\\157.3734\\\\295.01\\\\93.72856\\\\152.6859\\\\295.01\\\\93.52158\\\\147.9984\\\\295.01\\\\93.06984\\\\140.9672\\\\295.01\\\\92.83925\\\\138.6234\\\\295.01\\\\92.12536\\\\136.2797\\\\295.01\\\\91.24645\\\\133.9359\\\\295.01\\\\90.72266\\\\131.5922\\\\295.01\\\\89.95934\\\\129.2484\\\\295.01\\\\88.9045\\\\126.9047\\\\295.01\\\\88.46478\\\\124.5609\\\\295.01\\\\87.35082\\\\122.2172\\\\295.01\\\\86.12689\\\\119.8734\\\\295.01\\\\85.13927\\\\117.5297\\\\295.01\\\\83.90684\\\\115.1859\\\\295.01\\\\82.26481\\\\112.8422\\\\295.01\\\\81.10777\\\\110.4984\\\\295.01\\\\77.92969\\\\106.3366\\\\295.01\\\\77.45313\\\\105.8109\\\\295.01\\\\75.98749\\\\103.4672\\\\295.01\\\\71.75832\\\\98.77969\\\\295.01\\\\68.55469\\\\95.61713\\\\295.01\\\\63.86719\\\\91.51749\\\\295.01\\\\61.52344\\\\90.08352\\\\295.01\\\\59.17969\\\\88.14508\\\\295.01\\\\56.83594\\\\86.47145\\\\295.01\\\\54.49219\\\\85.40367\\\\295.01\\\\52.14844\\\\83.63686\\\\295.01\\\\49.80469\\\\82.85554\\\\295.01\\\\47.46094\\\\81.48983\\\\295.01\\\\45.11719\\\\80.5947\\\\295.01\\\\42.77344\\\\79.16859\\\\295.01\\\\40.42969\\\\78.71358\\\\295.01\\\\38.08594\\\\78.12252\\\\295.01\\\\35.74219\\\\77.10697\\\\295.01\\\\33.39844\\\\76.50327\\\\295.01\\\\31.05469\\\\76.07934\\\\295.01\\\\28.71094\\\\75.31472\\\\295.01\\\\26.36719\\\\74.67744\\\\295.01\\\\24.02344\\\\74.42056\\\\295.01\\\\16.99219\\\\74.12224\\\\295.01\\\\12.30469\\\\74.07317\\\\295.01\\\\7.617188\\\\74.17641\\\\295.01\\\\2.929688\\\\74.40835\\\\295.01\\\\0.5859375\\\\74.66957\\\\295.01\\\\-4.101563\\\\76.07934\\\\295.01\\\\-6.445313\\\\76.52584\\\\295.01\\\\-8.789063\\\\77.22805\\\\295.01\\\\-11.13281\\\\78.25741\\\\295.01\\\\-15.82031\\\\79.32516\\\\295.01\\\\-18.16406\\\\80.7914\\\\295.01\\\\-20.50781\\\\81.66099\\\\295.01\\\\-22.85156\\\\83.04977\\\\295.01\\\\-25.19531\\\\83.83018\\\\295.01\\\\-27.53906\\\\85.65853\\\\295.01\\\\-29.88281\\\\86.7363\\\\295.01\\\\-32.22656\\\\88.3882\\\\295.01\\\\-34.57031\\\\90.3647\\\\295.01\\\\-36.52969\\\\91.74844\\\\295.01\\\\-41.60156\\\\95.97882\\\\295.01\\\\-46.52696\\\\101.1234\\\\295.01\\\\-48.63281\\\\103.7908\\\\295.01\\\\-50.13672\\\\105.8109\\\\295.01\\\\-52.07761\\\\108.1547\\\\295.01\\\\-53.5798\\\\110.4984\\\\295.01\\\\-54.92211\\\\112.8422\\\\295.01\\\\-56.62057\\\\115.1859\\\\295.01\\\\-57.39671\\\\117.5297\\\\295.01\\\\-58.88357\\\\119.8734\\\\295.01\\\\-59.73514\\\\122.2172\\\\295.01\\\\-61.08647\\\\124.5609\\\\295.01\\\\-61.55913\\\\126.9047\\\\295.01\\\\-62.3125\\\\129.2484\\\\295.01\\\\-63.44866\\\\131.5922\\\\295.01\\\\-64.00282\\\\133.9359\\\\295.01\\\\-64.36475\\\\136.2797\\\\295.01\\\\-65.6146\\\\140.9672\\\\295.01\\\\-65.84246\\\\143.3109\\\\295.01\\\\-66.18851\\\\147.9984\\\\295.01\\\\-66.41199\\\\152.6859\\\\295.01\\\\-66.39387\\\\157.3734\\\\295.01\\\\-66.14896\\\\162.0609\\\\295.01\\\\-65.75378\\\\166.7484\\\\295.01\\\\-65.45489\\\\169.0922\\\\295.01\\\\-64.61088\\\\171.4359\\\\295.01\\\\-63.74142\\\\176.1234\\\\295.01\\\\-62.87364\\\\178.4672\\\\295.01\\\\-61.83551\\\\180.8109\\\\295.01\\\\-61.31699\\\\183.1547\\\\295.01\\\\-60.39734\\\\185.4984\\\\295.01\\\\-59.19819\\\\187.8422\\\\295.01\\\\-57.87465\\\\190.1859\\\\295.01\\\\-56.84821\\\\192.5297\\\\295.01\\\\-55.66406\\\\194.6634\\\\295.01\\\\-54.06846\\\\197.2172\\\\295.01\\\\-50.97656\\\\201.7629\\\\295.01\\\\-47.3112\\\\206.5922\\\\295.01\\\\-42.79436\\\\211.2797\\\\295.01\\\\-39.25781\\\\214.7107\\\\295.01\\\\-34.57031\\\\218.463\\\\295.01\\\\-32.22656\\\\219.9411\\\\295.01\\\\-27.96936\\\\222.9984\\\\295.01\\\\-27.53906\\\\223.3836\\\\295.01\\\\-25.19531\\\\224.406\\\\295.01\\\\-22.85156\\\\225.8094\\\\295.01\\\\-20.50781\\\\226.8905\\\\295.01\\\\-18.16406\\\\228.4055\\\\295.01\\\\-15.82031\\\\229.1319\\\\295.01\\\\-14.95299\\\\230.0297\\\\295.01\\\\-15.82031\\\\231.3646\\\\295.01\\\\-16.96898\\\\232.3734\\\\295.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848849599600001.538291679804\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"358\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"8\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0632\\\\298.01\\\\-27.53906\\\\233.1042\\\\298.01\\\\-29.88281\\\\233.0819\\\\298.01\\\\-34.57031\\\\233.131\\\\298.01\\\\-43.94531\\\\233.1221\\\\298.01\\\\-50.97656\\\\233.1736\\\\298.01\\\\-62.69531\\\\233.1397\\\\298.01\\\\-65.03906\\\\232.8376\\\\298.01\\\\-69.72656\\\\232.9839\\\\298.01\\\\-79.10156\\\\233.0245\\\\298.01\\\\-90.82031\\\\233.0382\\\\298.01\\\\-93.16406\\\\233.0859\\\\298.01\\\\-109.5703\\\\233.1132\\\\298.01\\\\-111.9141\\\\233.1791\\\\298.01\\\\-114.2578\\\\233.7139\\\\298.01\\\\-118.9453\\\\233.9793\\\\298.01\\\\-128.3203\\\\234.0284\\\\298.01\\\\-130.6641\\\\233.9793\\\\298.01\\\\-135.3516\\\\234.0591\\\\298.01\\\\-137.6953\\\\234.0284\\\\298.01\\\\-142.3828\\\\234.0855\\\\298.01\\\\-144.7266\\\\234.0284\\\\298.01\\\\-151.7578\\\\234.002\\\\298.01\\\\-158.7891\\\\234.0263\\\\298.01\\\\-163.4766\\\\233.9784\\\\298.01\\\\-165.8203\\\\234.0072\\\\298.01\\\\-168.1641\\\\234.1756\\\\298.01\\\\-170.5078\\\\234.2214\\\\298.01\\\\-179.8828\\\\234.1934\\\\298.01\\\\-186.9141\\\\234.2721\\\\298.01\\\\-189.2578\\\\234.2289\\\\298.01\\\\-193.9453\\\\234.2431\\\\298.01\\\\-198.6328\\\\234.1692\\\\298.01\\\\-200.9766\\\\234.2326\\\\298.01\\\\-205.6641\\\\234.1271\\\\298.01\\\\-212.6953\\\\234.2224\\\\298.01\\\\-215.0391\\\\234.1992\\\\298.01\\\\-222.0703\\\\234.3115\\\\298.01\\\\-224.4141\\\\234.2224\\\\298.01\\\\-226.7578\\\\234.2502\\\\298.01\\\\-233.7891\\\\234.0906\\\\298.01\\\\-238.4766\\\\234.0329\\\\298.01\\\\-243.1641\\\\234.0283\\\\298.01\\\\-247.8516\\\\233.7949\\\\298.01\\\\-250.1953\\\\233.8681\\\\298.01\\\\-252.5391\\\\234.7626\\\\298.01\\\\-254.3469\\\\237.0609\\\\298.01\\\\-255.6034\\\\239.4047\\\\298.01\\\\-254.5181\\\\241.7484\\\\298.01\\\\-254.2274\\\\244.0922\\\\298.01\\\\-254.181\\\\248.7797\\\\298.01\\\\-253.9355\\\\251.1234\\\\298.01\\\\-253.5926\\\\253.4672\\\\298.01\\\\-252.7483\\\\255.8109\\\\298.01\\\\-250.8092\\\\258.1547\\\\298.01\\\\-248.713\\\\260.4984\\\\298.01\\\\-246.263\\\\262.8422\\\\298.01\\\\-244.1406\\\\265.1859\\\\298.01\\\\-241.6671\\\\267.5297\\\\298.01\\\\-239.4754\\\\269.8734\\\\298.01\\\\-237.0156\\\\272.2172\\\\298.01\\\\-233.7891\\\\275.382\\\\298.01\\\\-231.4453\\\\277.8249\\\\298.01\\\\-229.1016\\\\279.9981\\\\298.01\\\\-226.7578\\\\282.5281\\\\298.01\\\\-224.4141\\\\284.6784\\\\298.01\\\\-222.0703\\\\287.2355\\\\298.01\\\\-220.5414\\\\288.6234\\\\298.01\\\\-218.2745\\\\290.9672\\\\298.01\\\\-217.3828\\\\291.6508\\\\298.01\\\\-212.6953\\\\294.5949\\\\298.01\\\\-210.3516\\\\295.3142\\\\298.01\\\\-208.0078\\\\296.4674\\\\298.01\\\\-205.6641\\\\296.8852\\\\298.01\\\\-203.3203\\\\297.1563\\\\298.01\\\\-196.2891\\\\297.1488\\\\298.01\\\\-193.9453\\\\297.0597\\\\298.01\\\\-182.2266\\\\296.9529\\\\298.01\\\\-168.1641\\\\296.8576\\\\298.01\\\\-154.1016\\\\296.9249\\\\298.01\\\\-149.4141\\\\296.8921\\\\298.01\\\\-128.3203\\\\296.9228\\\\298.01\\\\-121.2891\\\\296.8623\\\\298.01\\\\-111.9141\\\\296.8549\\\\298.01\\\\-107.2266\\\\296.8266\\\\298.01\\\\-102.5391\\\\296.8731\\\\298.01\\\\-95.50781\\\\296.8453\\\\298.01\\\\-88.47656\\\\296.9218\\\\298.01\\\\-83.78906\\\\296.9016\\\\298.01\\\\-69.72656\\\\296.979\\\\298.01\\\\-67.38281\\\\296.9514\\\\298.01\\\\-65.03906\\\\297.2199\\\\298.01\\\\-62.69531\\\\296.9622\\\\298.01\\\\-55.66406\\\\296.9926\\\\298.01\\\\-50.97656\\\\296.9467\\\\298.01\\\\-48.63281\\\\297.3652\\\\298.01\\\\-46.28906\\\\297.0439\\\\298.01\\\\-43.94531\\\\297.2875\\\\298.01\\\\-39.25781\\\\297.0121\\\\298.01\\\\-34.57031\\\\297.1564\\\\298.01\\\\-32.22656\\\\297.3612\\\\298.01\\\\-29.88281\\\\297.4229\\\\298.01\\\\-27.53906\\\\297.1687\\\\298.01\\\\-25.19531\\\\297.4334\\\\298.01\\\\-18.16406\\\\297.3612\\\\298.01\\\\-15.82031\\\\297.4441\\\\298.01\\\\-13.47656\\\\297.4125\\\\298.01\\\\-11.13281\\\\297.2468\\\\298.01\\\\-8.789063\\\\297.4125\\\\298.01\\\\-6.445313\\\\297.373\\\\298.01\\\\-4.101563\\\\297.4195\\\\298.01\\\\-1.757813\\\\297.077\\\\298.01\\\\0.5859375\\\\297.4229\\\\298.01\\\\2.929688\\\\297.4125\\\\298.01\\\\5.273438\\\\296.9489\\\\298.01\\\\7.617188\\\\297.3168\\\\298.01\\\\9.960938\\\\296.9377\\\\298.01\\\\12.30469\\\\296.8998\\\\298.01\\\\14.64844\\\\297.1975\\\\298.01\\\\16.99219\\\\296.8579\\\\298.01\\\\28.71094\\\\296.878\\\\298.01\\\\40.42969\\\\296.8163\\\\298.01\\\\42.77344\\\\296.8369\\\\298.01\\\\49.80469\\\\296.734\\\\298.01\\\\59.17969\\\\296.6906\\\\298.01\\\\61.52344\\\\296.6365\\\\298.01\\\\68.55469\\\\296.6278\\\\298.01\\\\73.24219\\\\296.5777\\\\298.01\\\\75.58594\\\\296.6191\\\\298.01\\\\84.96094\\\\296.5284\\\\298.01\\\\96.67969\\\\296.5538\\\\298.01\\\\103.7109\\\\296.479\\\\298.01\\\\115.4297\\\\296.4259\\\\298.01\\\\122.4609\\\\296.3434\\\\298.01\\\\129.4922\\\\296.3495\\\\298.01\\\\131.8359\\\\295.9141\\\\298.01\\\\136.5234\\\\296.2256\\\\298.01\\\\138.8672\\\\295.833\\\\298.01\\\\143.5547\\\\295.9857\\\\298.01\\\\145.8984\\\\295.8791\\\\298.01\\\\152.9297\\\\295.833\\\\298.01\\\\164.6484\\\\295.6819\\\\298.01\\\\171.6797\\\\295.6819\\\\298.01\\\\181.0547\\\\295.5401\\\\298.01\\\\185.7422\\\\295.5742\\\\298.01\\\\192.7734\\\\295.557\\\\298.01\\\\197.4609\\\\295.8012\\\\298.01\\\\202.1484\\\\295.6819\\\\298.01\\\\204.4922\\\\295.3698\\\\298.01\\\\206.8359\\\\294.803\\\\298.01\\\\209.1797\\\\294.3656\\\\298.01\\\\211.5234\\\\292.4764\\\\298.01\\\\213.8672\\\\291.1765\\\\298.01\\\\216.2109\\\\289.5873\\\\298.01\\\\217.229\\\\288.6234\\\\298.01\\\\218.5547\\\\287.0752\\\\298.01\\\\221.7097\\\\283.9359\\\\298.01\\\\225.5859\\\\279.8775\\\\298.01\\\\227.9297\\\\277.5633\\\\298.01\\\\230.2734\\\\275.0808\\\\298.01\\\\233.1989\\\\272.2172\\\\298.01\\\\234.9609\\\\270.2887\\\\298.01\\\\237.7384\\\\267.5297\\\\298.01\\\\241.9922\\\\263.0843\\\\298.01\\\\244.3359\\\\260.7643\\\\298.01\\\\246.788\\\\258.1547\\\\298.01\\\\249.0234\\\\255.451\\\\298.01\\\\250.3651\\\\253.4672\\\\298.01\\\\251.5297\\\\251.1234\\\\298.01\\\\252.1947\\\\248.7797\\\\298.01\\\\252.4915\\\\246.4359\\\\298.01\\\\252.5755\\\\241.7484\\\\298.01\\\\252.8592\\\\239.4047\\\\298.01\\\\252.9236\\\\237.0609\\\\298.01\\\\252.2924\\\\234.7172\\\\298.01\\\\251.3672\\\\233.4697\\\\298.01\\\\249.0234\\\\232.882\\\\298.01\\\\244.3359\\\\232.9048\\\\298.01\\\\241.9922\\\\233.0145\\\\298.01\\\\232.6172\\\\232.9048\\\\298.01\\\\227.9297\\\\233.0007\\\\298.01\\\\216.2109\\\\233.0632\\\\298.01\\\\211.5234\\\\233.0537\\\\298.01\\\\206.8359\\\\232.9699\\\\298.01\\\\204.4922\\\\232.7322\\\\298.01\\\\199.8047\\\\232.7186\\\\298.01\\\\190.4297\\\\232.7719\\\\298.01\\\\183.3984\\\\232.7719\\\\298.01\\\\181.0547\\\\232.7322\\\\298.01\\\\174.0234\\\\232.7048\\\\298.01\\\\171.6797\\\\232.7322\\\\298.01\\\\166.9922\\\\232.6908\\\\298.01\\\\157.6172\\\\232.7975\\\\298.01\\\\155.2734\\\\232.7588\\\\298.01\\\\148.2422\\\\232.7875\\\\298.01\\\\143.5547\\\\232.7614\\\\298.01\\\\138.8672\\\\232.6477\\\\298.01\\\\124.8047\\\\232.6179\\\\298.01\\\\122.4609\\\\232.6477\\\\298.01\\\\113.0859\\\\232.6027\\\\298.01\\\\110.7422\\\\232.4552\\\\298.01\\\\108.3984\\\\232.2192\\\\298.01\\\\106.0547\\\\231.6764\\\\298.01\\\\103.7109\\\\231.8559\\\\298.01\\\\102.8237\\\\232.3734\\\\298.01\\\\101.3672\\\\232.9839\\\\298.01\\\\99.02344\\\\233.0951\\\\298.01\\\\87.30469\\\\233.0819\\\\298.01\\\\84.96094\\\\233.1091\\\\298.01\\\\80.27344\\\\233.0726\\\\298.01\\\\77.92969\\\\233.1132\\\\298.01\\\\70.89844\\\\233.1397\\\\298.01\\\\68.55469\\\\233.1132\\\\298.01\\\\52.14844\\\\233.131\\\\298.01\\\\45.11719\\\\233.0859\\\\298.01\\\\44.16726\\\\232.3734\\\\298.01\\\\42.77344\\\\231.0206\\\\298.01\\\\42.04498\\\\230.0297\\\\298.01\\\\42.77344\\\\229.2462\\\\298.01\\\\45.11719\\\\228.5664\\\\298.01\\\\47.46094\\\\227.0695\\\\298.01\\\\49.80469\\\\226.0552\\\\298.01\\\\52.14844\\\\224.5723\\\\298.01\\\\54.49219\\\\223.6857\\\\298.01\\\\56.83594\\\\221.8519\\\\298.01\\\\59.17969\\\\220.2086\\\\298.01\\\\61.52344\\\\218.8854\\\\298.01\\\\64.94469\\\\215.9672\\\\298.01\\\\66.21094\\\\215.0191\\\\298.01\\\\67.72036\\\\213.6234\\\\298.01\\\\68.55469\\\\212.6986\\\\298.01\\\\70.89844\\\\210.5055\\\\298.01\\\\74.53191\\\\206.5922\\\\298.01\\\\76.54903\\\\204.2484\\\\298.01\\\\78.26105\\\\201.9047\\\\298.01\\\\80.27344\\\\198.9262\\\\298.01\\\\82.61719\\\\195.2822\\\\298.01\\\\82.98087\\\\194.8734\\\\298.01\\\\84.96094\\\\190.9255\\\\298.01\\\\85.43701\\\\190.1859\\\\298.01\\\\86.33423\\\\187.8422\\\\298.01\\\\87.78722\\\\185.4984\\\\298.01\\\\88.60233\\\\183.1547\\\\298.01\\\\89.10253\\\\180.8109\\\\298.01\\\\90.17504\\\\178.4672\\\\298.01\\\\90.88959\\\\176.1234\\\\298.01\\\\91.43783\\\\173.7797\\\\298.01\\\\92.39601\\\\171.4359\\\\298.01\\\\92.95762\\\\169.0922\\\\298.01\\\\93.55695\\\\159.7172\\\\298.01\\\\93.65527\\\\157.3734\\\\298.01\\\\93.67542\\\\152.6859\\\\298.01\\\\93.61213\\\\150.3422\\\\298.01\\\\93.22542\\\\143.3109\\\\298.01\\\\93.06345\\\\140.9672\\\\298.01\\\\92.77563\\\\138.6234\\\\298.01\\\\91.21714\\\\133.9359\\\\298.01\\\\90.67235\\\\131.5922\\\\298.01\\\\89.88776\\\\129.2484\\\\298.01\\\\88.8737\\\\126.9047\\\\298.01\\\\88.44087\\\\124.5609\\\\298.01\\\\86.09712\\\\119.8734\\\\298.01\\\\85.05786\\\\117.5297\\\\298.01\\\\83.87151\\\\115.1859\\\\298.01\\\\82.22388\\\\112.8422\\\\298.01\\\\81.09117\\\\110.4984\\\\298.01\\\\77.92969\\\\106.4052\\\\298.01\\\\77.3894\\\\105.8109\\\\298.01\\\\75.94332\\\\103.4672\\\\298.01\\\\71.71799\\\\98.77969\\\\298.01\\\\68.55469\\\\95.65721\\\\298.01\\\\63.86719\\\\91.54878\\\\298.01\\\\61.52344\\\\90.1121\\\\298.01\\\\59.17969\\\\88.15762\\\\298.01\\\\56.83594\\\\86.51503\\\\298.01\\\\54.49219\\\\85.42721\\\\298.01\\\\52.14844\\\\83.64907\\\\298.01\\\\49.80469\\\\82.89023\\\\298.01\\\\47.46094\\\\81.50237\\\\298.01\\\\45.11719\\\\80.62591\\\\298.01\\\\42.77344\\\\79.18153\\\\298.01\\\\40.42969\\\\78.7203\\\\298.01\\\\38.08594\\\\78.1349\\\\298.01\\\\35.74219\\\\77.11755\\\\298.01\\\\33.39844\\\\76.51949\\\\298.01\\\\31.05469\\\\76.07934\\\\298.01\\\\26.36719\\\\74.67744\\\\298.01\\\\24.02344\\\\74.42056\\\\298.01\\\\14.64844\\\\74.07317\\\\298.01\\\\9.960938\\\\74.11538\\\\298.01\\\\2.929688\\\\74.40105\\\\298.01\\\\0.5859375\\\\74.67952\\\\298.01\\\\-1.757813\\\\75.31406\\\\298.01\\\\-4.101563\\\\76.07065\\\\298.01\\\\-6.445313\\\\76.49051\\\\298.01\\\\-8.789063\\\\77.17276\\\\298.01\\\\-11.13281\\\\78.20097\\\\298.01\\\\-15.82031\\\\79.31967\\\\298.01\\\\-18.16406\\\\80.76948\\\\298.01\\\\-20.50781\\\\81.64266\\\\298.01\\\\-22.85156\\\\83.0305\\\\298.01\\\\-25.19531\\\\83.7937\\\\298.01\\\\-27.53906\\\\85.63515\\\\298.01\\\\-29.88281\\\\86.7363\\\\298.01\\\\-32.22656\\\\88.36089\\\\298.01\\\\-34.57031\\\\90.3302\\\\298.01\\\\-36.56719\\\\91.74844\\\\298.01\\\\-41.60156\\\\95.93605\\\\298.01\\\\-46.58845\\\\101.1234\\\\298.01\\\\-50.17995\\\\105.8109\\\\298.01\\\\-52.10386\\\\108.1547\\\\298.01\\\\-53.63992\\\\110.4984\\\\298.01\\\\-54.95532\\\\112.8422\\\\298.01\\\\-56.64794\\\\115.1859\\\\298.01\\\\-57.4403\\\\117.5297\\\\298.01\\\\-58.91927\\\\119.8734\\\\298.01\\\\-59.78655\\\\122.2172\\\\298.01\\\\-61.11754\\\\124.5609\\\\298.01\\\\-61.58921\\\\126.9047\\\\298.01\\\\-62.38012\\\\129.2484\\\\298.01\\\\-63.49118\\\\131.5922\\\\298.01\\\\-64.02599\\\\133.9359\\\\298.01\\\\-64.3932\\\\136.2797\\\\298.01\\\\-65.11897\\\\138.6234\\\\298.01\\\\-65.64544\\\\140.9672\\\\298.01\\\\-66.23938\\\\147.9984\\\\298.01\\\\-66.46289\\\\152.6859\\\\298.01\\\\-66.48911\\\\155.0297\\\\298.01\\\\-66.34437\\\\159.7172\\\\298.01\\\\-65.99894\\\\164.4047\\\\298.01\\\\-65.49149\\\\169.0922\\\\298.01\\\\-64.6875\\\\171.4359\\\\298.01\\\\-63.7739\\\\176.1234\\\\298.01\\\\-62.9398\\\\178.4672\\\\298.01\\\\-61.86011\\\\180.8109\\\\298.01\\\\-61.33423\\\\183.1547\\\\298.01\\\\-60.43332\\\\185.4984\\\\298.01\\\\-58.00781\\\\190.0632\\\\298.01\\\\-56.85406\\\\192.5297\\\\298.01\\\\-55.66406\\\\194.7283\\\\298.01\\\\-54.11692\\\\197.2172\\\\298.01\\\\-50.97656\\\\201.8369\\\\298.01\\\\-47.36435\\\\206.5922\\\\298.01\\\\-45.04395\\\\208.9359\\\\298.01\\\\-42.83026\\\\211.2797\\\\298.01\\\\-39.25781\\\\214.7435\\\\298.01\\\\-34.57031\\\\218.4974\\\\298.01\\\\-32.22656\\\\219.9595\\\\298.01\\\\-28.02053\\\\222.9984\\\\298.01\\\\-27.53906\\\\223.4238\\\\298.01\\\\-25.19531\\\\224.4187\\\\298.01\\\\-22.85156\\\\225.8252\\\\298.01\\\\-20.50781\\\\226.9067\\\\298.01\\\\-18.16406\\\\228.4286\\\\298.01\\\\-15.82031\\\\229.1235\\\\298.01\\\\-14.9447\\\\230.0297\\\\298.01\\\\-15.82031\\\\231.3969\\\\298.01\\\\-16.94484\\\\232.3734\\\\298.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848866600500001.541849142008\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"358\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"9\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-252.9673\\\\255.8109\\\\301.01\\\\-248.7477\\\\260.4984\\\\301.01\\\\-245.5078\\\\263.6263\\\\301.01\\\\-244.1215\\\\265.1859\\\\301.01\\\\-241.6381\\\\267.5297\\\\301.01\\\\-239.4403\\\\269.8734\\\\301.01\\\\-233.7891\\\\275.3765\\\\301.01\\\\-231.4453\\\\277.8215\\\\301.01\\\\-229.1016\\\\280.0093\\\\301.01\\\\-226.7578\\\\282.5186\\\\301.01\\\\-224.4141\\\\284.6481\\\\301.01\\\\-222.0703\\\\287.2057\\\\301.01\\\\-220.5227\\\\288.6234\\\\301.01\\\\-218.2914\\\\290.9672\\\\301.01\\\\-217.3828\\\\291.6599\\\\301.01\\\\-212.6953\\\\294.5939\\\\301.01\\\\-210.3516\\\\295.3008\\\\301.01\\\\-208.0078\\\\296.4424\\\\301.01\\\\-205.6641\\\\296.8595\\\\301.01\\\\-203.3203\\\\297.1563\\\\301.01\\\\-196.2891\\\\297.1563\\\\301.01\\\\-193.9453\\\\297.0746\\\\301.01\\\\-186.9141\\\\297.033\\\\301.01\\\\-177.5391\\\\296.8785\\\\301.01\\\\-165.8203\\\\296.8345\\\\301.01\\\\-151.7578\\\\296.8757\\\\301.01\\\\-149.4141\\\\296.8513\\\\301.01\\\\-140.0391\\\\296.8521\\\\301.01\\\\-137.6953\\\\296.886\\\\301.01\\\\-118.9453\\\\296.8623\\\\301.01\\\\-111.9141\\\\296.8078\\\\301.01\\\\-104.8828\\\\296.8456\\\\301.01\\\\-97.85156\\\\296.836\\\\301.01\\\\-93.16406\\\\296.8927\\\\301.01\\\\-86.13281\\\\296.9203\\\\301.01\\\\-81.44531\\\\296.8927\\\\301.01\\\\-74.41406\\\\296.9514\\\\301.01\\\\-58.00781\\\\296.9428\\\\301.01\\\\-53.32031\\\\296.9579\\\\301.01\\\\-50.97656\\\\296.9175\\\\301.01\\\\-48.63281\\\\297.0362\\\\301.01\\\\-46.28906\\\\296.9267\\\\301.01\\\\-43.94531\\\\297.2032\\\\301.01\\\\-41.60156\\\\297.3168\\\\301.01\\\\-39.25781\\\\296.8998\\\\301.01\\\\-34.57031\\\\297.3691\\\\301.01\\\\-32.22656\\\\297.2089\\\\301.01\\\\-29.88281\\\\297.4229\\\\301.01\\\\-27.53906\\\\297.3299\\\\301.01\\\\-25.19531\\\\297.4125\\\\301.01\\\\-13.47656\\\\297.4125\\\\301.01\\\\-11.13281\\\\297.252\\\\301.01\\\\-8.789063\\\\297.4055\\\\301.01\\\\-6.445313\\\\297.3015\\\\301.01\\\\-4.101563\\\\297.4125\\\\301.01\\\\-1.757813\\\\297.1261\\\\301.01\\\\0.5859375\\\\297.4229\\\\301.01\\\\2.929688\\\\297.4125\\\\301.01\\\\5.273438\\\\297.1687\\\\301.01\\\\7.617188\\\\297.0362\\\\301.01\\\\9.960938\\\\297.1516\\\\301.01\\\\12.30469\\\\296.8579\\\\301.01\\\\14.64844\\\\296.9396\\\\301.01\\\\19.33594\\\\296.8369\\\\301.01\\\\28.71094\\\\296.868\\\\301.01\\\\38.08594\\\\296.7858\\\\301.01\\\\45.11719\\\\296.7858\\\\301.01\\\\49.80469\\\\296.7115\\\\301.01\\\\54.49219\\\\296.7115\\\\301.01\\\\63.86719\\\\296.6154\\\\301.01\\\\66.21094\\\\296.6278\\\\301.01\\\\77.92969\\\\296.5901\\\\301.01\\\\80.27344\\\\296.5362\\\\301.01\\\\91.99219\\\\296.5284\\\\301.01\\\\94.33594\\\\296.5461\\\\301.01\\\\103.7109\\\\296.4917\\\\301.01\\\\106.0547\\\\296.5208\\\\301.01\\\\108.3984\\\\296.4462\\\\301.01\\\\120.1172\\\\296.3831\\\\301.01\\\\129.4922\\\\296.3139\\\\301.01\\\\134.1797\\\\295.9385\\\\301.01\\\\136.5234\\\\296.3023\\\\301.01\\\\138.8672\\\\295.833\\\\301.01\\\\141.2109\\\\295.939\\\\301.01\\\\155.2734\\\\295.7849\\\\301.01\\\\159.9609\\\\295.7849\\\\301.01\\\\169.3359\\\\295.6638\\\\301.01\\\\176.3672\\\\295.6638\\\\301.01\\\\181.0547\\\\295.5742\\\\301.01\\\\185.7422\\\\295.6093\\\\301.01\\\\192.7734\\\\295.5742\\\\301.01\\\\197.4609\\\\295.8172\\\\301.01\\\\202.1484\\\\295.6638\\\\301.01\\\\204.4922\\\\295.3698\\\\301.01\\\\206.8359\\\\294.8185\\\\301.01\\\\209.1797\\\\294.3828\\\\301.01\\\\211.5234\\\\292.4841\\\\301.01\\\\213.8672\\\\291.1916\\\\301.01\\\\216.2109\\\\289.6171\\\\301.01\\\\217.2534\\\\288.6234\\\\301.01\\\\218.5547\\\\287.0864\\\\301.01\\\\221.7097\\\\283.9359\\\\301.01\\\\225.5859\\\\279.9023\\\\301.01\\\\227.9297\\\\277.5431\\\\301.01\\\\230.2734\\\\275.0279\\\\301.01\\\\233.1732\\\\272.2172\\\\301.01\\\\234.9609\\\\270.242\\\\301.01\\\\237.6891\\\\267.5297\\\\301.01\\\\241.9922\\\\263.0863\\\\301.01\\\\244.3359\\\\260.7816\\\\301.01\\\\246.7498\\\\258.1547\\\\301.01\\\\249.0234\\\\255.4868\\\\301.01\\\\250.5381\\\\253.4672\\\\301.01\\\\251.9462\\\\251.1234\\\\301.01\\\\252.6083\\\\248.7797\\\\301.01\\\\253.1286\\\\246.4359\\\\301.01\\\\253.2715\\\\244.0922\\\\301.01\\\\253.323\\\\241.7484\\\\301.01\\\\254.0203\\\\239.4047\\\\301.01\\\\254.2562\\\\237.0609\\\\301.01\\\\253.7109\\\\236.3782\\\\301.01\\\\252.7413\\\\234.7172\\\\301.01\\\\251.3672\\\\233.1433\\\\301.01\\\\249.0234\\\\232.7162\\\\301.01\\\\246.6797\\\\232.5994\\\\301.01\\\\244.3359\\\\232.6145\\\\301.01\\\\241.9922\\\\232.8788\\\\301.01\\\\239.6484\\\\232.9594\\\\301.01\\\\232.6172\\\\232.9487\\\\301.01\\\\223.2422\\\\233.0498\\\\301.01\\\\211.5234\\\\233.0726\\\\301.01\\\\206.8359\\\\233.0007\\\\301.01\\\\204.4922\\\\232.7186\\\\301.01\\\\202.1484\\\\232.7048\\\\301.01\\\\195.1172\\\\232.7719\\\\301.01\\\\192.7734\\\\232.7456\\\\301.01\\\\183.3984\\\\232.7588\\\\301.01\\\\174.0234\\\\232.7322\\\\301.01\\\\164.6484\\\\232.7322\\\\301.01\\\\152.9297\\\\232.8003\\\\301.01\\\\145.8984\\\\232.7875\\\\301.01\\\\141.2109\\\\232.6908\\\\301.01\\\\134.1797\\\\232.6329\\\\301.01\\\\127.1484\\\\232.6329\\\\301.01\\\\124.8047\\\\232.5873\\\\301.01\\\\122.4609\\\\232.0753\\\\301.01\\\\120.1172\\\\232.6027\\\\301.01\\\\113.0859\\\\232.5873\\\\301.01\\\\110.7422\\\\231.8582\\\\301.01\\\\106.0547\\\\231.5762\\\\301.01\\\\103.7109\\\\231.6158\\\\301.01\\\\102.652\\\\232.3734\\\\301.01\\\\101.3672\\\\233.0044\\\\301.01\\\\99.02344\\\\233.091\\\\301.01\\\\89.64844\\\\233.0819\\\\301.01\\\\84.96094\\\\233.1132\\\\301.01\\\\66.21094\\\\233.1132\\\\301.01\\\\61.52344\\\\233.131\\\\301.01\\\\47.46094\\\\233.1132\\\\301.01\\\\45.11719\\\\233.0671\\\\301.01\\\\44.1887\\\\232.3734\\\\301.01\\\\42.77344\\\\231.0305\\\\301.01\\\\42.03629\\\\230.0297\\\\301.01\\\\42.77344\\\\229.2325\\\\301.01\\\\45.11719\\\\228.5376\\\\301.01\\\\47.46094\\\\227.0364\\\\301.01\\\\49.80469\\\\226.0128\\\\301.01\\\\52.14844\\\\224.5587\\\\301.01\\\\54.49219\\\\223.6608\\\\301.01\\\\56.83594\\\\221.833\\\\301.01\\\\59.17969\\\\220.1609\\\\301.01\\\\61.52344\\\\218.8188\\\\301.01\\\\64.90625\\\\215.9672\\\\301.01\\\\66.21094\\\\214.9687\\\\301.01\\\\67.67578\\\\213.6234\\\\301.01\\\\68.55469\\\\212.6704\\\\301.01\\\\70.89844\\\\210.4727\\\\301.01\\\\74.47953\\\\206.5922\\\\301.01\\\\76.50275\\\\204.2484\\\\301.01\\\\78.21651\\\\201.9047\\\\301.01\\\\80.27344\\\\198.8597\\\\301.01\\\\82.61719\\\\195.1853\\\\301.01\\\\82.89342\\\\194.8734\\\\301.01\\\\84.96094\\\\190.8408\\\\301.01\\\\85.38912\\\\190.1859\\\\301.01\\\\86.29591\\\\187.8422\\\\301.01\\\\87.73726\\\\185.4984\\\\301.01\\\\88.56714\\\\183.1547\\\\301.01\\\\89.0239\\\\180.8109\\\\301.01\\\\90.08218\\\\178.4672\\\\301.01\\\\90.826\\\\176.1234\\\\301.01\\\\91.33673\\\\173.7797\\\\301.01\\\\92.27242\\\\171.4359\\\\301.01\\\\92.92352\\\\169.0922\\\\301.01\\\\93.52101\\\\159.7172\\\\301.01\\\\93.68569\\\\155.0297\\\\301.01\\\\93.66122\\\\150.3422\\\\301.01\\\\93.03977\\\\140.9672\\\\301.01\\\\92.73054\\\\138.6234\\\\301.01\\\\91.17617\\\\133.9359\\\\301.01\\\\90.61806\\\\131.5922\\\\301.01\\\\89.82677\\\\129.2484\\\\301.01\\\\88.83517\\\\126.9047\\\\301.01\\\\88.39883\\\\124.5609\\\\301.01\\\\87.14217\\\\122.2172\\\\301.01\\\\83.83617\\\\115.1859\\\\301.01\\\\82.61719\\\\113.3482\\\\301.01\\\\82.16586\\\\112.8422\\\\301.01\\\\81.0612\\\\110.4984\\\\301.01\\\\77.92969\\\\106.4629\\\\301.01\\\\77.33624\\\\105.8109\\\\301.01\\\\75.88316\\\\103.4672\\\\301.01\\\\71.66676\\\\98.77969\\\\301.01\\\\68.55469\\\\95.70351\\\\301.01\\\\63.86719\\\\91.59751\\\\301.01\\\\61.52344\\\\90.16005\\\\301.01\\\\59.17969\\\\88.18321\\\\301.01\\\\56.83594\\\\86.57265\\\\301.01\\\\54.49219\\\\85.45554\\\\301.01\\\\52.14844\\\\83.67484\\\\301.01\\\\49.80469\\\\82.92365\\\\301.01\\\\47.46094\\\\81.52637\\\\301.01\\\\45.11719\\\\80.65604\\\\301.01\\\\42.77344\\\\79.19824\\\\301.01\\\\40.42969\\\\78.73884\\\\301.01\\\\38.08594\\\\78.15919\\\\301.01\\\\35.74219\\\\77.15002\\\\301.01\\\\33.39844\\\\76.53034\\\\301.01\\\\31.05469\\\\76.09643\\\\301.01\\\\26.36719\\\\74.69731\\\\301.01\\\\24.02344\\\\74.428\\\\301.01\\\\16.99219\\\\74.14042\\\\301.01\\\\12.30469\\\\74.06655\\\\301.01\\\\7.617188\\\\74.18246\\\\301.01\\\\2.929688\\\\74.39614\\\\301.01\\\\0.5859375\\\\74.67952\\\\301.01\\\\-4.101563\\\\76.04402\\\\301.01\\\\-6.445313\\\\76.46788\\\\301.01\\\\-8.789063\\\\77.11446\\\\301.01\\\\-11.13281\\\\78.1283\\\\301.01\\\\-15.82031\\\\79.30067\\\\301.01\\\\-18.16406\\\\80.75135\\\\301.01\\\\-20.50781\\\\81.63853\\\\301.01\\\\-22.85156\\\\82.99707\\\\301.01\\\\-25.19531\\\\83.76582\\\\301.01\\\\-27.53906\\\\85.60597\\\\301.01\\\\-29.88281\\\\86.70937\\\\301.01\\\\-32.22656\\\\88.32835\\\\301.01\\\\-34.57031\\\\90.28696\\\\301.01\\\\-36.60469\\\\91.74844\\\\301.01\\\\-41.60156\\\\95.87829\\\\301.01\\\\-46.64644\\\\101.1234\\\\301.01\\\\-50.23326\\\\105.8109\\\\301.01\\\\-52.14844\\\\108.1547\\\\301.01\\\\-53.7083\\\\110.4984\\\\301.01\\\\-54.99341\\\\112.8422\\\\301.01\\\\-56.69257\\\\115.1859\\\\301.01\\\\-57.50126\\\\117.5297\\\\301.01\\\\-58.9611\\\\119.8734\\\\301.01\\\\-59.86389\\\\122.2172\\\\301.01\\\\-61.15642\\\\124.5609\\\\301.01\\\\-61.63161\\\\126.9047\\\\301.01\\\\-62.48454\\\\129.2484\\\\301.01\\\\-63.52435\\\\131.5922\\\\301.01\\\\-64.05602\\\\133.9359\\\\301.01\\\\-64.42265\\\\136.2797\\\\301.01\\\\-65.18555\\\\138.6234\\\\301.01\\\\-65.68493\\\\140.9672\\\\301.01\\\\-66.26755\\\\147.9984\\\\301.01\\\\-66.50095\\\\152.6859\\\\301.01\\\\-66.53545\\\\155.0297\\\\301.01\\\\-66.39441\\\\159.7172\\\\301.01\\\\-66.05307\\\\164.4047\\\\301.01\\\\-65.51514\\\\169.0922\\\\301.01\\\\-64.74208\\\\171.4359\\\\301.01\\\\-64.20454\\\\173.7797\\\\301.01\\\\-63.7793\\\\176.1234\\\\301.01\\\\-62.91559\\\\178.4672\\\\301.01\\\\-61.86849\\\\180.8109\\\\301.01\\\\-61.33743\\\\183.1547\\\\301.01\\\\-60.35156\\\\185.6212\\\\301.01\\\\-57.99851\\\\190.1859\\\\301.01\\\\-56.86614\\\\192.5297\\\\301.01\\\\-55.66406\\\\194.816\\\\301.01\\\\-54.17318\\\\197.2172\\\\301.01\\\\-52.51101\\\\199.5609\\\\301.01\\\\-50.98609\\\\201.9047\\\\301.01\\\\-47.40362\\\\206.5922\\\\301.01\\\\-45.07058\\\\208.9359\\\\301.01\\\\-42.88062\\\\211.2797\\\\301.01\\\\-39.25781\\\\214.7825\\\\301.01\\\\-34.57031\\\\218.5488\\\\301.01\\\\-32.22656\\\\219.9976\\\\301.01\\\\-28.02734\\\\222.9984\\\\301.01\\\\-27.53906\\\\223.4429\\\\301.01\\\\-25.19531\\\\224.4472\\\\301.01\\\\-22.85156\\\\225.8554\\\\301.01\\\\-20.50781\\\\226.9317\\\\301.01\\\\-18.16406\\\\228.4426\\\\301.01\\\\-15.82031\\\\229.1456\\\\301.01\\\\-14.96138\\\\230.0297\\\\301.01\\\\-15.82031\\\\231.3675\\\\301.01\\\\-16.95703\\\\232.3734\\\\301.01\\\\-18.16406\\\\233.0671\\\\301.01\\\\-20.50781\\\\233.0537\\\\301.01\\\\-27.53906\\\\233.1221\\\\301.01\\\\-29.88281\\\\233.1042\\\\301.01\\\\-39.25781\\\\233.1397\\\\301.01\\\\-46.28906\\\\233.1221\\\\301.01\\\\-48.63281\\\\233.1652\\\\301.01\\\\-60.35156\\\\233.1818\\\\301.01\\\\-62.69531\\\\233.1483\\\\301.01\\\\-65.03906\\\\232.8735\\\\301.01\\\\-67.38281\\\\232.8735\\\\301.01\\\\-69.72656\\\\232.9839\\\\301.01\\\\-79.10156\\\\232.9942\\\\301.01\\\\-83.78906\\\\233.0537\\\\301.01\\\\-86.13281\\\\233.0283\\\\301.01\\\\-107.2266\\\\233.1132\\\\301.01\\\\-109.5703\\\\233.6324\\\\301.01\\\\-111.9141\\\\233.2523\\\\301.01\\\\-114.2578\\\\233.855\\\\301.01\\\\-118.9453\\\\233.9675\\\\301.01\\\\-123.6328\\\\234.0359\\\\301.01\\\\-128.3203\\\\234.054\\\\301.01\\\\-130.6641\\\\233.9859\\\\301.01\\\\-133.0078\\\\234.0284\\\\301.01\\\\-137.6953\\\\234.0158\\\\301.01\\\\-142.3828\\\\234.0805\\\\301.01\\\\-149.4141\\\\234.0088\\\\301.01\\\\-154.1016\\\\234.054\\\\301.01\\\\-165.8203\\\\233.9784\\\\301.01\\\\-168.1641\\\\234.1138\\\\301.01\\\\-172.8516\\\\234.1797\\\\301.01\\\\-179.8828\\\\234.1797\\\\301.01\\\\-184.5703\\\\234.2539\\\\301.01\\\\-198.6328\\\\234.1652\\\\301.01\\\\-200.9766\\\\234.2187\\\\301.01\\\\-205.6641\\\\234.1147\\\\301.01\\\\-208.0078\\\\234.1692\\\\301.01\\\\-217.3828\\\\234.2088\\\\301.01\\\\-222.0703\\\\234.2967\\\\301.01\\\\-224.4141\\\\234.2088\\\\301.01\\\\-229.1016\\\\234.2088\\\\301.01\\\\-238.4766\\\\234.0063\\\\301.01\\\\-243.1641\\\\233.9589\\\\301.01\\\\-245.5078\\\\233.8656\\\\301.01\\\\-247.8516\\\\233.8749\\\\301.01\\\\-250.1953\\\\233.9791\\\\301.01\\\\-252.5391\\\\234.4847\\\\301.01\\\\-254.8828\\\\236.4281\\\\301.01\\\\-255.4874\\\\237.0609\\\\301.01\\\\-256.2803\\\\239.4047\\\\301.01\\\\-255.8475\\\\241.7484\\\\301.01\\\\-255.5735\\\\244.0922\\\\301.01\\\\-255.6104\\\\246.4359\\\\301.01\\\\-255.4959\\\\248.7797\\\\301.01\\\\-254.8828\\\\250.669\\\\301.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848890601900001.533619501923\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"361\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"10\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n";
const char* k_rtStruct_json03 =
"                        \"Value\" : \"-258.4268\\\\239.4047\\\\304.01\\\\-257.2636\\\\244.0922\\\\304.01\\\\-257.1989\\\\246.4359\\\\304.01\\\\-256.6301\\\\248.7797\\\\304.01\\\\-256.215\\\\251.1234\\\\304.01\\\\-254.8828\\\\253.3128\\\\304.01\\\\-253.1956\\\\255.8109\\\\304.01\\\\-252.5391\\\\256.5626\\\\304.01\\\\-250.1953\\\\258.8727\\\\304.01\\\\-247.8516\\\\261.3969\\\\304.01\\\\-245.5078\\\\263.5693\\\\304.01\\\\-244.0789\\\\265.1859\\\\304.01\\\\-241.5988\\\\267.5297\\\\304.01\\\\-239.4249\\\\269.8734\\\\304.01\\\\-233.7891\\\\275.3867\\\\304.01\\\\-231.4453\\\\277.8311\\\\304.01\\\\-229.1016\\\\280.0036\\\\304.01\\\\-226.7578\\\\282.5281\\\\304.01\\\\-224.4141\\\\284.6849\\\\304.01\\\\-222.0703\\\\287.2221\\\\304.01\\\\-220.5406\\\\288.6234\\\\304.01\\\\-218.2914\\\\290.9672\\\\304.01\\\\-217.3828\\\\291.6599\\\\304.01\\\\-214.8704\\\\293.3109\\\\304.01\\\\-212.6953\\\\294.634\\\\304.01\\\\-210.3516\\\\295.3279\\\\304.01\\\\-208.0078\\\\296.4424\\\\304.01\\\\-205.6641\\\\296.8732\\\\304.01\\\\-203.3203\\\\297.1639\\\\304.01\\\\-196.2891\\\\297.1639\\\\304.01\\\\-189.2578\\\\297.0197\\\\304.01\\\\-184.5703\\\\297.0141\\\\304.01\\\\-177.5391\\\\296.8785\\\\304.01\\\\-161.1328\\\\296.851\\\\304.01\\\\-149.4141\\\\296.8598\\\\304.01\\\\-128.3203\\\\296.895\\\\304.01\\\\-118.9453\\\\296.8887\\\\304.01\\\\-114.2578\\\\296.8266\\\\304.01\\\\-107.2266\\\\296.817\\\\304.01\\\\-104.8828\\\\296.8459\\\\304.01\\\\-97.85156\\\\296.8456\\\\304.01\\\\-93.16406\\\\296.9116\\\\304.01\\\\-90.82031\\\\296.8828\\\\304.01\\\\-81.44531\\\\296.9305\\\\304.01\\\\-79.10156\\\\296.9116\\\\304.01\\\\-72.07031\\\\296.979\\\\304.01\\\\-69.72656\\\\296.96\\\\304.01\\\\-58.00781\\\\296.9557\\\\304.01\\\\-53.32031\\\\297.4055\\\\304.01\\\\-50.97656\\\\297.1979\\\\304.01\\\\-48.63281\\\\297.1858\\\\304.01\\\\-46.28906\\\\297.4229\\\\304.01\\\\-41.60156\\\\297.3806\\\\304.01\\\\-39.25781\\\\297.0362\\\\304.01\\\\-36.91406\\\\297.4125\\\\304.01\\\\-34.57031\\\\297.4229\\\\304.01\\\\-32.22656\\\\297.1747\\\\304.01\\\\-29.88281\\\\297.4334\\\\304.01\\\\-25.19531\\\\297.4125\\\\304.01\\\\-18.16406\\\\297.4334\\\\304.01\\\\-8.789063\\\\297.4195\\\\304.01\\\\-6.445313\\\\297.3383\\\\304.01\\\\-4.101563\\\\297.4125\\\\304.01\\\\0.5859375\\\\297.4195\\\\304.01\\\\2.929688\\\\297.0439\\\\304.01\\\\5.273438\\\\297.273\\\\304.01\\\\7.617188\\\\296.9841\\\\304.01\\\\12.30469\\\\296.8673\\\\304.01\\\\26.36719\\\\296.8574\\\\304.01\\\\35.74219\\\\296.8163\\\\304.01\\\\38.08594\\\\296.7654\\\\304.01\\\\45.11719\\\\296.7761\\\\304.01\\\\52.14844\\\\296.6929\\\\304.01\\\\54.49219\\\\296.7115\\\\304.01\\\\61.52344\\\\296.6724\\\\304.01\\\\63.86719\\\\296.6241\\\\304.01\\\\91.99219\\\\296.4947\\\\304.01\\\\94.33594\\\\296.5412\\\\304.01\\\\103.7109\\\\296.466\\\\304.01\\\\106.0547\\\\296.4801\\\\304.01\\\\115.4297\\\\296.391\\\\304.01\\\\117.7734\\\\296.4051\\\\304.01\\\\124.8047\\\\296.3288\\\\304.01\\\\131.8359\\\\296.2987\\\\304.01\\\\134.1797\\\\295.9161\\\\304.01\\\\136.5234\\\\295.8172\\\\304.01\\\\143.5547\\\\295.8791\\\\304.01\\\\148.2422\\\\295.8791\\\\304.01\\\\155.2734\\\\295.7849\\\\304.01\\\\159.9609\\\\295.7684\\\\304.01\\\\169.3359\\\\295.6454\\\\304.01\\\\171.6797\\\\295.6819\\\\304.01\\\\178.7109\\\\295.6454\\\\304.01\\\\181.0547\\\\295.5742\\\\304.01\\\\185.7422\\\\295.5916\\\\304.01\\\\192.7734\\\\295.557\\\\304.01\\\\197.4609\\\\295.8172\\\\304.01\\\\202.1484\\\\295.6819\\\\304.01\\\\204.4922\\\\295.3842\\\\304.01\\\\206.8359\\\\294.8185\\\\304.01\\\\209.1797\\\\294.3828\\\\304.01\\\\211.5234\\\\292.4841\\\\304.01\\\\213.8672\\\\291.1765\\\\304.01\\\\216.2109\\\\289.5873\\\\304.01\\\\217.229\\\\288.6234\\\\304.01\\\\218.5547\\\\287.0752\\\\304.01\\\\221.7204\\\\283.9359\\\\304.01\\\\225.5859\\\\279.9071\\\\304.01\\\\227.9297\\\\277.5303\\\\304.01\\\\230.2734\\\\275.0172\\\\304.01\\\\233.1467\\\\272.2172\\\\304.01\\\\234.9609\\\\270.242\\\\304.01\\\\237.7046\\\\267.5297\\\\304.01\\\\239.6484\\\\265.4887\\\\304.01\\\\244.3359\\\\260.7987\\\\304.01\\\\249.0234\\\\255.6807\\\\304.01\\\\251.3672\\\\252.8391\\\\304.01\\\\252.5586\\\\251.1234\\\\304.01\\\\254.0274\\\\248.7797\\\\304.01\\\\254.9669\\\\246.4359\\\\304.01\\\\255.696\\\\241.7484\\\\304.01\\\\255.9729\\\\239.4047\\\\304.01\\\\255.8408\\\\237.0609\\\\304.01\\\\254.6498\\\\234.7172\\\\304.01\\\\253.7109\\\\233.7838\\\\304.01\\\\251.3672\\\\232.8072\\\\304.01\\\\249.0234\\\\232.5688\\\\304.01\\\\246.6797\\\\232.4189\\\\304.01\\\\244.3359\\\\232.4365\\\\304.01\\\\241.9922\\\\232.8195\\\\304.01\\\\239.6484\\\\232.9699\\\\304.01\\\\234.9609\\\\232.9906\\\\304.01\\\\230.2734\\\\232.9594\\\\304.01\\\\225.5859\\\\233.0305\\\\304.01\\\\211.5234\\\\233.0632\\\\304.01\\\\206.8359\\\\232.9906\\\\304.01\\\\204.4922\\\\232.7048\\\\304.01\\\\199.8047\\\\232.6908\\\\304.01\\\\195.1172\\\\232.7456\\\\304.01\\\\190.4297\\\\232.7186\\\\304.01\\\\183.3984\\\\232.7588\\\\304.01\\\\174.0234\\\\232.7322\\\\304.01\\\\164.6484\\\\232.7322\\\\304.01\\\\157.6172\\\\232.7848\\\\304.01\\\\143.5547\\\\232.7746\\\\304.01\\\\138.8672\\\\232.6477\\\\304.01\\\\131.8359\\\\232.6623\\\\304.01\\\\127.1484\\\\232.6179\\\\304.01\\\\124.8047\\\\232.2817\\\\304.01\\\\122.4609\\\\231.7518\\\\304.01\\\\120.1172\\\\232.5557\\\\304.01\\\\113.0859\\\\232.5396\\\\304.01\\\\110.7422\\\\232.0028\\\\304.01\\\\108.3984\\\\231.6882\\\\304.01\\\\106.0547\\\\231.5762\\\\304.01\\\\103.7109\\\\231.6396\\\\304.01\\\\102.8085\\\\232.3734\\\\304.01\\\\101.3672\\\\233.0819\\\\304.01\\\\99.02344\\\\233.1221\\\\304.01\\\\91.99219\\\\233.091\\\\304.01\\\\84.96094\\\\233.131\\\\304.01\\\\80.27344\\\\233.1132\\\\304.01\\\\70.89844\\\\233.1483\\\\304.01\\\\59.17969\\\\233.1221\\\\304.01\\\\54.49219\\\\233.1483\\\\304.01\\\\47.46094\\\\233.1397\\\\304.01\\\\45.11719\\\\233.0951\\\\304.01\\\\44.15678\\\\232.3734\\\\304.01\\\\42.77344\\\\231.0402\\\\304.01\\\\42.0277\\\\230.0297\\\\304.01\\\\42.77344\\\\229.2236\\\\304.01\\\\45.11719\\\\228.5376\\\\304.01\\\\47.46094\\\\227.0172\\\\304.01\\\\49.80469\\\\226.0045\\\\304.01\\\\52.14844\\\\224.5497\\\\304.01\\\\54.49219\\\\223.6396\\\\304.01\\\\56.83594\\\\221.8073\\\\304.01\\\\59.17969\\\\220.1493\\\\304.01\\\\61.52344\\\\218.7845\\\\304.01\\\\64.88177\\\\215.9672\\\\304.01\\\\66.21094\\\\214.941\\\\304.01\\\\68.55469\\\\212.6457\\\\304.01\\\\70.89844\\\\210.4545\\\\304.01\\\\74.44025\\\\206.5922\\\\304.01\\\\76.45425\\\\204.2484\\\\304.01\\\\78.18552\\\\201.9047\\\\304.01\\\\80.27344\\\\198.8021\\\\304.01\\\\82.61719\\\\195.0944\\\\304.01\\\\82.81684\\\\194.8734\\\\304.01\\\\85.33871\\\\190.1859\\\\304.01\\\\86.25837\\\\187.8422\\\\304.01\\\\87.69531\\\\185.4984\\\\304.01\\\\88.53373\\\\183.1547\\\\304.01\\\\88.98369\\\\180.8109\\\\304.01\\\\90.01319\\\\178.4672\\\\304.01\\\\90.75238\\\\176.1234\\\\304.01\\\\91.26373\\\\173.7797\\\\304.01\\\\92.16087\\\\171.4359\\\\304.01\\\\92.84856\\\\169.0922\\\\304.01\\\\93.08162\\\\166.7484\\\\304.01\\\\93.46188\\\\159.7172\\\\304.01\\\\93.63705\\\\157.3734\\\\304.01\\\\93.71725\\\\155.0297\\\\304.01\\\\94.1173\\\\152.6859\\\\304.01\\\\95.2567\\\\150.3422\\\\304.01\\\\94.33594\\\\149.3719\\\\304.01\\\\93.55707\\\\147.9984\\\\304.01\\\\93.31775\\\\145.6547\\\\304.01\\\\93.0161\\\\140.9672\\\\304.01\\\\92.68435\\\\138.6234\\\\304.01\\\\91.79828\\\\136.2797\\\\304.01\\\\91.12964\\\\133.9359\\\\304.01\\\\90.57047\\\\131.5922\\\\304.01\\\\89.74536\\\\129.2484\\\\304.01\\\\88.82685\\\\126.9047\\\\304.01\\\\88.37492\\\\124.5609\\\\304.01\\\\87.09542\\\\122.2172\\\\304.01\\\\83.81286\\\\115.1859\\\\304.01\\\\82.08039\\\\112.8422\\\\304.01\\\\81.03055\\\\110.4984\\\\304.01\\\\77.92969\\\\106.5352\\\\304.01\\\\77.27783\\\\105.8109\\\\304.01\\\\75.79013\\\\103.4672\\\\304.01\\\\71.63571\\\\98.77969\\\\304.01\\\\68.55469\\\\95.73435\\\\304.01\\\\63.86719\\\\91.66604\\\\304.01\\\\61.52344\\\\90.20212\\\\304.01\\\\59.17969\\\\88.20197\\\\304.01\\\\56.83594\\\\86.62436\\\\304.01\\\\54.49219\\\\85.48727\\\\304.01\\\\52.14844\\\\83.70115\\\\304.01\\\\49.80469\\\\82.94526\\\\304.01\\\\47.46094\\\\81.53912\\\\304.01\\\\45.11719\\\\80.67555\\\\304.01\\\\42.77344\\\\79.20674\\\\304.01\\\\38.08594\\\\78.20923\\\\304.01\\\\35.74219\\\\77.17549\\\\304.01\\\\33.39844\\\\76.53049\\\\304.01\\\\31.05469\\\\76.11316\\\\304.01\\\\26.36719\\\\74.72801\\\\304.01\\\\24.02344\\\\74.428\\\\304.01\\\\16.99219\\\\74.14042\\\\304.01\\\\12.30469\\\\74.08618\\\\304.01\\\\7.617188\\\\74.18843\\\\304.01\\\\2.929688\\\\74.3889\\\\304.01\\\\0.5859375\\\\74.66957\\\\304.01\\\\-4.101563\\\\76.00713\\\\304.01\\\\-6.445313\\\\76.45148\\\\304.01\\\\-8.789063\\\\77.1\\\\304.01\\\\-11.13281\\\\78.06062\\\\304.01\\\\-15.82031\\\\79.2827\\\\304.01\\\\-18.16406\\\\80.74213\\\\304.01\\\\-20.50781\\\\81.62949\\\\304.01\\\\-22.85156\\\\82.96635\\\\304.01\\\\-25.19531\\\\83.75838\\\\304.01\\\\-27.53906\\\\85.57612\\\\304.01\\\\-29.88281\\\\86.67031\\\\304.01\\\\-32.22656\\\\88.3215\\\\304.01\\\\-34.57031\\\\90.27851\\\\304.01\\\\-36.65471\\\\91.74844\\\\304.01\\\\-39.25781\\\\93.84944\\\\304.01\\\\-41.60156\\\\95.82625\\\\304.01\\\\-46.73052\\\\101.1234\\\\304.01\\\\-50.25593\\\\105.8109\\\\304.01\\\\-52.17377\\\\108.1547\\\\304.01\\\\-53.74715\\\\110.4984\\\\304.01\\\\-55.05993\\\\112.8422\\\\304.01\\\\-56.70641\\\\115.1859\\\\304.01\\\\-57.55294\\\\117.5297\\\\304.01\\\\-59.00451\\\\119.8734\\\\304.01\\\\-59.93574\\\\122.2172\\\\304.01\\\\-61.16483\\\\124.5609\\\\304.01\\\\-61.67763\\\\126.9047\\\\304.01\\\\-62.54883\\\\129.2484\\\\304.01\\\\-63.58032\\\\131.5922\\\\304.01\\\\-64.08025\\\\133.9359\\\\304.01\\\\-64.47405\\\\136.2797\\\\304.01\\\\-65.29311\\\\138.6234\\\\304.01\\\\-65.704\\\\140.9672\\\\304.01\\\\-66.11191\\\\145.6547\\\\304.01\\\\-66.44185\\\\150.3422\\\\304.01\\\\-66.55173\\\\155.0297\\\\304.01\\\\-66.54406\\\\157.3734\\\\304.01\\\\-66.41875\\\\159.7172\\\\304.01\\\\-66.08267\\\\164.4047\\\\304.01\\\\-65.54951\\\\169.0922\\\\304.01\\\\-64.81466\\\\171.4359\\\\304.01\\\\-64.22777\\\\173.7797\\\\304.01\\\\-63.82658\\\\176.1234\\\\304.01\\\\-63.00951\\\\178.4672\\\\304.01\\\\-61.88151\\\\180.8109\\\\304.01\\\\-61.35514\\\\183.1547\\\\304.01\\\\-60.35156\\\\185.6658\\\\304.01\\\\-58.00781\\\\190.247\\\\304.01\\\\-55.71056\\\\194.8734\\\\304.01\\\\-54.20731\\\\197.2172\\\\304.01\\\\-52.52815\\\\199.5609\\\\304.01\\\\-51.09558\\\\201.9047\\\\304.01\\\\-48.63281\\\\204.9516\\\\304.01\\\\-47.4356\\\\206.5922\\\\304.01\\\\-45.0904\\\\208.9359\\\\304.01\\\\-42.90285\\\\211.2797\\\\304.01\\\\-39.25781\\\\214.8017\\\\304.01\\\\-34.92887\\\\218.3109\\\\304.01\\\\-34.57031\\\\218.6566\\\\304.01\\\\-32.22656\\\\220.0173\\\\304.01\\\\-31.40713\\\\220.6547\\\\304.01\\\\-28.07253\\\\222.9984\\\\304.01\\\\-27.53906\\\\223.4814\\\\304.01\\\\-25.19531\\\\224.4633\\\\304.01\\\\-22.85156\\\\225.8906\\\\304.01\\\\-20.50781\\\\226.9488\\\\304.01\\\\-18.16406\\\\228.4606\\\\304.01\\\\-15.82031\\\\229.1542\\\\304.01\\\\-14.96987\\\\230.0297\\\\304.01\\\\-15.82031\\\\231.3705\\\\304.01\\\\-16.94484\\\\232.3734\\\\304.01\\\\-18.16406\\\\233.0671\\\\304.01\\\\-25.19531\\\\233.091\\\\304.01\\\\-27.53906\\\\233.131\\\\304.01\\\\-43.94531\\\\233.1397\\\\304.01\\\\-46.28906\\\\233.1221\\\\304.01\\\\-55.66406\\\\233.19\\\\304.01\\\\-62.69531\\\\233.1652\\\\304.01\\\\-65.03906\\\\232.8767\\\\304.01\\\\-67.38281\\\\232.8735\\\\304.01\\\\-72.07031\\\\233.0044\\\\304.01\\\\-79.10156\\\\232.9735\\\\304.01\\\\-83.78906\\\\233.0441\\\\304.01\\\\-102.5391\\\\233.0951\\\\304.01\\\\-104.8828\\\\233.1525\\\\304.01\\\\-107.2266\\\\233.3238\\\\304.01\\\\-109.5703\\\\234.0489\\\\304.01\\\\-116.6016\\\\233.9967\\\\304.01\\\\-123.6328\\\\234.0805\\\\304.01\\\\-128.3203\\\\234.0855\\\\304.01\\\\-130.6641\\\\234.0088\\\\304.01\\\\-135.3516\\\\234.0722\\\\304.01\\\\-137.6953\\\\234.0591\\\\304.01\\\\-142.3828\\\\234.1266\\\\304.01\\\\-149.4141\\\\234.0722\\\\304.01\\\\-158.7891\\\\234.0772\\\\304.01\\\\-165.8203\\\\233.9901\\\\304.01\\\\-168.1641\\\\234.1182\\\\304.01\\\\-172.8516\\\\234.1837\\\\304.01\\\\-177.5391\\\\234.1837\\\\304.01\\\\-184.5703\\\\234.2574\\\\304.01\\\\-193.9453\\\\234.1916\\\\304.01\\\\-200.9766\\\\234.2224\\\\304.01\\\\-205.6641\\\\234.1652\\\\304.01\\\\-212.6953\\\\234.2362\\\\304.01\\\\-217.3828\\\\234.2502\\\\304.01\\\\-222.0703\\\\234.3418\\\\304.01\\\\-233.7891\\\\234.1026\\\\304.01\\\\-238.4766\\\\234.0555\\\\304.01\\\\-243.1641\\\\234.0626\\\\304.01\\\\-245.5078\\\\233.9906\\\\304.01\\\\-247.8516\\\\234.0236\\\\304.01\\\\-252.5391\\\\234.2887\\\\304.01\\\\-254.8828\\\\235.3755\\\\304.01\\\\-256.6911\\\\237.0609\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848911603100001.549938975401\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"361\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"11\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0859\\\\307.01\\\\-27.53906\\\\233.1221\\\\307.01\\\\-46.28906\\\\233.131\\\\307.01\\\\-58.00781\\\\233.19\\\\307.01\\\\-62.69531\\\\233.1736\\\\307.01\\\\-65.03906\\\\232.5396\\\\307.01\\\\-67.38281\\\\232.8735\\\\307.01\\\\-72.07031\\\\233.0145\\\\307.01\\\\-79.10156\\\\232.9839\\\\307.01\\\\-83.78906\\\\233.0441\\\\307.01\\\\-95.50781\\\\233.0859\\\\307.01\\\\-97.85156\\\\233.1345\\\\307.01\\\\-100.1953\\\\233.09\\\\307.01\\\\-102.5391\\\\233.2952\\\\307.01\\\\-104.8828\\\\234.0051\\\\307.01\\\\-107.2266\\\\233.8864\\\\307.01\\\\-109.5703\\\\234.0891\\\\307.01\\\\-116.6016\\\\233.9967\\\\307.01\\\\-123.6328\\\\234.0941\\\\307.01\\\\-128.3203\\\\234.1127\\\\307.01\\\\-130.6641\\\\234.0463\\\\307.01\\\\-133.0078\\\\234.099\\\\307.01\\\\-140.0391\\\\234.099\\\\307.01\\\\-142.3828\\\\234.1453\\\\307.01\\\\-149.4141\\\\234.099\\\\307.01\\\\-151.7578\\\\234.1313\\\\307.01\\\\-165.8203\\\\233.9901\\\\307.01\\\\-168.1641\\\\234.1182\\\\307.01\\\\-172.8516\\\\234.1973\\\\307.01\\\\-175.1953\\\\234.1934\\\\307.01\\\\-186.9141\\\\234.302\\\\307.01\\\\-189.2578\\\\234.2431\\\\307.01\\\\-191.6016\\\\234.2721\\\\307.01\\\\-198.6328\\\\234.2187\\\\307.01\\\\-200.9766\\\\234.2326\\\\307.01\\\\-205.6641\\\\234.1523\\\\307.01\\\\-208.0078\\\\234.2088\\\\307.01\\\\-217.3828\\\\234.2644\\\\307.01\\\\-222.0703\\\\234.3731\\\\307.01\\\\-226.7578\\\\234.3084\\\\307.01\\\\-233.7891\\\\234.1271\\\\307.01\\\\-243.1641\\\\234.1783\\\\307.01\\\\-247.8516\\\\233.9385\\\\307.01\\\\-252.5391\\\\234.1396\\\\307.01\\\\-254.8828\\\\234.6465\\\\307.01\\\\-257.2266\\\\235.5375\\\\307.01\\\\-259.5389\\\\237.0609\\\\307.01\\\\-261.1549\\\\239.4047\\\\307.01\\\\-261.1453\\\\241.7484\\\\307.01\\\\-260.4097\\\\244.0922\\\\307.01\\\\-258.4325\\\\248.7797\\\\307.01\\\\-257.2266\\\\250.7551\\\\307.01\\\\-255.4608\\\\253.4672\\\\307.01\\\\-252.5391\\\\256.6329\\\\307.01\\\\-250.1953\\\\258.8892\\\\307.01\\\\-247.8516\\\\261.4029\\\\307.01\\\\-245.5078\\\\263.58\\\\307.01\\\\-244.0815\\\\265.1859\\\\307.01\\\\-241.6044\\\\267.5297\\\\307.01\\\\-239.4249\\\\269.8734\\\\307.01\\\\-236.9699\\\\272.2172\\\\307.01\\\\-233.7891\\\\275.3877\\\\307.01\\\\-231.4453\\\\277.8249\\\\307.01\\\\-229.1016\\\\280.0036\\\\307.01\\\\-226.7578\\\\282.5186\\\\307.01\\\\-225.1687\\\\283.9359\\\\307.01\\\\-222.0703\\\\287.2188\\\\307.01\\\\-220.5469\\\\288.6234\\\\307.01\\\\-218.3018\\\\290.9672\\\\307.01\\\\-217.3828\\\\291.669\\\\307.01\\\\-212.6953\\\\294.6293\\\\307.01\\\\-210.3516\\\\295.3142\\\\307.01\\\\-208.0078\\\\296.4338\\\\307.01\\\\-205.6641\\\\296.8865\\\\307.01\\\\-203.3203\\\\297.1639\\\\307.01\\\\-196.2891\\\\297.1639\\\\307.01\\\\-191.6016\\\\297.0361\\\\307.01\\\\-186.9141\\\\297.0219\\\\307.01\\\\-179.8828\\\\296.9526\\\\307.01\\\\-175.1953\\\\296.8713\\\\307.01\\\\-161.1328\\\\296.8585\\\\307.01\\\\-158.7891\\\\296.8764\\\\307.01\\\\-149.4141\\\\296.8598\\\\307.01\\\\-147.0703\\\\296.8931\\\\307.01\\\\-140.0391\\\\296.8779\\\\307.01\\\\-128.3203\\\\296.9292\\\\307.01\\\\-118.9453\\\\296.9053\\\\307.01\\\\-114.2578\\\\296.845\\\\307.01\\\\-97.85156\\\\296.8459\\\\307.01\\\\-90.82031\\\\296.9116\\\\307.01\\\\-79.10156\\\\296.9116\\\\307.01\\\\-72.07031\\\\296.979\\\\307.01\\\\-69.72656\\\\296.9409\\\\307.01\\\\-65.03906\\\\296.9644\\\\307.01\\\\-58.00781\\\\296.9377\\\\307.01\\\\-55.66406\\\\297.3491\\\\307.01\\\\-53.32031\\\\297.1261\\\\307.01\\\\-50.97656\\\\297.3612\\\\307.01\\\\-48.63281\\\\296.9285\\\\307.01\\\\-46.28906\\\\297.4229\\\\307.01\\\\-41.60156\\\\297.416\\\\307.01\\\\-39.25781\\\\297.3425\\\\307.01\\\\-34.57031\\\\297.4229\\\\307.01\\\\-29.88281\\\\297.2875\\\\307.01\\\\-27.53906\\\\297.3947\\\\307.01\\\\-22.85156\\\\297.3077\\\\307.01\\\\-20.50781\\\\297.4125\\\\307.01\\\\-15.82031\\\\297.3768\\\\307.01\\\\-13.47656\\\\297.4125\\\\307.01\\\\-11.13281\\\\297.2579\\\\307.01\\\\-8.789063\\\\297.4229\\\\307.01\\\\-4.101563\\\\297.4055\\\\307.01\\\\0.5859375\\\\297.3148\\\\307.01\\\\2.929688\\\\296.9267\\\\307.01\\\\5.273438\\\\297.2621\\\\307.01\\\\7.617188\\\\297.257\\\\307.01\\\\9.960938\\\\296.9447\\\\307.01\\\\12.30469\\\\296.868\\\\307.01\\\\24.02344\\\\296.8266\\\\307.01\\\\26.36719\\\\296.878\\\\307.01\\\\33.39844\\\\296.7761\\\\307.01\\\\47.46094\\\\296.7546\\\\307.01\\\\52.14844\\\\296.6839\\\\307.01\\\\61.52344\\\\296.6635\\\\307.01\\\\68.55469\\\\296.6028\\\\307.01\\\\77.92969\\\\296.5859\\\\307.01\\\\80.27344\\\\296.5309\\\\307.01\\\\84.96094\\\\296.5362\\\\307.01\\\\91.99219\\\\296.4947\\\\307.01\\\\96.67969\\\\296.5208\\\\307.01\\\\108.3984\\\\296.4194\\\\307.01\\\\117.7734\\\\296.3986\\\\307.01\\\\120.1172\\\\296.3578\\\\307.01\\\\131.8359\\\\296.2929\\\\307.01\\\\134.1797\\\\295.8184\\\\307.01\\\\138.8672\\\\295.9218\\\\307.01\\\\141.2109\\\\296.2507\\\\307.01\\\\143.5547\\\\295.8486\\\\307.01\\\\150.5859\\\\295.8791\\\\307.01\\\\155.2734\\\\295.8012\\\\307.01\\\\159.9609\\\\295.7684\\\\307.01\\\\169.3359\\\\295.6454\\\\307.01\\\\171.6797\\\\295.7173\\\\307.01\\\\181.0547\\\\295.5742\\\\307.01\\\\183.3984\\\\295.6272\\\\307.01\\\\190.4297\\\\295.557\\\\307.01\\\\192.7734\\\\295.5742\\\\307.01\\\\197.4609\\\\295.8172\\\\307.01\\\\199.8047\\\\295.8012\\\\307.01\\\\202.1484\\\\295.6998\\\\307.01\\\\204.4922\\\\295.3989\\\\307.01\\\\206.8359\\\\294.8185\\\\307.01\\\\209.1797\\\\294.3828\\\\307.01\\\\211.5234\\\\292.4841\\\\307.01\\\\213.8672\\\\291.1765\\\\307.01\\\\216.2109\\\\289.6034\\\\307.01\\\\217.2454\\\\288.6234\\\\307.01\\\\218.5547\\\\287.0864\\\\307.01\\\\221.7204\\\\283.9359\\\\307.01\\\\225.5859\\\\279.8821\\\\307.01\\\\228.5326\\\\276.9047\\\\307.01\\\\230.2734\\\\275.0424\\\\307.01\\\\233.137\\\\272.2172\\\\307.01\\\\234.9609\\\\270.245\\\\307.01\\\\237.6922\\\\267.5297\\\\307.01\\\\246.856\\\\258.1547\\\\307.01\\\\251.3768\\\\253.4672\\\\307.01\\\\253.166\\\\251.1234\\\\307.01\\\\255.1239\\\\248.7797\\\\307.01\\\\256.5218\\\\246.4359\\\\307.01\\\\257.6041\\\\244.0922\\\\307.01\\\\258.817\\\\241.7484\\\\307.01\\\\259.2578\\\\239.4047\\\\307.01\\\\258.9535\\\\237.0609\\\\307.01\\\\256.0547\\\\234.1196\\\\307.01\\\\253.7109\\\\233.131\\\\307.01\\\\251.3672\\\\232.5372\\\\307.01\\\\244.3359\\\\232.7139\\\\307.01\\\\241.9922\\\\232.9015\\\\307.01\\\\239.6484\\\\232.9906\\\\307.01\\\\234.9609\\\\232.9803\\\\307.01\\\\225.5859\\\\233.0305\\\\307.01\\\\211.5234\\\\233.0632\\\\307.01\\\\206.8359\\\\233.0007\\\\307.01\\\\204.4922\\\\232.7186\\\\307.01\\\\199.8047\\\\232.6767\\\\307.01\\\\195.1172\\\\232.7588\\\\307.01\\\\190.4297\\\\232.7186\\\\307.01\\\\183.3984\\\\232.7588\\\\307.01\\\\169.3359\\\\232.7456\\\\307.01\\\\166.9922\\\\232.7186\\\\307.01\\\\157.6172\\\\232.7848\\\\307.01\\\\152.9297\\\\232.7588\\\\307.01\\\\143.5547\\\\232.7875\\\\307.01\\\\138.8672\\\\232.6477\\\\307.01\\\\131.8359\\\\232.6179\\\\307.01\\\\127.1484\\\\232.4192\\\\307.01\\\\124.8047\\\\231.7722\\\\307.01\\\\122.4609\\\\231.7224\\\\307.01\\\\120.1172\\\\231.7625\\\\307.01\\\\117.7734\\\\232.2817\\\\307.01\\\\115.4297\\\\232.5232\\\\307.01\\\\113.0859\\\\232.4546\\\\307.01\\\\110.7422\\\\232.4888\\\\307.01\\\\108.3984\\\\231.7722\\\\307.01\\\\106.0547\\\\231.6215\\\\307.01\\\\103.7109\\\\231.648\\\\307.01\\\\102.8453\\\\232.3734\\\\307.01\\\\101.3672\\\\233.1042\\\\307.01\\\\94.33594\\\\233.0951\\\\307.01\\\\89.64844\\\\233.1221\\\\307.01\\\\77.92969\\\\233.1221\\\\307.01\\\\59.17969\\\\233.1483\\\\307.01\\\\47.46094\\\\233.131\\\\307.01\\\\45.11719\\\\233.0951\\\\307.01\\\\44.1495\\\\232.3734\\\\307.01\\\\42.77344\\\\231.0667\\\\307.01\\\\42.01079\\\\230.0297\\\\307.01\\\\42.77344\\\\229.206\\\\307.01\\\\45.11719\\\\228.5128\\\\307.01\\\\47.46094\\\\226.9947\\\\307.01\\\\49.80469\\\\225.986\\\\307.01\\\\52.14844\\\\224.5319\\\\307.01\\\\54.49219\\\\223.5919\\\\307.01\\\\55.18826\\\\222.9984\\\\307.01\\\\59.17969\\\\220.1266\\\\307.01\\\\61.52344\\\\218.7716\\\\307.01\\\\64.83179\\\\215.9672\\\\307.01\\\\66.21094\\\\214.9167\\\\307.01\\\\70.89844\\\\210.4059\\\\307.01\\\\74.42065\\\\206.5922\\\\307.01\\\\76.41272\\\\204.2484\\\\307.01\\\\78.09221\\\\201.9047\\\\307.01\\\\80.27344\\\\198.7387\\\\307.01\\\\82.73348\\\\194.8734\\\\307.01\\\\84.0608\\\\192.5297\\\\307.01\\\\85.28558\\\\190.1859\\\\307.01\\\\86.21609\\\\187.8422\\\\307.01\\\\87.62429\\\\185.4984\\\\307.01\\\\88.50529\\\\183.1547\\\\307.01\\\\88.94127\\\\180.8109\\\\307.01\\\\89.95934\\\\178.4672\\\\307.01\\\\90.70085\\\\176.1234\\\\307.01\\\\91.22121\\\\173.7797\\\\307.01\\\\92.07523\\\\171.4359\\\\307.01\\\\92.80196\\\\169.0922\\\\307.01\\\\93.06984\\\\166.7484\\\\307.01\\\\93.43011\\\\159.7172\\\\307.01\\\\93.78402\\\\155.0297\\\\307.01\\\\95.35853\\\\152.6859\\\\307.01\\\\97.21561\\\\150.3422\\\\307.01\\\\96.67969\\\\149.7189\\\\307.01\\\\94.33594\\\\148.4892\\\\307.01\\\\93.77909\\\\147.9984\\\\307.01\\\\93.3065\\\\145.6547\\\\307.01\\\\92.98377\\\\140.9672\\\\307.01\\\\92.62429\\\\138.6234\\\\307.01\\\\91.70932\\\\136.2797\\\\307.01\\\\90.52432\\\\131.5922\\\\307.01\\\\88.79446\\\\126.9047\\\\307.01\\\\88.35513\\\\124.5609\\\\307.01\\\\86.99378\\\\122.2172\\\\307.01\\\\85.96972\\\\119.8734\\\\307.01\\\\84.78261\\\\117.5297\\\\307.01\\\\83.77131\\\\115.1859\\\\307.01\\\\82.02393\\\\112.8422\\\\307.01\\\\80.98469\\\\110.4984\\\\307.01\\\\77.92969\\\\106.5841\\\\307.01\\\\77.24248\\\\105.8109\\\\307.01\\\\75.73918\\\\103.4672\\\\307.01\\\\71.58269\\\\98.77969\\\\307.01\\\\68.55469\\\\95.79888\\\\307.01\\\\63.86719\\\\91.72031\\\\307.01\\\\61.52344\\\\90.225\\\\307.01\\\\59.17969\\\\88.22054\\\\307.01\\\\56.83594\\\\86.64962\\\\307.01\\\\54.49219\\\\85.5138\\\\307.01\\\\52.14844\\\\83.72078\\\\307.01\\\\49.80469\\\\82.97671\\\\307.01\\\\47.46094\\\\81.57265\\\\307.01\\\\45.11719\\\\80.68514\\\\307.01\\\\42.77344\\\\79.22858\\\\307.01\\\\38.08594\\\\78.2206\\\\307.01\\\\35.74219\\\\77.19827\\\\307.01\\\\33.39844\\\\76.55239\\\\307.01\\\\31.05469\\\\76.12139\\\\307.01\\\\26.36719\\\\74.72801\\\\307.01\\\\24.02344\\\\74.43276\\\\307.01\\\\16.99219\\\\74.13425\\\\307.01\\\\14.64844\\\\74.0982\\\\307.01\\\\7.617188\\\\74.18246\\\\307.01\\\\2.929688\\\\74.39614\\\\307.01\\\\0.5859375\\\\74.65002\\\\307.01\\\\-4.101563\\\\75.99764\\\\307.01\\\\-6.445313\\\\76.44152\\\\307.01\\\\-8.789063\\\\77.05959\\\\307.01\\\\-11.13281\\\\78.04713\\\\307.01\\\\-15.82031\\\\79.25529\\\\307.01\\\\-18.16406\\\\80.70029\\\\307.01\\\\-20.50781\\\\81.59003\\\\307.01\\\\-22.85156\\\\82.96635\\\\307.01\\\\-25.19531\\\\83.75323\\\\307.01\\\\-27.53906\\\\85.54559\\\\307.01\\\\-29.88281\\\\86.62055\\\\307.01\\\\-32.22656\\\\88.30287\\\\307.01\\\\-34.57031\\\\90.24761\\\\307.01\\\\-36.72696\\\\91.74844\\\\307.01\\\\-39.25781\\\\93.79108\\\\307.01\\\\-41.60156\\\\95.80312\\\\307.01\\\\-46.76263\\\\101.1234\\\\307.01\\\\-50.32931\\\\105.8109\\\\307.01\\\\-52.19967\\\\108.1547\\\\307.01\\\\-53.78753\\\\110.4984\\\\307.01\\\\-55.12727\\\\112.8422\\\\307.01\\\\-56.73218\\\\115.1859\\\\307.01\\\\-57.60399\\\\117.5297\\\\307.01\\\\-59.0554\\\\119.8734\\\\307.01\\\\-59.9868\\\\122.2172\\\\307.01\\\\-61.19757\\\\124.5609\\\\307.01\\\\-61.69523\\\\126.9047\\\\307.01\\\\-63.64114\\\\131.5922\\\\307.01\\\\-64.53993\\\\136.2797\\\\307.01\\\\-65.34997\\\\138.6234\\\\307.01\\\\-65.7409\\\\140.9672\\\\307.01\\\\-66.1507\\\\145.6547\\\\307.01\\\\-66.472\\\\150.3422\\\\307.01\\\\-66.58739\\\\155.0297\\\\307.01\\\\-66.57152\\\\157.3734\\\\307.01\\\\-66.46494\\\\159.7172\\\\307.01\\\\-66.11516\\\\164.4047\\\\307.01\\\\-65.59342\\\\169.0922\\\\307.01\\\\-64.28482\\\\173.7797\\\\307.01\\\\-63.89509\\\\176.1234\\\\307.01\\\\-63.1595\\\\178.4672\\\\307.01\\\\-61.99355\\\\180.8109\\\\307.01\\\\-61.42098\\\\183.1547\\\\307.01\\\\-60.5483\\\\185.4984\\\\307.01\\\\-60.35156\\\\185.7601\\\\307.01\\\\-56.91528\\\\192.5297\\\\307.01\\\\-55.83274\\\\194.8734\\\\307.01\\\\-54.25148\\\\197.2172\\\\307.01\\\\-52.56362\\\\199.5609\\\\307.01\\\\-51.16442\\\\201.9047\\\\307.01\\\\-48.63281\\\\205.0156\\\\307.01\\\\-47.47334\\\\206.5922\\\\307.01\\\\-45.13073\\\\208.9359\\\\307.01\\\\-42.93779\\\\211.2797\\\\307.01\\\\-39.25781\\\\214.8144\\\\307.01\\\\-34.97234\\\\218.3109\\\\307.01\\\\-34.57031\\\\218.6988\\\\307.01\\\\-32.22656\\\\220.0478\\\\307.01\\\\-28.16808\\\\222.9984\\\\307.01\\\\-27.53906\\\\223.5539\\\\307.01\\\\-25.19531\\\\224.4929\\\\307.01\\\\-22.85156\\\\225.9097\\\\307.01\\\\-20.50781\\\\226.9751\\\\307.01\\\\-18.16406\\\\228.4825\\\\307.01\\\\-15.82031\\\\229.1508\\\\307.01\\\\-14.96138\\\\230.0297\\\\307.01\\\\-15.82031\\\\231.3794\\\\307.01\\\\-16.92257\\\\232.3734\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848933604400001.474954755728\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"376\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"12\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.0951\\\\310.01\\\\-29.88281\\\\233.1132\\\\310.01\\\\-32.22656\\\\233.1483\\\\310.01\\\\-36.91406\\\\233.1221\\\\310.01\\\\-46.28906\\\\233.1397\\\\310.01\\\\-50.97656\\\\233.1736\\\\310.01\\\\-60.35156\\\\233.19\\\\310.01\\\\-62.69531\\\\233.1652\\\\310.01\\\\-65.03906\\\\232.5396\\\\310.01\\\\-67.38281\\\\232.8852\\\\310.01\\\\-72.07031\\\\232.9942\\\\310.01\\\\-86.13281\\\\233.0183\\\\310.01\\\\-88.47656\\\\233.0671\\\\310.01\\\\-95.50781\\\\233.0859\\\\310.01\\\\-97.85156\\\\233.3123\\\\310.01\\\\-102.5391\\\\233.9389\\\\310.01\\\\-104.8828\\\\234.062\\\\310.01\\\\-107.2266\\\\233.5222\\\\310.01\\\\-109.5703\\\\234.0754\\\\310.01\\\\-116.6016\\\\234.0158\\\\310.01\\\\-133.0078\\\\234.1313\\\\310.01\\\\-137.6953\\\\234.1127\\\\310.01\\\\-142.3828\\\\234.1741\\\\310.01\\\\-147.0703\\\\234.0855\\\\310.01\\\\-151.7578\\\\234.1127\\\\310.01\\\\-165.8203\\\\234.0141\\\\310.01\\\\-168.1641\\\\234.1313\\\\310.01\\\\-182.2266\\\\234.2252\\\\310.01\\\\-186.9141\\\\234.2721\\\\310.01\\\\-189.2578\\\\234.2252\\\\310.01\\\\-198.6328\\\\234.2187\\\\310.01\\\\-200.9766\\\\234.261\\\\310.01\\\\-205.6641\\\\234.1564\\\\310.01\\\\-208.0078\\\\234.2088\\\\310.01\\\\-215.0391\\\\234.2678\\\\310.01\\\\-222.0703\\\\234.3601\\\\310.01\\\\-226.7578\\\\234.3296\\\\310.01\\\\-233.7891\\\\234.1478\\\\310.01\\\\-238.4766\\\\234.1954\\\\310.01\\\\-243.1641\\\\234.1652\\\\310.01\\\\-245.5078\\\\233.9895\\\\310.01\\\\-247.8516\\\\233.9042\\\\310.01\\\\-252.5391\\\\234.0772\\\\310.01\\\\-254.8828\\\\234.2605\\\\310.01\\\\-257.2266\\\\234.9111\\\\310.01\\\\-259.5703\\\\236.1157\\\\310.01\\\\-260.7422\\\\237.0609\\\\310.01\\\\-262.3813\\\\239.4047\\\\310.01\\\\-262.5138\\\\241.7484\\\\310.01\\\\-261.5641\\\\244.0922\\\\310.01\\\\-260.7806\\\\246.4359\\\\310.01\\\\-259.5703\\\\248.5863\\\\310.01\\\\-257.6038\\\\251.1234\\\\310.01\\\\-255.672\\\\253.4672\\\\310.01\\\\-252.5391\\\\256.6564\\\\310.01\\\\-250.1953\\\\258.9219\\\\310.01\\\\-247.8516\\\\261.4344\\\\310.01\\\\-245.5078\\\\263.6147\\\\310.01\\\\-244.1124\\\\265.1859\\\\310.01\\\\-241.6101\\\\267.5297\\\\310.01\\\\-239.4375\\\\269.8734\\\\310.01\\\\-236.9858\\\\272.2172\\\\310.01\\\\-233.7891\\\\275.3877\\\\310.01\\\\-231.4453\\\\277.8215\\\\310.01\\\\-229.1016\\\\280.0036\\\\310.01\\\\-226.7578\\\\282.5409\\\\310.01\\\\-225.1803\\\\283.9359\\\\310.01\\\\-222.0703\\\\287.2221\\\\310.01\\\\-220.5288\\\\288.6234\\\\310.01\\\\-218.3186\\\\290.9672\\\\310.01\\\\-217.3828\\\\291.678\\\\310.01\\\\-212.6953\\\\294.6293\\\\310.01\\\\-210.3516\\\\295.3008\\\\310.01\\\\-208.0078\\\\296.4508\\\\310.01\\\\-205.6641\\\\296.8858\\\\310.01\\\\-203.3203\\\\297.1639\\\\310.01\\\\-196.2891\\\\297.1446\\\\310.01\\\\-191.6016\\\\297.0175\\\\310.01\\\\-179.8828\\\\296.9378\\\\310.01\\\\-177.5391\\\\296.8855\\\\310.01\\\\-165.8203\\\\296.8345\\\\310.01\\\\-158.7891\\\\296.8672\\\\310.01\\\\-154.1016\\\\296.851\\\\310.01\\\\-135.3516\\\\296.8786\\\\310.01\\\\-128.3203\\\\296.9134\\\\310.01\\\\-121.2891\\\\296.8971\\\\310.01\\\\-111.9141\\\\296.8359\\\\310.01\\\\-97.85156\\\\296.8453\\\\310.01\\\\-88.47656\\\\296.9028\\\\310.01\\\\-86.13281\\\\296.8927\\\\310.01\\\\-69.72656\\\\296.9622\\\\310.01\\\\-65.03906\\\\296.934\\\\310.01\\\\-58.00781\\\\296.9358\\\\310.01\\\\-55.66406\\\\297.3383\\\\310.01\\\\-53.32031\\\\296.9926\\\\310.01\\\\-50.97656\\\\297.0405\\\\310.01\\\\-48.63281\\\\296.9267\\\\310.01\\\\-46.28906\\\\297.409\\\\310.01\\\\-43.94531\\\\297.3148\\\\310.01\\\\-41.60156\\\\297.373\\\\310.01\\\\-39.25781\\\\297.0516\\\\310.01\\\\-36.91406\\\\297.1798\\\\310.01\\\\-34.57031\\\\297.1975\\\\310.01\\\\-32.22656\\\\297.3947\\\\310.01\\\\-29.88281\\\\297.2767\\\\310.01\\\\-27.53906\\\\297.2779\\\\310.01\\\\-25.19531\\\\297.3806\\\\310.01\\\\-22.85156\\\\297.106\\\\310.01\\\\-20.50781\\\\297.4055\\\\310.01\\\\-18.16406\\\\297.1577\\\\310.01\\\\-15.82031\\\\297.2089\\\\310.01\\\\-13.47656\\\\297.4229\\\\310.01\\\\-11.13281\\\\297.2307\\\\310.01\\\\-8.789063\\\\297.1626\\\\310.01\\\\-6.445313\\\\297.4229\\\\310.01\\\\-4.101563\\\\297.3546\\\\310.01\\\\-1.757813\\\\297.0439\\\\310.01\\\\0.5859375\\\\297.0849\\\\310.01\\\\2.929688\\\\296.8985\\\\310.01\\\\5.273438\\\\297.2462\\\\310.01\\\\7.617188\\\\296.8985\\\\310.01\\\\9.960938\\\\297.1152\\\\310.01\\\\12.30469\\\\296.8477\\\\310.01\\\\19.33594\\\\296.8579\\\\310.01\\\\24.02344\\\\296.8161\\\\310.01\\\\26.36719\\\\296.8473\\\\310.01\\\\33.39844\\\\296.7643\\\\310.01\\\\42.77344\\\\296.7533\\\\310.01\\\\45.11719\\\\296.7115\\\\310.01\\\\59.17969\\\\296.6814\\\\310.01\\\\68.55469\\\\296.5984\\\\310.01\\\\75.58594\\\\296.6107\\\\310.01\\\\80.27344\\\\296.5362\\\\310.01\\\\84.96094\\\\296.5616\\\\310.01\\\\87.30469\\\\296.5098\\\\310.01\\\\99.02344\\\\296.5005\\\\310.01\\\\101.3672\\\\296.466\\\\310.01\\\\110.7422\\\\296.4189\\\\310.01\\\\113.0859\\\\296.3782\\\\310.01\\\\117.7734\\\\296.3976\\\\310.01\\\\120.1172\\\\296.3578\\\\310.01\\\\127.1484\\\\296.3288\\\\310.01\\\\129.4922\\\\296.2873\\\\310.01\\\\131.8359\\\\296.3348\\\\310.01\\\\134.1797\\\\295.9141\\\\310.01\\\\136.5234\\\\296.1612\\\\310.01\\\\138.8672\\\\296.033\\\\310.01\\\\141.2109\\\\296.2967\\\\310.01\\\\143.5547\\\\295.9477\\\\310.01\\\\145.8984\\\\295.864\\\\310.01\\\\150.5859\\\\295.894\\\\310.01\\\\152.9297\\\\295.833\\\\310.01\\\\162.3047\\\\295.7516\\\\310.01\\\\166.9922\\\\295.6638\\\\310.01\\\\171.6797\\\\295.7173\\\\310.01\\\\181.0547\\\\295.6093\\\\310.01\\\\185.7422\\\\295.6272\\\\310.01\\\\188.0859\\\\295.557\\\\310.01\\\\192.7734\\\\295.5742\\\\310.01\\\\197.4609\\\\295.8012\\\\310.01\\\\199.8047\\\\295.8012\\\\310.01\\\\202.1484\\\\295.6819\\\\310.01\\\\204.4922\\\\295.4287\\\\310.01\\\\206.8359\\\\294.8307\\\\310.01\\\\209.1797\\\\294.3828\\\\310.01\\\\211.5234\\\\292.4764\\\\310.01\\\\213.8672\\\\291.1765\\\\310.01\\\\216.2109\\\\289.625\\\\310.01\\\\217.2743\\\\288.6234\\\\310.01\\\\218.5547\\\\287.0983\\\\310.01\\\\221.7204\\\\283.9359\\\\310.01\\\\225.5859\\\\279.8821\\\\310.01\\\\227.9297\\\\277.5431\\\\310.01\\\\230.2734\\\\275.0606\\\\310.01\\\\233.113\\\\272.2172\\\\310.01\\\\235.3208\\\\269.8734\\\\310.01\\\\237.7111\\\\267.5297\\\\310.01\\\\239.9438\\\\265.1859\\\\310.01\\\\246.8767\\\\258.1547\\\\310.01\\\\249.1396\\\\255.8109\\\\310.01\\\\251.4974\\\\253.4672\\\\310.01\\\\255.4942\\\\248.7797\\\\310.01\\\\257.2692\\\\246.4359\\\\310.01\\\\259.2847\\\\244.0922\\\\310.01\\\\260.0775\\\\241.7484\\\\310.01\\\\260.576\\\\239.4047\\\\310.01\\\\260.1205\\\\237.0609\\\\310.01\\\\258.8931\\\\234.7172\\\\310.01\\\\258.3984\\\\234.2411\\\\310.01\\\\256.0547\\\\233.35\\\\310.01\\\\253.7109\\\\232.9379\\\\310.01\\\\251.3672\\\\232.7162\\\\310.01\\\\249.0234\\\\232.8224\\\\310.01\\\\244.3359\\\\232.8437\\\\310.01\\\\239.6484\\\\233.0108\\\\310.01\\\\237.3047\\\\233.0007\\\\310.01\\\\232.6172\\\\233.0726\\\\310.01\\\\227.9297\\\\233.0305\\\\310.01\\\\211.5234\\\\233.0726\\\\310.01\\\\206.8359\\\\233.0245\\\\310.01\\\\204.4922\\\\232.7456\\\\310.01\\\\202.1484\\\\232.6886\\\\310.01\\\\195.1172\\\\232.7848\\\\310.01\\\\190.4297\\\\232.7186\\\\310.01\\\\176.3672\\\\232.7719\\\\310.01\\\\166.9922\\\\232.7456\\\\310.01\\\\159.9609\\\\232.7975\\\\310.01\\\\150.5859\\\\232.7746\\\\310.01\\\\145.8984\\\\232.8003\\\\310.01\\\\138.8672\\\\232.6623\\\\310.01\\\\134.1797\\\\232.6623\\\\310.01\\\\131.8359\\\\232.4897\\\\310.01\\\\129.4922\\\\231.8945\\\\310.01\\\\124.8047\\\\231.6823\\\\310.01\\\\120.1172\\\\231.737\\\\310.01\\\\117.7734\\\\232.3211\\\\310.01\\\\115.4297\\\\232.6329\\\\310.01\\\\110.7422\\\\232.6623\\\\310.01\\\\108.3984\\\\231.9109\\\\310.01\\\\106.0547\\\\231.602\\\\310.01\\\\103.7109\\\\231.6193\\\\310.01\\\\102.8551\\\\232.3734\\\\310.01\\\\101.3672\\\\233.1132\\\\310.01\\\\94.33594\\\\233.0951\\\\310.01\\\\87.30469\\\\233.1397\\\\310.01\\\\82.61719\\\\233.1132\\\\310.01\\\\75.58594\\\\233.1483\\\\310.01\\\\70.89844\\\\233.1221\\\\310.01\\\\59.17969\\\\233.1568\\\\310.01\\\\52.14844\\\\233.1221\\\\310.01\\\\47.46094\\\\233.131\\\\310.01\\\\45.11719\\\\233.0859\\\\310.01\\\\44.16726\\\\232.3734\\\\310.01\\\\42.77344\\\\231.0498\\\\310.01\\\\42.0192\\\\230.0297\\\\310.01\\\\42.77344\\\\229.2101\\\\310.01\\\\45.11719\\\\228.4825\\\\310.01\\\\47.46094\\\\226.9683\\\\310.01\\\\49.80469\\\\225.9571\\\\310.01\\\\52.14844\\\\224.5099\\\\310.01\\\\54.49219\\\\223.5692\\\\310.01\\\\55.14323\\\\222.9984\\\\310.01\\\\59.17969\\\\220.1045\\\\310.01\\\\61.52344\\\\218.7182\\\\310.01\\\\64.78396\\\\215.9672\\\\310.01\\\\66.21094\\\\214.8859\\\\310.01\\\\70.89844\\\\210.3839\\\\310.01\\\\74.36692\\\\206.5922\\\\310.01\\\\76.38396\\\\204.2484\\\\310.01\\\\81.22672\\\\197.2172\\\\310.01\\\\84.03577\\\\192.5297\\\\310.01\\\\85.25792\\\\190.1859\\\\310.01\\\\86.1923\\\\187.8422\\\\310.01\\\\87.47085\\\\185.4984\\\\310.01\\\\88.48811\\\\183.1547\\\\310.01\\\\88.90029\\\\180.8109\\\\310.01\\\\89.90248\\\\178.4672\\\\310.01\\\\90.64713\\\\176.1234\\\\310.01\\\\91.18042\\\\173.7797\\\\310.01\\\\92.74409\\\\169.0922\\\\310.01\\\\93.04629\\\\166.7484\\\\310.01\\\\93.38223\\\\159.7172\\\\310.01\\\\93.66732\\\\155.0297\\\\310.01\\\\94.19643\\\\152.6859\\\\310.01\\\\95.64145\\\\150.3422\\\\310.01\\\\94.33594\\\\149.1322\\\\310.01\\\\93.50089\\\\147.9984\\\\310.01\\\\93.25756\\\\145.6547\\\\310.01\\\\92.96472\\\\140.9672\\\\310.01\\\\92.55265\\\\138.6234\\\\310.01\\\\91.62743\\\\136.2797\\\\310.01\\\\90.46875\\\\131.5922\\\\310.01\\\\88.75411\\\\126.9047\\\\310.01\\\\88.33083\\\\124.5609\\\\310.01\\\\86.91406\\\\122.2172\\\\310.01\\\\85.93851\\\\119.8734\\\\310.01\\\\84.69238\\\\117.5297\\\\310.01\\\\83.74721\\\\115.1859\\\\310.01\\\\81.96295\\\\112.8422\\\\310.01\\\\80.91831\\\\110.4984\\\\310.01\\\\79.14664\\\\108.1547\\\\310.01\\\\77.20256\\\\105.8109\\\\310.01\\\\75.65157\\\\103.4672\\\\310.01\\\\71.53253\\\\98.77969\\\\310.01\\\\68.55469\\\\95.82625\\\\310.01\\\\66.21094\\\\93.7748\\\\310.01\\\\63.79395\\\\91.74844\\\\310.01\\\\61.52344\\\\90.26997\\\\310.01\\\\59.17969\\\\88.23898\\\\310.01\\\\56.83594\\\\86.67812\\\\310.01\\\\54.49219\\\\85.52697\\\\310.01\\\\52.14844\\\\83.74062\\\\310.01\\\\49.80469\\\\82.99707\\\\310.01\\\\47.46094\\\\81.59437\\\\310.01\\\\45.11719\\\\80.70401\\\\310.01\\\\42.77344\\\\79.24621\\\\310.01\\\\38.08594\\\\78.23525\\\\310.01\\\\35.74219\\\\77.20986\\\\310.01\\\\33.39844\\\\76.56908\\\\310.01\\\\31.05469\\\\76.1376\\\\310.01\\\\28.71094\\\\75.50471\\\\310.01\\\\26.36719\\\\74.72801\\\\310.01\\\\24.02344\\\\74.43276\\\\310.01\\\\16.99219\\\\74.15237\\\\310.01\\\\12.30469\\\\74.10455\\\\310.01\\\\7.617188\\\\74.19435\\\\310.01\\\\2.929688\\\\74.40105\\\\310.01\\\\0.5859375\\\\74.6557\\\\310.01\\\\-4.101563\\\\75.97835\\\\310.01\\\\-6.445313\\\\76.43703\\\\310.01\\\\-8.789063\\\\77.04008\\\\310.01\\\\-11.13281\\\\78.03346\\\\310.01\\\\-15.82031\\\\79.22787\\\\310.01\\\\-18.16406\\\\80.65422\\\\310.01\\\\-20.50781\\\\81.57233\\\\310.01\\\\-22.85156\\\\82.94526\\\\310.01\\\\-25.19531\\\\83.73331\\\\310.01\\\\-27.53906\\\\85.52371\\\\310.01\\\\-29.88281\\\\86.58486\\\\310.01\\\\-32.22656\\\\88.25871\\\\310.01\\\\-34.57031\\\\90.22985\\\\310.01\\\\-36.76382\\\\91.74844\\\\310.01\\\\-39.25781\\\\93.74467\\\\310.01\\\\-41.60156\\\\95.77628\\\\310.01\\\\-46.83174\\\\101.1234\\\\310.01\\\\-50.36664\\\\105.8109\\\\310.01\\\\-52.22528\\\\108.1547\\\\310.01\\\\-53.85835\\\\110.4984\\\\310.01\\\\-55.16944\\\\112.8422\\\\310.01\\\\-56.75659\\\\115.1859\\\\310.01\\\\-57.66267\\\\117.5297\\\\310.01\\\\-59.07907\\\\119.8734\\\\310.01\\\\-60.05458\\\\122.2172\\\\310.01\\\\-61.23355\\\\124.5609\\\\310.01\\\\-61.71977\\\\126.9047\\\\310.01\\\\-63.70555\\\\131.5922\\\\310.01\\\\-64.61088\\\\136.2797\\\\310.01\\\\-65.42969\\\\138.6234\\\\310.01\\\\-65.75874\\\\140.9672\\\\310.01\\\\-66.18872\\\\145.6547\\\\310.01\\\\-66.50977\\\\150.3422\\\\310.01\\\\-66.63708\\\\155.0297\\\\310.01\\\\-66.4952\\\\159.7172\\\\310.01\\\\-66.1377\\\\164.4047\\\\310.01\\\\-65.63528\\\\169.0922\\\\310.01\\\\-65.06631\\\\171.4359\\\\310.01\\\\-64.31938\\\\173.7797\\\\310.01\\\\-63.93199\\\\176.1234\\\\310.01\\\\-63.24552\\\\178.4672\\\\310.01\\\\-62.07078\\\\180.8109\\\\310.01\\\\-61.44824\\\\183.1547\\\\310.01\\\\-60.69909\\\\185.4984\\\\310.01\\\\-59.32978\\\\187.8422\\\\310.01\\\\-58.20172\\\\190.1859\\\\310.01\\\\-56.94638\\\\192.5297\\\\310.01\\\\-55.91395\\\\194.8734\\\\310.01\\\\-52.60492\\\\199.5609\\\\310.01\\\\-51.24368\\\\201.9047\\\\310.01\\\\-48.63281\\\\205.082\\\\310.01\\\\-47.51002\\\\206.5922\\\\310.01\\\\-45.16488\\\\208.9359\\\\310.01\\\\-42.98322\\\\211.2797\\\\310.01\\\\-39.25781\\\\214.8471\\\\310.01\\\\-34.57031\\\\218.7393\\\\310.01\\\\-32.22656\\\\220.0723\\\\310.01\\\\-31.49524\\\\220.6547\\\\310.01\\\\-28.1901\\\\222.9984\\\\310.01\\\\-27.53906\\\\223.5692\\\\310.01\\\\-25.19531\\\\224.5146\\\\310.01\\\\-22.85156\\\\225.9463\\\\310.01\\\\-20.50781\\\\226.9932\\\\310.01\\\\-18.16406\\\\228.5043\\\\310.01\\\\-15.82031\\\\229.1644\\\\310.01\\\\-14.9747\\\\230.0297\\\\310.01\\\\-15.82031\\\\231.3705\\\\310.01\\\\-16.91176\\\\232.3734\\\\310.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848957605700001.539833993627\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"369\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"13\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"45.11719\\\\233.1042\\\\313.01\\\\44.14645\\\\232.3734\\\\313.01\\\\42.77344\\\\231.0593\\\\313.01\\\\42.01079\\\\230.0297\\\\313.01\\\\42.77344\\\\229.2013\\\\313.01\\\\45.11719\\\\228.4738\\\\313.01\\\\47.46094\\\\226.9815\\\\313.01\\\\49.80469\\\\225.902\\\\313.01\\\\52.14844\\\\224.4881\\\\313.01\\\\54.49219\\\\223.5423\\\\313.01\\\\55.12733\\\\222.9984\\\\313.01\\\\59.17969\\\\220.0829\\\\313.01\\\\61.52344\\\\218.6617\\\\313.01\\\\64.74609\\\\215.9672\\\\313.01\\\\66.21094\\\\214.8439\\\\313.01\\\\70.89844\\\\210.3514\\\\313.01\\\\72.25302\\\\208.9359\\\\313.01\\\\76.36958\\\\204.2484\\\\313.01\\\\77.92969\\\\201.9154\\\\313.01\\\\80.27344\\\\198.618\\\\313.01\\\\81.20244\\\\197.2172\\\\313.01\\\\82.5531\\\\194.8734\\\\313.01\\\\84.01786\\\\192.5297\\\\313.01\\\\85.20026\\\\190.1859\\\\313.01\\\\86.15661\\\\187.8422\\\\313.01\\\\87.38582\\\\185.4984\\\\313.01\\\\88.4649\\\\183.1547\\\\313.01\\\\88.8737\\\\180.8109\\\\313.01\\\\89.82677\\\\178.4672\\\\313.01\\\\90.60566\\\\176.1234\\\\313.01\\\\91.14483\\\\173.7797\\\\313.01\\\\91.8785\\\\171.4359\\\\313.01\\\\92.70221\\\\169.0922\\\\313.01\\\\93.03451\\\\166.7484\\\\313.01\\\\93.47005\\\\157.3734\\\\313.01\\\\93.61861\\\\152.6859\\\\313.01\\\\93.64014\\\\150.3422\\\\313.01\\\\93.34677\\\\147.9984\\\\313.01\\\\92.94547\\\\140.9672\\\\313.01\\\\92.48362\\\\138.6234\\\\313.01\\\\91.57636\\\\136.2797\\\\313.01\\\\90.42757\\\\131.5922\\\\313.01\\\\89.48472\\\\129.2484\\\\313.01\\\\88.73901\\\\126.9047\\\\313.01\\\\88.29956\\\\124.5609\\\\313.01\\\\86.86148\\\\122.2172\\\\313.01\\\\85.9046\\\\119.8734\\\\313.01\\\\84.65003\\\\117.5297\\\\313.01\\\\83.69191\\\\115.1859\\\\313.01\\\\81.91267\\\\112.8422\\\\313.01\\\\80.87713\\\\110.4984\\\\313.01\\\\79.10793\\\\108.1547\\\\313.01\\\\77.14613\\\\105.8109\\\\313.01\\\\75.58594\\\\103.498\\\\313.01\\\\73.56179\\\\101.1234\\\\313.01\\\\71.46389\\\\98.77969\\\\313.01\\\\68.55469\\\\95.89069\\\\313.01\\\\66.21094\\\\93.83636\\\\313.01\\\\63.71433\\\\91.74844\\\\313.01\\\\61.52344\\\\90.29206\\\\313.01\\\\59.17969\\\\88.26962\\\\313.01\\\\56.83594\\\\86.74574\\\\313.01\\\\54.49219\\\\85.57699\\\\313.01\\\\52.14844\\\\83.78355\\\\313.01\\\\49.80469\\\\83.01695\\\\313.01\\\\47.46094\\\\81.62056\\\\313.01\\\\45.11719\\\\80.73152\\\\313.01\\\\42.77344\\\\79.27779\\\\313.01\\\\38.08594\\\\78.27902\\\\313.01\\\\35.74219\\\\77.23351\\\\313.01\\\\33.39844\\\\76.57487\\\\313.01\\\\31.05469\\\\76.15349\\\\313.01\\\\28.71094\\\\75.56659\\\\313.01\\\\26.36719\\\\74.72801\\\\313.01\\\\24.02344\\\\74.44027\\\\313.01\\\\14.64844\\\\74.0982\\\\313.01\\\\7.617188\\\\74.19435\\\\313.01\\\\2.929688\\\\74.39614\\\\313.01\\\\0.5859375\\\\74.6557\\\\313.01\\\\-1.757813\\\\75.2781\\\\313.01\\\\-4.101563\\\\75.98805\\\\313.01\\\\-6.445313\\\\76.43114\\\\313.01\\\\-8.789063\\\\77.04008\\\\313.01\\\\-11.13281\\\\78.03108\\\\313.01\\\\-13.47656\\\\78.68081\\\\313.01\\\\-15.82031\\\\79.16585\\\\313.01\\\\-18.16406\\\\80.60848\\\\313.01\\\\-20.50781\\\\81.54504\\\\313.01\\\\-22.85156\\\\82.93452\\\\313.01\\\\-25.19531\\\\83.69951\\\\313.01\\\\-27.53906\\\\85.48667\\\\313.01\\\\-29.88281\\\\86.58486\\\\313.01\\\\-32.22656\\\\88.2459\\\\313.01\\\\-34.57031\\\\90.18356\\\\313.01\\\\-36.84147\\\\91.74844\\\\313.01\\\\-39.25781\\\\93.7097\\\\313.01\\\\-41.60156\\\\95.73894\\\\313.01\\\\-46.90146\\\\101.1234\\\\313.01\\\\-48.62366\\\\103.4672\\\\313.01\\\\-52.24294\\\\108.1547\\\\313.01\\\\-53.89533\\\\110.4984\\\\313.01\\\\-55.24722\\\\112.8422\\\\313.01\\\\-56.79365\\\\115.1859\\\\313.01\\\\-57.7046\\\\117.5297\\\\313.01\\\\-59.10275\\\\119.8734\\\\313.01\\\\-60.1423\\\\122.2172\\\\313.01\\\\-61.27319\\\\124.5609\\\\313.01\\\\-61.73931\\\\126.9047\\\\313.01\\\\-62.82649\\\\129.2484\\\\313.01\\\\-63.74772\\\\131.5922\\\\313.01\\\\-64.64844\\\\136.2797\\\\313.01\\\\-65.49149\\\\138.6234\\\\313.01\\\\-66.03151\\\\143.3109\\\\313.01\\\\-66.55173\\\\150.3422\\\\313.01\\\\-66.7085\\\\155.0297\\\\313.01\\\\-66.5332\\\\159.7172\\\\313.01\\\\-66.37591\\\\162.0609\\\\313.01\\\\-65.93555\\\\166.7484\\\\313.01\\\\-65.66541\\\\169.0922\\\\313.01\\\\-65.13599\\\\171.4359\\\\313.01\\\\-64.35547\\\\173.7797\\\\313.01\\\\-63.97324\\\\176.1234\\\\313.01\\\\-63.31245\\\\178.4672\\\\313.01\\\\-62.13423\\\\180.8109\\\\313.01\\\\-60.7784\\\\185.4984\\\\313.01\\\\-59.36625\\\\187.8422\\\\313.01\\\\-58.29068\\\\190.1859\\\\313.01\\\\-56.97235\\\\192.5297\\\\313.01\\\\-55.9882\\\\194.8734\\\\313.01\\\\-55.66406\\\\195.2305\\\\313.01\\\\-53.32031\\\\198.6979\\\\313.01\\\\-52.65765\\\\199.5609\\\\313.01\\\\-51.33394\\\\201.9047\\\\313.01\\\\-48.63281\\\\205.1581\\\\313.01\\\\-47.54197\\\\206.5922\\\\313.01\\\\-45.18493\\\\208.9359\\\\313.01\\\\-43.00636\\\\211.2797\\\\313.01\\\\-39.25781\\\\214.8799\\\\313.01\\\\-34.57031\\\\218.8154\\\\313.01\\\\-32.22656\\\\220.1045\\\\313.01\\\\-31.54811\\\\220.6547\\\\313.01\\\\-28.24903\\\\222.9984\\\\313.01\\\\-27.53906\\\\223.618\\\\313.01\\\\-25.19531\\\\224.5319\\\\313.01\\\\-22.85156\\\\226.0067\\\\313.01\\\\-20.50781\\\\227.0116\\\\313.01\\\\-18.16406\\\\228.5043\\\\313.01\\\\-15.82031\\\\229.173\\\\313.01\\\\-14.98326\\\\230.0297\\\\313.01\\\\-15.82031\\\\231.3494\\\\313.01\\\\-16.91331\\\\232.3734\\\\313.01\\\\-18.16406\\\\233.1042\\\\313.01\\\\-22.85156\\\\233.131\\\\313.01\\\\-29.88281\\\\233.1132\\\\313.01\\\\-36.91406\\\\233.1483\\\\313.01\\\\-46.28906\\\\233.1397\\\\313.01\\\\-50.97656\\\\233.1818\\\\313.01\\\\-62.69531\\\\233.1652\\\\313.01\\\\-65.03906\\\\232.5396\\\\313.01\\\\-67.38281\\\\232.8735\\\\313.01\\\\-72.07031\\\\232.9942\\\\313.01\\\\-81.44531\\\\233.0343\\\\313.01\\\\-86.13281\\\\233.0283\\\\313.01\\\\-95.50781\\\\233.0951\\\\313.01\\\\-97.85156\\\\233.3584\\\\313.01\\\\-100.1953\\\\233.9981\\\\313.01\\\\-102.5391\\\\234.0754\\\\313.01\\\\-107.2266\\\\233.9926\\\\313.01\\\\-109.5703\\\\234.0754\\\\313.01\\\\-114.2578\\\\234.0284\\\\313.01\\\\-123.6328\\\\234.0805\\\\313.01\\\\-130.6641\\\\234.0855\\\\313.01\\\\-135.3516\\\\234.1407\\\\313.01\\\\-137.6953\\\\234.1127\\\\313.01\\\\-142.3828\\\\234.1889\\\\313.01\\\\-147.0703\\\\234.1038\\\\313.01\\\\-154.1016\\\\234.0513\\\\313.01\\\\-158.7891\\\\234.0691\\\\313.01\\\\-165.8203\\\\234.0211\\\\313.01\\\\-168.1641\\\\234.1579\\\\313.01\\\\-172.8516\\\\234.2175\\\\313.01\\\\-175.1953\\\\234.1894\\\\313.01\\\\-184.5703\\\\234.2686\\\\313.01\\\\-189.2578\\\\234.2252\\\\313.01\\\\-193.9453\\\\234.2289\\\\313.01\\\\-200.9766\\\\234.2902\\\\313.01\\\\-205.6641\\\\234.1783\\\\313.01\\\\-210.3516\\\\234.2822\\\\313.01\\\\-215.0391\\\\234.3418\\\\313.01\\\\-222.0703\\\\234.3731\\\\313.01\\\\-226.7578\\\\234.3601\\\\313.01\\\\-233.7891\\\\234.1731\\\\313.01\\\\-238.4766\\\\234.2088\\\\313.01\\\\-243.1641\\\\234.1523\\\\313.01\\\\-247.8516\\\\233.8669\\\\313.01\\\\-250.1953\\\\233.9436\\\\313.01\\\\-254.8828\\\\234.2296\\\\313.01\\\\-257.2266\\\\234.63\\\\313.01\\\\-259.5703\\\\235.982\\\\313.01\\\\-260.9235\\\\237.0609\\\\313.01\\\\-262.7961\\\\239.4047\\\\313.01\\\\-262.9268\\\\241.7484\\\\313.01\\\\-262.2656\\\\244.0922\\\\313.01\\\\-261.032\\\\246.4359\\\\313.01\\\\-259.6734\\\\248.7797\\\\313.01\\\\-257.5789\\\\251.1234\\\\313.01\\\\-255.7048\\\\253.4672\\\\313.01\\\\-252.5391\\\\256.6491\\\\313.01\\\\-250.1953\\\\258.9388\\\\313.01\\\\-247.8516\\\\261.453\\\\313.01\\\\-245.5078\\\\263.6436\\\\313.01\\\\-244.1278\\\\265.1859\\\\313.01\\\\-241.655\\\\267.5297\\\\313.01\\\\-239.4557\\\\269.8734\\\\313.01\\\\-233.7891\\\\275.4082\\\\313.01\\\\-231.4453\\\\277.8406\\\\313.01\\\\-229.1016\\\\280.0036\\\\313.01\\\\-226.7578\\\\282.5503\\\\313.01\\\\-225.1923\\\\283.9359\\\\313.01\\\\-222.0703\\\\287.2287\\\\313.01\\\\-220.5585\\\\288.6234\\\\313.01\\\\-218.3552\\\\290.9672\\\\313.01\\\\-217.3828\\\\291.7043\\\\313.01\\\\-214.921\\\\293.3109\\\\313.01\\\\-212.6953\\\\294.6401\\\\313.01\\\\-210.3516\\\\295.3416\\\\313.01\\\\-208.0078\\\\296.4592\\\\313.01\\\\-205.6641\\\\296.8858\\\\313.01\\\\-203.3203\\\\297.1639\\\\313.01\\\\-196.2891\\\\297.1254\\\\313.01\\\\-193.9453\\\\297.0099\\\\313.01\\\\-177.5391\\\\296.8925\\\\313.01\\\\-165.8203\\\\296.8423\\\\313.01\\\\-154.1016\\\\296.851\\\\313.01\\\\-147.0703\\\\296.8843\\\\313.01\\\\-144.7266\\\\296.8517\\\\313.01\\\\-137.6953\\\\296.894\\\\313.01\\\\-125.9766\\\\296.9214\\\\313.01\\\\-116.6016\\\\296.8896\\\\313.01\\\\-111.9141\\\\296.8447\\\\313.01\\\\-95.50781\\\\296.8738\\\\313.01\\\\-93.16406\\\\296.9016\\\\313.01\\\\-83.78906\\\\296.9028\\\\313.01\\\\-74.41406\\\\296.9514\\\\313.01\\\\-65.03906\\\\296.9644\\\\313.01\\\\-60.35156\\\\296.916\\\\313.01\\\\-58.00781\\\\296.9377\\\\313.01\\\\-55.66406\\\\297.3491\\\\313.01\\\\-53.32031\\\\296.9377\\\\313.01\\\\-50.97656\\\\296.9285\\\\313.01\\\\-48.63281\\\\297.1917\\\\313.01\\\\-46.28906\\\\297.1807\\\\313.01\\\\-43.94531\\\\297.2621\\\\313.01\\\\-41.60156\\\\297.2199\\\\313.01\\\\-39.25781\\\\297.3984\\\\313.01\\\\-36.91406\\\\297.402\\\\313.01\\\\-34.57031\\\\297.0362\\\\313.01\\\\-32.22656\\\\297.4125\\\\313.01\\\\-29.88281\\\\297.4125\\\\313.01\\\\-27.53906\\\\297.2416\\\\313.01\\\\-25.19531\\\\297.4125\\\\313.01\\\\-22.85156\\\\297.3879\\\\313.01\\\\-20.50781\\\\297.0121\\\\313.01\\\\-18.16406\\\\297.3299\\\\313.01\\\\-15.82031\\\\297.1454\\\\313.01\\\\-11.13281\\\\297.2468\\\\313.01\\\\-8.789063\\\\296.9557\\\\313.01\\\\-6.445313\\\\297.4125\\\\313.01\\\\-4.101563\\\\296.9731\\\\313.01\\\\2.929688\\\\296.9191\\\\313.01\\\\5.273438\\\\297.0809\\\\313.01\\\\7.617188\\\\296.8877\\\\313.01\\\\12.30469\\\\296.8055\\\\313.01\\\\16.99219\\\\296.837\\\\313.01\\\\21.67969\\\\296.8058\\\\313.01\\\\31.05469\\\\296.8159\\\\313.01\\\\35.74219\\\\296.7436\\\\313.01\\\\40.42969\\\\296.7743\\\\313.01\\\\47.46094\\\\296.734\\\\313.01\\\\52.14844\\\\296.6724\\\\313.01\\\\61.52344\\\\296.6635\\\\313.01\\\\66.21094\\\\296.5942\\\\313.01\\\\73.24219\\\\296.5901\\\\313.01\\\\80.27344\\\\296.5284\\\\313.01\\\\82.61719\\\\296.5569\\\\313.01\\\\87.30469\\\\296.4947\\\\313.01\\\\91.99219\\\\296.5134\\\\313.01\\\\96.67969\\\\296.473\\\\313.01\\\\115.4297\\\\296.3986\\\\313.01\\\\134.1797\\\\296.2873\\\\313.01\\\\136.5234\\\\295.9046\\\\313.01\\\\138.8672\\\\295.9102\\\\313.01\\\\141.2109\\\\296.0271\\\\313.01\\\\143.5547\\\\295.8654\\\\313.01\\\\148.2422\\\\295.864\\\\313.01\\\\159.9609\\\\295.7849\\\\313.01\\\\164.6484\\\\295.6819\\\\313.01\\\\169.3359\\\\295.6998\\\\313.01\\\\178.7109\\\\295.6638\\\\313.01\\\\188.0859\\\\295.5742\\\\313.01\\\\192.7734\\\\295.5916\\\\313.01\\\\197.4609\\\\295.8172\\\\313.01\\\\199.8047\\\\295.8012\\\\313.01\\\\202.1484\\\\295.6998\\\\313.01\\\\204.4922\\\\295.4287\\\\313.01\\\\206.8359\\\\294.8307\\\\313.01\\\\209.1797\\\\294.3768\\\\313.01\\\\211.5234\\\\292.4688\\\\313.01\\\\213.8672\\\\291.1765\\\\313.01\\\\216.2109\\\\289.627\\\\313.01\\\\217.2822\\\\288.6234\\\\313.01\\\\219.371\\\\286.2797\\\\313.01\\\\221.7773\\\\283.9359\\\\313.01\\\\225.5859\\\\279.9482\\\\313.01\\\\228.6426\\\\276.9047\\\\313.01\\\\230.2734\\\\275.138\\\\313.01\\\\233.135\\\\272.2172\\\\313.01\\\\235.3423\\\\269.8734\\\\313.01\\\\237.73\\\\267.5297\\\\313.01\\\\239.6484\\\\265.4964\\\\313.01\\\\246.9323\\\\258.1547\\\\313.01\\\\251.4581\\\\253.4672\\\\313.01\\\\256.0547\\\\248.1981\\\\313.01\\\\257.454\\\\246.4359\\\\313.01\\\\259.6265\\\\244.0922\\\\313.01\\\\261.2413\\\\239.4047\\\\313.01\\\\260.4736\\\\237.0609\\\\313.01\\\\259.4369\\\\234.7172\\\\313.01\\\\258.3984\\\\233.723\\\\313.01\\\\256.0547\\\\233.1051\\\\313.01\\\\253.7109\\\\232.7947\\\\313.01\\\\249.0234\\\\232.8704\\\\313.01\\\\244.3359\\\\232.8072\\\\313.01\\\\239.6484\\\\233.0207\\\\313.01\\\\237.3047\\\\233.0007\\\\313.01\\\\232.6172\\\\233.0726\\\\313.01\\\\227.9297\\\\233.0305\\\\313.01\\\\216.2109\\\\233.0632\\\\313.01\\\\206.8359\\\\233.0207\\\\313.01\\\\204.4922\\\\232.7456\\\\313.01\\\\202.1484\\\\232.6745\\\\313.01\\\\197.4609\\\\232.7322\\\\313.01\\\\192.7734\\\\232.7456\\\\313.01\\\\188.0859\\\\232.7048\\\\313.01\\\\178.7109\\\\232.7588\\\\313.01\\\\174.0234\\\\232.7322\\\\313.01\\\\166.9922\\\\232.7456\\\\313.01\\\\162.3047\\\\232.7848\\\\313.01\\\\157.6172\\\\232.7746\\\\313.01\\\\148.2422\\\\232.8003\\\\313.01\\\\143.5547\\\\232.7746\\\\313.01\\\\141.2109\\\\232.6908\\\\313.01\\\\134.1797\\\\232.6329\\\\313.01\\\\131.8359\\\\232.0401\\\\313.01\\\\129.4922\\\\231.6858\\\\313.01\\\\127.1484\\\\231.6664\\\\313.01\\\\120.1172\\\\231.7875\\\\313.01\\\\117.7734\\\\232.6329\\\\313.01\\\\110.7422\\\\232.6623\\\\313.01\\\\108.3984\\\\231.8903\\\\313.01\\\\106.0547\\\\231.6022\\\\313.01\\\\103.7109\\\\231.5567\\\\313.01\\\\102.832\\\\232.3734\\\\313.01\\\\101.3672\\\\233.1221\\\\313.01\\\\94.33594\\\\233.1042\\\\313.01\\\\87.30469\\\\233.1397\\\\313.01\\\\82.61719\\\\233.1132\\\\313.01\\\\77.92969\\\\233.1397\\\\313.01\\\\59.17969\\\\233.1568\\\\313.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848978606900001.507522632828\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"374\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.1132\\\\316.01\\\\-34.57031\\\\233.1132\\\\316.01\\\\-39.25781\\\\233.1568\\\\316.01\\\\-41.60156\\\\233.131\\\\316.01\\\\-50.97656\\\\233.1818\\\\316.01\\\\-55.66406\\\\233.1652\\\\316.01\\\\-60.35156\\\\233.2393\\\\316.01\\\\-62.69531\\\\233.1818\\\\316.01\\\\-65.03906\\\\232.7048\\\\316.01\\\\-67.38281\\\\232.8967\\\\316.01\\\\-72.07031\\\\233.0145\\\\316.01\\\\-81.44531\\\\233.0479\\\\316.01\\\\-86.13281\\\\233.0382\\\\316.01\\\\-95.50781\\\\233.0951\\\\316.01\\\\-97.85156\\\\233.3265\\\\316.01\\\\-100.1953\\\\233.9981\\\\316.01\\\\-102.5391\\\\234.0805\\\\316.01\\\\-107.2266\\\\234.0231\\\\316.01\\\\-111.9141\\\\234.0891\\\\316.01\\\\-114.2578\\\\234.0411\\\\316.01\\\\-121.2891\\\\234.054\\\\316.01\\\\-123.6328\\\\234.1127\\\\316.01\\\\-130.6641\\\\234.0672\\\\316.01\\\\-133.0078\\\\234.1127\\\\316.01\\\\-142.3828\\\\234.1741\\\\316.01\\\\-147.0703\\\\234.1174\\\\316.01\\\\-151.7578\\\\234.1127\\\\316.01\\\\-161.1328\\\\234.0263\\\\316.01\\\\-165.8203\\\\234.0387\\\\316.01\\\\-168.1641\\\\234.1894\\\\316.01\\\\-170.5078\\\\234.2503\\\\316.01\\\\-175.1953\\\\234.2073\\\\316.01\\\\-186.9141\\\\234.2539\\\\316.01\\\\-189.2578\\\\234.2112\\\\316.01\\\\-193.9453\\\\234.2289\\\\316.01\\\\-200.9766\\\\234.3204\\\\316.01\\\\-205.6641\\\\234.1916\\\\316.01\\\\-212.6953\\\\234.3573\\\\316.01\\\\-222.0703\\\\234.4053\\\\316.01\\\\-226.7578\\\\234.3731\\\\316.01\\\\-233.7891\\\\234.2125\\\\316.01\\\\-238.4766\\\\234.226\\\\316.01\\\\-243.1641\\\\234.1068\\\\316.01\\\\-247.8516\\\\233.8141\\\\316.01\\\\-250.1953\\\\233.8749\\\\316.01\\\\-254.8828\\\\234.2268\\\\316.01\\\\-257.2266\\\\234.5565\\\\316.01\\\\-259.5703\\\\235.9516\\\\316.01\\\\-260.9418\\\\237.0609\\\\316.01\\\\-262.8647\\\\239.4047\\\\316.01\\\\-263.0318\\\\241.7484\\\\316.01\\\\-262.4791\\\\244.0922\\\\316.01\\\\-261.0516\\\\246.4359\\\\316.01\\\\-259.5703\\\\248.5972\\\\316.01\\\\-257.2266\\\\251.3685\\\\316.01\\\\-255.6122\\\\253.4672\\\\316.01\\\\-252.5391\\\\256.6377\\\\316.01\\\\-250.1953\\\\258.9164\\\\316.01\\\\-247.8516\\\\261.4313\\\\316.01\\\\-245.5078\\\\263.6035\\\\316.01\\\\-244.1\\\\265.1859\\\\316.01\\\\-241.649\\\\267.5297\\\\316.01\\\\-239.4557\\\\269.8734\\\\316.01\\\\-233.7891\\\\275.4139\\\\316.01\\\\-231.4453\\\\277.8628\\\\316.01\\\\-229.1016\\\\280.0036\\\\316.01\\\\-226.7578\\\\282.5439\\\\316.01\\\\-225.1864\\\\283.9359\\\\316.01\\\\-222.0703\\\\287.2355\\\\316.01\\\\-220.5764\\\\288.6234\\\\316.01\\\\-218.3692\\\\290.9672\\\\316.01\\\\-214.9922\\\\293.3109\\\\316.01\\\\-212.6953\\\\294.6526\\\\316.01\\\\-210.3516\\\\295.3416\\\\316.01\\\\-208.0078\\\\296.4756\\\\316.01\\\\-205.6641\\\\296.892\\\\316.01\\\\-203.3203\\\\297.1563\\\\316.01\\\\-196.2891\\\\297.1297\\\\316.01\\\\-193.9453\\\\297.0023\\\\316.01\\\\-184.5703\\\\296.9636\\\\316.01\\\\-177.5391\\\\296.8925\\\\316.01\\\\-172.8516\\\\296.9032\\\\316.01\\\\-168.1641\\\\296.8568\\\\316.01\\\\-163.4766\\\\296.8895\\\\316.01\\\\-156.4453\\\\296.8506\\\\316.01\\\\-147.0703\\\\296.8921\\\\316.01\\\\-144.7266\\\\296.8517\\\\316.01\\\\-125.9766\\\\296.9402\\\\316.01\\\\-121.2891\\\\296.8887\\\\316.01\\\\-118.9453\\\\296.9077\\\\316.01\\\\-111.9141\\\\296.8723\\\\316.01\\\\-102.5391\\\\296.9016\\\\316.01\\\\-95.50781\\\\296.8837\\\\316.01\\\\-88.47656\\\\296.9322\\\\316.01\\\\-83.78906\\\\296.9234\\\\316.01\\\\-67.38281\\\\296.9841\\\\316.01\\\\-65.03906\\\\297.1698\\\\316.01\\\\-62.69531\\\\296.9377\\\\316.01\\\\-58.00781\\\\296.9377\\\\316.01\\\\-55.66406\\\\297.2922\\\\316.01\\\\-53.32031\\\\297.2468\\\\316.01\\\\-50.97656\\\\296.9396\\\\316.01\\\\-48.63281\\\\297.1798\\\\316.01\\\\-46.28906\\\\296.9579\\\\316.01\\\\-43.94531\\\\297.1687\\\\316.01\\\\-41.60156\\\\296.9285\\\\316.01\\\\-39.25781\\\\297.2089\\\\316.01\\\\-36.91406\\\\296.9303\\\\316.01\\\\-34.57031\\\\296.9396\\\\316.01\\\\-32.22656\\\\297.1373\\\\316.01\\\\-29.88281\\\\297.4229\\\\316.01\\\\-27.53906\\\\296.9396\\\\316.01\\\\-25.19531\\\\296.9954\\\\316.01\\\\-22.85156\\\\297.3383\\\\316.01\\\\-20.50781\\\\296.9358\\\\316.01\\\\-18.16406\\\\296.9467\\\\316.01\\\\-15.82031\\\\297.1454\\\\316.01\\\\-13.47656\\\\297.0061\\\\316.01\\\\-11.13281\\\\297.2089\\\\316.01\\\\-8.789063\\\\296.9067\\\\316.01\\\\-6.445313\\\\297.1687\\\\316.01\\\\-4.101563\\\\296.9358\\\\316.01\\\\-1.757813\\\\297.2875\\\\316.01\\\\0.5859375\\\\296.9081\\\\316.01\\\\7.617188\\\\296.8985\\\\316.01\\\\12.30469\\\\296.8161\\\\316.01\\\\14.64844\\\\296.8372\\\\316.01\\\\19.33594\\\\296.7743\\\\316.01\\\\21.67969\\\\296.8055\\\\316.01\\\\31.05469\\\\296.7843\\\\316.01\\\\35.74219\\\\296.7247\\\\316.01\\\\40.42969\\\\296.7743\\\\316.01\\\\42.77344\\\\296.7154\\\\316.01\\\\45.11719\\\\296.7546\\\\316.01\\\\49.80469\\\\296.6839\\\\316.01\\\\63.86719\\\\296.6346\\\\316.01\\\\66.21094\\\\296.5862\\\\316.01\\\\75.58594\\\\296.5696\\\\316.01\\\\77.92969\\\\296.5208\\\\316.01\\\\82.61719\\\\296.549\\\\316.01\\\\89.64844\\\\296.4947\\\\316.01\\\\94.33594\\\\296.5005\\\\316.01\\\\96.67969\\\\296.4597\\\\316.01\\\\108.3984\\\\296.4393\\\\316.01\\\\113.0859\\\\296.3846\\\\316.01\\\\120.1172\\\\296.3782\\\\316.01\\\\127.1484\\\\296.308\\\\316.01\\\\134.1797\\\\296.2929\\\\316.01\\\\136.5234\\\\295.9009\\\\316.01\\\\138.8672\\\\295.9433\\\\316.01\\\\148.2422\\\\295.833\\\\316.01\\\\150.5859\\\\295.864\\\\316.01\\\\152.9297\\\\295.8012\\\\316.01\\\\157.6172\\\\295.8012\\\\316.01\\\\166.9922\\\\295.6638\\\\316.01\\\\174.0234\\\\295.6998\\\\316.01\\\\183.3984\\\\295.6093\\\\316.01\\\\192.7734\\\\295.6093\\\\316.01\\\\197.4609\\\\295.8012\\\\316.01\\\\202.1484\\\\295.6998\\\\316.01\\\\204.4922\\\\295.4137\\\\316.01\\\\206.8359\\\\294.8185\\\\316.01\\\\209.1797\\\\294.3768\\\\316.01\\\\211.5234\\\\292.4764\\\\316.01\\\\213.8672\\\\291.1916\\\\316.01\\\\216.2109\\\\289.6328\\\\316.01\\\\217.2822\\\\288.6234\\\\316.01\\\\219.3814\\\\286.2797\\\\316.01\\\\221.8029\\\\283.9359\\\\316.01\\\\225.5859\\\\279.9599\\\\316.01\\\\227.9297\\\\277.6382\\\\316.01\\\\230.2734\\\\275.1772\\\\316.01\\\\233.2298\\\\272.2172\\\\316.01\\\\234.9609\\\\270.3593\\\\316.01\\\\239.6484\\\\265.57\\\\316.01\\\\246.9653\\\\258.1547\\\\316.01\\\\251.4385\\\\253.4672\\\\316.01\\\\256.0547\\\\248.2196\\\\316.01\\\\257.4986\\\\246.4359\\\\316.01\\\\259.7266\\\\244.0922\\\\316.01\\\\260.7422\\\\241.4555\\\\316.01\\\\261.4258\\\\239.4047\\\\316.01\\\\260.7422\\\\237.3192\\\\316.01\\\\259.477\\\\234.7172\\\\316.01\\\\258.3984\\\\233.6994\\\\316.01\\\\256.0547\\\\233.0592\\\\316.01\\\\253.7109\\\\232.7025\\\\316.01\\\\251.3672\\\\232.81\\\\316.01\\\\246.6797\\\\232.7563\\\\316.01\\\\241.9922\\\\232.9906\\\\316.01\\\\237.3047\\\\233.0007\\\\316.01\\\\232.6172\\\\233.0632\\\\316.01\\\\227.9297\\\\233.0305\\\\316.01\\\\211.5234\\\\233.0537\\\\316.01\\\\206.8359\\\\233.0245\\\\316.01\\\\204.4922\\\\232.7588\\\\316.01\\\\202.1484\\\\232.6623\\\\316.01\\\\195.1172\\\\232.7186\\\\316.01\\\\188.0859\\\\232.7186\\\\316.01\\\\183.3984\\\\232.7719\\\\316.01\\\\181.0547\\\\232.7322\\\\316.01\\\\169.3359\\\\232.7588\\\\316.01\\\\166.9922\\\\232.7322\\\\316.01\\\\157.6172\\\\232.7875\\\\316.01\\\\152.9297\\\\232.7456\\\\316.01\\\\145.8984\\\\232.7875\\\\316.01\\\\141.2109\\\\232.6623\\\\316.01\\\\136.5234\\\\232.6329\\\\316.01\\\\134.1797\\\\232.0805\\\\316.01\\\\131.8359\\\\231.7245\\\\316.01\\\\127.1484\\\\231.6801\\\\316.01\\\\122.4609\\\\231.7572\\\\316.01\\\\120.1172\\\\232.5873\\\\316.01\\\\115.4297\\\\232.6908\\\\316.01\\\\110.7422\\\\232.6623\\\\316.01\\\\108.3984\\\\231.8903\\\\316.01\\\\106.0547\\\\231.6087\\\\316.01\\\\103.7109\\\\231.5483\\\\316.01\\\\102.8224\\\\232.3734\\\\316.01\\\\101.3672\\\\233.1132\\\\316.01\\\\99.02344\\\\233.1397\\\\316.01\\\\80.27344\\\\233.1132\\\\316.01\\\\68.55469\\\\233.1568\\\\316.01\\\\52.14844\\\\233.1483\\\\316.01\\\\45.11719\\\\233.1132\\\\316.01\\\\44.13628\\\\232.3734\\\\316.01\\\\42.77344\\\\231.0686\\\\316.01\\\\42.00247\\\\230.0297\\\\316.01\\\\42.77344\\\\229.1926\\\\316.01\\\\45.11719\\\\228.465\\\\316.01\\\\47.46094\\\\226.9633\\\\316.01\\\\49.80469\\\\225.8906\\\\316.01\\\\52.14844\\\\224.4715\\\\316.01\\\\54.49219\\\\223.5304\\\\316.01\\\\55.10944\\\\222.9984\\\\316.01\\\\59.17969\\\\220.0478\\\\316.01\\\\61.52344\\\\218.6274\\\\316.01\\\\64.72018\\\\215.9672\\\\316.01\\\\66.21094\\\\214.8258\\\\316.01\\\\70.89844\\\\210.3064\\\\316.01\\\\72.20967\\\\208.9359\\\\316.01\\\\76.32568\\\\204.2484\\\\316.01\\\\77.88355\\\\201.9047\\\\316.01\\\\81.18286\\\\197.2172\\\\316.01\\\\82.483\\\\194.8734\\\\316.01\\\\83.97971\\\\192.5297\\\\316.01\\\\85.15485\\\\190.1859\\\\316.01\\\\86.11506\\\\187.8422\\\\316.01\\\\88.44123\\\\183.1547\\\\316.01\\\\88.85645\\\\180.8109\\\\316.01\\\\90.57047\\\\176.1234\\\\316.01\\\\91.12964\\\\173.7797\\\\316.01\\\\91.82967\\\\171.4359\\\\316.01\\\\92.67867\\\\169.0922\\\\316.01\\\\93.02273\\\\166.7484\\\\316.01\\\\93.23618\\\\162.0609\\\\316.01\\\\93.4022\\\\157.3734\\\\316.01\\\\93.45381\\\\152.6859\\\\316.01\\\\93.30592\\\\147.9984\\\\316.01\\\\93.08162\\\\143.3109\\\\316.01\\\\92.91119\\\\140.9672\\\\316.01\\\\92.40625\\\\138.6234\\\\316.01\\\\91.51611\\\\136.2797\\\\316.01\\\\90.39767\\\\131.5922\\\\316.01\\\\89.42243\\\\129.2484\\\\316.01\\\\88.71695\\\\126.9047\\\\316.01\\\\88.25335\\\\124.5609\\\\316.01\\\\86.80245\\\\122.2172\\\\316.01\\\\85.8848\\\\119.8734\\\\316.01\\\\84.58316\\\\117.5297\\\\316.01\\\\83.66635\\\\115.1859\\\\316.01\\\\81.8702\\\\112.8422\\\\316.01\\\\80.80811\\\\110.4984\\\\316.01\\\\79.08266\\\\108.1547\\\\316.01\\\\77.10529\\\\105.8109\\\\316.01\\\\75.58594\\\\103.5963\\\\316.01\\\\73.49802\\\\101.1234\\\\316.01\\\\71.40904\\\\98.77969\\\\316.01\\\\68.55469\\\\95.92896\\\\316.01\\\\63.67357\\\\91.74844\\\\316.01\\\\61.52344\\\\90.32737\\\\316.01\\\\59.17969\\\\88.27599\\\\316.01\\\\56.83594\\\\86.7886\\\\316.01\\\\54.49219\\\\85.58972\\\\316.01\\\\52.14844\\\\83.79642\\\\316.01\\\\49.80469\\\\83.02673\\\\316.01\\\\47.46094\\\\81.63853\\\\316.01\\\\45.11719\\\\80.76684\\\\316.01\\\\42.77344\\\\79.30589\\\\316.01\\\\38.08594\\\\78.27902\\\\316.01\\\\35.74219\\\\77.25775\\\\316.01\\\\33.39844\\\\76.59698\\\\316.01\\\\31.05469\\\\76.1613\\\\316.01\\\\28.71094\\\\75.56659\\\\316.01\\\\26.36719\\\\74.74911\\\\316.01\\\\24.02344\\\\74.44027\\\\316.01\\\\14.64844\\\\74.10455\\\\316.01\\\\9.960938\\\\74.14651\\\\316.01\\\\5.273438\\\\74.273\\\\316.01\\\\2.929688\\\\74.39614\\\\316.01\\\\0.5859375\\\\74.68565\\\\316.01\\\\-1.757813\\\\75.2776\\\\316.01\\\\-4.101563\\\\75.97835\\\\316.01\\\\-6.445313\\\\76.42603\\\\316.01\\\\-8.789063\\\\77.04977\\\\316.01\\\\-11.13281\\\\78.0173\\\\316.01\\\\-13.47656\\\\78.67566\\\\316.01\\\\-15.82031\\\\79.13776\\\\316.01\\\\-18.16406\\\\80.56435\\\\316.01\\\\-20.50781\\\\81.50463\\\\316.01\\\\-22.85156\\\\82.9015\\\\316.01\\\\-25.19531\\\\83.68569\\\\316.01\\\\-27.53906\\\\85.50075\\\\316.01\\\\-29.88281\\\\86.57327\\\\316.01\\\\-32.22656\\\\88.24621\\\\316.01\\\\-34.57031\\\\90.17396\\\\316.01\\\\-36.86175\\\\91.74844\\\\316.01\\\\-39.25781\\\\93.66087\\\\316.01\\\\-41.60156\\\\95.69225\\\\316.01\\\\-46.92759\\\\101.1234\\\\316.01\\\\-50.43298\\\\105.8109\\\\316.01\\\\-52.27513\\\\108.1547\\\\316.01\\\\-53.92769\\\\110.4984\\\\316.01\\\\-55.28938\\\\112.8422\\\\316.01\\\\-56.8119\\\\115.1859\\\\316.01\\\\-57.76507\\\\117.5297\\\\316.01\\\\-59.14436\\\\119.8734\\\\316.01\\\\-61.2854\\\\124.5609\\\\316.01\\\\-61.77901\\\\126.9047\\\\316.01\\\\-62.87364\\\\129.2484\\\\316.01\\\\-63.78268\\\\131.5922\\\\316.01\\\\-64.18945\\\\133.9359\\\\316.01\\\\-64.71442\\\\136.2797\\\\316.01\\\\-65.51514\\\\138.6234\\\\316.01\\\\-65.81827\\\\140.9672\\\\316.01\\\\-66.45339\\\\147.9984\\\\316.01\\\\-66.7085\\\\152.6859\\\\316.01\\\\-66.75646\\\\155.0297\\\\316.01\\\\-66.5637\\\\159.7172\\\\316.01\\\\-66.21094\\\\164.4047\\\\316.01\\\\-65.704\\\\169.0922\\\\316.01\\\\-65.23297\\\\171.4359\\\\316.01\\\\-64.38361\\\\173.7797\\\\316.01\\\\-64.01566\\\\176.1234\\\\316.01\\\\-63.38504\\\\178.4672\\\\316.01\\\\-62.17049\\\\180.8109\\\\316.01\\\\-60.82787\\\\185.4984\\\\316.01\\\\-59.41992\\\\187.8422\\\\316.01\\\\-58.36174\\\\190.1859\\\\316.01\\\\-56.99887\\\\192.5297\\\\316.01\\\\-56.07342\\\\194.8734\\\\316.01\\\\-55.66406\\\\195.3297\\\\316.01\\\\-53.32031\\\\198.7587\\\\316.01\\\\-52.68821\\\\199.5609\\\\316.01\\\\-51.4049\\\\201.9047\\\\316.01\\\\-50.97656\\\\202.358\\\\316.01\\\\-47.5612\\\\206.5922\\\\316.01\\\\-45.21313\\\\208.9359\\\\316.01\\\\-43.05138\\\\211.2797\\\\316.01\\\\-39.25781\\\\214.893\\\\316.01\\\\-35.15625\\\\218.3109\\\\316.01\\\\-34.57031\\\\218.8701\\\\316.01\\\\-32.22656\\\\220.1266\\\\316.01\\\\-31.58335\\\\220.6547\\\\316.01\\\\-28.28872\\\\222.9984\\\\316.01\\\\-27.53906\\\\223.6396\\\\316.01\\\\-25.19531\\\\224.5407\\\\316.01\\\\-22.85156\\\\226.027\\\\316.01\\\\-20.50781\\\\227.0401\\\\316.01\\\\-18.16406\\\\228.5211\\\\316.01\\\\-15.82031\\\\229.1818\\\\316.01\\\\-14.99192\\\\230.0297\\\\316.01\\\\-15.82031\\\\231.3521\\\\316.01\\\\-16.89078\\\\232.3734\\\\316.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851016723500001.512636853846\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"371\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"15\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n";
const char* k_rtStruct_json04 =
"                        \"Value\" : \"-18.16406\\\\233.1221\\\\319.01\\\\-32.22656\\\\233.131\\\\319.01\\\\-39.25781\\\\233.1568\\\\319.01\\\\-43.94531\\\\233.1397\\\\319.01\\\\-53.32031\\\\233.19\\\\319.01\\\\-58.00781\\\\233.1818\\\\319.01\\\\-60.35156\\\\233.5756\\\\319.01\\\\-62.69531\\\\233.19\\\\319.01\\\\-65.03906\\\\232.9081\\\\319.01\\\\-67.38281\\\\232.8852\\\\319.01\\\\-72.07031\\\\233.0044\\\\319.01\\\\-76.75781\\\\233.0343\\\\319.01\\\\-83.78906\\\\233.0343\\\\319.01\\\\-88.47656\\\\233.0859\\\\319.01\\\\-93.16406\\\\233.0859\\\\319.01\\\\-97.85156\\\\233.1569\\\\319.01\\\\-100.1953\\\\233.8986\\\\319.01\\\\-102.5391\\\\234.0359\\\\319.01\\\\-107.2266\\\\234.0231\\\\319.01\\\\-109.5703\\\\234.0754\\\\319.01\\\\-118.9453\\\\234.0411\\\\319.01\\\\-128.3203\\\\234.1127\\\\319.01\\\\-130.6641\\\\234.054\\\\319.01\\\\-142.3828\\\\234.1407\\\\319.01\\\\-147.0703\\\\234.0855\\\\319.01\\\\-151.7578\\\\234.1127\\\\319.01\\\\-158.7891\\\\234.0387\\\\319.01\\\\-165.8203\\\\234.0513\\\\319.01\\\\-170.5078\\\\234.2214\\\\319.01\\\\-175.1953\\\\234.1797\\\\319.01\\\\-186.9141\\\\234.2012\\\\319.01\\\\-189.2578\\\\234.1743\\\\319.01\\\\-191.6016\\\\234.2431\\\\319.01\\\\-198.6328\\\\234.2326\\\\319.01\\\\-200.9766\\\\234.2902\\\\319.01\\\\-205.6641\\\\234.1916\\\\319.01\\\\-212.6953\\\\234.3084\\\\319.01\\\\-215.0391\\\\234.2967\\\\319.01\\\\-222.0703\\\\234.3891\\\\319.01\\\\-224.4141\\\\234.3115\\\\319.01\\\\-229.1016\\\\234.3573\\\\319.01\\\\-233.7891\\\\234.2125\\\\319.01\\\\-238.4766\\\\234.1954\\\\319.01\\\\-243.1641\\\\234.0555\\\\319.01\\\\-247.8516\\\\233.7286\\\\319.01\\\\-250.1953\\\\233.7565\\\\319.01\\\\-252.5391\\\\233.9759\\\\319.01\\\\-257.2266\\\\234.7266\\\\319.01\\\\-259.5703\\\\235.9456\\\\319.01\\\\-260.9676\\\\237.0609\\\\319.01\\\\-262.9079\\\\239.4047\\\\319.01\\\\-263.0764\\\\241.7484\\\\319.01\\\\-262.579\\\\244.0922\\\\319.01\\\\-261.0551\\\\246.4359\\\\319.01\\\\-259.5703\\\\248.578\\\\319.01\\\\-257.2266\\\\251.3205\\\\319.01\\\\-255.5183\\\\253.4672\\\\319.01\\\\-252.5391\\\\256.6434\\\\319.01\\\\-250.1953\\\\258.9331\\\\319.01\\\\-247.8516\\\\261.4281\\\\319.01\\\\-245.5078\\\\263.6319\\\\319.01\\\\-244.1062\\\\265.1859\\\\319.01\\\\-241.6212\\\\267.5297\\\\319.01\\\\-239.4403\\\\269.8734\\\\319.01\\\\-233.7891\\\\275.424\\\\319.01\\\\-231.4453\\\\277.8534\\\\319.01\\\\-229.1016\\\\280.0498\\\\319.01\\\\-226.7578\\\\282.5503\\\\319.01\\\\-225.1923\\\\283.9359\\\\319.01\\\\-222.0703\\\\287.2521\\\\319.01\\\\-220.5829\\\\288.6234\\\\319.01\\\\-218.3594\\\\290.9672\\\\319.01\\\\-214.9922\\\\293.3109\\\\319.01\\\\-212.6953\\\\294.6526\\\\319.01\\\\-210.3516\\\\295.3282\\\\319.01\\\\-208.0078\\\\296.4917\\\\319.01\\\\-205.6641\\\\296.899\\\\319.01\\\\-203.3203\\\\297.1488\\\\319.01\\\\-196.2891\\\\297.1047\\\\319.01\\\\-193.9453\\\\296.9722\\\\319.01\\\\-184.5703\\\\296.9529\\\\319.01\\\\-177.5391\\\\296.8713\\\\319.01\\\\-163.4766\\\\296.8895\\\\319.01\\\\-144.7266\\\\296.8434\\\\319.01\\\\-135.3516\\\\296.8869\\\\319.01\\\\-133.0078\\\\296.8697\\\\319.01\\\\-116.6016\\\\296.916\\\\319.01\\\\-111.9141\\\\296.8811\\\\319.01\\\\-102.5391\\\\296.8641\\\\319.01\\\\-93.16406\\\\296.8746\\\\319.01\\\\-88.47656\\\\296.913\\\\319.01\\\\-81.44531\\\\296.8949\\\\319.01\\\\-79.10156\\\\296.9358\\\\319.01\\\\-67.38281\\\\296.9535\\\\319.01\\\\-65.03906\\\\296.9872\\\\319.01\\\\-62.69531\\\\296.9267\\\\319.01\\\\-58.00781\\\\296.9175\\\\319.01\\\\-55.66406\\\\297.0091\\\\319.01\\\\-50.97656\\\\296.9267\\\\319.01\\\\-46.28906\\\\296.9447\\\\319.01\\\\-43.94531\\\\297.0627\\\\319.01\\\\-41.60156\\\\296.8867\\\\319.01\\\\-36.91406\\\\296.8877\\\\319.01\\\\-32.22656\\\\296.9557\\\\319.01\\\\-29.88281\\\\297.2089\\\\319.01\\\\-27.53906\\\\296.9067\\\\319.01\\\\-22.85156\\\\296.998\\\\319.01\\\\-20.50781\\\\296.916\\\\319.01\\\\-15.82031\\\\297.0405\\\\319.01\\\\-13.47656\\\\296.9622\\\\319.01\\\\-11.13281\\\\296.9926\\\\319.01\\\\-8.789063\\\\296.9081\\\\319.01\\\\-4.101563\\\\296.9447\\\\319.01\\\\-1.757813\\\\297.0516\\\\319.01\\\\0.5859375\\\\296.8877\\\\319.01\\\\7.617188\\\\296.8888\\\\319.01\\\\16.99219\\\\296.8369\\\\319.01\\\\19.33594\\\\296.7851\\\\319.01\\\\28.71094\\\\296.7743\\\\319.01\\\\35.74219\\\\296.7043\\\\319.01\\\\45.11719\\\\296.7229\\\\319.01\\\\56.83594\\\\296.6313\\\\319.01\\\\63.86719\\\\296.6228\\\\319.01\\\\73.24219\\\\296.5616\\\\319.01\\\\75.58594\\\\296.5859\\\\319.01\\\\77.92969\\\\296.5154\\\\319.01\\\\91.99219\\\\296.5208\\\\319.01\\\\96.67969\\\\296.4597\\\\319.01\\\\101.3672\\\\296.4668\\\\319.01\\\\106.0547\\\\296.4124\\\\319.01\\\\108.3984\\\\296.4393\\\\319.01\\\\117.7734\\\\296.3922\\\\319.01\\\\122.4609\\\\296.391\\\\319.01\\\\127.1484\\\\296.3229\\\\319.01\\\\134.1797\\\\296.2817\\\\319.01\\\\136.5234\\\\295.884\\\\319.01\\\\138.8672\\\\296.2261\\\\319.01\\\\141.2109\\\\296.1486\\\\319.01\\\\145.8984\\\\295.864\\\\319.01\\\\152.9297\\\\295.833\\\\319.01\\\\155.2734\\\\295.7849\\\\319.01\\\\162.3047\\\\295.7849\\\\319.01\\\\164.6484\\\\295.7346\\\\319.01\\\\176.3672\\\\295.6638\\\\319.01\\\\183.3984\\\\295.6454\\\\319.01\\\\188.0859\\\\295.5916\\\\319.01\\\\192.7734\\\\295.6093\\\\319.01\\\\197.4609\\\\295.8172\\\\319.01\\\\202.1484\\\\295.7173\\\\319.01\\\\204.4922\\\\295.3989\\\\319.01\\\\206.8359\\\\294.803\\\\319.01\\\\209.1797\\\\294.3707\\\\319.01\\\\211.5234\\\\292.4764\\\\319.01\\\\213.8672\\\\291.2065\\\\319.01\\\\216.2109\\\\289.6522\\\\319.01\\\\217.2899\\\\288.6234\\\\319.01\\\\218.5547\\\\287.1418\\\\319.01\\\\221.8375\\\\283.9359\\\\319.01\\\\225.5859\\\\279.9363\\\\319.01\\\\227.9297\\\\277.6265\\\\319.01\\\\230.2734\\\\275.1683\\\\319.01\\\\233.3338\\\\272.2172\\\\319.01\\\\234.9609\\\\270.4222\\\\319.01\\\\244.3359\\\\260.8793\\\\319.01\\\\249.0234\\\\255.926\\\\319.01\\\\251.398\\\\253.4672\\\\319.01\\\\256.0547\\\\248.2237\\\\319.01\\\\257.5125\\\\246.4359\\\\319.01\\\\259.786\\\\244.0922\\\\319.01\\\\260.7422\\\\241.626\\\\319.01\\\\261.5214\\\\239.4047\\\\319.01\\\\260.7422\\\\237.1834\\\\319.01\\\\259.4941\\\\234.7172\\\\319.01\\\\258.3984\\\\233.6884\\\\319.01\\\\256.0547\\\\233.0498\\\\319.01\\\\253.7109\\\\232.7848\\\\319.01\\\\251.3672\\\\233.0989\\\\319.01\\\\249.0234\\\\232.8375\\\\319.01\\\\246.6797\\\\232.7692\\\\319.01\\\\241.9922\\\\233.0207\\\\319.01\\\\237.3047\\\\233.0108\\\\319.01\\\\232.6172\\\\233.0819\\\\319.01\\\\227.9297\\\\233.0402\\\\319.01\\\\211.5234\\\\233.0819\\\\319.01\\\\206.8359\\\\233.0245\\\\319.01\\\\204.4922\\\\232.7975\\\\319.01\\\\202.1484\\\\232.6767\\\\319.01\\\\199.8047\\\\232.6623\\\\319.01\\\\190.4297\\\\232.7588\\\\319.01\\\\185.7422\\\\232.7848\\\\319.01\\\\181.0547\\\\232.7588\\\\319.01\\\\169.3359\\\\232.7588\\\\319.01\\\\166.9922\\\\232.7048\\\\319.01\\\\159.9609\\\\232.7719\\\\319.01\\\\152.9297\\\\232.7346\\\\319.01\\\\148.2422\\\\232.7875\\\\319.01\\\\143.5547\\\\232.7481\\\\319.01\\\\138.8672\\\\232.5056\\\\319.01\\\\136.5234\\\\232.5056\\\\319.01\\\\134.1797\\\\232.0429\\\\319.01\\\\131.8359\\\\231.7224\\\\319.01\\\\129.4922\\\\231.6882\\\\319.01\\\\122.4609\\\\231.7224\\\\319.01\\\\120.1172\\\\232.6179\\\\319.01\\\\117.7734\\\\232.6767\\\\319.01\\\\113.0859\\\\232.6767\\\\319.01\\\\110.7422\\\\232.5557\\\\319.01\\\\108.3984\\\\231.8031\\\\319.01\\\\106.0547\\\\231.589\\\\319.01\\\\103.7109\\\\231.5244\\\\319.01\\\\102.7966\\\\232.3734\\\\319.01\\\\101.3672\\\\233.1042\\\\319.01\\\\96.67969\\\\233.131\\\\319.01\\\\89.64844\\\\233.1221\\\\319.01\\\\75.58594\\\\233.1483\\\\319.01\\\\70.89844\\\\233.1042\\\\319.01\\\\66.21094\\\\233.1652\\\\319.01\\\\56.83594\\\\233.131\\\\319.01\\\\52.14844\\\\233.1397\\\\319.01\\\\45.11719\\\\233.1042\\\\319.01\\\\44.14645\\\\232.3734\\\\319.01\\\\42.77344\\\\231.0593\\\\319.01\\\\42.01079\\\\230.0297\\\\319.01\\\\42.77344\\\\229.2013\\\\319.01\\\\45.11719\\\\228.447\\\\319.01\\\\47.46094\\\\226.9284\\\\319.01\\\\49.80469\\\\225.886\\\\319.01\\\\52.14844\\\\224.4503\\\\319.01\\\\54.49219\\\\223.4528\\\\319.01\\\\55.01404\\\\222.9984\\\\319.01\\\\59.17969\\\\220.0237\\\\319.01\\\\61.52344\\\\218.6082\\\\319.01\\\\64.67848\\\\215.9672\\\\319.01\\\\66.21094\\\\214.8015\\\\319.01\\\\70.89844\\\\210.2358\\\\319.01\\\\72.148\\\\208.9359\\\\319.01\\\\76.26517\\\\204.2484\\\\319.01\\\\77.81339\\\\201.9047\\\\319.01\\\\81.15081\\\\197.2172\\\\319.01\\\\82.41605\\\\194.8734\\\\319.01\\\\83.95089\\\\192.5297\\\\319.01\\\\85.09115\\\\190.1859\\\\319.01\\\\86.09138\\\\187.8422\\\\319.01\\\\88.39328\\\\183.1547\\\\319.01\\\\88.81863\\\\180.8109\\\\319.01\\\\90.54276\\\\176.1234\\\\319.01\\\\91.10133\\\\173.7797\\\\319.01\\\\91.76778\\\\171.4359\\\\319.01\\\\92.65485\\\\169.0922\\\\319.01\\\\92.99918\\\\166.7484\\\\319.01\\\\93.21165\\\\162.0609\\\\319.01\\\\93.36247\\\\157.3734\\\\319.01\\\\93.37488\\\\152.6859\\\\319.01\\\\93.27335\\\\147.9984\\\\319.01\\\\93.05807\\\\143.3109\\\\319.01\\\\92.86326\\\\140.9672\\\\319.01\\\\92.33733\\\\138.6234\\\\319.01\\\\91.43783\\\\136.2797\\\\319.01\\\\90.94626\\\\133.9359\\\\319.01\\\\90.36204\\\\131.5922\\\\319.01\\\\89.36162\\\\129.2484\\\\319.01\\\\88.68583\\\\126.9047\\\\319.01\\\\88.21825\\\\124.5609\\\\319.01\\\\86.72572\\\\122.2172\\\\319.01\\\\85.8493\\\\119.8734\\\\319.01\\\\84.53275\\\\117.5297\\\\319.01\\\\83.62613\\\\115.1859\\\\319.01\\\\81.8292\\\\112.8422\\\\319.01\\\\80.77354\\\\110.4984\\\\319.01\\\\79.05143\\\\108.1547\\\\319.01\\\\77.04742\\\\105.8109\\\\319.01\\\\75.58594\\\\103.674\\\\319.01\\\\73.4361\\\\101.1234\\\\319.01\\\\71.3485\\\\98.77969\\\\319.01\\\\68.55469\\\\95.97556\\\\319.01\\\\63.59904\\\\91.74844\\\\319.01\\\\61.52344\\\\90.34084\\\\319.01\\\\59.17969\\\\88.31962\\\\319.01\\\\56.83594\\\\86.83331\\\\319.01\\\\54.49219\\\\85.63074\\\\319.01\\\\52.14844\\\\83.82526\\\\319.01\\\\49.80469\\\\83.06862\\\\319.01\\\\47.46094\\\\81.65692\\\\319.01\\\\45.11719\\\\80.79234\\\\319.01\\\\42.77344\\\\79.31967\\\\319.01\\\\38.08594\\\\78.27902\\\\319.01\\\\35.74219\\\\77.28263\\\\319.01\\\\33.39844\\\\76.61404\\\\319.01\\\\31.05469\\\\76.1767\\\\319.01\\\\28.71094\\\\75.56659\\\\319.01\\\\26.36719\\\\74.77422\\\\319.01\\\\24.02344\\\\74.45255\\\\319.01\\\\14.64844\\\\74.10455\\\\319.01\\\\12.30469\\\\74.10455\\\\319.01\\\\5.273438\\\\74.26746\\\\319.01\\\\2.929688\\\\74.40346\\\\319.01\\\\0.5859375\\\\74.73805\\\\319.01\\\\-4.101563\\\\75.99764\\\\319.01\\\\-6.445313\\\\76.4192\\\\319.01\\\\-8.789063\\\\77.021\\\\319.01\\\\-11.13281\\\\78.0173\\\\319.01\\\\-13.47656\\\\78.67566\\\\319.01\\\\-15.82031\\\\79.11396\\\\319.01\\\\-18.16406\\\\80.51486\\\\319.01\\\\-20.50781\\\\81.46942\\\\319.01\\\\-22.85156\\\\82.85554\\\\319.01\\\\-25.19531\\\\83.66675\\\\319.01\\\\-27.53906\\\\85.46803\\\\319.01\\\\-29.88281\\\\86.52827\\\\319.01\\\\-32.22656\\\\88.23955\\\\319.01\\\\-34.57031\\\\90.13984\\\\319.01\\\\-36.9035\\\\91.74844\\\\319.01\\\\-41.60156\\\\95.65716\\\\319.01\\\\-46.9745\\\\101.1234\\\\319.01\\\\-50.48889\\\\105.8109\\\\319.01\\\\-52.30941\\\\108.1547\\\\319.01\\\\-53.97684\\\\110.4984\\\\319.01\\\\-55.34668\\\\112.8422\\\\319.01\\\\-56.83594\\\\115.1859\\\\319.01\\\\-57.84288\\\\117.5297\\\\319.01\\\\-59.17969\\\\119.8734\\\\319.01\\\\-60.35156\\\\122.2895\\\\319.01\\\\-61.30981\\\\124.5609\\\\319.01\\\\-61.83551\\\\126.9047\\\\319.01\\\\-62.94936\\\\129.2484\\\\319.01\\\\-63.81696\\\\131.5922\\\\319.01\\\\-64.21995\\\\133.9359\\\\319.01\\\\-64.78502\\\\136.2797\\\\319.01\\\\-65.56069\\\\138.6234\\\\319.01\\\\-65.84246\\\\140.9672\\\\319.01\\\\-66.49079\\\\147.9984\\\\319.01\\\\-66.73695\\\\152.6859\\\\319.01\\\\-66.80727\\\\155.0297\\\\319.01\\\\-66.62016\\\\159.7172\\\\319.01\\\\-66.24458\\\\164.4047\\\\319.01\\\\-65.7409\\\\169.0922\\\\319.01\\\\-65.32193\\\\171.4359\\\\319.01\\\\-64.43269\\\\173.7797\\\\319.01\\\\-64.03687\\\\176.1234\\\\319.01\\\\-63.43099\\\\178.4672\\\\319.01\\\\-62.2521\\\\180.8109\\\\319.01\\\\-61.52967\\\\183.1547\\\\319.01\\\\-60.90088\\\\185.4984\\\\319.01\\\\-59.4697\\\\187.8422\\\\319.01\\\\-58.42364\\\\190.1859\\\\319.01\\\\-57.02597\\\\192.5297\\\\319.01\\\\-56.12809\\\\194.8734\\\\319.01\\\\-55.66406\\\\195.3972\\\\319.01\\\\-53.32031\\\\198.8418\\\\319.01\\\\-52.73438\\\\199.5609\\\\319.01\\\\-51.45956\\\\201.9047\\\\319.01\\\\-48.63281\\\\205.2587\\\\319.01\\\\-47.59325\\\\206.5922\\\\319.01\\\\-45.25587\\\\208.9359\\\\319.01\\\\-43.11045\\\\211.2797\\\\319.01\\\\-41.60156\\\\212.6377\\\\319.01\\\\-39.25781\\\\214.941\\\\319.01\\\\-35.2231\\\\218.3109\\\\319.01\\\\-34.57031\\\\218.9344\\\\319.01\\\\-32.22656\\\\220.1609\\\\319.01\\\\-31.63622\\\\220.6547\\\\319.01\\\\-28.34042\\\\222.9984\\\\319.01\\\\-27.53906\\\\223.6754\\\\319.01\\\\-25.19531\\\\224.5542\\\\319.01\\\\-22.85156\\\\226.0607\\\\319.01\\\\-20.50781\\\\227.0596\\\\319.01\\\\-18.16406\\\\228.5294\\\\319.01\\\\-15.82031\\\\229.1996\\\\319.01\\\\-15.00954\\\\230.0297\\\\319.01\\\\-15.82031\\\\231.333\\\\319.01\\\\-16.88058\\\\232.3734\\\\319.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851035724600001.533642609670\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"359\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"16\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"101.3672\\\\233.1221\\\\322.01\\\\80.27344\\\\233.131\\\\322.01\\\\75.58594\\\\233.1568\\\\322.01\\\\70.89844\\\\233.131\\\\322.01\\\\61.52344\\\\233.1652\\\\322.01\\\\45.11719\\\\233.1132\\\\322.01\\\\44.12896\\\\232.3734\\\\322.01\\\\42.77344\\\\231.076\\\\322.01\\\\42.00247\\\\230.0297\\\\322.01\\\\42.77344\\\\229.1926\\\\322.01\\\\45.11719\\\\228.4243\\\\322.01\\\\47.46094\\\\226.9242\\\\322.01\\\\49.80469\\\\225.8587\\\\322.01\\\\52.14844\\\\224.4422\\\\322.01\\\\54.49219\\\\223.4268\\\\322.01\\\\54.97742\\\\222.9984\\\\322.01\\\\59.17969\\\\220.0136\\\\322.01\\\\61.52344\\\\218.5471\\\\322.01\\\\64.65119\\\\215.9672\\\\322.01\\\\66.21094\\\\214.7764\\\\322.01\\\\70.89844\\\\210.1964\\\\322.01\\\\72.1096\\\\208.9359\\\\322.01\\\\76.2437\\\\204.2484\\\\322.01\\\\77.92969\\\\201.7078\\\\322.01\\\\81.11842\\\\197.2172\\\\322.01\\\\82.30748\\\\194.8734\\\\322.01\\\\82.61719\\\\194.5523\\\\322.01\\\\83.92992\\\\192.5297\\\\322.01\\\\85.04084\\\\190.1859\\\\322.01\\\\87.1582\\\\185.4984\\\\322.01\\\\88.36839\\\\183.1547\\\\322.01\\\\88.7899\\\\180.8109\\\\322.01\\\\89.56792\\\\178.4672\\\\322.01\\\\90.50697\\\\176.1234\\\\322.01\\\\91.68128\\\\171.4359\\\\322.01\\\\92.59956\\\\169.0922\\\\322.01\\\\92.97371\\\\166.7484\\\\322.01\\\\93.0934\\\\164.4047\\\\322.01\\\\93.33585\\\\157.3734\\\\322.01\\\\93.34813\\\\152.6859\\\\322.01\\\\93.22983\\\\147.9984\\\\322.01\\\\93.04629\\\\143.3109\\\\322.01\\\\92.83467\\\\140.9672\\\\322.01\\\\92.2954\\\\138.6234\\\\322.01\\\\91.39597\\\\136.2797\\\\322.01\\\\90.90752\\\\133.9359\\\\322.01\\\\90.32093\\\\131.5922\\\\322.01\\\\89.30564\\\\129.2484\\\\322.01\\\\88.67188\\\\126.9047\\\\322.01\\\\88.16579\\\\124.5609\\\\322.01\\\\86.67472\\\\122.2172\\\\322.01\\\\85.81149\\\\119.8734\\\\322.01\\\\84.49661\\\\117.5297\\\\322.01\\\\83.58636\\\\115.1859\\\\322.01\\\\81.76791\\\\112.8422\\\\322.01\\\\80.71002\\\\110.4984\\\\322.01\\\\79.01383\\\\108.1547\\\\322.01\\\\77.02567\\\\105.8109\\\\322.01\\\\75.58594\\\\103.7458\\\\322.01\\\\73.38867\\\\101.1234\\\\322.01\\\\71.30643\\\\98.77969\\\\322.01\\\\68.55469\\\\96.01372\\\\322.01\\\\63.52539\\\\91.74844\\\\322.01\\\\61.52344\\\\90.35172\\\\322.01\\\\59.17969\\\\88.35252\\\\322.01\\\\56.83594\\\\86.87999\\\\322.01\\\\54.49219\\\\85.66346\\\\322.01\\\\52.14844\\\\83.84647\\\\322.01\\\\49.80469\\\\83.09609\\\\322.01\\\\47.46094\\\\81.6853\\\\322.01\\\\45.11719\\\\80.80066\\\\322.01\\\\42.77344\\\\79.34901\\\\322.01\\\\38.08594\\\\78.30383\\\\322.01\\\\35.74219\\\\77.32118\\\\322.01\\\\33.39844\\\\76.64365\\\\322.01\\\\31.05469\\\\76.1918\\\\322.01\\\\28.71094\\\\75.59623\\\\322.01\\\\26.36719\\\\74.80752\\\\322.01\\\\24.02344\\\\74.45255\\\\322.01\\\\14.64844\\\\74.12846\\\\322.01\\\\12.30469\\\\74.11021\\\\322.01\\\\5.273438\\\\74.2905\\\\322.01\\\\2.929688\\\\74.40346\\\\322.01\\\\0.5859375\\\\74.73805\\\\322.01\\\\-4.101563\\\\76.01651\\\\322.01\\\\-6.445313\\\\76.43631\\\\322.01\\\\-8.789063\\\\77.04008\\\\322.01\\\\-11.13281\\\\78.04468\\\\322.01\\\\-13.47656\\\\78.69472\\\\322.01\\\\-15.82031\\\\79.13468\\\\322.01\\\\-18.16406\\\\80.51486\\\\322.01\\\\-20.50781\\\\81.47456\\\\322.01\\\\-22.85156\\\\82.85554\\\\322.01\\\\-25.19531\\\\83.68029\\\\322.01\\\\-27.53906\\\\85.48688\\\\322.01\\\\-29.88281\\\\86.53931\\\\322.01\\\\-32.22656\\\\88.25278\\\\322.01\\\\-34.57031\\\\90.1353\\\\322.01\\\\-36.86266\\\\91.74844\\\\322.01\\\\-39.25781\\\\93.66832\\\\322.01\\\\-41.60156\\\\95.6376\\\\322.01\\\\-47.00521\\\\101.1234\\\\322.01\\\\-50.52413\\\\105.8109\\\\322.01\\\\-50.97656\\\\106.3405\\\\322.01\\\\-54.00098\\\\110.4984\\\\322.01\\\\-56.85406\\\\115.1859\\\\322.01\\\\-57.8924\\\\117.5297\\\\322.01\\\\-59.20324\\\\119.8734\\\\322.01\\\\-60.36086\\\\122.2172\\\\322.01\\\\-61.32914\\\\124.5609\\\\322.01\\\\-61.85643\\\\126.9047\\\\322.01\\\\-63.04688\\\\129.2484\\\\322.01\\\\-63.83418\\\\131.5922\\\\322.01\\\\-64.24364\\\\133.9359\\\\322.01\\\\-65.59342\\\\138.6234\\\\322.01\\\\-65.87014\\\\140.9672\\\\322.01\\\\-66.32201\\\\145.6547\\\\322.01\\\\-66.68098\\\\150.3422\\\\322.01\\\\-66.85014\\\\155.0297\\\\322.01\\\\-66.7664\\\\157.3734\\\\322.01\\\\-66.48803\\\\162.0609\\\\322.01\\\\-65.75874\\\\169.0922\\\\322.01\\\\-65.3637\\\\171.4359\\\\322.01\\\\-64.4847\\\\173.7797\\\\322.01\\\\-64.05982\\\\176.1234\\\\322.01\\\\-63.44819\\\\178.4672\\\\322.01\\\\-62.28937\\\\180.8109\\\\322.01\\\\-61.54255\\\\183.1547\\\\322.01\\\\-60.94105\\\\185.4984\\\\322.01\\\\-59.49768\\\\187.8422\\\\322.01\\\\-58.49549\\\\190.1859\\\\322.01\\\\-57.04611\\\\192.5297\\\\322.01\\\\-56.17723\\\\194.8734\\\\322.01\\\\-55.66406\\\\195.4638\\\\322.01\\\\-53.32031\\\\198.8875\\\\322.01\\\\-52.75635\\\\199.5609\\\\322.01\\\\-51.52395\\\\201.9047\\\\322.01\\\\-50.97656\\\\202.4906\\\\322.01\\\\-47.61296\\\\206.5922\\\\322.01\\\\-45.29157\\\\208.9359\\\\322.01\\\\-43.14595\\\\211.2797\\\\322.01\\\\-41.60156\\\\212.6678\\\\322.01\\\\-39.25781\\\\214.9694\\\\322.01\\\\-35.26563\\\\218.3109\\\\322.01\\\\-34.57031\\\\218.971\\\\322.01\\\\-32.22656\\\\220.1965\\\\322.01\\\\-31.68909\\\\220.6547\\\\322.01\\\\-28.39057\\\\222.9984\\\\322.01\\\\-27.53906\\\\223.7158\\\\322.01\\\\-25.19531\\\\224.5815\\\\322.01\\\\-22.85156\\\\226.0936\\\\322.01\\\\-20.50781\\\\227.1104\\\\322.01\\\\-18.16406\\\\228.5341\\\\322.01\\\\-15.82031\\\\229.2136\\\\322.01\\\\-15.0185\\\\230.0297\\\\322.01\\\\-15.82031\\\\231.3232\\\\322.01\\\\-16.88058\\\\232.3734\\\\322.01\\\\-18.16406\\\\233.1221\\\\322.01\\\\-22.85156\\\\233.1483\\\\322.01\\\\-32.22656\\\\233.131\\\\322.01\\\\-34.57031\\\\233.1652\\\\322.01\\\\-41.60156\\\\233.131\\\\322.01\\\\-58.00781\\\\233.1981\\\\322.01\\\\-60.35156\\\\233.6712\\\\322.01\\\\-65.03906\\\\232.6477\\\\322.01\\\\-67.38281\\\\232.8735\\\\322.01\\\\-69.72656\\\\232.9735\\\\322.01\\\\-74.41406\\\\233.0245\\\\322.01\\\\-86.13281\\\\233.0479\\\\322.01\\\\-88.47656\\\\233.0859\\\\322.01\\\\-95.50781\\\\233.0951\\\\322.01\\\\-97.85156\\\\233.2679\\\\322.01\\\\-100.1953\\\\233.9446\\\\322.01\\\\-102.5391\\\\233.6928\\\\322.01\\\\-104.8828\\\\234.0489\\\\322.01\\\\-107.2266\\\\233.6082\\\\322.01\\\\-109.5703\\\\234.062\\\\322.01\\\\-114.2578\\\\234.0672\\\\322.01\\\\-118.9453\\\\234.0158\\\\322.01\\\\-125.9766\\\\234.1221\\\\322.01\\\\-130.6641\\\\234.0591\\\\322.01\\\\-137.6953\\\\234.1038\\\\322.01\\\\-142.3828\\\\234.1741\\\\322.01\\\\-147.0703\\\\234.1078\\\\322.01\\\\-151.7578\\\\234.1127\\\\322.01\\\\-154.1016\\\\234.0722\\\\322.01\\\\-161.1328\\\\234.0513\\\\322.01\\\\-165.8203\\\\234.0722\\\\322.01\\\\-168.1641\\\\234.1994\\\\322.01\\\\-177.5391\\\\234.2214\\\\322.01\\\\-189.2578\\\\234.1743\\\\322.01\\\\-191.6016\\\\234.2721\\\\322.01\\\\-198.6328\\\\234.2326\\\\322.01\\\\-200.9766\\\\234.2902\\\\322.01\\\\-203.3203\\\\234.205\\\\322.01\\\\-215.0391\\\\234.2967\\\\322.01\\\\-217.3828\\\\234.3573\\\\322.01\\\\-222.0703\\\\234.3917\\\\322.01\\\\-224.4141\\\\234.3447\\\\322.01\\\\-229.1016\\\\234.4078\\\\322.01\\\\-233.7891\\\\234.2125\\\\322.01\\\\-238.4766\\\\234.2088\\\\322.01\\\\-243.1641\\\\234.0441\\\\322.01\\\\-247.8516\\\\233.665\\\\322.01\\\\-250.1953\\\\233.6877\\\\322.01\\\\-252.5391\\\\233.9291\\\\322.01\\\\-257.2266\\\\234.8983\\\\322.01\\\\-259.5703\\\\235.9345\\\\322.01\\\\-260.9983\\\\237.0609\\\\322.01\\\\-262.9464\\\\239.4047\\\\322.01\\\\-263.0954\\\\241.7484\\\\322.01\\\\-262.6598\\\\244.0922\\\\322.01\\\\-261.9141\\\\245.2447\\\\322.01\\\\-259.5703\\\\248.5569\\\\322.01\\\\-257.2266\\\\251.3205\\\\322.01\\\\-255.5536\\\\253.4672\\\\322.01\\\\-252.5391\\\\256.6377\\\\322.01\\\\-250.1953\\\\258.905\\\\322.01\\\\-247.8516\\\\261.4186\\\\322.01\\\\-245.5078\\\\263.615\\\\322.01\\\\-244.0908\\\\265.1859\\\\322.01\\\\-241.6044\\\\267.5297\\\\322.01\\\\-239.4094\\\\269.8734\\\\322.01\\\\-233.7891\\\\275.4182\\\\322.01\\\\-231.4453\\\\277.8469\\\\322.01\\\\-229.1016\\\\280.0557\\\\322.01\\\\-226.7578\\\\282.5473\\\\322.01\\\\-224.4141\\\\284.7202\\\\322.01\\\\-222.0703\\\\287.2521\\\\322.01\\\\-220.5943\\\\288.6234\\\\322.01\\\\-218.3764\\\\290.9672\\\\322.01\\\\-215.0295\\\\293.3109\\\\322.01\\\\-212.6953\\\\294.6526\\\\322.01\\\\-210.3516\\\\295.2914\\\\322.01\\\\-209.9242\\\\295.6547\\\\322.01\\\\-208.0078\\\\296.5075\\\\322.01\\\\-205.6641\\\\296.899\\\\322.01\\\\-203.3203\\\\297.134\\\\322.01\\\\-198.6328\\\\297.1414\\\\322.01\\\\-193.9453\\\\296.9471\\\\322.01\\\\-189.2578\\\\296.9713\\\\322.01\\\\-182.2266\\\\296.9337\\\\322.01\\\\-177.5391\\\\296.8792\\\\322.01\\\\-163.4766\\\\296.8978\\\\322.01\\\\-149.4141\\\\296.8432\\\\322.01\\\\-142.3828\\\\296.886\\\\322.01\\\\-133.0078\\\\296.8703\\\\322.01\\\\-130.6641\\\\296.896\\\\322.01\\\\-121.2891\\\\296.8971\\\\322.01\\\\-114.2578\\\\296.9339\\\\322.01\\\\-111.9141\\\\296.9077\\\\322.01\\\\-95.50781\\\\296.8847\\\\322.01\\\\-90.82031\\\\296.9145\\\\322.01\\\\-81.44531\\\\296.8949\\\\322.01\\\\-74.41406\\\\296.9447\\\\322.01\\\\-65.03906\\\\296.9557\\\\322.01\\\\-60.35156\\\\296.916\\\\322.01\\\\-50.97656\\\\296.9067\\\\322.01\\\\-46.28906\\\\296.9267\\\\322.01\\\\-43.94531\\\\297.0141\\\\322.01\\\\-41.60156\\\\296.8673\\\\322.01\\\\-34.57031\\\\296.9285\\\\322.01\\\\-27.53906\\\\296.9067\\\\322.01\\\\-22.85156\\\\296.9377\\\\322.01\\\\-11.13281\\\\296.9377\\\\322.01\\\\-8.789063\\\\296.9067\\\\322.01\\\\7.617188\\\\296.8877\\\\322.01\\\\14.64844\\\\296.8266\\\\322.01\\\\16.99219\\\\296.847\\\\322.01\\\\24.02344\\\\296.7643\\\\322.01\\\\31.05469\\\\296.7229\\\\322.01\\\\45.11719\\\\296.7229\\\\322.01\\\\49.80469\\\\296.6724\\\\322.01\\\\52.14844\\\\296.6929\\\\322.01\\\\61.52344\\\\296.6191\\\\322.01\\\\73.24219\\\\296.5777\\\\322.01\\\\84.96094\\\\296.5079\\\\322.01\\\\91.99219\\\\296.5208\\\\322.01\\\\96.67969\\\\296.4597\\\\322.01\\\\101.3672\\\\296.4604\\\\322.01\\\\115.4297\\\\296.3846\\\\322.01\\\\122.4609\\\\296.3766\\\\322.01\\\\124.8047\\\\296.3139\\\\322.01\\\\131.8359\\\\296.2987\\\\322.01\\\\134.1797\\\\296.1332\\\\322.01\\\\136.5234\\\\295.85\\\\322.01\\\\138.8672\\\\295.9433\\\\322.01\\\\143.5547\\\\295.8543\\\\322.01\\\\148.2422\\\\295.8486\\\\322.01\\\\157.6172\\\\295.7849\\\\322.01\\\\164.6484\\\\295.6998\\\\322.01\\\\174.0234\\\\295.6454\\\\322.01\\\\183.3984\\\\295.6638\\\\322.01\\\\188.0859\\\\295.5742\\\\322.01\\\\192.7734\\\\295.5916\\\\322.01\\\\197.4609\\\\295.833\\\\322.01\\\\202.1484\\\\295.7346\\\\322.01\\\\204.4922\\\\295.3842\\\\322.01\\\\206.8359\\\\294.8107\\\\322.01\\\\209.1797\\\\294.3828\\\\322.01\\\\211.5234\\\\292.4996\\\\322.01\\\\213.8672\\\\291.1916\\\\322.01\\\\216.2109\\\\289.625\\\\322.01\\\\217.2743\\\\288.6234\\\\322.01\\\\219.3824\\\\286.2797\\\\322.01\\\\221.8496\\\\283.9359\\\\322.01\\\\223.2422\\\\282.3931\\\\322.01\\\\225.5859\\\\279.9482\\\\322.01\\\\227.9297\\\\277.6329\\\\322.01\\\\230.2734\\\\275.1512\\\\322.01\\\\233.3577\\\\272.2172\\\\322.01\\\\234.9609\\\\270.4549\\\\322.01\\\\237.8726\\\\267.5297\\\\322.01\\\\244.3359\\\\260.9081\\\\322.01\\\\246.6797\\\\258.3968\\\\322.01\\\\251.3775\\\\253.4672\\\\322.01\\\\256.0547\\\\248.2237\\\\322.01\\\\257.5039\\\\246.4359\\\\322.01\\\\259.7775\\\\244.0922\\\\322.01\\\\260.7422\\\\241.7324\\\\322.01\\\\261.5918\\\\239.4047\\\\322.01\\\\260.8048\\\\237.0609\\\\322.01\\\\259.4941\\\\234.7172\\\\322.01\\\\258.3984\\\\233.6932\\\\322.01\\\\256.0547\\\\233.0686\\\\322.01\\\\253.7109\\\\232.8224\\\\322.01\\\\251.3672\\\\233.8731\\\\322.01\\\\249.0234\\\\233.2281\\\\322.01\\\\246.6797\\\\232.7947\\\\322.01\\\\241.9922\\\\233.0207\\\\322.01\\\\237.3047\\\\233.0207\\\\322.01\\\\232.6172\\\\233.0726\\\\322.01\\\\227.9297\\\\233.0305\\\\322.01\\\\218.5547\\\\233.0498\\\\322.01\\\\211.5234\\\\233.091\\\\322.01\\\\206.8359\\\\233.0245\\\\322.01\\\\202.1484\\\\232.6623\\\\322.01\\\\190.4297\\\\232.7588\\\\322.01\\\\174.0234\\\\232.7588\\\\322.01\\\\171.6797\\\\232.7719\\\\322.01\\\\164.6484\\\\232.7048\\\\322.01\\\\159.9609\\\\232.7588\\\\322.01\\\\152.9297\\\\232.7481\\\\322.01\\\\148.2422\\\\232.7875\\\\322.01\\\\143.5547\\\\232.7614\\\\322.01\\\\141.2109\\\\232.6329\\\\322.01\\\\138.8672\\\\231.8626\\\\322.01\\\\136.5234\\\\231.9556\\\\322.01\\\\134.1797\\\\232.4196\\\\322.01\\\\131.8359\\\\231.7424\\\\322.01\\\\124.8047\\\\231.7024\\\\322.01\\\\122.4609\\\\232.0168\\\\322.01\\\\120.1172\\\\232.6329\\\\322.01\\\\115.4297\\\\232.7071\\\\322.01\\\\110.7422\\\\232.6027\\\\322.01\\\\108.3984\\\\231.8445\\\\322.01\\\\106.0547\\\\231.5827\\\\322.01\\\\103.7109\\\\231.4945\\\\322.01\\\\102.7858\\\\232.3734\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851068726500001.546012551665\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"368\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"17\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-18.16406\\\\233.1132\\\\325.01\\\\-34.57031\\\\233.1568\\\\325.01\\\\-43.94531\\\\233.1397\\\\325.01\\\\-53.32031\\\\233.1652\\\\325.01\\\\-62.69531\\\\233.2507\\\\325.01\\\\-65.03906\\\\232.6027\\\\325.01\\\\-67.38281\\\\232.8617\\\\325.01\\\\-69.72656\\\\232.9735\\\\325.01\\\\-74.41406\\\\233.0343\\\\325.01\\\\-83.78906\\\\233.0245\\\\325.01\\\\-88.47656\\\\233.0951\\\\325.01\\\\-95.50781\\\\233.0859\\\\325.01\\\\-97.85156\\\\233.4143\\\\325.01\\\\-100.1953\\\\234.0359\\\\325.01\\\\-102.5391\\\\234.1029\\\\325.01\\\\-104.8828\\\\233.4395\\\\325.01\\\\-109.5703\\\\234.0489\\\\325.01\\\\-111.9141\\\\234.0805\\\\325.01\\\\-116.6016\\\\234.054\\\\325.01\\\\-128.3203\\\\234.1174\\\\325.01\\\\-130.6641\\\\234.0591\\\\325.01\\\\-137.6953\\\\234.1221\\\\325.01\\\\-142.3828\\\\234.2191\\\\325.01\\\\-147.0703\\\\234.1029\\\\325.01\\\\-151.7578\\\\234.1127\\\\325.01\\\\-156.4453\\\\234.0772\\\\325.01\\\\-158.7891\\\\234.1085\\\\325.01\\\\-163.4766\\\\234.0463\\\\325.01\\\\-165.8203\\\\234.0722\\\\325.01\\\\-168.1641\\\\234.1994\\\\325.01\\\\-177.5391\\\\234.2357\\\\325.01\\\\-179.8828\\\\234.1837\\\\325.01\\\\-184.5703\\\\234.2574\\\\325.01\\\\-189.2578\\\\234.2012\\\\325.01\\\\-191.6016\\\\234.2721\\\\325.01\\\\-196.2891\\\\234.2574\\\\325.01\\\\-200.9766\\\\234.3204\\\\325.01\\\\-205.6641\\\\234.2187\\\\325.01\\\\-215.0391\\\\234.3146\\\\325.01\\\\-217.3828\\\\234.3917\\\\325.01\\\\-222.0703\\\\234.4409\\\\325.01\\\\-224.4141\\\\234.3917\\\\325.01\\\\-229.1016\\\\234.4578\\\\325.01\\\\-233.7891\\\\234.2398\\\\325.01\\\\-238.4766\\\\234.2644\\\\325.01\\\\-243.1641\\\\234.0329\\\\325.01\\\\-247.8516\\\\233.6513\\\\325.01\\\\-250.1953\\\\233.6663\\\\325.01\\\\-252.5391\\\\234.2918\\\\325.01\\\\-254.8828\\\\234.4928\\\\325.01\\\\-257.2266\\\\234.9479\\\\325.01\\\\-259.5703\\\\235.9244\\\\325.01\\\\-261.0111\\\\237.0609\\\\325.01\\\\-262.9464\\\\239.4047\\\\325.01\\\\-263.1001\\\\241.7484\\\\325.01\\\\-262.7014\\\\244.0922\\\\325.01\\\\-261.9141\\\\245.3016\\\\325.01\\\\-259.5703\\\\248.4308\\\\325.01\\\\-259.1987\\\\248.7797\\\\325.01\\\\-257.2266\\\\251.1994\\\\325.01\\\\-255.4771\\\\253.4672\\\\325.01\\\\-252.5391\\\\256.632\\\\325.01\\\\-250.1953\\\\258.8544\\\\325.01\\\\-247.8516\\\\261.3872\\\\325.01\\\\-245.5078\\\\263.5746\\\\325.01\\\\-244.0755\\\\265.1859\\\\325.01\\\\-241.6044\\\\267.5297\\\\325.01\\\\-239.3906\\\\269.8734\\\\325.01\\\\-236.1328\\\\273.1332\\\\325.01\\\\-233.7891\\\\275.4082\\\\325.01\\\\-231.4453\\\\277.8406\\\\325.01\\\\-229.8915\\\\279.2484\\\\325.01\\\\-226.7578\\\\282.5634\\\\325.01\\\\-224.4141\\\\284.7142\\\\325.01\\\\-222.0703\\\\287.2492\\\\325.01\\\\-220.6122\\\\288.6234\\\\325.01\\\\-218.3665\\\\290.9672\\\\325.01\\\\-215.0391\\\\293.3202\\\\325.01\\\\-212.6953\\\\294.6589\\\\325.01\\\\-210.3516\\\\295.3953\\\\325.01\\\\-208.0078\\\\296.5198\\\\325.01\\\\-205.6641\\\\296.899\\\\325.01\\\\-203.3203\\\\297.134\\\\325.01\\\\-198.6328\\\\297.134\\\\325.01\\\\-193.9453\\\\296.9855\\\\325.01\\\\-189.2578\\\\296.9891\\\\325.01\\\\-179.8828\\\\296.8863\\\\325.01\\\\-168.1641\\\\296.9185\\\\325.01\\\\-161.1328\\\\296.8737\\\\325.01\\\\-156.4453\\\\296.8978\\\\325.01\\\\-149.4141\\\\296.8678\\\\325.01\\\\-144.7266\\\\296.9186\\\\325.01\\\\-140.0391\\\\296.886\\\\325.01\\\\-128.3203\\\\296.8794\\\\325.01\\\\-125.9766\\\\296.916\\\\325.01\\\\-116.6016\\\\296.916\\\\325.01\\\\-114.2578\\\\296.9518\\\\325.01\\\\-100.1953\\\\296.9218\\\\325.01\\\\-95.50781\\\\296.8847\\\\325.01\\\\-88.47656\\\\296.934\\\\325.01\\\\-81.44531\\\\296.925\\\\325.01\\\\-76.75781\\\\296.9557\\\\325.01\\\\-72.07031\\\\296.9428\\\\325.01\\\\-69.72656\\\\297.0516\\\\325.01\\\\-67.38281\\\\296.9622\\\\325.01\\\\-53.32031\\\\296.9053\\\\325.01\\\\-41.60156\\\\296.8985\\\\325.01\\\\-22.85156\\\\296.9081\\\\325.01\\\\-20.50781\\\\296.9841\\\\325.01\\\\-18.16406\\\\297.3277\\\\325.01\\\\-15.82031\\\\296.9175\\\\325.01\\\\-13.47656\\\\296.9175\\\\325.01\\\\-11.13281\\\\297.3806\\\\325.01\\\\-8.789063\\\\296.9067\\\\325.01\\\\-6.445313\\\\296.9285\\\\325.01\\\\0.5859375\\\\296.878\\\\325.01\\\\2.929688\\\\296.9095\\\\325.01\\\\9.960938\\\\296.837\\\\325.01\\\\21.67969\\\\296.8055\\\\325.01\\\\33.39844\\\\296.7135\\\\325.01\\\\38.08594\\\\296.7324\\\\325.01\\\\42.77344\\\\296.7021\\\\325.01\\\\52.14844\\\\296.6929\\\\325.01\\\\59.17969\\\\296.6635\\\\325.01\\\\66.21094\\\\296.5984\\\\325.01\\\\77.92969\\\\296.582\\\\325.01\\\\80.27344\\\\296.5336\\\\325.01\\\\91.99219\\\\296.5079\\\\325.01\\\\94.33594\\\\296.473\\\\325.01\\\\101.3672\\\\296.474\\\\325.01\\\\106.0547\\\\296.4051\\\\325.01\\\\115.4297\\\\296.3986\\\\325.01\\\\122.4609\\\\296.3557\\\\325.01\\\\124.8047\\\\296.308\\\\325.01\\\\129.4922\\\\296.3139\\\\325.01\\\\134.1797\\\\296.1332\\\\325.01\\\\136.5234\\\\295.833\\\\325.01\\\\143.5547\\\\295.8172\\\\325.01\\\\148.2422\\\\295.864\\\\325.01\\\\152.9297\\\\295.7516\\\\325.01\\\\157.6172\\\\295.7849\\\\325.01\\\\169.3359\\\\295.6454\\\\325.01\\\\176.3672\\\\295.6093\\\\325.01\\\\183.3984\\\\295.6638\\\\325.01\\\\188.0859\\\\295.5742\\\\325.01\\\\192.7734\\\\295.5742\\\\325.01\\\\197.4609\\\\295.8172\\\\325.01\\\\202.1484\\\\295.7346\\\\325.01\\\\204.4922\\\\295.4287\\\\325.01\\\\206.8359\\\\294.8107\\\\325.01\\\\209.1797\\\\294.3828\\\\325.01\\\\211.5234\\\\292.5155\\\\325.01\\\\213.8672\\\\291.2065\\\\325.01\\\\216.2109\\\\289.625\\\\325.01\\\\217.2743\\\\288.6234\\\\325.01\\\\218.5547\\\\287.1312\\\\325.01\\\\221.8125\\\\283.9359\\\\325.01\\\\225.5859\\\\279.9313\\\\325.01\\\\227.9297\\\\277.6329\\\\325.01\\\\230.2734\\\\275.1294\\\\325.01\\\\233.3415\\\\272.2172\\\\325.01\\\\234.9609\\\\270.437\\\\325.01\\\\237.8862\\\\267.5297\\\\325.01\\\\244.7266\\\\260.4984\\\\325.01\\\\246.6797\\\\258.4162\\\\325.01\\\\251.4181\\\\253.4672\\\\325.01\\\\256.0547\\\\248.2155\\\\325.01\\\\257.5107\\\\246.4359\\\\325.01\\\\259.7775\\\\244.0922\\\\325.01\\\\260.7694\\\\241.7484\\\\325.01\\\\261.6139\\\\239.4047\\\\325.01\\\\260.8724\\\\237.0609\\\\325.01\\\\259.4996\\\\234.7172\\\\325.01\\\\258.3984\\\\233.6932\\\\325.01\\\\256.0547\\\\233.0686\\\\325.01\\\\253.7109\\\\232.8467\\\\325.01\\\\251.3672\\\\233.6969\\\\325.01\\\\249.0234\\\\232.8344\\\\325.01\\\\246.6797\\\\232.8072\\\\325.01\\\\241.9922\\\\233.0305\\\\325.01\\\\237.3047\\\\233.0207\\\\325.01\\\\232.6172\\\\233.0592\\\\325.01\\\\227.9297\\\\233.0305\\\\325.01\\\\220.8984\\\\233.0402\\\\325.01\\\\211.5234\\\\233.0819\\\\325.01\\\\206.8359\\\\233.0207\\\\325.01\\\\202.1484\\\\232.6477\\\\325.01\\\\197.4609\\\\232.7322\\\\325.01\\\\185.7422\\\\232.7322\\\\325.01\\\\171.6797\\\\232.7848\\\\325.01\\\\164.6484\\\\232.7186\\\\325.01\\\\159.9609\\\\232.7588\\\\325.01\\\\152.9297\\\\232.7456\\\\325.01\\\\143.5547\\\\232.7614\\\\325.01\\\\141.2109\\\\232.6477\\\\325.01\\\\138.8672\\\\231.7333\\\\325.01\\\\136.5234\\\\231.9353\\\\325.01\\\\134.1797\\\\232.6179\\\\325.01\\\\131.8359\\\\232.4552\\\\325.01\\\\129.4922\\\\231.9931\\\\325.01\\\\127.1484\\\\231.7424\\\\325.01\\\\124.8047\\\\231.7224\\\\325.01\\\\121.8516\\\\232.3734\\\\325.01\\\\120.1172\\\\232.6767\\\\325.01\\\\115.4297\\\\232.7346\\\\325.01\\\\108.3984\\\\232.6788\\\\325.01\\\\106.0547\\\\231.6083\\\\325.01\\\\103.7109\\\\231.4658\\\\325.01\\\\102.7466\\\\232.3734\\\\325.01\\\\101.3672\\\\233.1132\\\\325.01\\\\99.02344\\\\233.1397\\\\325.01\\\\89.64844\\\\233.1221\\\\325.01\\\\84.96094\\\\233.1483\\\\325.01\\\\77.92969\\\\233.131\\\\325.01\\\\70.89844\\\\233.1483\\\\325.01\\\\68.55469\\\\233.1221\\\\325.01\\\\63.86719\\\\233.1652\\\\325.01\\\\47.46094\\\\233.131\\\\325.01\\\\45.11719\\\\233.1042\\\\325.01\\\\44.13916\\\\232.3734\\\\325.01\\\\42.77344\\\\231.0852\\\\325.01\\\\41.99423\\\\230.0297\\\\325.01\\\\42.77344\\\\229.1792\\\\325.01\\\\45.11719\\\\228.4149\\\\325.01\\\\47.46094\\\\226.9069\\\\325.01\\\\49.80469\\\\225.8247\\\\325.01\\\\52.14844\\\\224.4343\\\\325.01\\\\54.49219\\\\223.4268\\\\325.01\\\\54.98124\\\\222.9984\\\\325.01\\\\59.17969\\\\219.9802\\\\325.01\\\\61.52344\\\\218.4654\\\\325.01\\\\66.21094\\\\214.7512\\\\325.01\\\\67.41649\\\\213.6234\\\\325.01\\\\72.08356\\\\208.9359\\\\325.01\\\\76.20634\\\\204.2484\\\\325.01\\\\77.92969\\\\201.635\\\\325.01\\\\81.10609\\\\197.2172\\\\325.01\\\\82.23734\\\\194.8734\\\\325.01\\\\82.61719\\\\194.4684\\\\325.01\\\\83.89715\\\\192.5297\\\\325.01\\\\87.08028\\\\185.4984\\\\325.01\\\\88.34971\\\\183.1547\\\\325.01\\\\88.75122\\\\180.8109\\\\325.01\\\\89.51726\\\\178.4672\\\\325.01\\\\90.45728\\\\176.1234\\\\325.01\\\\91.58888\\\\171.4359\\\\325.01\\\\92.52214\\\\169.0922\\\\325.01\\\\92.92603\\\\166.7484\\\\325.01\\\\93.08162\\\\164.4047\\\\325.01\\\\93.24734\\\\159.7172\\\\325.01\\\\93.34813\\\\155.0297\\\\325.01\\\\93.33585\\\\152.6859\\\\325.01\\\\93.21733\\\\147.9984\\\\325.01\\\\93.03451\\\\143.3109\\\\325.01\\\\92.80149\\\\140.9672\\\\325.01\\\\92.20603\\\\138.6234\\\\325.01\\\\91.35603\\\\136.2797\\\\325.01\\\\90.86391\\\\133.9359\\\\325.01\\\\90.26195\\\\131.5922\\\\325.01\\\\89.23982\\\\129.2484\\\\325.01\\\\88.11904\\\\124.5609\\\\325.01\\\\86.61727\\\\122.2172\\\\325.01\\\\85.77421\\\\119.8734\\\\325.01\\\\84.43931\\\\117.5297\\\\325.01\\\\83.54303\\\\115.1859\\\\325.01\\\\81.72218\\\\112.8422\\\\325.01\\\\80.63217\\\\110.4984\\\\325.01\\\\78.97623\\\\108.1547\\\\325.01\\\\76.99889\\\\105.8109\\\\325.01\\\\75.58594\\\\103.8485\\\\325.01\\\\71.2717\\\\98.77969\\\\325.01\\\\68.55469\\\\96.10746\\\\325.01\\\\63.40781\\\\91.74844\\\\325.01\\\\61.52344\\\\90.39472\\\\325.01\\\\59.17969\\\\88.40859\\\\325.01\\\\56.83594\\\\86.96328\\\\325.01\\\\54.49219\\\\85.72954\\\\325.01\\\\52.14844\\\\83.90226\\\\325.01\\\\49.80469\\\\83.13991\\\\325.01\\\\47.46094\\\\81.75262\\\\325.01\\\\45.11719\\\\80.8488\\\\325.01\\\\42.77344\\\\79.45107\\\\325.01\\\\40.42969\\\\78.81117\\\\325.01\\\\38.08594\\\\78.3781\\\\325.01\\\\35.74219\\\\77.40307\\\\325.01\\\\33.39844\\\\76.69214\\\\325.01\\\\28.71094\\\\75.71996\\\\325.01\\\\26.36719\\\\74.93087\\\\325.01\\\\24.02344\\\\74.48033\\\\325.01\\\\14.64844\\\\74.16439\\\\325.01\\\\12.30469\\\\74.15256\\\\325.01\\\\5.273438\\\\74.32497\\\\325.01\\\\2.929688\\\\74.428\\\\325.01\\\\0.5859375\\\\74.75261\\\\325.01\\\\-4.101563\\\\76.07065\\\\325.01\\\\-6.445313\\\\76.48616\\\\325.01\\\\-8.789063\\\\77.12093\\\\325.01\\\\-11.13281\\\\78.12252\\\\325.01\\\\-13.47656\\\\78.72694\\\\325.01\\\\-15.82031\\\\79.19357\\\\325.01\\\\-18.16406\\\\80.59765\\\\325.01\\\\-20.50781\\\\81.5181\\\\325.01\\\\-22.85156\\\\82.87881\\\\325.01\\\\-25.19531\\\\83.74062\\\\325.01\\\\-27.53906\\\\85.55424\\\\325.01\\\\-29.88281\\\\86.58486\\\\325.01\\\\-32.22656\\\\88.27789\\\\325.01\\\\-34.57031\\\\90.20212\\\\325.01\\\\-36.80389\\\\91.74844\\\\325.01\\\\-39.25781\\\\93.7097\\\\325.01\\\\-41.60156\\\\95.68683\\\\325.01\\\\-46.93845\\\\101.1234\\\\325.01\\\\-48.63281\\\\103.3971\\\\325.01\\\\-52.29492\\\\108.1547\\\\325.01\\\\-53.96273\\\\110.4984\\\\325.01\\\\-55.34668\\\\112.8422\\\\325.01\\\\-56.84198\\\\115.1859\\\\325.01\\\\-57.82686\\\\117.5297\\\\325.01\\\\-59.19159\\\\119.8734\\\\325.01\\\\-60.36086\\\\122.2172\\\\325.01\\\\-61.3241\\\\124.5609\\\\325.01\\\\-61.87306\\\\126.9047\\\\325.01\\\\-63.0335\\\\129.2484\\\\325.01\\\\-63.83386\\\\131.5922\\\\325.01\\\\-64.24364\\\\133.9359\\\\325.01\\\\-65.60407\\\\138.6234\\\\325.01\\\\-65.90472\\\\140.9672\\\\325.01\\\\-66.56733\\\\147.9984\\\\325.01\\\\-66.85014\\\\152.6859\\\\325.01\\\\-66.89514\\\\155.0297\\\\325.01\\\\-66.80727\\\\157.3734\\\\325.01\\\\-66.51716\\\\162.0609\\\\325.01\\\\-66.30065\\\\164.4047\\\\325.01\\\\-65.77621\\\\169.0922\\\\325.01\\\\-65.44237\\\\171.4359\\\\325.01\\\\-64.52861\\\\173.7797\\\\325.01\\\\-64.08521\\\\176.1234\\\\325.01\\\\-63.46983\\\\178.4672\\\\325.01\\\\-62.2521\\\\180.8109\\\\325.01\\\\-61.54234\\\\183.1547\\\\325.01\\\\-60.97238\\\\185.4984\\\\325.01\\\\-59.53244\\\\187.8422\\\\325.01\\\\-58.54048\\\\190.1859\\\\325.01\\\\-57.08706\\\\192.5297\\\\325.01\\\\-56.2388\\\\194.8734\\\\325.01\\\\-55.66406\\\\195.557\\\\325.01\\\\-53.32031\\\\198.94\\\\325.01\\\\-52.79036\\\\199.5609\\\\325.01\\\\-51.59609\\\\201.9047\\\\325.01\\\\-48.63281\\\\205.3395\\\\325.01\\\\-47.64024\\\\206.5922\\\\325.01\\\\-45.34446\\\\208.9359\\\\325.01\\\\-43.1875\\\\211.2797\\\\325.01\\\\-41.60156\\\\212.7042\\\\325.01\\\\-39.25781\\\\215.0065\\\\325.01\\\\-36.91406\\\\216.9296\\\\325.01\\\\-34.57031\\\\219.0371\\\\325.01\\\\-32.22656\\\\220.2086\\\\325.01\\\\-29.88281\\\\221.9258\\\\325.01\\\\-27.53906\\\\223.7543\\\\325.01\\\\-25.19531\\\\224.6046\\\\325.01\\\\-22.85156\\\\226.1349\\\\325.01\\\\-20.50781\\\\227.1316\\\\325.01\\\\-18.16406\\\\228.5423\\\\325.01\\\\-15.82031\\\\229.2136\\\\325.01\\\\-15.0185\\\\230.0297\\\\325.01\\\\-15.82031\\\\231.2992\\\\325.01\\\\-16.9148\\\\232.3734\\\\325.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851092727900001.507332422180\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"368\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"18\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-95.50781\\\\233.1042\\\\328.01\\\\-97.85156\\\\233.9217\\\\328.01\\\\-100.1953\\\\234.0411\\\\328.01\\\\-104.8828\\\\234.1505\\\\328.01\\\\-107.2266\\\\234.0891\\\\328.01\\\\-114.2578\\\\234.0855\\\\328.01\\\\-123.6328\\\\234.1407\\\\328.01\\\\-125.9766\\\\234.099\\\\328.01\\\\-135.3516\\\\234.0642\\\\328.01\\\\-142.3828\\\\234.1453\\\\328.01\\\\-147.0703\\\\234.0672\\\\328.01\\\\-158.7891\\\\234.0691\\\\328.01\\\\-165.8203\\\\234.0513\\\\328.01\\\\-170.5078\\\\234.2214\\\\328.01\\\\-177.5391\\\\234.2214\\\\328.01\\\\-179.8828\\\\234.1662\\\\328.01\\\\-186.9141\\\\234.2431\\\\328.01\\\\-189.2578\\\\234.1743\\\\328.01\\\\-191.6016\\\\234.2289\\\\328.01\\\\-196.2891\\\\234.2012\\\\328.01\\\\-200.9766\\\\234.3052\\\\328.01\\\\-203.3203\\\\234.2326\\\\328.01\\\\-208.0078\\\\234.2187\\\\328.01\\\\-212.6953\\\\234.3266\\\\328.01\\\\-215.0391\\\\234.3146\\\\328.01\\\\-222.0703\\\\234.4926\\\\328.01\\\\-224.4141\\\\234.4242\\\\328.01\\\\-229.1016\\\\234.4409\\\\328.01\\\\-233.7891\\\\234.2678\\\\328.01\\\\-236.1328\\\\234.2789\\\\328.01\\\\-240.8203\\\\234.1822\\\\328.01\\\\-243.1641\\\\234.0396\\\\328.01\\\\-247.8516\\\\233.6445\\\\328.01\\\\-250.1953\\\\233.6456\\\\328.01\\\\-252.0338\\\\234.7172\\\\328.01\\\\-252.5391\\\\236.7571\\\\328.01\\\\-254.5061\\\\234.7172\\\\328.01\\\\-254.8828\\\\234.6373\\\\328.01\\\\-257.2266\\\\235.072\\\\328.01\\\\-259.5703\\\\235.9195\\\\328.01\\\\-261.0352\\\\237.0609\\\\328.01\\\\-262.9576\\\\239.4047\\\\328.01\\\\-263.0812\\\\241.7484\\\\328.01\\\\-262.6933\\\\244.0922\\\\328.01\\\\-261.0624\\\\246.4359\\\\328.01\\\\-259.5703\\\\248.3512\\\\328.01\\\\-259.053\\\\248.7797\\\\328.01\\\\-255.4084\\\\253.4672\\\\328.01\\\\-252.5391\\\\256.6272\\\\328.01\\\\-250.1953\\\\258.8713\\\\328.01\\\\-247.8516\\\\261.3969\\\\328.01\\\\-245.5078\\\\263.5681\\\\328.01\\\\-244.0815\\\\265.1859\\\\328.01\\\\-241.6044\\\\267.5297\\\\328.01\\\\-239.4063\\\\269.8734\\\\328.01\\\\-233.7891\\\\275.4299\\\\328.01\\\\-231.4453\\\\277.8564\\\\328.01\\\\-229.8974\\\\279.2484\\\\328.01\\\\-226.7578\\\\282.5634\\\\328.01\\\\-224.4141\\\\284.7263\\\\328.01\\\\-222.0703\\\\287.2521\\\\328.01\\\\-220.5943\\\\288.6234\\\\328.01\\\\-218.3665\\\\290.9672\\\\328.01\\\\-215.0107\\\\293.3109\\\\328.01\\\\-212.6953\\\\294.6589\\\\328.01\\\\-210.3516\\\\295.2445\\\\328.01\\\\-209.8801\\\\295.6547\\\\328.01\\\\-208.0078\\\\296.512\\\\328.01\\\\-205.6641\\\\296.8998\\\\328.01\\\\-203.3203\\\\297.1414\\\\328.01\\\\-198.6328\\\\297.1488\\\\328.01\\\\-193.9453\\\\296.9968\\\\328.01\\\\-184.5703\\\\296.9452\\\\328.01\\\\-177.5391\\\\296.8713\\\\328.01\\\\-170.5078\\\\296.8886\\\\328.01\\\\-154.1016\\\\296.8744\\\\328.01\\\\-149.4141\\\\296.8513\\\\328.01\\\\-144.7266\\\\296.9097\\\\328.01\\\\-140.0391\\\\296.8603\\\\328.01\\\\-130.6641\\\\296.8439\\\\328.01\\\\-121.2891\\\\296.9065\\\\328.01\\\\-109.5703\\\\296.9339\\\\328.01\\\\-97.85156\\\\296.8641\\\\328.01\\\\-83.78906\\\\296.9234\\\\328.01\\\\-74.41406\\\\296.9234\\\\328.01\\\\-69.72656\\\\296.9706\\\\328.01\\\\-67.38281\\\\296.9428\\\\328.01\\\\-60.35156\\\\296.9447\\\\328.01\\\\-53.32031\\\\296.9145\\\\328.01\\\\-39.25781\\\\296.9377\\\\328.01\\\\-36.91406\\\\296.8789\\\\328.01\\\\-29.88281\\\\296.9095\\\\328.01\\\\-22.85156\\\\296.8973\\\\328.01\\\\-18.16406\\\\296.9841\\\\328.01\\\\-15.82031\\\\297.088\\\\328.01\\\\-13.47656\\\\296.9175\\\\328.01\\\\-11.13281\\\\296.9731\\\\328.01\\\\-8.789063\\\\296.9175\\\\328.01\\\\-1.757813\\\\296.9191\\\\328.01\\\\19.33594\\\\296.7952\\\\328.01\\\\21.67969\\\\296.8159\\\\328.01\\\\26.36719\\\\296.7533\\\\328.01\\\\31.05469\\\\296.7733\\\\328.01\\\\33.39844\\\\296.7229\\\\328.01\\\\38.08594\\\\296.7421\\\\328.01\\\\47.46094\\\\296.7324\\\\328.01\\\\49.80469\\\\296.6724\\\\328.01\\\\59.17969\\\\296.675\\\\328.01\\\\66.21094\\\\296.6107\\\\328.01\\\\68.55469\\\\296.6228\\\\328.01\\\\75.58594\\\\296.5696\\\\328.01\\\\77.92969\\\\296.5901\\\\328.01\\\\80.27344\\\\296.5208\\\\328.01\\\\82.61719\\\\296.5616\\\\328.01\\\\87.30469\\\\296.5079\\\\328.01\\\\103.7109\\\\296.4597\\\\328.01\\\\113.0859\\\\296.3846\\\\328.01\\\\117.7734\\\\296.3986\\\\328.01\\\\124.8047\\\\296.3288\\\\328.01\\\\129.4922\\\\296.3288\\\\328.01\\\\131.8359\\\\295.9477\\\\328.01\\\\134.1797\\\\296.1527\\\\328.01\\\\138.8672\\\\295.8486\\\\328.01\\\\143.5547\\\\295.9433\\\\328.01\\\\145.8984\\\\295.864\\\\328.01\\\\148.2422\\\\295.8791\\\\328.01\\\\152.9297\\\\295.8012\\\\328.01\\\\159.9609\\\\295.8012\\\\328.01\\\\164.6484\\\\295.7346\\\\328.01\\\\171.6797\\\\295.6998\\\\328.01\\\\176.3672\\\\295.6272\\\\328.01\\\\183.3984\\\\295.7173\\\\328.01\\\\188.0859\\\\295.5742\\\\328.01\\\\192.7734\\\\295.6093\\\\328.01\\\\197.4609\\\\295.8486\\\\328.01\\\\202.1484\\\\295.7516\\\\328.01\\\\204.4922\\\\295.4287\\\\328.01\\\\206.8359\\\\294.803\\\\328.01\\\\209.1797\\\\294.3828\\\\328.01\\\\211.5234\\\\292.5075\\\\328.01\\\\213.8672\\\\291.2212\\\\328.01\\\\216.2109\\\\289.6308\\\\328.01\\\\217.2743\\\\288.6234\\\\328.01\\\\218.5547\\\\287.1312\\\\328.01\\\\221.809\\\\283.9359\\\\328.01\\\\225.5859\\\\279.9599\\\\328.01\\\\227.9297\\\\277.6776\\\\328.01\\\\230.2734\\\\275.1425\\\\328.01\\\\233.3301\\\\272.2172\\\\328.01\\\\234.9609\\\\270.437\\\\328.01\\\\237.8817\\\\267.5297\\\\328.01\\\\246.9583\\\\258.1547\\\\328.01\\\\251.4177\\\\253.4672\\\\328.01\\\\256.0547\\\\248.1981\\\\328.01\\\\257.4792\\\\246.4359\\\\328.01\\\\259.8178\\\\244.0922\\\\328.01\\\\261.6764\\\\239.4047\\\\328.01\\\\260.9047\\\\237.0609\\\\328.01\\\\259.4996\\\\234.7172\\\\328.01\\\\258.3984\\\\233.6884\\\\328.01\\\\256.0547\\\\233.0779\\\\328.01\\\\253.7109\\\\232.916\\\\328.01\\\\251.3672\\\\233.9331\\\\328.01\\\\249.0234\\\\233.1868\\\\328.01\\\\246.6797\\\\232.8317\\\\328.01\\\\241.9922\\\\233.0305\\\\328.01\\\\237.3047\\\\233.0305\\\\328.01\\\\232.6172\\\\233.0819\\\\328.01\\\\227.9297\\\\233.0498\\\\328.01\\\\223.2422\\\\233.091\\\\328.01\\\\220.8984\\\\233.0632\\\\328.01\\\\211.5234\\\\233.0819\\\\328.01\\\\206.8359\\\\233.0537\\\\328.01\\\\202.1484\\\\232.6477\\\\328.01\\\\197.4609\\\\232.7456\\\\328.01\\\\188.0859\\\\232.7322\\\\328.01\\\\178.7109\\\\232.7975\\\\328.01\\\\174.0234\\\\232.7588\\\\328.01\\\\166.9922\\\\232.7588\\\\328.01\\\\162.3047\\\\232.7186\\\\328.01\\\\152.9297\\\\232.7588\\\\328.01\\\\143.5547\\\\232.8253\\\\328.01\\\\141.2109\\\\232.721\\\\328.01\\\\138.8672\\\\232.1391\\\\328.01\\\\136.5234\\\\232.6027\\\\328.01\\\\134.1797\\\\232.6477\\\\328.01\\\\131.8359\\\\232.5716\\\\328.01\\\\129.4922\\\\231.8784\\\\328.01\\\\127.1484\\\\231.7224\\\\328.01\\\\124.8047\\\\231.7315\\\\328.01\\\\122.4609\\\\232.6623\\\\328.01\\\\115.4297\\\\232.7875\\\\328.01\\\\108.3984\\\\232.7071\\\\328.01\\\\106.0547\\\\231.641\\\\328.01\\\\103.7109\\\\231.4558\\\\328.01\\\\102.6978\\\\232.3734\\\\328.01\\\\101.3672\\\\233.0951\\\\328.01\\\\99.02344\\\\233.1483\\\\328.01\\\\94.33594\\\\233.1221\\\\328.01\\\\89.64844\\\\233.1397\\\\328.01\\\\77.92969\\\\233.1221\\\\328.01\\\\70.89844\\\\233.1568\\\\328.01\\\\47.46094\\\\233.1483\\\\328.01\\\\45.11719\\\\233.1221\\\\328.01\\\\44.12626\\\\232.3734\\\\328.01\\\\42.77344\\\\231.0868\\\\328.01\\\\41.98608\\\\230.0297\\\\328.01\\\\42.77344\\\\229.1708\\\\328.01\\\\45.11719\\\\228.3863\\\\328.01\\\\47.46094\\\\226.8852\\\\328.01\\\\49.80469\\\\225.7875\\\\328.01\\\\52.14844\\\\224.4137\\\\328.01\\\\54.49219\\\\223.3972\\\\328.01\\\\54.94433\\\\222.9984\\\\328.01\\\\59.17969\\\\219.9666\\\\328.01\\\\61.52344\\\\218.3946\\\\328.01\\\\63.86719\\\\216.6084\\\\328.01\\\\66.21094\\\\214.7201\\\\328.01\\\\72.05011\\\\208.9359\\\\328.01\\\\76.14435\\\\204.2484\\\\328.01\\\\77.59603\\\\201.9047\\\\328.01\\\\77.92969\\\\201.5351\\\\328.01\\\\81.09001\\\\197.2172\\\\328.01\\\\82.18169\\\\194.8734\\\\328.01\\\\82.61719\\\\194.396\\\\328.01\\\\83.85225\\\\192.5297\\\\328.01\\\\85.99004\\\\187.8422\\\\328.01\\\\87.00566\\\\185.4984\\\\328.01\\\\88.31177\\\\183.1547\\\\328.01\\\\88.73158\\\\180.8109\\\\328.01\\\\89.43767\\\\178.4672\\\\328.01\\\\90.393\\\\176.1234\\\\328.01\\\\91.51611\\\\171.4359\\\\328.01\\\\92.42188\\\\169.0922\\\\328.01\\\\92.89885\\\\166.7484\\\\328.01\\\\93.06984\\\\164.4047\\\\328.01\\\\93.28426\\\\157.3734\\\\328.01\\\\93.30979\\\\152.6859\\\\328.01\\\\93.18761\\\\147.9984\\\\328.01\\\\93.02273\\\\143.3109\\\\328.01\\\\92.74458\\\\140.9672\\\\328.01\\\\92.09059\\\\138.6234\\\\328.01\\\\91.29035\\\\136.2797\\\\328.01\\\\90.7983\\\\133.9359\\\\328.01\\\\90.19531\\\\131.5922\\\\328.01\\\\89.1515\\\\129.2484\\\\328.01\\\\88.04228\\\\124.5609\\\\328.01\\\\86.57227\\\\122.2172\\\\328.01\\\\85.71805\\\\119.8734\\\\328.01\\\\84.35456\\\\117.5297\\\\328.01\\\\83.45424\\\\115.1859\\\\328.01\\\\81.67584\\\\112.8422\\\\328.01\\\\80.51793\\\\110.4984\\\\328.01\\\\78.89664\\\\108.1547\\\\328.01\\\\76.93359\\\\105.8109\\\\328.01\\\\75.58594\\\\104.0007\\\\328.01\\\\71.0907\\\\98.77969\\\\328.01\\\\68.55469\\\\96.27782\\\\328.01\\\\66.21094\\\\94.34207\\\\328.01\\\\63.23686\\\\91.74844\\\\328.01\\\\61.52344\\\\90.49887\\\\328.01\\\\59.17969\\\\88.51089\\\\328.01\\\\54.49219\\\\85.80405\\\\328.01\\\\52.14844\\\\84.03836\\\\328.01\\\\49.80469\\\\83.19375\\\\328.01\\\\47.46094\\\\81.85665\\\\328.01\\\\45.11719\\\\80.92008\\\\328.01\\\\43.41961\\\\80.02969\\\\328.01\\\\42.77344\\\\79.5718\\\\328.01\\\\40.42969\\\\78.83472\\\\328.01\\\\38.08594\\\\78.42924\\\\328.01\\\\35.74219\\\\77.53945\\\\328.01\\\\33.39844\\\\76.75893\\\\328.01\\\\28.71094\\\\75.81826\\\\328.01\\\\26.36719\\\\75.0248\\\\328.01\\\\24.02344\\\\74.52504\\\\328.01\\\\21.67969\\\\74.40105\\\\328.01\\\\14.64844\\\\74.17623\\\\328.01\\\\12.30469\\\\74.17031\\\\328.01\\\\7.617188\\\\74.25909\\\\328.01\\\\2.929688\\\\74.428\\\\328.01\\\\0.5859375\\\\74.71389\\\\328.01\\\\-4.101563\\\\76.07934\\\\328.01\\\\-6.445313\\\\76.51406\\\\328.01\\\\-8.789063\\\\77.13158\\\\328.01\\\\-11.13281\\\\78.13784\\\\328.01\\\\-13.47656\\\\78.72694\\\\328.01\\\\-15.82031\\\\79.21071\\\\328.01\\\\-18.16406\\\\80.62277\\\\328.01\\\\-20.50781\\\\81.54421\\\\328.01\\\\-22.85156\\\\82.91264\\\\328.01\\\\-25.19531\\\\83.73855\\\\328.01\\\\-27.53906\\\\85.55424\\\\328.01\\\\-29.88281\\\\86.58486\\\\328.01\\\\-32.22656\\\\88.27042\\\\328.01\\\\-34.57031\\\\90.21121\\\\328.01\\\\-36.82314\\\\91.74844\\\\328.01\\\\-39.25781\\\\93.68283\\\\328.01\\\\-41.60156\\\\95.69698\\\\328.01\\\\-43.94531\\\\98.10894\\\\328.01\\\\-46.95976\\\\101.1234\\\\328.01\\\\-50.47743\\\\105.8109\\\\328.01\\\\-52.28932\\\\108.1547\\\\328.01\\\\-53.96273\\\\110.4984\\\\328.01\\\\-56.82392\\\\115.1859\\\\328.01\\\\-57.82686\\\\117.5297\\\\328.01\\\\-59.17969\\\\119.8734\\\\328.01\\\\-60.35156\\\\122.2886\\\\328.01\\\\-61.31202\\\\124.5609\\\\328.01\\\\-61.84362\\\\126.9047\\\\328.01\\\\-62.97818\\\\129.2484\\\\328.01\\\\-63.81696\\\\131.5922\\\\328.01\\\\-64.22777\\\\133.9359\\\\328.01\\\\-65.57173\\\\138.6234\\\\328.01\\\\-65.88971\\\\140.9672\\\\328.01\\\\-66.54406\\\\147.9984\\\\328.01\\\\-66.83923\\\\152.6859\\\\328.01\\\\-66.87236\\\\155.0297\\\\328.01\\\\-66.8178\\\\157.3734\\\\328.01\\\\-66.51716\\\\162.0609\\\\328.01\\\\-66.31799\\\\164.4047\\\\328.01\\\\-65.7848\\\\169.0922\\\\328.01\\\\-65.44237\\\\171.4359\\\\328.01\\\\-64.53993\\\\173.7797\\\\328.01\\\\-64.09171\\\\176.1234\\\\328.01\\\\-63.46994\\\\178.4672\\\\328.01\\\\-62.26713\\\\180.8109\\\\328.01\\\\-61.52344\\\\183.1547\\\\328.01\\\\-61.02607\\\\185.4984\\\\328.01\\\\-59.56421\\\\187.8422\\\\328.01\\\\-58.60754\\\\190.1859\\\\328.01\\\\-57.1322\\\\192.5297\\\\328.01\\\\-56.30392\\\\194.8734\\\\328.01\\\\-55.66406\\\\195.646\\\\328.01\\\\-53.32031\\\\199.0141\\\\328.01\\\\-52.84091\\\\199.5609\\\\328.01\\\\-51.67393\\\\201.9047\\\\328.01\\\\-48.63281\\\\205.3802\\\\328.01\\\\-47.66054\\\\206.5922\\\\328.01\\\\-45.38484\\\\208.9359\\\\328.01\\\\-43.23581\\\\211.2797\\\\328.01\\\\-41.60156\\\\212.7535\\\\328.01\\\\-39.25781\\\\215.0201\\\\328.01\\\\-36.91406\\\\216.9578\\\\328.01\\\\-34.57031\\\\219.0567\\\\328.01\\\\-32.22656\\\\220.2589\\\\328.01\\\\-31.7772\\\\220.6547\\\\328.01\\\\-28.47822\\\\222.9984\\\\328.01\\\\-27.53906\\\\223.7774\\\\328.01\\\\-25.19531\\\\224.6141\\\\328.01\\\\-22.85156\\\\226.1618\\\\328.01\\\\-20.50781\\\\227.1643\\\\328.01\\\\-18.16406\\\\228.5648\\\\328.01\\\\-15.82031\\\\229.3696\\\\328.01\\\\-15.16846\\\\230.0297\\\\328.01\\\\-15.82031\\\\231.0044\\\\328.01\\\\-17.12821\\\\232.3734\\\\328.01\\\\-18.16406\\\\233.0441\\\\328.01\\\\-20.50781\\\\233.1397\\\\328.01\\\\-34.57031\\\\233.1568\\\\328.01\\\\-43.94531\\\\233.1397\\\\328.01\\\\-48.63281\\\\233.1736\\\\328.01\\\\-58.00781\\\\233.19\\\\328.01\\\\-60.35156\\\\233.3635\\\\328.01\\\\-62.69531\\\\233.7748\\\\328.01\\\\-65.03906\\\\232.6767\\\\328.01\\\\-69.72656\\\\232.9839\\\\328.01\\\\-81.44531\\\\233.0537\\\\328.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851113729100001.470312254320\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"562\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"19\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-15.82031\\\\230.0018\\\\331.01\\\\-18.16406\\\\232.6603\\\\331.01\\\\-20.50781\\\\233.131\\\\331.01\\\\-29.88281\\\\233.131\\\\331.01\\\\-43.94531\\\\233.1568\\\\331.01\\\\-53.32031\\\\233.1981\\\\331.01\\\\-58.00781\\\\233.19\\\\331.01\\\\-62.69531\\\\233.3017\\\\331.01\\\\-65.03906\\\\232.9665\\\\331.01\\\\-67.38281\\\\232.9081\\\\331.01\\\\-72.07031\\\\233.0441\\\\331.01\\\\-76.75781\\\\233.0382\\\\331.01\\\\-81.44531\\\\233.0859\\\\331.01\\\\-86.13281\\\\233.0671\\\\331.01\\\\-88.47656\\\\233.1042\\\\331.01\\\\-95.50781\\\\233.1132\\\\331.01\\\\-97.85156\\\\234.054\\\\331.01\\\\-104.8828\\\\234.2191\\\\331.01\\\\-107.2266\\\\234.1551\\\\331.01\\\\-111.9141\\\\234.2039\\\\331.01\\\\-114.2578\\\\234.1596\\\\331.01\\\\-123.6328\\\\234.2233\\\\331.01\\\\-125.9766\\\\234.1641\\\\331.01\\\\-135.3516\\\\234.1174\\\\331.01\\\\-142.3828\\\\234.1785\\\\331.01\\\\-147.0703\\\\234.0904\\\\331.01\\\\-158.7891\\\\234.1085\\\\331.01\\\\-165.8203\\\\234.0952\\\\331.01\\\\-168.1641\\\\234.2214\\\\331.01\\\\-177.5391\\\\234.2835\\\\331.01\\\\-182.2266\\\\234.2539\\\\331.01\\\\-186.9141\\\\234.2902\\\\331.01\\\\-189.2578\\\\234.2431\\\\331.01\\\\-191.6016\\\\234.2902\\\\331.01\\\\-198.6328\\\\234.3084\\\\331.01\\\\-200.9766\\\\234.3864\\\\331.01\\\\-203.3203\\\\234.3235\\\\331.01\\\\-208.0078\\\\234.3115\\\\331.01\\\\-212.6953\\\\234.4578\\\\331.01\\\\-215.0391\\\\234.4266\\\\331.01\\\\-219.7266\\\\234.5859\\\\331.01\\\\-222.0703\\\\234.6051\\\\331.01\\\\-224.4141\\\\234.5285\\\\331.01\\\\-229.1016\\\\234.5121\\\\331.01\\\\-233.7891\\\\234.3601\\\\331.01\\\\-238.4766\\\\234.2935\\\\331.01\\\\-243.1641\\\\234.1026\\\\331.01\\\\-245.5078\\\\233.8227\\\\331.01\\\\-247.8516\\\\233.6637\\\\331.01\\\\-250.1953\\\\233.665\\\\331.01\\\\-252.0243\\\\234.7172\\\\331.01\\\\-252.5391\\\\236.7571\\\\331.01\\\\-253.125\\\\237.0609\\\\331.01\\\\-252.5391\\\\237.1236\\\\331.01\\\\-250.1953\\\\237.9965\\\\331.01\\\\-247.8516\\\\238.0423\\\\331.01\\\\-236.1328\\\\238.0359\\\\331.01\\\\-229.1016\\\\238.0082\\\\331.01\\\\-226.7578\\\\238.0391\\\\331.01\\\\-219.7266\\\\238.0207\\\\331.01\\\\-215.0391\\\\238.0838\\\\331.01\\\\-210.3516\\\\238.0525\\\\331.01\\\\-208.0078\\\\238.1037\\\\331.01\\\\-198.6328\\\\238.1236\\\\331.01\\\\-196.2891\\\\238.1015\\\\331.01\\\\-191.6016\\\\238.1621\\\\331.01\\\\-186.9141\\\\238.0992\\\\331.01\\\\-182.2266\\\\238.1527\\\\331.01\\\\-172.8516\\\\238.1823\\\\331.01\\\\-163.4766\\\\238.1513\\\\331.01\\\\-156.4453\\\\238.192\\\\331.01\\\\-154.1016\\\\238.1706\\\\331.01\\\\-144.7266\\\\238.192\\\\331.01\\\\-142.3828\\\\238.2227\\\\331.01\\\\-137.6953\\\\238.1717\\\\331.01\\\\-133.0078\\\\238.2124\\\\331.01\\\\-128.3203\\\\238.1483\\\\331.01\\\\-125.9766\\\\238.2014\\\\331.01\\\\-116.6016\\\\238.1596\\\\331.01\\\\-100.1953\\\\238.1906\\\\331.01\\\\-93.16406\\\\238.1906\\\\331.01\\\\-88.47656\\\\238.1483\\\\331.01\\\\-79.10156\\\\238.2014\\\\331.01\\\\-74.41406\\\\238.1513\\\\331.01\\\\-69.72656\\\\238.1717\\\\331.01\\\\-62.69531\\\\238.1498\\\\331.01\\\\-58.00781\\\\238.1717\\\\331.01\\\\-48.63281\\\\238.1197\\\\331.01\\\\-43.94531\\\\238.1609\\\\331.01\\\\-41.60156\\\\238.1309\\\\331.01\\\\-29.88281\\\\238.1498\\\\331.01\\\\-25.19531\\\\238.1156\\\\331.01\\\\-15.82031\\\\238.1582\\\\331.01\\\\-8.789063\\\\238.1156\\\\331.01\\\\-6.445313\\\\238.1582\\\\331.01\\\\0.5859375\\\\238.1596\\\\331.01\\\\5.273438\\\\238.1272\\\\331.01\\\\12.30469\\\\238.1796\\\\331.01\\\\16.99219\\\\238.1695\\\\331.01\\\\24.02344\\\\238.0968\\\\331.01\\\\26.36719\\\\238.1061\\\\331.01\\\\31.05469\\\\238.0254\\\\331.01\\\\45.11719\\\\238.0254\\\\331.01\\\\49.80469\\\\237.9679\\\\331.01\\\\52.14844\\\\237.9883\\\\331.01\\\\59.17969\\\\237.9553\\\\331.01\\\\63.86719\\\\237.9632\\\\331.01\\\\66.21094\\\\237.9217\\\\331.01\\\\77.92969\\\\237.9347\\\\331.01\\\\80.27344\\\\237.8105\\\\331.01\\\\82.61719\\\\237.9372\\\\331.01\\\\84.96094\\\\237.4751\\\\331.01\\\\87.30469\\\\237.8936\\\\331.01\\\\89.64844\\\\237.9217\\\\331.01\\\\91.99219\\\\237.7291\\\\331.01\\\\94.33594\\\\237.2086\\\\331.01\\\\99.02344\\\\237.2496\\\\331.01\\\\101.3672\\\\236.9089\\\\331.01\\\\106.0547\\\\236.8813\\\\331.01\\\\110.7422\\\\236.9438\\\\331.01\\\\117.7734\\\\236.8147\\\\331.01\\\\124.8047\\\\236.8069\\\\331.01\\\\131.8359\\\\236.8627\\\\331.01\\\\134.1797\\\\236.7867\\\\331.01\\\\141.2109\\\\236.7996\\\\331.01\\\\150.5859\\\\236.7134\\\\331.01\\\\152.9297\\\\236.7413\\\\331.01\\\\157.6172\\\\236.6997\\\\331.01\\\\162.3047\\\\236.7022\\\\331.01\\\\171.6797\\\\236.6496\\\\331.01\\\\181.0547\\\\236.6756\\\\331.01\\\\185.7422\\\\236.5877\\\\331.01\\\\190.4297\\\\236.5877\\\\331.01\\\\195.1172\\\\236.5409\\\\331.01\\\\199.8047\\\\236.5758\\\\331.01\\\\206.8359\\\\236.515\\\\331.01\\\\225.5859\\\\236.4299\\\\331.01\\\\227.9297\\\\236.4541\\\\331.01\\\\239.6484\\\\236.4137\\\\331.01\\\\246.6797\\\\236.4174\\\\331.01\\\\249.0234\\\\236.3047\\\\331.01\\\\251.3672\\\\235.5069\\\\331.01\\\\253.7109\\\\236.5074\\\\331.01\\\\254.3945\\\\237.0609\\\\331.01\\\\255.8271\\\\239.4047\\\\331.01\\\\255.6086\\\\241.7484\\\\331.01\\\\254.6224\\\\244.0922\\\\331.01\\\\253.7109\\\\245.1008\\\\331.01\\\\252.7054\\\\246.4359\\\\331.01\\\\251.3672\\\\248.4433\\\\331.01\\\\248.6766\\\\251.1234\\\\331.01\\\\246.6797\\\\253.313\\\\331.01\\\\244.3359\\\\255.5254\\\\331.01\\\\239.5975\\\\260.4984\\\\331.01\\\\232.5846\\\\267.5297\\\\331.01\\\\230.2734\\\\269.8828\\\\331.01\\\\227.9297\\\\272.1213\\\\331.01\\\\225.5859\\\\274.5504\\\\331.01\\\\218.4426\\\\281.5922\\\\331.01\\\\215.9517\\\\283.9359\\\\331.01\\\\213.5485\\\\286.2797\\\\331.01\\\\211.5234\\\\288.0286\\\\331.01\\\\209.1797\\\\289.5822\\\\331.01\\\\206.8359\\\\290.8047\\\\331.01\\\\199.8047\\\\290.6986\\\\331.01\\\\197.4609\\\\290.7428\\\\331.01\\\\195.1172\\\\290.1895\\\\331.01\\\\190.4297\\\\290.0456\\\\331.01\\\\185.7422\\\\289.9906\\\\331.01\\\\169.3359\\\\290.0001\\\\331.01\\\\166.9922\\\\289.9612\\\\331.01\\\\159.9609\\\\289.9643\\\\331.01\\\\152.9297\\\\289.9091\\\\331.01\\\\134.1797\\\\289.9193\\\\331.01\\\\129.4922\\\\289.8967\\\\331.01\\\\120.1172\\\\289.9292\\\\331.01\\\\113.0859\\\\289.8948\\\\331.01\\\\108.3984\\\\289.9267\\\\331.01\\\\96.67969\\\\289.8987\\\\331.01\\\\94.33594\\\\289.9418\\\\331.01\\\\87.30469\\\\289.9091\\\\331.01\\\\84.96094\\\\289.9418\\\\331.01\\\\77.92969\\\\289.9091\\\\331.01\\\\68.55469\\\\289.9418\\\\331.01\\\\63.86719\\\\289.8967\\\\331.01\\\\56.83594\\\\289.9418\\\\331.01\\\\52.14844\\\\289.9069\\\\331.01\\\\47.46094\\\\289.9418\\\\331.01\\\\42.77344\\\\289.8967\\\\331.01\\\\35.74219\\\\289.9292\\\\331.01\\\\16.99219\\\\289.9319\\\\331.01\\\\5.273438\\\\289.8967\\\\331.01\\\\2.929688\\\\289.9418\\\\331.01\\\\-1.757813\\\\289.9193\\\\331.01\\\\-11.13281\\\\289.9739\\\\331.01\\\\-22.85156\\\\289.9643\\\\331.01\\\\-32.22656\\\\290.0094\\\\331.01\\\\-36.91406\\\\290.0001\\\\331.01\\\\-43.94531\\\\290.0409\\\\331.01\\\\-53.32031\\\\290.0094\\\\331.01\\\\-62.69531\\\\290.0409\\\\331.01\\\\-72.07031\\\\290.032\\\\331.01\\\\-74.41406\\\\290.0545\\\\331.01\\\\-76.75781\\\\290.5839\\\\331.01\\\\-79.10156\\\\290.2641\\\\331.01\\\\-81.44531\\\\290.7153\\\\331.01\\\\-83.78906\\\\290.9129\\\\331.01\\\\-86.13281\\\\290.1897\\\\331.01\\\\-88.47656\\\\290.5401\\\\331.01\\\\-90.82031\\\\291.2065\\\\331.01\\\\-93.16406\\\\291.1611\\\\331.01\\\\-95.50781\\\\291.3054\\\\331.01\\\\-97.85156\\\\291.2781\\\\331.01\\\\-104.8828\\\\291.3188\\\\331.01\\\\-107.2266\\\\291.2918\\\\331.01\\\\-111.9141\\\\291.3319\\\\331.01\\\\-114.2578\\\\291.3054\\\\331.01\\\\-121.2891\\\\291.3578\\\\331.01\\\\-125.9766\\\\291.3319\\\\331.01\\\\-133.0078\\\\291.3954\\\\331.01\\\\-175.1953\\\\291.5322\\\\331.01\\\\-182.2266\\\\291.5427\\\\331.01\\\\-191.6016\\\\291.6034\\\\331.01\\\\-200.9766\\\\291.5935\\\\331.01\\\\-205.6641\\\\292.3098\\\\331.01\\\\-208.0078\\\\292.3435\\\\331.01\\\\-210.3516\\\\292.0615\\\\331.01\\\\-211.9404\\\\290.9672\\\\331.01\\\\-212.6953\\\\290.2194\\\\331.01\\\\-215.0391\\\\288.6748\\\\331.01\\\\-219.7266\\\\284.4174\\\\331.01\\\\-229.1016\\\\275.1351\\\\331.01\\\\-231.4453\\\\272.9203\\\\331.01\\\\-232.0349\\\\272.2172\\\\331.01\\\\-233.7891\\\\270.4738\\\\331.01\\\\-236.1328\\\\268.2849\\\\331.01\\\\-238.4766\\\\265.8145\\\\331.01\\\\-240.8203\\\\263.537\\\\331.01\\\\-243.7982\\\\260.4984\\\\331.01\\\\-247.8516\\\\256.5038\\\\331.01\\\\-250.1953\\\\254.0828\\\\331.01\\\\-252.5391\\\\251.896\\\\331.01\\\\-254.8828\\\\249.3695\\\\331.01\\\\-255.6047\\\\248.7797\\\\331.01\\\\-257.5359\\\\246.4359\\\\331.01\\\\-258.3608\\\\244.0922\\\\331.01\\\\-258.3411\\\\241.7484\\\\331.01\\\\-257.8627\\\\239.4047\\\\331.01\\\\-257.2266\\\\238.6151\\\\331.01\\\\-255.1194\\\\237.0609\\\\331.01\\\\-257.2266\\\\235.0688\\\\331.01\\\\-259.5703\\\\235.9095\\\\331.01\\\\-261.0516\\\\237.0609\\\\331.01\\\\-261.9141\\\\238.0086\\\\331.01\\\\-262.9709\\\\239.4047\\\\331.01\\\\-263.0859\\\\241.7484\\\\331.01\\\\-262.7175\\\\244.0922\\\\331.01\\\\-261.9141\\\\245.282\\\\331.01\\\\-259.5703\\\\248.3058\\\\331.01\\\\-259.0049\\\\248.7797\\\\331.01\\\\-255.4084\\\\253.4672\\\\331.01\\\\-252.5391\\\\256.6166\\\\331.01\\\\-250.1953\\\\258.8892\\\\331.01\\\\-247.8516\\\\261.3909\\\\331.01\\\\-246.2402\\\\262.8422\\\\331.01\\\\-244.0969\\\\265.1859\\\\331.01\\\\-241.6044\\\\267.5297\\\\331.01\\\\-239.4219\\\\269.8734\\\\331.01\\\\-237.0058\\\\272.2172\\\\331.01\\\\-231.4453\\\\277.8564\\\\331.01\\\\-229.8974\\\\279.2484\\\\331.01\\\\-226.7578\\\\282.5503\\\\331.01\\\\-225.2042\\\\283.9359\\\\331.01\\\\-222.0703\\\\287.2617\\\\331.01\\\\-220.5943\\\\288.6234\\\\331.01\\\\-218.3789\\\\290.9672\\\\331.01\\\\-215.0295\\\\293.3109\\\\331.01\\\\-212.6953\\\\294.6589\\\\331.01\\\\-210.3516\\\\295.2362\\\\331.01\\\\-209.869\\\\295.6547\\\\331.01\\\\-208.0078\\\\296.4917\\\\331.01\\\\-205.6641\\\\296.8928\\\\331.01\\\\-203.3203\\\\297.1297\\\\331.01\\\\-196.2891\\\\297.0883\\\\331.01\\\\-193.9453\\\\296.959\\\\331.01\\\\-182.2266\\\\296.9206\\\\331.01\\\\-177.5391\\\\296.8568\\\\331.01\\\\-165.8203\\\\296.85\\\\331.01\\\\-158.7891\\\\296.8757\\\\331.01\\\\-151.7578\\\\296.8266\\\\331.01\\\\-144.7266\\\\296.8852\\\\331.01\\\\-140.0391\\\\296.8437\\\\331.01\\\\-135.3516\\\\296.8618\\\\331.01\\\\-130.6641\\\\296.8089\\\\331.01\\\\-109.5703\\\\296.9242\\\\331.01\\\\-97.85156\\\\296.8545\\\\331.01\\\\-90.82031\\\\296.8927\\\\331.01\\\\-83.78906\\\\296.8837\\\\331.01\\\\-76.75781\\\\296.9409\\\\331.01\\\\-55.66406\\\\296.934\\\\331.01\\\\-50.97656\\\\296.9053\\\\331.01\\\\-43.94531\\\\296.9191\\\\331.01\\\\-39.25781\\\\297.0516\\\\331.01\\\\-36.91406\\\\296.8688\\\\331.01\\\\-32.22656\\\\296.9207\\\\331.01\\\\-27.53906\\\\296.8888\\\\331.01\\\\-25.19531\\\\297.2621\\\\331.01\\\\-22.85156\\\\296.8985\\\\331.01\\\\-18.16406\\\\296.9489\\\\331.01\\\\-15.82031\\\\297.2814\\\\331.01\\\\-13.47656\\\\296.9067\\\\331.01\\\\-8.789063\\\\296.9285\\\\331.01\\\\7.617188\\\\296.8574\\\\331.01\\\\12.30469\\\\296.8579\\\\331.01\\\\21.67969\\\\296.7843\\\\331.01\\\\38.08594\\\\296.734\\\\331.01\\\\54.49219\\\\296.7043\\\\331.01\\\\59.17969\\\\296.6518\\\\331.01\\\\70.89844\\\\296.6068\\\\331.01\\\\80.27344\\\\296.549\\\\331.01\\\\87.30469\\\\296.5412\\\\331.01\\\\94.33594\\\\296.5005\\\\331.01\\\\101.3672\\\\296.4257\\\\331.01\\\\103.7109\\\\296.4801\\\\331.01\\\\115.4297\\\\296.3782\\\\331.01\\\\124.8047\\\\296.3703\\\\331.01\\\\129.4922\\\\296.3288\\\\331.01\\\\131.8359\\\\295.9433\\\\331.01\\\\134.1797\\\\295.9857\\\\331.01\\\\138.8672\\\\295.864\\\\331.01\\\\143.5547\\\\295.939\\\\331.01\\\\145.8984\\\\295.864\\\\331.01\\\\152.9297\\\\295.8172\\\\331.01\\\\159.9609\\\\295.8172\\\\331.01\\\\164.6484\\\\295.7516\\\\331.01\\\\171.6797\\\\295.7173\\\\331.01\\\\176.3672\\\\295.6454\\\\331.01\\\\183.3984\\\\295.6998\\\\331.01\\\\188.0859\\\\295.5742\\\\331.01\\\\192.7734\\\\295.6093\\\\331.01\\\\197.4609\\\\295.8486\\\\331.01\\\\202.1484\\\\295.7849\\\\331.01\\\\204.4922\\\\295.4439\\\\331.01\\\\206.8359\\\\294.8185\\\\331.01\\\\209.1797\\\\294.3828\\\\331.01\\\\211.5234\\\\292.5155\\\\331.01\\\\213.8672\\\\291.2065\\\\331.01\\\\216.2109\\\\289.6192\\\\331.01\\\\217.2678\\\\288.6234\\\\331.01\\\\218.5547\\\\287.1143\\\\331.01\\\\221.8249\\\\283.9359\\\\331.01\\\\225.5859\\\\279.9988\\\\331.01\\\\227.9297\\\\277.6831\\\\331.01\\\\230.2734\\\\275.1513\\\\331.01\\\\233.3252\\\\272.2172\\\\331.01\\\\234.9609\\\\270.4053\\\\331.01\\\\237.8731\\\\267.5297\\\\331.01\\\\240.1012\\\\265.1859\\\\331.01\\\\242.4443\\\\262.8422\\\\331.01\\\\251.4558\\\\253.4672\\\\331.01\\\\253.4477\\\\251.1234\\\\331.01\\\\256.0547\\\\248.2287\\\\331.01\\\\257.4762\\\\246.4359\\\\331.01\\\\259.8178\\\\244.0922\\\\331.01\\\\261.7753\\\\239.4047\\\\331.01\\\\261.0251\\\\237.0609\\\\331.01\\\\259.4941\\\\234.7172\\\\331.01\\\\258.3984\\\\233.6932\\\\331.01\\\\256.0547\\\\233.0961\\\\331.01\\\\253.7109\\\\232.9379\\\\331.01\\\\251.3672\\\\233.7695\\\\331.01\\\\249.0234\\\\232.8723\\\\331.01\\\\246.6797\\\\232.8195\\\\331.01\\\\241.9922\\\\233.0305\\\\331.01\\\\237.3047\\\\233.0305\\\\331.01\\\\232.6172\\\\233.1001\\\\331.01\\\\227.9297\\\\233.0686\\\\331.01\\\\225.5859\\\\233.1001\\\\331.01\\\\220.8984\\\\233.0726\\\\331.01\\\\211.5234\\\\233.0726\\\\331.01\\\\206.8359\\\\233.0441\\\\331.01\\\\202.1484\\\\232.6179\\\\331.01\\\\195.1172\\\\232.7456\\\\331.01\\\\178.7109\\\\232.7719\\\\331.01\\\\174.0234\\\\232.7456\\\\331.01\\\\162.3047\\\\232.7322\\\\331.01\\\\157.6172\\\\232.7875\\\\331.01\\\\150.5859\\\\232.8253\\\\331.01\\\\143.5547\\\\232.8253\\\\331.01\\\\138.8672\\\\232.6643\\\\331.01\\\\131.8359\\\\232.6477\\\\331.01\\\\129.4922\\\\231.969\\\\331.01\\\\127.1484\\\\231.7258\\\\331.01\\\\124.8047\\\\231.7518\\\\331.01\\\\122.4609\\\\232.4013\\\\331.01\\\\120.1172\\\\232.6788\\\\331.01\\\\113.0859\\\\232.7875\\\\331.01\\\\108.3984\\\\232.7481\\\\331.01\\\\103.7109\\\\231.505\\\\331.01\\\\102.6762\\\\232.3734\\\\331.01\\\\101.3672\\\\233.0766\\\\331.01\\\\99.02344\\\\233.1483\\\\331.01\\\\96.67969\\\\233.1221\\\\331.01\\\\89.64844\\\\233.1483\\\\331.01\\\\84.96094\\\\233.1221\\\\331.01\\\\80.27344\\\\233.1483\\\\331.01\\\\75.58594\\\\233.131\\\\331.01\\\\61.52344\\\\233.1397\\\\331.01\\\\59.17969\\\\233.1736\\\\331.01\\\\54.49219\\\\233.131\\\\331.01\\\\47.46094\\\\233.1397\\\\331.01\\\\45.11719\\\\233.1132\\\\331.01\\\\44.1435\\\\232.3734\\\\331.01\\\\42.77344\\\\231.0613\\\\331.01\\\\42.00247\\\\230.0297\\\\331.01\\\\42.77344\\\\229.1878\\\\331.01\\\\45.11719\\\\228.3765\\\\331.01\\\\47.46094\\\\226.8637\\\\331.01\\\\49.80469\\\\225.7647\\\\331.01\\\\52.14844\\\\224.4265\\\\331.01\\\\54.49219\\\\223.3249\\\\331.01\\\\59.17969\\\\219.9393\\\\331.01\\\\61.52344\\\\218.3393\\\\331.01\\\\63.86719\\\\216.5828\\\\331.01\\\\66.21094\\\\214.673\\\\331.01\\\\67.32799\\\\213.6234\\\\331.01\\\\72.01612\\\\208.9359\\\\331.01\\\\74.09509\\\\206.5922\\\\331.01\\\\76.07555\\\\204.2484\\\\331.01\\\\77.53638\\\\201.9047\\\\331.01\\\\77.92969\\\\201.4729\\\\331.01\\\\81.07359\\\\197.2172\\\\331.01\\\\82.12826\\\\194.8734\\\\331.01\\\\82.61719\\\\194.3306\\\\331.01\\\\83.8237\\\\192.5297\\\\331.01\\\\84.79842\\\\190.1859\\\\331.01\\\\85.95763\\\\187.8422\\\\331.01\\\\86.92441\\\\185.4984\\\\331.01\\\\88.28735\\\\183.1547\\\\331.01\\\\88.71695\\\\180.8109\\\\331.01\\\\89.3761\\\\178.4672\\\\331.01\\\\90.33585\\\\176.1234\\\\331.01\\\\90.93139\\\\173.7797\\\\331.01\\\\91.42718\\\\171.4359\\\\331.01\\\\92.2997\\\\169.0922\\\\331.01\\\\92.84734\\\\166.7484\\\\331.01\\\\93.04629\\\\164.4047\\\\331.01\\\\93.26022\\\\157.3734\\\\331.01\\\\93.2657\\\\152.6859\\\\331.01\\\\93.16406\\\\147.9984\\\\331.01\\\\92.9856\\\\143.3109\\\\331.01\\\\92.63075\\\\140.9672\\\\331.01\\\\91.86198\\\\138.6234\\\\331.01\\\\91.20483\\\\136.2797\\\\331.01\\\\90.71938\\\\133.9359\\\\331.01\\\\90.07662\\\\131.5922\\\\331.01\\\\89.03426\\\\129.2484\\\\331.01\\\\88.59089\\\\126.9047\\\\331.01\\\\87.9126\\\\124.5609\\\\331.01\\\\86.48499\\\\122.2172\\\\331.01\\\\85.62569\\\\119.8734\\\\331.01\\\\84.24227\\\\117.5297\\\\331.01\\\\83.30611\\\\115.1859\\\\331.01\\\\81.58318\\\\112.8422\\\\331.01\\\\80.28274\\\\110.4984\\\\331.01\\\\78.78014\\\\108.1547\\\\331.01\\\\76.8465\\\\105.8109\\\\331.01\\\\75.58594\\\\104.1452\\\\331.01\\\\70.89844\\\\98.8085\\\\331.01\\\\68.55469\\\\96.42617\\\\331.01\\\\66.21094\\\\94.49095\\\\331.01\\\\63.14011\\\\91.74844\\\\331.01\\\\61.52344\\\\90.55122\\\\331.01\\\\59.17969\\\\88.55907\\\\331.01\\\\56.83594\\\\87.2702\\\\331.01\\\\54.49219\\\\85.83469\\\\331.01\\\\52.14844\\\\84.10928\\\\331.01\\\\49.80469\\\\83.22182\\\\331.01\\\\47.46094\\\\81.90617\\\\331.01\\\\45.11719\\\\80.96379\\\\331.01\\\\43.32682\\\\80.02969\\\\331.01\\\\42.77344\\\\79.62586\\\\331.01\\\\40.42969\\\\78.84627\\\\331.01\\\\38.08594\\\\78.46938\\\\331.01\\\\33.39844\\\\76.78945\\\\331.01\\\\28.71094\\\\75.85264\\\\331.01\\\\26.36719\\\\75.06985\\\\331.01\\\\24.02344\\\\74.54173\\\\331.01\\\\21.67969\\\\74.41319\\\\331.01\\\\14.64844\\\\74.19387\\\\331.01\\\\12.30469\\\\74.18209\\\\331.01\\\\7.617188\\\\74.26453\\\\331.01\\\\2.929688\\\\74.44027\\\\331.01\\\\0.5859375\\\\74.71766\\\\331.01\\\\-4.101563\\\\76.07934\\\\331.01\\\\-6.445313\\\\76.52496\\\\331.01\\\\-8.789063\\\\77.12093\\\\331.01\\\\-11.13281\\\\78.13784\\\\331.01\\\\-15.82031\\\\79.21476\\\\331.01\\\\-18.16406\\\\80.64018\\\\331.01\\\\-20.50781\\\\81.58089\\\\331.01\\\\-22.85156\\\\82.93452\\\\331.01\\\\-25.19531\\\\83.7171\\\\331.01\\\\-27.53906\\\\85.53682\\\\331.01\\\\-29.88281\\\\86.56181\\\\331.01\\\\-32.22656\\\\88.24535\\\\331.01\\\\-34.57031\\\\90.17438\\\\331.01\\\\-36.86355\\\\91.74844\\\\331.01\\\\-39.25781\\\\93.65373\\\\331.01\\\\-41.60156\\\\95.64732\\\\331.01\\\\-43.94531\\\\98.08269\\\\331.01\\\\-46.99951\\\\101.1234\\\\331.01\\\\-50.52413\\\\105.8109\\\\331.01\\\\-50.97656\\\\106.3405\\\\331.01\\\\-53.99293\\\\110.4984\\\\331.01\\\\-56.84198\\\\115.1859\\\\331.01\\\\-57.84288\\\\117.5297\\\\331.01\\\\-59.19744\\\\119.8734\\\\331.01\\\\-60.39734\\\\122.2172\\\\331.01\\\\-61.32914\\\\124.5609\\\\331.01\\\\-61.87757\\\\126.9047\\\\331.01\\\\-63.0335\\\\129.2484\\\\331.01\\\\-63.83955\\\\131.5922\\\\331.01\\\\-64.25171\\\\133.9359\\\\331.01\\\\-65.59342\\\\138.6234\\\\331.01\\\\-66.13319\\\\143.3109\\\\331.01\\\\-66.54406\\\\147.9984\\\\331.01\\\\-66.85014\\\\152.6859\\\\331.01\\\\-66.87236\\\\155.0297\\\\331.01\\\\-66.69005\\\\159.7172\\\\331.01\\\\-66.30065\\\\164.4047\\\\331.01\\\\-65.77621\\\\169.0922\\\\331.01\\\\-65.40382\\\\171.4359\\\\331.01\\\\-64.53993\\\\173.7797\\\\331.01\\\\-64.07877\\\\176.1234\\\\331.01\\\\-63.49142\\\\178.4672\\\\331.01\\\\-62.30469\\\\180.8109\\\\331.01\\\\-61.53546\\\\183.1547\\\\331.01\\\\-61.04526\\\\185.4984\\\\331.01\\\\-59.60582\\\\187.8422\\\\331.01\\\\-58.66488\\\\190.1859\\\\331.01\\\\-57.16698\\\\192.5297\\\\331.01\\\\-56.36295\\\\194.8734\\\\331.01\\\\-55.66406\\\\195.7081\\\\331.01\\\\-53.32031\\\\199.0835\\\\331.01\\\\-52.89063\\\\199.5609\\\\331.01\\\\-51.73193\\\\201.9047\\\\331.01\\\\-49.63058\\\\204.2484\\\\331.01\\\\-47.6863\\\\206.5922\\\\331.01\\\\-45.43213\\\\208.9359\\\\331.01\\\\-43.27911\\\\211.2797\\\\331.01\\\\-41.60156\\\\212.8009\\\\331.01\\\\-39.25781\\\\215.0557\\\\331.01\\\\-36.91406\\\\216.9837\\\\331.01\\\\-34.57031\\\\219.0804\\\\331.01\\\\-32.22656\\\\220.3119\\\\331.01\\\\-29.88281\\\\221.9844\\\\331.01\\\\-27.53906\\\\223.8002\\\\331.01\\\\-25.19531\\\\224.6377\\\\331.01\\\\-22.85156\\\\226.1792\\\\331.01\\\\-20.50781\\\\227.1868\\\\331.01\\\\-18.16406\\\\228.6441\\\\331.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851133730200001.543478973601\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n";
const char* k_rtStruct_json05 =
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"567\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"20\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-15.98772\\\\230.0297\\\\334.01\\\\-18.16406\\\\232.5688\\\\334.01\\\\-20.50781\\\\233.1397\\\\334.01\\\\-34.57031\\\\233.1397\\\\334.01\\\\-41.60156\\\\233.1981\\\\334.01\\\\-46.28906\\\\233.1652\\\\334.01\\\\-50.97656\\\\233.2061\\\\334.01\\\\-62.69531\\\\233.1981\\\\334.01\\\\-67.38281\\\\232.9839\\\\334.01\\\\-72.07031\\\\233.0766\\\\334.01\\\\-81.44531\\\\233.1042\\\\334.01\\\\-86.13281\\\\233.0766\\\\334.01\\\\-90.82031\\\\233.131\\\\334.01\\\\-93.16406\\\\233.4871\\\\334.01\\\\-95.50781\\\\233.1884\\\\334.01\\\\-97.85156\\\\234.1127\\\\334.01\\\\-104.8828\\\\234.2828\\\\334.01\\\\-107.2266\\\\234.215\\\\334.01\\\\-109.5703\\\\234.2828\\\\334.01\\\\-116.6016\\\\234.2703\\\\334.01\\\\-121.2891\\\\234.303\\\\334.01\\\\-130.6641\\\\234.2081\\\\334.01\\\\-133.0078\\\\234.2426\\\\334.01\\\\-137.6953\\\\234.2081\\\\334.01\\\\-142.3828\\\\234.3198\\\\334.01\\\\-147.0703\\\\234.2191\\\\334.01\\\\-156.4453\\\\234.1685\\\\334.01\\\\-158.7891\\\\234.2273\\\\334.01\\\\-161.1328\\\\234.1641\\\\334.01\\\\-165.8203\\\\234.1641\\\\334.01\\\\-168.1641\\\\234.3426\\\\334.01\\\\-175.1953\\\\234.3328\\\\334.01\\\\-177.5391\\\\234.381\\\\334.01\\\\-179.8828\\\\234.3426\\\\334.01\\\\-186.9141\\\\234.3837\\\\334.01\\\\-189.2578\\\\234.3359\\\\334.01\\\\-193.9453\\\\234.4194\\\\334.01\\\\-198.6328\\\\234.4053\\\\334.01\\\\-200.9766\\\\234.4535\\\\334.01\\\\-203.3203\\\\234.4053\\\\334.01\\\\-208.0078\\\\234.4078\\\\334.01\\\\-212.6953\\\\234.5484\\\\334.01\\\\-215.0391\\\\234.5301\\\\334.01\\\\-219.7266\\\\234.6855\\\\334.01\\\\-224.4141\\\\234.6051\\\\334.01\\\\-231.4453\\\\234.5285\\\\334.01\\\\-243.1641\\\\234.1437\\\\334.01\\\\-247.8516\\\\233.6847\\\\334.01\\\\-250.1953\\\\233.672\\\\334.01\\\\-252.0368\\\\234.7172\\\\334.01\\\\-252.5391\\\\236.6006\\\\334.01\\\\-253.2552\\\\237.0609\\\\334.01\\\\-252.5391\\\\237.1579\\\\334.01\\\\-250.1953\\\\238.0169\\\\334.01\\\\-247.8516\\\\238.0779\\\\334.01\\\\-243.1641\\\\238.0359\\\\334.01\\\\-222.0703\\\\238.0611\\\\334.01\\\\-215.0391\\\\238.1343\\\\334.01\\\\-210.3516\\\\238.1037\\\\334.01\\\\-203.3203\\\\238.1419\\\\334.01\\\\-193.9453\\\\238.1633\\\\334.01\\\\-186.9141\\\\238.0901\\\\334.01\\\\-175.1953\\\\238.2427\\\\334.01\\\\-163.4766\\\\238.1727\\\\334.01\\\\-156.4453\\\\238.2124\\\\334.01\\\\-144.7266\\\\238.192\\\\334.01\\\\-142.3828\\\\238.2328\\\\334.01\\\\-137.6953\\\\238.1927\\\\334.01\\\\-114.2578\\\\238.2009\\\\334.01\\\\-109.5703\\\\238.1609\\\\334.01\\\\-102.5391\\\\238.202\\\\334.01\\\\-97.85156\\\\238.1609\\\\334.01\\\\-93.16406\\\\238.1906\\\\334.01\\\\-88.47656\\\\238.1369\\\\334.01\\\\-86.13281\\\\238.1695\\\\334.01\\\\-81.44531\\\\238.1483\\\\334.01\\\\-76.75781\\\\238.1913\\\\334.01\\\\-67.38281\\\\238.1386\\\\334.01\\\\-65.03906\\\\238.1596\\\\334.01\\\\-53.32031\\\\238.1197\\\\334.01\\\\-43.94531\\\\238.1498\\\\334.01\\\\-32.22656\\\\238.1197\\\\334.01\\\\-25.19531\\\\238.1483\\\\334.01\\\\-22.85156\\\\238.1084\\\\334.01\\\\-18.16406\\\\238.1483\\\\334.01\\\\-8.789063\\\\238.1156\\\\334.01\\\\-1.757813\\\\238.1483\\\\334.01\\\\2.929688\\\\238.1177\\\\334.01\\\\14.64844\\\\238.1369\\\\334.01\\\\28.71094\\\\238.073\\\\334.01\\\\42.77344\\\\237.9922\\\\334.01\\\\47.46094\\\\238.0047\\\\334.01\\\\54.49219\\\\237.9503\\\\334.01\\\\61.52344\\\\237.9583\\\\334.01\\\\66.21094\\\\237.9084\\\\334.01\\\\70.89844\\\\237.9294\\\\334.01\\\\77.92969\\\\237.9084\\\\334.01\\\\82.61719\\\\237.9318\\\\334.01\\\\84.96094\\\\237.4483\\\\334.01\\\\87.30469\\\\237.4347\\\\334.01\\\\89.64844\\\\237.735\\\\334.01\\\\91.99219\\\\236.9602\\\\334.01\\\\96.67969\\\\236.8598\\\\334.01\\\\99.02344\\\\236.9089\\\\334.01\\\\103.7109\\\\236.9145\\\\334.01\\\\108.3984\\\\236.8773\\\\334.01\\\\113.0859\\\\236.9438\\\\334.01\\\\117.7734\\\\236.8147\\\\334.01\\\\127.1484\\\\236.8051\\\\334.01\\\\131.8359\\\\236.88\\\\334.01\\\\136.5234\\\\236.7721\\\\334.01\\\\141.2109\\\\236.7996\\\\334.01\\\\143.5547\\\\236.7556\\\\334.01\\\\152.9297\\\\236.7134\\\\334.01\\\\155.2734\\\\236.673\\\\334.01\\\\159.9609\\\\236.7413\\\\334.01\\\\174.0234\\\\236.612\\\\334.01\\\\176.3672\\\\236.6756\\\\334.01\\\\181.0547\\\\236.6625\\\\334.01\\\\185.7422\\\\236.564\\\\334.01\\\\190.4297\\\\236.5997\\\\334.01\\\\197.4609\\\\236.5524\\\\334.01\\\\199.8047\\\\236.5758\\\\334.01\\\\206.8359\\\\236.493\\\\334.01\\\\225.5859\\\\236.4299\\\\334.01\\\\244.3359\\\\236.4273\\\\334.01\\\\249.0234\\\\236.379\\\\334.01\\\\251.3672\\\\235.5749\\\\334.01\\\\253.7109\\\\236.4964\\\\334.01\\\\254.4123\\\\237.0609\\\\334.01\\\\255.858\\\\239.4047\\\\334.01\\\\255.6086\\\\241.7484\\\\334.01\\\\254.6009\\\\244.0922\\\\334.01\\\\253.7109\\\\245.0623\\\\334.01\\\\252.6891\\\\246.4359\\\\334.01\\\\251.3672\\\\248.4433\\\\334.01\\\\248.6766\\\\251.1234\\\\334.01\\\\246.6797\\\\253.2569\\\\334.01\\\\244.3359\\\\255.5082\\\\334.01\\\\241.9922\\\\258.0435\\\\334.01\\\\239.6484\\\\260.3833\\\\334.01\\\\237.3047\\\\262.8111\\\\334.01\\\\234.9609\\\\265.1107\\\\334.01\\\\232.6172\\\\267.5191\\\\334.01\\\\225.5957\\\\274.5609\\\\334.01\\\\220.7587\\\\279.2484\\\\334.01\\\\218.4622\\\\281.5922\\\\334.01\\\\215.9334\\\\283.9359\\\\334.01\\\\213.5136\\\\286.2797\\\\334.01\\\\211.5234\\\\288.0286\\\\334.01\\\\209.1797\\\\289.5707\\\\334.01\\\\206.8359\\\\290.8047\\\\334.01\\\\204.4922\\\\290.8047\\\\334.01\\\\199.8047\\\\290.7131\\\\334.01\\\\197.4609\\\\290.7579\\\\334.01\\\\195.1172\\\\290.4285\\\\334.01\\\\192.7734\\\\290.6412\\\\334.01\\\\190.4297\\\\290.0366\\\\334.01\\\\183.3984\\\\289.9869\\\\334.01\\\\176.3672\\\\290.0042\\\\334.01\\\\169.3359\\\\289.9643\\\\334.01\\\\159.9609\\\\289.9643\\\\334.01\\\\155.2734\\\\289.9193\\\\334.01\\\\150.5859\\\\289.9418\\\\334.01\\\\145.8984\\\\289.8967\\\\334.01\\\\136.5234\\\\289.9516\\\\334.01\\\\131.8359\\\\289.9091\\\\334.01\\\\124.8047\\\\289.939\\\\334.01\\\\110.7422\\\\289.9091\\\\334.01\\\\96.67969\\\\289.9193\\\\334.01\\\\91.99219\\\\289.9418\\\\334.01\\\\77.92969\\\\289.9217\\\\334.01\\\\63.86719\\\\289.9193\\\\334.01\\\\54.49219\\\\289.9418\\\\334.01\\\\52.14844\\\\289.9169\\\\334.01\\\\45.11719\\\\289.9292\\\\334.01\\\\33.39844\\\\289.8967\\\\334.01\\\\28.71094\\\\289.939\\\\334.01\\\\19.33594\\\\289.9319\\\\334.01\\\\16.99219\\\\289.9546\\\\334.01\\\\7.617188\\\\289.9193\\\\334.01\\\\2.929688\\\\289.9447\\\\334.01\\\\-1.757813\\\\289.9193\\\\334.01\\\\-11.13281\\\\289.9643\\\\334.01\\\\-15.82031\\\\289.9418\\\\334.01\\\\-20.50781\\\\289.9832\\\\334.01\\\\-32.22656\\\\289.9925\\\\334.01\\\\-34.57031\\\\289.9739\\\\334.01\\\\-41.60156\\\\290.0363\\\\334.01\\\\-46.28906\\\\290.0001\\\\334.01\\\\-55.66406\\\\290.0275\\\\334.01\\\\-60.35156\\\\290.0094\\\\334.01\\\\-67.38281\\\\290.0366\\\\334.01\\\\-69.72656\\\\290.8044\\\\334.01\\\\-72.07031\\\\290.032\\\\334.01\\\\-74.41406\\\\290.0632\\\\334.01\\\\-76.75781\\\\291.0641\\\\334.01\\\\-79.10156\\\\290.6554\\\\334.01\\\\-81.44531\\\\291.2212\\\\334.01\\\\-86.13281\\\\291.2357\\\\334.01\\\\-88.47656\\\\290.1748\\\\334.01\\\\-90.82031\\\\291.2501\\\\334.01\\\\-102.5391\\\\291.2918\\\\334.01\\\\-104.8828\\\\291.2642\\\\334.01\\\\-107.4707\\\\290.9672\\\\334.01\\\\-109.5703\\\\291.3054\\\\334.01\\\\-114.2578\\\\291.3054\\\\334.01\\\\-118.9453\\\\291.3578\\\\334.01\\\\-125.9766\\\\291.345\\\\334.01\\\\-130.6641\\\\291.3954\\\\334.01\\\\-140.0391\\\\291.3954\\\\334.01\\\\-151.7578\\\\291.4549\\\\334.01\\\\-163.4766\\\\291.4776\\\\334.01\\\\-168.1641\\\\291.5108\\\\334.01\\\\-184.5703\\\\291.5531\\\\334.01\\\\-189.2578\\\\291.5836\\\\334.01\\\\-196.2891\\\\291.5836\\\\334.01\\\\-198.6328\\\\291.8617\\\\334.01\\\\-200.9766\\\\291.6205\\\\334.01\\\\-203.3203\\\\292.0964\\\\334.01\\\\-205.6641\\\\292.3344\\\\334.01\\\\-208.0078\\\\292.3654\\\\334.01\\\\-210.3516\\\\292.0912\\\\334.01\\\\-212.1875\\\\290.9672\\\\334.01\\\\-215.0391\\\\288.6954\\\\334.01\\\\-217.3828\\\\286.5314\\\\334.01\\\\-219.7266\\\\284.4869\\\\334.01\\\\-222.0703\\\\282.1065\\\\334.01\\\\-225.0175\\\\279.2484\\\\334.01\\\\-227.3079\\\\276.9047\\\\334.01\\\\-229.8221\\\\274.5609\\\\334.01\\\\-231.4453\\\\272.9496\\\\334.01\\\\-232.0564\\\\272.2172\\\\334.01\\\\-233.7891\\\\270.4845\\\\334.01\\\\-236.1328\\\\268.3081\\\\334.01\\\\-238.4766\\\\265.837\\\\334.01\\\\-240.8203\\\\263.5652\\\\334.01\\\\-243.8257\\\\260.4984\\\\334.01\\\\-246.2413\\\\258.1547\\\\334.01\\\\-248.4672\\\\255.8109\\\\334.01\\\\-250.9794\\\\253.4672\\\\334.01\\\\-252.5391\\\\251.9189\\\\334.01\\\\-254.8828\\\\249.4032\\\\334.01\\\\-255.6368\\\\248.7797\\\\334.01\\\\-257.6019\\\\246.4359\\\\334.01\\\\-258.3731\\\\244.0922\\\\334.01\\\\-258.3472\\\\241.7484\\\\334.01\\\\-257.8529\\\\239.4047\\\\334.01\\\\-257.2266\\\\238.6262\\\\334.01\\\\-255.1592\\\\237.0609\\\\334.01\\\\-257.2266\\\\235.0319\\\\334.01\\\\-259.5703\\\\235.8942\\\\334.01\\\\-261.0516\\\\237.0609\\\\334.01\\\\-261.9141\\\\237.997\\\\334.01\\\\-262.9828\\\\239.4047\\\\334.01\\\\-263.0954\\\\241.7484\\\\334.01\\\\-262.7562\\\\244.0922\\\\334.01\\\\-261.9141\\\\245.3244\\\\334.01\\\\-259.5703\\\\248.3447\\\\334.01\\\\-259.0576\\\\248.7797\\\\334.01\\\\-255.4217\\\\253.4672\\\\334.01\\\\-252.5391\\\\256.6166\\\\334.01\\\\-250.1953\\\\258.8944\\\\334.01\\\\-247.8516\\\\261.3969\\\\334.01\\\\-245.5078\\\\263.5919\\\\334.01\\\\-244.0908\\\\265.1859\\\\334.01\\\\-241.5988\\\\267.5297\\\\334.01\\\\-239.4375\\\\269.8734\\\\334.01\\\\-237\\\\272.2172\\\\334.01\\\\-231.4453\\\\277.8564\\\\334.01\\\\-229.8857\\\\279.2484\\\\334.01\\\\-227.7252\\\\281.5922\\\\334.01\\\\-225.2042\\\\283.9359\\\\334.01\\\\-222.0703\\\\287.2521\\\\334.01\\\\-220.5943\\\\288.6234\\\\334.01\\\\-218.3814\\\\290.9672\\\\334.01\\\\-215.0391\\\\293.3202\\\\334.01\\\\-212.6953\\\\294.6652\\\\334.01\\\\-210.3516\\\\295.2812\\\\334.01\\\\-209.947\\\\295.6547\\\\334.01\\\\-208.0078\\\\296.5075\\\\334.01\\\\-205.6641\\\\296.8852\\\\334.01\\\\-203.3203\\\\297.1267\\\\334.01\\\\-196.2891\\\\297.0739\\\\334.01\\\\-193.9453\\\\296.9265\\\\334.01\\\\-182.2266\\\\296.9128\\\\334.01\\\\-179.8828\\\\296.8785\\\\334.01\\\\-170.5078\\\\296.8576\\\\334.01\\\\-156.4453\\\\296.8931\\\\334.01\\\\-151.7578\\\\296.8432\\\\334.01\\\\-147.0703\\\\296.894\\\\334.01\\\\-140.0391\\\\296.8608\\\\334.01\\\\-135.3516\\\\296.8709\\\\334.01\\\\-130.6641\\\\296.8266\\\\334.01\\\\-118.9453\\\\296.8981\\\\334.01\\\\-114.2578\\\\296.8981\\\\334.01\\\\-109.5703\\\\296.942\\\\334.01\\\\-97.85156\\\\296.8828\\\\334.01\\\\-86.13281\\\\296.9218\\\\334.01\\\\-79.10156\\\\296.913\\\\334.01\\\\-76.75781\\\\296.9514\\\\334.01\\\\-67.38281\\\\296.9953\\\\334.01\\\\-58.00781\\\\296.934\\\\334.01\\\\-55.66406\\\\297.1698\\\\334.01\\\\-53.32031\\\\297.1516\\\\334.01\\\\-50.97656\\\\296.9358\\\\334.01\\\\-43.94531\\\\296.9285\\\\334.01\\\\-39.25781\\\\296.9899\\\\334.01\\\\-36.91406\\\\296.8888\\\\334.01\\\\-34.57031\\\\296.8998\\\\334.01\\\\-32.22656\\\\297.252\\\\334.01\\\\-29.88281\\\\296.9011\\\\334.01\\\\-27.53906\\\\296.8789\\\\334.01\\\\-25.19531\\\\297.3806\\\\334.01\\\\-22.85156\\\\297.2719\\\\334.01\\\\-20.50781\\\\297.0591\\\\334.01\\\\-18.16406\\\\297.0665\\\\334.01\\\\-15.82031\\\\296.9285\\\\334.01\\\\-13.47656\\\\296.9377\\\\334.01\\\\-11.13281\\\\297.0627\\\\334.01\\\\-8.789063\\\\296.9095\\\\334.01\\\\-1.757813\\\\296.9095\\\\334.01\\\\7.617188\\\\296.8473\\\\334.01\\\\24.02344\\\\296.8058\\\\334.01\\\\28.71094\\\\296.7421\\\\334.01\\\\31.05469\\\\296.7743\\\\334.01\\\\40.42969\\\\296.7229\\\\334.01\\\\47.46094\\\\296.7229\\\\334.01\\\\61.52344\\\\296.6346\\\\334.01\\\\63.86719\\\\296.6518\\\\334.01\\\\68.55469\\\\296.5942\\\\334.01\\\\75.58594\\\\296.582\\\\334.01\\\\80.27344\\\\296.5412\\\\334.01\\\\87.30469\\\\296.5412\\\\334.01\\\\94.33594\\\\296.4668\\\\334.01\\\\101.3672\\\\296.4257\\\\334.01\\\\103.7109\\\\296.4597\\\\334.01\\\\108.3984\\\\296.4051\\\\334.01\\\\113.0859\\\\296.4393\\\\334.01\\\\122.4609\\\\296.3578\\\\334.01\\\\129.4922\\\\296.3288\\\\334.01\\\\131.8359\\\\295.9278\\\\334.01\\\\134.1797\\\\296.1292\\\\334.01\\\\136.5234\\\\296.0546\\\\334.01\\\\138.8672\\\\295.8486\\\\334.01\\\\143.5547\\\\295.8172\\\\334.01\\\\155.2734\\\\295.833\\\\334.01\\\\166.9922\\\\295.7684\\\\334.01\\\\174.0234\\\\295.6454\\\\334.01\\\\178.7109\\\\295.6819\\\\334.01\\\\183.3984\\\\295.6638\\\\334.01\\\\188.0859\\\\295.5916\\\\334.01\\\\192.7734\\\\295.6093\\\\334.01\\\\197.4609\\\\295.833\\\\334.01\\\\202.1484\\\\295.8172\\\\334.01\\\\204.4922\\\\295.4439\\\\334.01\\\\206.8359\\\\294.803\\\\334.01\\\\209.1797\\\\294.3717\\\\334.01\\\\211.5234\\\\292.5155\\\\334.01\\\\213.8672\\\\291.1916\\\\334.01\\\\216.2109\\\\289.5954\\\\334.01\\\\217.2437\\\\288.6234\\\\334.01\\\\218.5547\\\\287.1034\\\\334.01\\\\221.7932\\\\283.9359\\\\334.01\\\\225.5859\\\\279.9242\\\\334.01\\\\227.9297\\\\277.6213\\\\334.01\\\\230.2734\\\\275.1202\\\\334.01\\\\233.3138\\\\272.2172\\\\334.01\\\\234.9609\\\\270.3912\\\\334.01\\\\237.8462\\\\267.5297\\\\334.01\\\\240.0901\\\\265.1859\\\\334.01\\\\242.4443\\\\262.8422\\\\334.01\\\\246.6797\\\\258.431\\\\334.01\\\\249.248\\\\255.8109\\\\334.01\\\\251.4581\\\\253.4672\\\\334.01\\\\253.4629\\\\251.1234\\\\334.01\\\\256.0547\\\\248.2519\\\\334.01\\\\257.4878\\\\246.4359\\\\334.01\\\\259.8235\\\\244.0922\\\\334.01\\\\260.8724\\\\241.7484\\\\334.01\\\\261.781\\\\239.4047\\\\334.01\\\\261.0668\\\\237.0609\\\\334.01\\\\259.4941\\\\234.7172\\\\334.01\\\\258.3984\\\\233.6932\\\\334.01\\\\256.0547\\\\233.1227\\\\334.01\\\\253.7109\\\\232.9594\\\\334.01\\\\251.3672\\\\233.6412\\\\334.01\\\\249.0234\\\\232.7692\\\\334.01\\\\246.6797\\\\232.7821\\\\334.01\\\\241.9922\\\\233.0207\\\\334.01\\\\237.3047\\\\233.0207\\\\334.01\\\\232.6172\\\\233.091\\\\334.01\\\\230.2734\\\\233.0592\\\\334.01\\\\225.5859\\\\233.0819\\\\334.01\\\\216.2109\\\\233.0726\\\\334.01\\\\206.8359\\\\233.0245\\\\334.01\\\\204.4922\\\\232.8704\\\\334.01\\\\202.1484\\\\232.6027\\\\334.01\\\\199.8047\\\\232.6623\\\\334.01\\\\192.7734\\\\232.7456\\\\334.01\\\\183.3984\\\\232.7719\\\\334.01\\\\181.0547\\\\232.7456\\\\334.01\\\\169.3359\\\\232.7186\\\\334.01\\\\164.6484\\\\232.7588\\\\334.01\\\\150.5859\\\\232.8129\\\\334.01\\\\141.2109\\\\232.7746\\\\334.01\\\\138.8672\\\\232.6788\\\\334.01\\\\134.1797\\\\232.6477\\\\334.01\\\\129.4922\\\\231.8537\\\\334.01\\\\127.1484\\\\231.9275\\\\334.01\\\\124.8047\\\\231.7976\\\\334.01\\\\120.1172\\\\232.7071\\\\334.01\\\\113.0859\\\\232.8253\\\\334.01\\\\108.3984\\\\232.8003\\\\334.01\\\\106.0547\\\\232.6477\\\\334.01\\\\103.7109\\\\231.6597\\\\334.01\\\\102.765\\\\232.3734\\\\334.01\\\\101.3672\\\\233.0479\\\\334.01\\\\99.02344\\\\233.1568\\\\334.01\\\\91.99219\\\\233.131\\\\334.01\\\\82.61719\\\\233.1483\\\\334.01\\\\66.21094\\\\233.131\\\\334.01\\\\61.52344\\\\233.1568\\\\334.01\\\\45.11719\\\\233.1132\\\\334.01\\\\44.13628\\\\232.3734\\\\334.01\\\\42.77344\\\\231.0868\\\\334.01\\\\41.98608\\\\230.0297\\\\334.01\\\\42.77344\\\\229.1708\\\\334.01\\\\45.11719\\\\228.3666\\\\334.01\\\\47.46094\\\\226.8342\\\\334.01\\\\49.80469\\\\225.7806\\\\334.01\\\\52.14844\\\\224.4214\\\\334.01\\\\54.49219\\\\223.3104\\\\334.01\\\\59.17969\\\\219.9071\\\\334.01\\\\61.51359\\\\218.3109\\\\334.01\\\\63.86719\\\\216.5456\\\\334.01\\\\66.21094\\\\214.6382\\\\334.01\\\\67.29213\\\\213.6234\\\\334.01\\\\71.99537\\\\208.9359\\\\334.01\\\\76.04661\\\\204.2484\\\\334.01\\\\77.48438\\\\201.9047\\\\334.01\\\\77.92969\\\\201.4099\\\\334.01\\\\81.04408\\\\197.2172\\\\334.01\\\\82.10064\\\\194.8734\\\\334.01\\\\82.61719\\\\194.2786\\\\334.01\\\\83.77148\\\\192.5297\\\\334.01\\\\84.75167\\\\190.1859\\\\334.01\\\\85.91919\\\\187.8422\\\\334.01\\\\86.84645\\\\185.4984\\\\334.01\\\\88.24593\\\\183.1547\\\\334.01\\\\88.69778\\\\180.8109\\\\334.01\\\\89.31935\\\\178.4672\\\\334.01\\\\90.28825\\\\176.1234\\\\334.01\\\\90.88572\\\\173.7797\\\\334.01\\\\91.32725\\\\171.4359\\\\334.01\\\\92.09135\\\\169.0922\\\\334.01\\\\92.73549\\\\166.7484\\\\334.01\\\\93.00426\\\\164.4047\\\\334.01\\\\93.15228\\\\159.7172\\\\334.01\\\\93.21165\\\\155.0297\\\\334.01\\\\93.16998\\\\150.3422\\\\334.01\\\\93.04629\\\\145.6547\\\\334.01\\\\92.89885\\\\143.3109\\\\334.01\\\\92.4685\\\\140.9672\\\\334.01\\\\91.65399\\\\138.6234\\\\334.01\\\\90.63558\\\\133.9359\\\\334.01\\\\89.94542\\\\131.5922\\\\334.01\\\\88.9603\\\\129.2484\\\\334.01\\\\88.56628\\\\126.9047\\\\334.01\\\\87.77809\\\\124.5609\\\\334.01\\\\86.38477\\\\122.2172\\\\334.01\\\\85.54688\\\\119.8734\\\\334.01\\\\84.18599\\\\117.5297\\\\334.01\\\\83.21418\\\\115.1859\\\\334.01\\\\81.51971\\\\112.8422\\\\334.01\\\\80.27344\\\\110.6408\\\\334.01\\\\78.70409\\\\108.1547\\\\334.01\\\\77.92969\\\\107.2539\\\\334.01\\\\75.58594\\\\104.2182\\\\334.01\\\\70.89844\\\\98.90157\\\\334.01\\\\68.45073\\\\96.43594\\\\334.01\\\\66.21094\\\\94.55936\\\\334.01\\\\63.09443\\\\91.74844\\\\334.01\\\\61.52344\\\\90.59526\\\\334.01\\\\59.17969\\\\88.5986\\\\334.01\\\\56.83594\\\\87.31498\\\\334.01\\\\54.49219\\\\85.86478\\\\334.01\\\\52.14844\\\\84.14245\\\\334.01\\\\49.80469\\\\83.22958\\\\334.01\\\\47.46094\\\\81.94525\\\\334.01\\\\45.11719\\\\80.9931\\\\334.01\\\\43.27874\\\\80.02969\\\\334.01\\\\42.77344\\\\79.65244\\\\334.01\\\\40.42969\\\\78.85207\\\\334.01\\\\38.08594\\\\78.4912\\\\334.01\\\\35.74219\\\\77.58901\\\\334.01\\\\33.39844\\\\76.8085\\\\334.01\\\\28.71094\\\\75.86382\\\\334.01\\\\26.36719\\\\75.08454\\\\334.01\\\\24.02344\\\\74.55878\\\\334.01\\\\21.67969\\\\74.41319\\\\334.01\\\\14.64844\\\\74.18806\\\\334.01\\\\7.617188\\\\74.26453\\\\334.01\\\\2.929688\\\\74.44027\\\\334.01\\\\0.5859375\\\\74.70742\\\\334.01\\\\-4.101563\\\\76.08793\\\\334.01\\\\-6.445313\\\\76.51406\\\\334.01\\\\-8.789063\\\\77.1104\\\\334.01\\\\-11.13281\\\\78.11278\\\\334.01\\\\-15.82031\\\\79.19739\\\\334.01\\\\-18.16406\\\\80.65051\\\\334.01\\\\-20.50781\\\\81.58109\\\\334.01\\\\-22.85156\\\\82.92365\\\\334.01\\\\-25.19531\\\\83.7171\\\\334.01\\\\-27.53906\\\\85.51434\\\\334.01\\\\-29.88281\\\\86.55049\\\\334.01\\\\-32.22656\\\\88.22651\\\\334.01\\\\-34.57031\\\\90.14594\\\\334.01\\\\-36.91406\\\\91.72009\\\\334.01\\\\-41.60156\\\\95.61852\\\\334.01\\\\-47.01968\\\\101.1234\\\\334.01\\\\-50.97656\\\\106.2919\\\\334.01\\\\-54.02066\\\\110.4984\\\\334.01\\\\-55.66406\\\\113.0483\\\\334.01\\\\-56.86023\\\\115.1859\\\\334.01\\\\-57.96204\\\\117.5297\\\\334.01\\\\-60.48373\\\\122.2172\\\\334.01\\\\-61.34826\\\\124.5609\\\\334.01\\\\-61.9119\\\\126.9047\\\\334.01\\\\-63.09862\\\\129.2484\\\\334.01\\\\-63.86719\\\\131.5922\\\\334.01\\\\-64.28482\\\\133.9359\\\\334.01\\\\-65.63528\\\\138.6234\\\\334.01\\\\-66.16119\\\\143.3109\\\\334.01\\\\-66.7664\\\\150.3422\\\\334.01\\\\-66.95463\\\\155.0297\\\\334.01\\\\-66.72736\\\\159.7172\\\\334.01\\\\-66.31696\\\\164.4047\\\\334.01\\\\-65.80171\\\\169.0922\\\\334.01\\\\-65.46725\\\\171.4359\\\\334.01\\\\-64.59868\\\\173.7797\\\\334.01\\\\-63.56812\\\\178.4672\\\\334.01\\\\-62.35712\\\\180.8109\\\\334.01\\\\-61.55333\\\\183.1547\\\\334.01\\\\-61.07733\\\\185.4984\\\\334.01\\\\-59.6588\\\\187.8422\\\\334.01\\\\-58.72026\\\\190.1859\\\\334.01\\\\-57.18415\\\\192.5297\\\\334.01\\\\-56.40602\\\\194.8734\\\\334.01\\\\-55.66406\\\\195.7821\\\\334.01\\\\-53.32031\\\\199.1387\\\\334.01\\\\-52.92969\\\\199.5609\\\\334.01\\\\-51.7693\\\\201.9047\\\\334.01\\\\-49.67076\\\\204.2484\\\\334.01\\\\-47.72281\\\\206.5922\\\\334.01\\\\-45.48062\\\\208.9359\\\\334.01\\\\-43.33069\\\\211.2797\\\\334.01\\\\-41.60156\\\\212.8348\\\\334.01\\\\-39.25781\\\\215.1041\\\\334.01\\\\-36.91406\\\\217.014\\\\334.01\\\\-34.57031\\\\219.1361\\\\334.01\\\\-32.22656\\\\220.3536\\\\334.01\\\\-29.88281\\\\222.028\\\\334.01\\\\-27.53906\\\\223.8316\\\\334.01\\\\-25.19531\\\\224.6377\\\\334.01\\\\-22.85156\\\\226.2194\\\\334.01\\\\-20.50781\\\\227.2216\\\\334.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851156731500001.532752123275\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"543\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"21\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-16.05469\\\\230.0297\\\\337.01\\\\-18.16406\\\\232.5531\\\\337.01\\\\-20.50781\\\\233.1221\\\\337.01\\\\-36.91406\\\\233.1483\\\\337.01\\\\-41.60156\\\\233.19\\\\337.01\\\\-43.94531\\\\233.1652\\\\337.01\\\\-58.00781\\\\233.2061\\\\337.01\\\\-62.69531\\\\233.1981\\\\337.01\\\\-65.03906\\\\233.0382\\\\337.01\\\\-67.38281\\\\232.9942\\\\337.01\\\\-69.72656\\\\233.0671\\\\337.01\\\\-74.41406\\\\233.0859\\\\337.01\\\\-86.13281\\\\233.0859\\\\337.01\\\\-90.82031\\\\233.131\\\\337.01\\\\-93.16406\\\\233.6525\\\\337.01\\\\-95.50781\\\\233.9503\\\\337.01\\\\-97.85156\\\\234.099\\\\337.01\\\\-102.5391\\\\234.2346\\\\337.01\\\\-107.2266\\\\234.2191\\\\337.01\\\\-111.9141\\\\234.2994\\\\337.01\\\\-114.2578\\\\234.2346\\\\337.01\\\\-123.6328\\\\234.2703\\\\337.01\\\\-125.9766\\\\234.2122\\\\337.01\\\\-130.6641\\\\234.1932\\\\337.01\\\\-133.0078\\\\234.2387\\\\337.01\\\\-137.6953\\\\234.1641\\\\337.01\\\\-142.3828\\\\234.2994\\\\337.01\\\\-147.0703\\\\234.1641\\\\337.01\\\\-149.4141\\\\234.2081\\\\337.01\\\\-156.4453\\\\234.1404\\\\337.01\\\\-158.7891\\\\234.1785\\\\337.01\\\\-163.4766\\\\234.1453\\\\337.01\\\\-165.8203\\\\234.1785\\\\337.01\\\\-168.1641\\\\234.3108\\\\337.01\\\\-184.5703\\\\234.3328\\\\337.01\\\\-189.2578\\\\234.3204\\\\337.01\\\\-193.9453\\\\234.4002\\\\337.01\\\\-200.9766\\\\234.471\\\\337.01\\\\-208.0078\\\\234.4053\\\\337.01\\\\-212.6953\\\\234.5285\\\\337.01\\\\-215.0391\\\\234.5285\\\\337.01\\\\-217.3828\\\\234.6247\\\\337.01\\\\-222.0703\\\\234.6649\\\\337.01\\\\-224.4141\\\\234.5484\\\\337.01\\\\-229.1016\\\\234.5121\\\\337.01\\\\-236.1328\\\\234.3573\\\\337.01\\\\-238.4766\\\\234.2224\\\\337.01\\\\-243.1641\\\\234.119\\\\337.01\\\\-247.8516\\\\233.6502\\\\337.01\\\\-250.1953\\\\233.6581\\\\337.01\\\\-252.28\\\\234.7172\\\\337.01\\\\-252.5391\\\\235.7872\\\\337.01\\\\-253.8707\\\\237.0609\\\\337.01\\\\-252.5391\\\\237.3071\\\\337.01\\\\-250.1953\\\\238.1959\\\\337.01\\\\-243.1641\\\\238.1697\\\\337.01\\\\-226.7578\\\\238.1965\\\\337.01\\\\-215.0391\\\\238.2418\\\\337.01\\\\-210.3516\\\\238.2328\\\\337.01\\\\-200.9766\\\\238.2793\\\\337.01\\\\-186.9141\\\\238.2513\\\\337.01\\\\-182.2266\\\\238.3066\\\\337.01\\\\-172.8516\\\\238.3418\\\\337.01\\\\-158.7891\\\\238.3078\\\\337.01\\\\-151.7578\\\\238.3537\\\\337.01\\\\-147.0703\\\\238.3165\\\\337.01\\\\-142.3828\\\\238.3368\\\\337.01\\\\-128.3203\\\\238.3351\\\\337.01\\\\-123.6328\\\\238.3078\\\\337.01\\\\-111.9141\\\\238.3297\\\\337.01\\\\-107.2266\\\\238.2891\\\\337.01\\\\-102.5391\\\\238.3193\\\\337.01\\\\-97.85156\\\\238.2891\\\\337.01\\\\-93.16406\\\\238.3297\\\\337.01\\\\-88.47656\\\\238.2715\\\\337.01\\\\-81.44531\\\\238.2709\\\\337.01\\\\-76.75781\\\\238.3193\\\\337.01\\\\-67.38281\\\\238.2801\\\\337.01\\\\-65.03906\\\\238.309\\\\337.01\\\\-53.32031\\\\238.2607\\\\337.01\\\\-39.25781\\\\238.2801\\\\337.01\\\\-34.57031\\\\238.309\\\\337.01\\\\-32.22656\\\\238.2793\\\\337.01\\\\-11.13281\\\\238.2516\\\\337.01\\\\-1.757813\\\\238.3179\\\\337.01\\\\2.929688\\\\238.2612\\\\337.01\\\\7.617188\\\\238.2801\\\\337.01\\\\9.960938\\\\238.2423\\\\337.01\\\\16.99219\\\\238.2423\\\\337.01\\\\31.05469\\\\238.1953\\\\337.01\\\\33.39844\\\\238.2234\\\\337.01\\\\42.77344\\\\238.1566\\\\337.01\\\\45.11719\\\\238.1953\\\\337.01\\\\54.49219\\\\238.1478\\\\337.01\\\\66.21094\\\\238.1119\\\\337.01\\\\75.58594\\\\238.1016\\\\337.01\\\\82.61719\\\\238.1289\\\\337.01\\\\84.96094\\\\237.6617\\\\337.01\\\\87.30469\\\\238.1036\\\\337.01\\\\89.64844\\\\238.0933\\\\337.01\\\\91.99219\\\\237.956\\\\337.01\\\\94.33594\\\\237.1551\\\\337.01\\\\96.67969\\\\237.0326\\\\337.01\\\\101.3672\\\\236.9958\\\\337.01\\\\103.7109\\\\237.0335\\\\337.01\\\\106.0547\\\\237.7065\\\\337.01\\\\108.3984\\\\237.0328\\\\337.01\\\\113.0859\\\\237.0932\\\\337.01\\\\117.7734\\\\236.961\\\\337.01\\\\120.1172\\\\237.0144\\\\337.01\\\\124.8047\\\\236.9785\\\\337.01\\\\127.1484\\\\236.9111\\\\337.01\\\\129.4922\\\\237.0144\\\\337.01\\\\134.1797\\\\236.9964\\\\337.01\\\\138.8672\\\\236.8935\\\\337.01\\\\150.5859\\\\236.8935\\\\337.01\\\\152.9297\\\\236.8455\\\\337.01\\\\164.6484\\\\236.8786\\\\337.01\\\\171.6797\\\\236.7721\\\\337.01\\\\181.0547\\\\236.8165\\\\337.01\\\\183.3984\\\\236.7296\\\\337.01\\\\190.4297\\\\236.7577\\\\337.01\\\\199.8047\\\\236.7158\\\\337.01\\\\202.1484\\\\236.6625\\\\337.01\\\\211.5234\\\\236.6215\\\\337.01\\\\216.2109\\\\236.6341\\\\337.01\\\\223.2422\\\\236.5608\\\\337.01\\\\230.2734\\\\236.612\\\\337.01\\\\237.3047\\\\236.564\\\\337.01\\\\246.6797\\\\236.5556\\\\337.01\\\\249.0234\\\\236.5217\\\\337.01\\\\251.3672\\\\235.8337\\\\337.01\\\\253.7109\\\\236.673\\\\337.01\\\\254.1778\\\\237.0609\\\\337.01\\\\255.7377\\\\239.4047\\\\337.01\\\\255.5197\\\\241.7484\\\\337.01\\\\254.5064\\\\244.0922\\\\337.01\\\\252.6128\\\\246.4359\\\\337.01\\\\251.3672\\\\248.2697\\\\337.01\\\\248.4421\\\\251.1234\\\\337.01\\\\246.6797\\\\253.0868\\\\337.01\\\\244.3359\\\\255.2826\\\\337.01\\\\241.9922\\\\257.8041\\\\337.01\\\\239.3325\\\\260.4984\\\\337.01\\\\236.9309\\\\262.8422\\\\337.01\\\\234.6654\\\\265.1859\\\\337.01\\\\232.2605\\\\267.5297\\\\337.01\\\\230.2734\\\\269.6653\\\\337.01\\\\227.9297\\\\271.8957\\\\337.01\\\\225.5859\\\\274.3181\\\\337.01\\\\223.2422\\\\276.5594\\\\337.01\\\\218.2125\\\\281.5922\\\\337.01\\\\215.6647\\\\283.9359\\\\337.01\\\\213.3404\\\\286.2797\\\\337.01\\\\211.5234\\\\287.78\\\\337.01\\\\209.1797\\\\289.4079\\\\337.01\\\\206.8359\\\\290.6449\\\\337.01\\\\202.1484\\\\290.6449\\\\337.01\\\\199.8047\\\\290.5828\\\\337.01\\\\197.4609\\\\290.6118\\\\337.01\\\\195.1172\\\\290.5487\\\\337.01\\\\192.7734\\\\290.6282\\\\337.01\\\\190.4297\\\\289.8361\\\\337.01\\\\183.3984\\\\289.8157\\\\337.01\\\\176.3672\\\\289.8256\\\\337.01\\\\169.3359\\\\289.7852\\\\337.01\\\\159.9609\\\\289.7852\\\\337.01\\\\155.2734\\\\289.7545\\\\337.01\\\\150.5859\\\\289.785\\\\337.01\\\\145.8984\\\\289.7439\\\\337.01\\\\129.4922\\\\289.765\\\\337.01\\\\122.4609\\\\289.7234\\\\337.01\\\\113.0859\\\\289.7246\\\\337.01\\\\108.3984\\\\289.7552\\\\337.01\\\\106.0547\\\\289.7246\\\\337.01\\\\91.99219\\\\289.7448\\\\337.01\\\\80.27344\\\\289.7234\\\\337.01\\\\73.24219\\\\289.7545\\\\337.01\\\\66.21094\\\\289.7545\\\\337.01\\\\56.83594\\\\289.7234\\\\337.01\\\\42.77344\\\\289.7246\\\\337.01\\\\40.42969\\\\289.765\\\\337.01\\\\31.05469\\\\289.7352\\\\337.01\\\\12.30469\\\\289.7852\\\\337.01\\\\2.929688\\\\289.765\\\\337.01\\\\-8.789063\\\\289.7854\\\\337.01\\\\-25.19531\\\\289.7753\\\\337.01\\\\-29.88281\\\\289.8052\\\\337.01\\\\-34.57031\\\\289.7854\\\\337.01\\\\-53.32031\\\\289.8354\\\\337.01\\\\-67.38281\\\\289.8354\\\\337.01\\\\-74.41406\\\\289.8648\\\\337.01\\\\-76.75781\\\\290.7283\\\\337.01\\\\-79.10156\\\\289.8554\\\\337.01\\\\-81.44531\\\\290.3386\\\\337.01\\\\-83.78906\\\\291.0974\\\\337.01\\\\-86.13281\\\\291.0974\\\\337.01\\\\-88.47656\\\\289.9266\\\\337.01\\\\-90.82031\\\\290.9763\\\\337.01\\\\-93.16406\\\\291.1297\\\\337.01\\\\-97.85156\\\\291.1137\\\\337.01\\\\-104.8828\\\\291.1611\\\\337.01\\\\-128.3203\\\\291.2212\\\\337.01\\\\-135.3516\\\\291.2642\\\\337.01\\\\-140.0391\\\\291.2501\\\\337.01\\\\-149.4141\\\\291.3054\\\\337.01\\\\-154.1016\\\\291.3054\\\\337.01\\\\-168.1641\\\\291.3705\\\\337.01\\\\-172.8516\\\\291.3705\\\\337.01\\\\-189.2578\\\\291.4549\\\\337.01\\\\-196.2891\\\\291.4549\\\\337.01\\\\-198.6328\\\\292.0931\\\\337.01\\\\-200.9766\\\\291.6703\\\\337.01\\\\-203.3203\\\\292.0439\\\\337.01\\\\-208.0078\\\\292.155\\\\337.01\\\\-210.3516\\\\291.8999\\\\337.01\\\\-211.6451\\\\290.9672\\\\337.01\\\\-212.6953\\\\290.0075\\\\337.01\\\\-215.0391\\\\288.4995\\\\337.01\\\\-219.7266\\\\284.2614\\\\337.01\\\\-226.7578\\\\277.3752\\\\337.01\\\\-229.1016\\\\275.0117\\\\337.01\\\\-231.4453\\\\272.749\\\\337.01\\\\-231.9032\\\\272.2172\\\\337.01\\\\-234.3526\\\\269.8734\\\\337.01\\\\-236.6545\\\\267.5297\\\\337.01\\\\-239.0497\\\\265.1859\\\\337.01\\\\-247.8516\\\\256.3387\\\\337.01\\\\-248.3156\\\\255.8109\\\\337.01\\\\-250.7947\\\\253.4672\\\\337.01\\\\-254.8828\\\\249.2404\\\\337.01\\\\-255.4349\\\\248.7797\\\\337.01\\\\-257.3968\\\\246.4359\\\\337.01\\\\-258.2731\\\\244.0922\\\\337.01\\\\-258.2375\\\\241.7484\\\\337.01\\\\-257.667\\\\239.4047\\\\337.01\\\\-257.2266\\\\238.8687\\\\337.01\\\\-255.0871\\\\237.0609\\\\337.01\\\\-257.2266\\\\235.0331\\\\337.01\\\\-259.5703\\\\235.8786\\\\337.01\\\\-261.0648\\\\237.0609\\\\337.01\\\\-261.9141\\\\237.9827\\\\337.01\\\\-262.9937\\\\239.4047\\\\337.01\\\\-263.0859\\\\241.7484\\\\337.01\\\\-262.7562\\\\244.0922\\\\337.01\\\\-261.9141\\\\245.3154\\\\337.01\\\\-259.5703\\\\248.3479\\\\337.01\\\\-259.0795\\\\248.7797\\\\337.01\\\\-255.473\\\\253.4672\\\\337.01\\\\-252.5391\\\\256.6491\\\\337.01\\\\-250.1953\\\\258.8997\\\\337.01\\\\-247.8516\\\\261.422\\\\337.01\\\\-245.5078\\\\263.5974\\\\337.01\\\\-244.0876\\\\265.1859\\\\337.01\\\\-241.5988\\\\267.5297\\\\337.01\\\\-239.4282\\\\269.8734\\\\337.01\\\\-236.9844\\\\272.2172\\\\337.01\\\\-233.7891\\\\275.424\\\\337.01\\\\-231.4453\\\\277.8406\\\\337.01\\\\-229.8799\\\\279.2484\\\\337.01\\\\-226.7578\\\\282.5503\\\\337.01\\\\-225.2042\\\\283.9359\\\\337.01\\\\-222.0703\\\\287.2521\\\\337.01\\\\-220.5943\\\\288.6234\\\\337.01\\\\-218.3718\\\\290.9672\\\\337.01\\\\-217.3828\\\\291.7129\\\\337.01\\\\-214.9922\\\\293.3109\\\\337.01\\\\-212.6953\\\\294.6589\\\\337.01\\\\-210.3516\\\\295.2864\\\\337.01\\\\-209.9019\\\\295.6547\\\\337.01\\\\-208.0078\\\\296.512\\\\337.01\\\\-205.6641\\\\296.899\\\\337.01\\\\-203.3203\\\\297.1108\\\\337.01\\\\-198.6328\\\\297.0992\\\\337.01\\\\-193.9453\\\\296.9203\\\\337.01\\\\-191.6016\\\\296.8886\\\\337.01\\\\-182.2266\\\\296.9061\\\\337.01\\\\-175.1953\\\\296.8497\\\\337.01\\\\-163.4766\\\\296.882\\\\337.01\\\\-154.1016\\\\296.8757\\\\337.01\\\\-151.7578\\\\296.8513\\\\337.01\\\\-142.3828\\\\296.8771\\\\337.01\\\\-140.0391\\\\296.8521\\\\337.01\\\\-128.3203\\\\296.8175\\\\337.01\\\\-109.5703\\\\296.916\\\\337.01\\\\-90.82031\\\\296.8938\\\\337.01\\\\-86.13281\\\\296.9322\\\\337.01\\\\-81.44531\\\\296.9041\\\\337.01\\\\-74.41406\\\\296.9622\\\\337.01\\\\-67.38281\\\\296.979\\\\337.01\\\\-58.00781\\\\296.9535\\\\337.01\\\\-55.66406\\\\297.2359\\\\337.01\\\\-53.32031\\\\297.1979\\\\337.01\\\\-50.97656\\\\296.9267\\\\337.01\\\\-43.94531\\\\296.9377\\\\337.01\\\\-39.25781\\\\296.878\\\\337.01\\\\-34.57031\\\\296.8985\\\\337.01\\\\-32.22656\\\\297.1454\\\\337.01\\\\-29.88281\\\\296.9011\\\\337.01\\\\-27.53906\\\\296.8899\\\\337.01\\\\-25.19531\\\\297.257\\\\337.01\\\\-22.85156\\\\297.2363\\\\337.01\\\\-20.50781\\\\296.8985\\\\337.01\\\\-8.789063\\\\296.9396\\\\337.01\\\\-6.445313\\\\297.0519\\\\337.01\\\\-4.101563\\\\296.8888\\\\337.01\\\\12.30469\\\\296.8163\\\\337.01\\\\16.99219\\\\296.8369\\\\337.01\\\\24.02344\\\\296.8163\\\\337.01\\\\28.71094\\\\296.7643\\\\337.01\\\\35.74219\\\\296.7743\\\\337.01\\\\45.11719\\\\296.7021\\\\337.01\\\\56.83594\\\\296.6839\\\\337.01\\\\61.52344\\\\296.6346\\\\337.01\\\\63.86719\\\\296.6548\\\\337.01\\\\70.89844\\\\296.582\\\\337.01\\\\87.30469\\\\296.5412\\\\337.01\\\\94.33594\\\\296.4801\\\\337.01\\\\103.7109\\\\296.4597\\\\337.01\\\\110.7422\\\\296.4059\\\\337.01\\\\113.0859\\\\296.4326\\\\337.01\\\\127.1484\\\\296.3578\\\\337.01\\\\134.1797\\\\296.2355\\\\337.01\\\\136.5234\\\\295.8685\\\\337.01\\\\138.8672\\\\296.0063\\\\337.01\\\\141.2109\\\\295.8486\\\\337.01\\\\143.5547\\\\295.8172\\\\337.01\\\\150.5859\\\\295.833\\\\337.01\\\\159.9609\\\\295.7849\\\\337.01\\\\164.6484\\\\295.8012\\\\337.01\\\\176.3672\\\\295.6638\\\\337.01\\\\178.7109\\\\295.6998\\\\337.01\\\\192.7734\\\\295.6093\\\\337.01\\\\195.1172\\\\295.7849\\\\337.01\\\\197.4609\\\\295.8486\\\\337.01\\\\202.1484\\\\295.7849\\\\337.01\\\\204.4922\\\\295.3989\\\\337.01\\\\206.8359\\\\294.7878\\\\337.01\\\\209.1797\\\\294.3606\\\\337.01\\\\211.5234\\\\292.5075\\\\337.01\\\\213.8672\\\\291.1916\\\\337.01\\\\216.2109\\\\289.5791\\\\337.01\\\\217.2271\\\\288.6234\\\\337.01\\\\218.5547\\\\287.1251\\\\337.01\\\\221.7773\\\\283.9359\\\\337.01\\\\225.5859\\\\279.9242\\\\337.01\\\\228.6029\\\\276.9047\\\\337.01\\\\230.8462\\\\274.5609\\\\337.01\\\\233.2856\\\\272.2172\\\\337.01\\\\234.9609\\\\270.3731\\\\337.01\\\\237.8504\\\\267.5297\\\\337.01\\\\240.0754\\\\265.1859\\\\337.01\\\\242.4628\\\\262.8422\\\\337.01\\\\246.6797\\\\258.4333\\\\337.01\\\\249.248\\\\255.8109\\\\337.01\\\\251.4573\\\\253.4672\\\\337.01\\\\256.0547\\\\248.2478\\\\337.01\\\\257.4822\\\\246.4359\\\\337.01\\\\259.856\\\\244.0922\\\\337.01\\\\260.9205\\\\241.7484\\\\337.01\\\\261.7867\\\\239.4047\\\\337.01\\\\261.0251\\\\237.0609\\\\337.01\\\\259.4996\\\\234.7172\\\\337.01\\\\258.3984\\\\233.6932\\\\337.01\\\\256.0547\\\\233.1313\\\\337.01\\\\253.7109\\\\233.0145\\\\337.01\\\\251.3672\\\\233.5155\\\\337.01\\\\249.0234\\\\232.7431\\\\337.01\\\\244.3359\\\\232.8555\\\\337.01\\\\241.9922\\\\233.0207\\\\337.01\\\\234.9609\\\\233.0537\\\\337.01\\\\227.9297\\\\233.0305\\\\337.01\\\\223.2422\\\\233.0632\\\\337.01\\\\206.8359\\\\233.0145\\\\337.01\\\\204.4922\\\\232.882\\\\337.01\\\\202.1484\\\\232.6027\\\\337.01\\\\195.1172\\\\232.7456\\\\337.01\\\\192.7734\\\\232.7186\\\\337.01\\\\183.3984\\\\232.7588\\\\337.01\\\\174.0234\\\\232.7186\\\\337.01\\\\157.6172\\\\232.7614\\\\337.01\\\\143.5547\\\\232.8253\\\\337.01\\\\136.5234\\\\232.6767\\\\337.01\\\\129.4922\\\\232.693\\\\337.01\\\\127.0996\\\\232.3734\\\\337.01\\\\124.8047\\\\232.7481\\\\337.01\\\\120.1172\\\\232.7614\\\\337.01\\\\113.0859\\\\232.8617\\\\337.01\\\\110.7422\\\\232.8253\\\\337.01\\\\106.0547\\\\232.8617\\\\337.01\\\\103.7109\\\\232.1634\\\\337.01\\\\101.3672\\\\232.998\\\\337.01\\\\99.02344\\\\233.1221\\\\337.01\\\\89.64844\\\\233.1483\\\\337.01\\\\84.96094\\\\233.1221\\\\337.01\\\\70.89844\\\\233.1568\\\\337.01\\\\68.55469\\\\233.131\\\\337.01\\\\54.49219\\\\233.1397\\\\337.01\\\\45.11719\\\\233.0951\\\\337.01\\\\44.15678\\\\232.3734\\\\337.01\\\\42.77344\\\\231.0868\\\\337.01\\\\41.98608\\\\230.0297\\\\337.01\\\\42.77344\\\\229.1708\\\\337.01\\\\45.11719\\\\228.3666\\\\337.01\\\\47.46094\\\\226.8342\\\\337.01\\\\49.80469\\\\225.7676\\\\337.01\\\\52.14844\\\\224.4137\\\\337.01\\\\54.49219\\\\223.2807\\\\337.01\\\\59.17969\\\\219.898\\\\337.01\\\\61.47501\\\\218.3109\\\\337.01\\\\63.86719\\\\216.4922\\\\337.01\\\\66.21094\\\\214.6099\\\\337.01\\\\70.89844\\\\210.0019\\\\337.01\\\\71.93934\\\\208.9359\\\\337.01\\\\76.00388\\\\204.2484\\\\337.01\\\\77.40801\\\\201.9047\\\\337.01\\\\77.92969\\\\201.3145\\\\337.01\\\\81.018\\\\197.2172\\\\337.01\\\\82.01642\\\\194.8734\\\\337.01\\\\83.7233\\\\192.5297\\\\337.01\\\\84.67807\\\\190.1859\\\\337.01\\\\85.88257\\\\187.8422\\\\337.01\\\\86.77662\\\\185.4984\\\\337.01\\\\88.21052\\\\183.1547\\\\337.01\\\\88.67678\\\\180.8109\\\\337.01\\\\89.25264\\\\178.4672\\\\337.01\\\\90.24816\\\\176.1234\\\\337.01\\\\90.83131\\\\173.7797\\\\337.01\\\\91.26373\\\\171.4359\\\\337.01\\\\92.6648\\\\166.7484\\\\337.01\\\\92.97371\\\\164.4047\\\\337.01\\\\93.12873\\\\159.7172\\\\337.01\\\\93.17584\\\\152.6859\\\\337.01\\\\93.0293\\\\145.6547\\\\337.01\\\\92.85534\\\\143.3109\\\\337.01\\\\92.3933\\\\140.9672\\\\337.01\\\\91.5518\\\\138.6234\\\\337.01\\\\90.58476\\\\133.9359\\\\337.01\\\\89.87284\\\\131.5922\\\\337.01\\\\88.92268\\\\129.2484\\\\337.01\\\\88.54911\\\\126.9047\\\\337.01\\\\87.68194\\\\124.5609\\\\337.01\\\\86.33789\\\\122.2172\\\\337.01\\\\85.47454\\\\119.8734\\\\337.01\\\\84.15683\\\\117.5297\\\\337.01\\\\83.14225\\\\115.1859\\\\337.01\\\\81.47615\\\\112.8422\\\\337.01\\\\80.27344\\\\110.7263\\\\337.01\\\\78.65778\\\\108.1547\\\\337.01\\\\77.92969\\\\107.3061\\\\337.01\\\\75.58594\\\\104.2673\\\\337.01\\\\70.89844\\\\98.97046\\\\337.01\\\\68.38074\\\\96.43594\\\\337.01\\\\66.21094\\\\94.59663\\\\337.01\\\\63.03367\\\\91.74844\\\\337.01\\\\61.52344\\\\90.60773\\\\337.01\\\\59.17969\\\\88.63029\\\\337.01\\\\56.83594\\\\87.3438\\\\337.01\\\\54.49219\\\\85.87691\\\\337.01\\\\52.14844\\\\84.18039\\\\337.01\\\\49.80469\\\\83.24171\\\\337.01\\\\47.46094\\\\81.97013\\\\337.01\\\\45.11719\\\\81.00246\\\\337.01\\\\42.77344\\\\79.69603\\\\337.01\\\\40.42969\\\\78.85781\\\\337.01\\\\38.08594\\\\78.49974\\\\337.01\\\\35.74219\\\\77.60603\\\\337.01\\\\33.39844\\\\76.81591\\\\337.01\\\\28.71094\\\\75.88577\\\\337.01\\\\26.36719\\\\75.09944\\\\337.01\\\\24.02344\\\\74.58508\\\\337.01\\\\21.67969\\\\74.41319\\\\337.01\\\\14.64844\\\\74.19961\\\\337.01\\\\7.617188\\\\74.26453\\\\337.01\\\\2.929688\\\\74.44027\\\\337.01\\\\0.5859375\\\\74.69731\\\\337.01\\\\-4.101563\\\\76.08793\\\\337.01\\\\-6.445313\\\\76.49226\\\\337.01\\\\-8.789063\\\\77.12093\\\\337.01\\\\-11.13281\\\\78.06062\\\\337.01\\\\-13.47656\\\\78.67705\\\\337.01\\\\-15.82031\\\\79.20602\\\\337.01\\\\-18.16406\\\\80.67077\\\\337.01\\\\-20.50781\\\\81.56818\\\\337.01\\\\-22.85156\\\\82.93452\\\\337.01\\\\-25.19531\\\\83.69257\\\\337.01\\\\-27.53906\\\\85.49158\\\\337.01\\\\-29.88281\\\\86.50658\\\\337.01\\\\-32.22656\\\\88.21391\\\\337.01\\\\-34.57031\\\\90.13181\\\\337.01\\\\-36.91406\\\\91.68333\\\\337.01\\\\-39.25781\\\\93.56712\\\\337.01\\\\-41.60156\\\\95.59077\\\\337.01\\\\-47.06793\\\\101.1234\\\\337.01\\\\-50.97656\\\\106.257\\\\337.01\\\\-54.06295\\\\110.4984\\\\337.01\\\\-55.66406\\\\112.9218\\\\337.01\\\\-56.87866\\\\115.1859\\\\337.01\\\\-58.00781\\\\117.468\\\\337.01\\\\-60.35156\\\\121.8745\\\\337.01\\\\-60.61105\\\\122.2172\\\\337.01\\\\-61.37396\\\\124.5609\\\\337.01\\\\-61.96955\\\\126.9047\\\\337.01\\\\-63.18912\\\\129.2484\\\\337.01\\\\-63.92686\\\\131.5922\\\\337.01\\\\-64.32825\\\\133.9359\\\\337.01\\\\-65.08414\\\\136.2797\\\\337.01\\\\-65.66541\\\\138.6234\\\\337.01\\\\-65.95693\\\\140.9672\\\\337.01\\\\-66.45103\\\\145.6547\\\\337.01\\\\-66.83923\\\\150.3422\\\\337.01\\\\-67.00504\\\\155.0297\\\\337.01\\\\-66.91849\\\\157.3734\\\\337.01\\\\-66.59546\\\\162.0609\\\\337.01\\\\-65.85083\\\\169.0922\\\\337.01\\\\-65.52673\\\\171.4359\\\\337.01\\\\-64.66129\\\\173.7797\\\\337.01\\\\-63.60744\\\\178.4672\\\\337.01\\\\-62.44127\\\\180.8109\\\\337.01\\\\-61.60844\\\\183.1547\\\\337.01\\\\-61.10444\\\\185.4984\\\\337.01\\\\-59.69611\\\\187.8422\\\\337.01\\\\-58.78688\\\\190.1859\\\\337.01\\\\-57.22429\\\\192.5297\\\\337.01\\\\-56.42923\\\\194.8734\\\\337.01\\\\-55.66406\\\\195.816\\\\337.01\\\\-53.32031\\\\199.2024\\\\337.01\\\\-52.98665\\\\199.5609\\\\337.01\\\\-51.81459\\\\201.9047\\\\337.01\\\\-49.69277\\\\204.2484\\\\337.01\\\\-47.77253\\\\206.5922\\\\337.01\\\\-45.4857\\\\208.9359\\\\337.01\\\\-43.94531\\\\210.7107\\\\337.01\\\\-39.25781\\\\215.1271\\\\337.01\\\\-36.91406\\\\217.0729\\\\337.01\\\\-34.57031\\\\219.1932\\\\337.01\\\\-32.22656\\\\220.397\\\\337.01\\\\-31.95135\\\\220.6547\\\\337.01\\\\-28.60054\\\\222.9984\\\\337.01\\\\-27.53906\\\\223.8551\\\\337.01\\\\-25.19531\\\\224.6817\\\\337.01\\\\-22.85156\\\\226.2544\\\\337.01\\\\-20.50781\\\\227.2455\\\\337.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851179732800001.515857303411\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"344\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"22\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"45.11719\\\\233.0951\\\\340.01\\\\43.64898\\\\232.3734\\\\340.01\\\\42.77344\\\\231.5882\\\\340.01\\\\41.51367\\\\230.0297\\\\340.01\\\\42.77344\\\\229.1609\\\\340.01\\\\45.11719\\\\228.3465\\\\340.01\\\\47.46094\\\\226.8296\\\\340.01\\\\49.80469\\\\225.7274\\\\340.01\\\\50.36671\\\\225.3422\\\\340.01\\\\54.49219\\\\223.2026\\\\340.01\\\\56.83594\\\\221.5721\\\\340.01\\\\61.40058\\\\218.3109\\\\340.01\\\\63.86719\\\\216.4497\\\\340.01\\\\66.21094\\\\214.5379\\\\340.01\\\\67.18151\\\\213.6234\\\\340.01\\\\71.87971\\\\208.9359\\\\340.01\\\\75.93159\\\\204.2484\\\\340.01\\\\77.34375\\\\201.9047\\\\340.01\\\\77.92969\\\\201.2338\\\\340.01\\\\81.00505\\\\197.2172\\\\340.01\\\\81.93835\\\\194.8734\\\\340.01\\\\83.70493\\\\192.5297\\\\340.01\\\\84.59618\\\\190.1859\\\\340.01\\\\85.85058\\\\187.8422\\\\340.01\\\\86.71875\\\\185.4984\\\\340.01\\\\88.16106\\\\183.1547\\\\340.01\\\\89.19947\\\\178.4672\\\\340.01\\\\90.19531\\\\176.1234\\\\340.01\\\\90.79267\\\\173.7797\\\\340.01\\\\91.21298\\\\171.4359\\\\340.01\\\\91.82967\\\\169.0922\\\\340.01\\\\92.58527\\\\166.7484\\\\340.01\\\\92.92848\\\\164.4047\\\\340.01\\\\93.04102\\\\162.0609\\\\340.01\\\\93.14051\\\\157.3734\\\\340.01\\\\93.15228\\\\152.6859\\\\340.01\\\\93.01096\\\\145.6547\\\\340.01\\\\92.82652\\\\143.3109\\\\340.01\\\\92.30957\\\\140.9672\\\\340.01\\\\91.49306\\\\138.6234\\\\340.01\\\\90.54668\\\\133.9359\\\\340.01\\\\89.79492\\\\131.5922\\\\340.01\\\\88.89133\\\\129.2484\\\\340.01\\\\88.5321\\\\126.9047\\\\340.01\\\\87.59558\\\\124.5609\\\\340.01\\\\86.30105\\\\122.2172\\\\340.01\\\\85.44\\\\119.8734\\\\340.01\\\\84.1048\\\\117.5297\\\\340.01\\\\83.04976\\\\115.1859\\\\340.01\\\\81.44531\\\\112.8422\\\\340.01\\\\78.61036\\\\108.1547\\\\340.01\\\\77.92969\\\\107.3653\\\\340.01\\\\75.58594\\\\104.3202\\\\340.01\\\\70.89844\\\\99.03911\\\\340.01\\\\68.31129\\\\96.43594\\\\340.01\\\\66.21094\\\\94.65131\\\\340.01\\\\63.00066\\\\91.74844\\\\340.01\\\\59.99457\\\\89.40469\\\\340.01\\\\59.17969\\\\88.66788\\\\340.01\\\\56.83594\\\\87.38558\\\\340.01\\\\52.7524\\\\84.71719\\\\340.01\\\\52.14844\\\\84.20734\\\\340.01\\\\49.80469\\\\83.25685\\\\340.01\\\\47.46094\\\\81.99566\\\\340.01\\\\45.11719\\\\81.03172\\\\340.01\\\\42.77344\\\\79.72434\\\\340.01\\\\40.42969\\\\78.86356\\\\340.01\\\\38.08594\\\\78.50819\\\\340.01\\\\33.39844\\\\76.83095\\\\340.01\\\\28.71094\\\\75.9072\\\\340.01\\\\26.36719\\\\75.12989\\\\340.01\\\\24.02344\\\\74.57621\\\\340.01\\\\21.67969\\\\74.42056\\\\340.01\\\\14.64844\\\\74.21133\\\\340.01\\\\9.960938\\\\74.21133\\\\340.01\\\\5.273438\\\\74.33174\\\\340.01\\\\2.929688\\\\74.44027\\\\340.01\\\\0.5859375\\\\74.68344\\\\340.01\\\\-4.101563\\\\76.06187\\\\340.01\\\\-6.445313\\\\76.48074\\\\340.01\\\\-8.789063\\\\77.09653\\\\340.01\\\\-11.13281\\\\78.03346\\\\340.01\\\\-13.47656\\\\78.66458\\\\340.01\\\\-15.82031\\\\79.18885\\\\340.01\\\\-18.16406\\\\80.65698\\\\340.01\\\\-20.50781\\\\81.53359\\\\340.01\\\\-22.85156\\\\82.9015\\\\340.01\\\\-25.19531\\\\83.66675\\\\340.01\\\\-27.53906\\\\85.45914\\\\340.01\\\\-29.88281\\\\86.475\\\\340.01\\\\-32.22656\\\\88.17611\\\\340.01\\\\-34.57031\\\\90.08352\\\\340.01\\\\-36.91406\\\\91.61217\\\\340.01\\\\-39.25781\\\\93.50252\\\\340.01\\\\-41.60156\\\\95.54475\\\\340.01\\\\-47.14196\\\\101.1234\\\\340.01\\\\-50.67958\\\\105.8109\\\\340.01\\\\-50.97656\\\\106.1445\\\\340.01\\\\-54.1083\\\\110.4984\\\\340.01\\\\-55.67351\\\\112.8422\\\\340.01\\\\-59.33949\\\\119.8734\\\\340.01\\\\-60.7103\\\\122.2172\\\\340.01\\\\-61.41526\\\\124.5609\\\\340.01\\\\-62.03443\\\\126.9047\\\\340.01\\\\-63.2848\\\\129.2484\\\\340.01\\\\-63.97519\\\\131.5922\\\\340.01\\\\-64.37412\\\\133.9359\\\\340.01\\\\-65.20158\\\\136.2797\\\\340.01\\\\-65.72266\\\\138.6234\\\\340.01\\\\-66.51716\\\\145.6547\\\\340.01\\\\-66.90674\\\\150.3422\\\\340.01\\\\-67.05817\\\\152.6859\\\\340.01\\\\-67.07191\\\\155.0297\\\\340.01\\\\-66.80727\\\\159.7172\\\\340.01\\\\-66.38559\\\\164.4047\\\\340.01\\\\-65.8744\\\\169.0922\\\\340.01\\\\-65.56069\\\\171.4359\\\\340.01\\\\-64.72816\\\\173.7797\\\\340.01\\\\-64.14594\\\\176.1234\\\\340.01\\\\-63.66697\\\\178.4672\\\\340.01\\\\-62.54883\\\\180.8109\\\\340.01\\\\-61.63273\\\\183.1547\\\\340.01\\\\-61.1263\\\\185.4984\\\\340.01\\\\-59.74519\\\\187.8422\\\\340.01\\\\-58.81712\\\\190.1859\\\\340.01\\\\-57.25644\\\\192.5297\\\\340.01\\\\-56.44758\\\\194.8734\\\\340.01\\\\-55.66406\\\\195.85\\\\340.01\\\\-53.32031\\\\199.2374\\\\340.01\\\\-53.01497\\\\199.5609\\\\340.01\\\\-51.82329\\\\201.9047\\\\340.01\\\\-49.72613\\\\204.2484\\\\340.01\\\\-47.79478\\\\206.5922\\\\340.01\\\\-45.51518\\\\208.9359\\\\340.01\\\\-43.94531\\\\210.7713\\\\340.01\\\\-41.60156\\\\212.8845\\\\340.01\\\\-39.25781\\\\215.1362\\\\340.01\\\\-36.91406\\\\217.0861\\\\340.01\\\\-34.57031\\\\219.2098\\\\340.01\\\\-32.22656\\\\220.4119\\\\340.01\\\\-29.88281\\\\222.0768\\\\340.01\\\\-27.53906\\\\223.8773\\\\340.01\\\\-25.19531\\\\224.7128\\\\340.01\\\\-22.85156\\\\226.2625\\\\340.01\\\\-20.50781\\\\227.2578\\\\340.01\\\\-18.16406\\\\228.6914\\\\340.01\\\\-15.82031\\\\229.7006\\\\340.01\\\\-15.45354\\\\230.0297\\\\340.01\\\\-15.82031\\\\230.6879\\\\340.01\\\\-17.62983\\\\232.3734\\\\340.01\\\\-18.16406\\\\232.6311\\\\340.01\\\\-20.50781\\\\233.1132\\\\340.01\\\\-34.57031\\\\233.131\\\\340.01\\\\-41.60156\\\\233.1818\\\\340.01\\\\-46.28906\\\\233.1652\\\\340.01\\\\-55.66406\\\\233.1981\\\\340.01\\\\-60.35156\\\\233.19\\\\340.01\\\\-62.69531\\\\233.246\\\\340.01\\\\-65.03906\\\\233.0382\\\\340.01\\\\-69.72656\\\\233.0671\\\\340.01\\\\-83.78906\\\\233.0766\\\\340.01\\\\-90.82031\\\\233.1132\\\\340.01\\\\-93.16406\\\\233.1964\\\\340.01\\\\-95.50781\\\\233.8279\\\\340.01\\\\-97.85156\\\\234.0941\\\\340.01\\\\-102.5391\\\\234.2306\\\\340.01\\\\-109.5703\\\\234.279\\\\340.01\\\\-116.6016\\\\234.2191\\\\340.01\\\\-121.2891\\\\234.2543\\\\340.01\\\\-130.6641\\\\234.1741\\\\340.01\\\\-133.0078\\\\234.2081\\\\340.01\\\\-137.6953\\\\234.1499\\\\340.01\\\\-142.3828\\\\234.2703\\\\340.01\\\\-147.0703\\\\234.1641\\\\340.01\\\\-151.7578\\\\234.1932\\\\340.01\\\\-156.4453\\\\234.1267\\\\340.01\\\\-158.7891\\\\234.1543\\\\340.01\\\\-163.4766\\\\234.1221\\\\340.01\\\\-168.1641\\\\234.2766\\\\340.01\\\\-175.1953\\\\234.2686\\\\340.01\\\\-177.5391\\\\234.2987\\\\340.01\\\\-184.5703\\\\234.2835\\\\340.01\\\\-186.9141\\\\234.3328\\\\340.01\\\\-189.2578\\\\234.2902\\\\340.01\\\\-193.9453\\\\234.4002\\\\340.01\\\\-196.2891\\\\234.3837\\\\340.01\\\\-200.9766\\\\234.4888\\\\340.01\\\\-203.3203\\\\234.3837\\\\340.01\\\\-208.0078\\\\234.3864\\\\340.01\\\\-212.6953\\\\234.5069\\\\340.01\\\\-215.0391\\\\234.4907\\\\340.01\\\\-222.0703\\\\234.6239\\\\340.01\\\\-224.4141\\\\234.4731\\\\340.01\\\\-231.4453\\\\234.4218\\\\340.01\\\\-233.7891\\\\234.3266\\\\340.01\\\\-243.1641\\\\234.0949\\\\340.01\\\\-247.8516\\\\233.6378\\\\340.01\\\\-250.1953\\\\233.6378\\\\340.01\\\\-252.5391\\\\234.0867\\\\340.01\\\\-254.6143\\\\234.7172\\\\340.01\\\\-254.8828\\\\234.9516\\\\340.01\\\\-257.2266\\\\234.9974\\\\340.01\\\\-259.5703\\\\235.8786\\\\340.01\\\\-261.108\\\\237.0609\\\\340.01\\\\-261.9141\\\\237.891\\\\340.01\\\\-263.0544\\\\239.4047\\\\340.01\\\\-263.1094\\\\241.7484\\\\340.01\\\\-262.7858\\\\244.0922\\\\340.01\\\\-261.9141\\\\245.3383\\\\340.01\\\\-259.5703\\\\248.3687\\\\340.01\\\\-259.1593\\\\248.7797\\\\340.01\\\\-255.7086\\\\253.4672\\\\340.01\\\\-252.5391\\\\256.7179\\\\340.01\\\\-250.1953\\\\258.9929\\\\340.01\\\\-247.8516\\\\261.4776\\\\340.01\\\\-245.5078\\\\263.6263\\\\340.01\\\\-244.1031\\\\265.1859\\\\340.01\\\\-241.5987\\\\267.5297\\\\340.01\\\\-239.4125\\\\269.8734\\\\340.01\\\\-236.9858\\\\272.2172\\\\340.01\\\\-233.7891\\\\275.424\\\\340.01\\\\-231.4453\\\\277.8469\\\\340.01\\\\-229.1016\\\\280.0326\\\\340.01\\\\-226.7578\\\\282.5503\\\\340.01\\\\-225.1803\\\\283.9359\\\\340.01\\\\-222.0703\\\\287.259\\\\340.01\\\\-220.6122\\\\288.6234\\\\340.01\\\\-218.3861\\\\290.9672\\\\340.01\\\\-214.9922\\\\293.3109\\\\340.01\\\\-212.6953\\\\294.6589\\\\340.01\\\\-210.3516\\\\295.3856\\\\340.01\\\\-208.0078\\\\296.5275\\\\340.01\\\\-205.6641\\\\296.892\\\\340.01\\\\-203.3203\\\\297.0948\\\\340.01\\\\-198.6328\\\\297.0876\\\\340.01\\\\-193.9453\\\\296.9006\\\\340.01\\\\-170.5078\\\\296.8421\\\\340.01\\\\-168.1641\\\\296.8731\\\\340.01\\\\-158.7891\\\\296.8427\\\\340.01\\\\-156.4453\\\\296.8667\\\\340.01\\\\-144.7266\\\\296.8678\\\\340.01\\\\-130.6641\\\\296.8089\\\\340.01\\\\-121.2891\\\\296.7995\\\\340.01\\\\-114.2578\\\\296.8629\\\\340.01\\\\-102.5391\\\\296.9188\\\\340.01\\\\-100.1953\\\\296.9004\\\\340.01\\\\-79.10156\\\\296.9041\\\\340.01\\\\-76.75781\\\\296.9428\\\\340.01\\\\-69.72656\\\\296.9622\\\\340.01\\\\-53.32031\\\\296.9428\\\\340.01\\\\-50.97656\\\\297.0061\\\\340.01\\\\-41.60156\\\\296.9731\\\\340.01\\\\-39.25781\\\\296.8877\\\\340.01\\\\-36.91406\\\\296.9467\\\\340.01\\\\-32.22656\\\\296.9731\\\\340.01\\\\-29.88281\\\\296.8888\\\\340.01\\\\-25.19531\\\\296.8798\\\\340.01\\\\-22.85156\\\\297.0362\\\\340.01\\\\-20.50781\\\\296.8789\\\\340.01\\\\-15.82031\\\\296.9535\\\\340.01\\\\-4.101563\\\\296.8673\\\\340.01\\\\-1.757813\\\\296.9081\\\\340.01\\\\19.33594\\\\296.7952\\\\340.01\\\\21.67969\\\\296.8163\\\\340.01\\\\33.39844\\\\296.734\\\\340.01\\\\42.77344\\\\296.7324\\\\340.01\\\\49.80469\\\\296.6724\\\\340.01\\\\66.21094\\\\296.6346\\\\340.01\\\\87.30469\\\\296.5154\\\\340.01\\\\96.67969\\\\296.5208\\\\340.01\\\\99.02344\\\\296.473\\\\340.01\\\\108.3984\\\\296.466\\\\340.01\\\\110.7422\\\\296.4326\\\\340.01\\\\117.7734\\\\296.446\\\\340.01\\\\131.8359\\\\296.308\\\\340.01\\\\134.1797\\\\296.1372\\\\340.01\\\\136.5234\\\\295.8701\\\\340.01\\\\152.9297\\\\295.8012\\\\340.01\\\\157.6172\\\\295.833\\\\340.01\\\\159.9609\\\\295.7849\\\\340.01\\\\164.6484\\\\295.8172\\\\340.01\\\\171.6797\\\\295.7684\\\\340.01\\\\176.3672\\\\295.6638\\\\340.01\\\\183.3984\\\\295.6819\\\\340.01\\\\190.4297\\\\295.6093\\\\340.01\\\\192.7734\\\\295.6272\\\\340.01\\\\197.4609\\\\295.833\\\\340.01\\\\202.1484\\\\295.7173\\\\340.01\\\\204.4922\\\\295.3679\\\\340.01\\\\206.8359\\\\294.7878\\\\340.01\\\\209.1797\\\\294.3717\\\\340.01\\\\211.5234\\\\292.5075\\\\340.01\\\\213.8672\\\\291.2065\\\\340.01\\\\216.2109\\\\289.6012\\\\340.01\\\\217.2437\\\\288.6234\\\\340.01\\\\218.5547\\\\287.1462\\\\340.01\\\\221.7714\\\\283.9359\\\\340.01\\\\225.5859\\\\279.9119\\\\340.01\\\\227.9297\\\\277.6044\\\\340.01\\\\230.2734\\\\275.1119\\\\340.01\\\\233.2526\\\\272.2172\\\\340.01\\\\234.9609\\\\270.3654\\\\340.01\\\\237.8731\\\\267.5297\\\\340.01\\\\240.1082\\\\265.1859\\\\340.01\\\\242.4774\\\\262.8422\\\\340.01\\\\246.6797\\\\258.4333\\\\340.01\\\\249.248\\\\255.8109\\\\340.01\\\\251.4573\\\\253.4672\\\\340.01\\\\256.0547\\\\248.2246\\\\340.01\\\\257.4792\\\\246.4359\\\\340.01\\\\259.8469\\\\244.0922\\\\340.01\\\\260.9666\\\\241.7484\\\\340.01\\\\261.8304\\\\239.4047\\\\340.01\\\\261.0668\\\\237.0609\\\\340.01\\\\259.4996\\\\234.7172\\\\340.01\\\\258.3984\\\\233.6932\\\\340.01\\\\256.0547\\\\233.1526\\\\340.01\\\\253.7109\\\\233.0245\\\\340.01\\\\251.3672\\\\233.382\\\\340.01\\\\249.0234\\\\232.6886\\\\340.01\\\\244.3359\\\\232.8195\\\\340.01\\\\241.9922\\\\233.0007\\\\340.01\\\\232.6172\\\\233.0305\\\\340.01\\\\227.9297\\\\233.0007\\\\340.01\\\\223.2422\\\\233.0441\\\\340.01\\\\209.1797\\\\233.0108\\\\340.01\\\\206.8359\\\\233.0343\\\\340.01\\\\204.4922\\\\232.916\\\\340.01\\\\202.1484\\\\232.6179\\\\340.01\\\\195.1172\\\\232.7322\\\\340.01\\\\181.0547\\\\232.7719\\\\340.01\\\\174.0234\\\\232.7456\\\\340.01\\\\166.9922\\\\232.7614\\\\340.01\\\\148.2422\\\\232.8497\\\\340.01\\\\141.2109\\\\232.8253\\\\340.01\\\\136.5234\\\\232.721\\\\340.01\\\\134.1797\\\\232.7614\\\\340.01\\\\129.4922\\\\232.7481\\\\340.01\\\\113.0859\\\\232.8617\\\\340.01\\\\106.0547\\\\232.8735\\\\340.01\\\\103.7109\\\\232.8253\\\\340.01\\\\101.3672\\\\232.9942\\\\340.01\\\\96.67969\\\\233.1397\\\\340.01\\\\68.55469\\\\233.1483\\\\340.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851194733700001.468081152243\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"342\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"23\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-13.47656\\\\296.8499\\\\343.01\\\\14.64844\\\\296.8324\\\\343.01\\\\33.39844\\\\296.8033\\\\343.01\\\\63.86719\\\\296.7919\\\\343.01\\\\87.30469\\\\296.7566\\\\343.01\\\\117.7734\\\\296.7458\\\\343.01\\\\131.8359\\\\296.7216\\\\343.01\\\\136.5234\\\\296.6649\\\\343.01\\\\138.8672\\\\295.4763\\\\343.01\\\\157.6172\\\\295.4763\\\\343.01\\\\159.9609\\\\295.4454\\\\343.01\\\\164.6484\\\\295.4763\\\\343.01\\\\166.9922\\\\295.4454\\\\343.01\\\\176.3672\\\\295.4154\\\\343.01\\\\192.7734\\\\295.4006\\\\343.01\\\\197.4609\\\\295.4763\\\\343.01\\\\202.1484\\\\295.4303\\\\343.01\\\\204.4922\\\\295.3165\\\\343.01\\\\206.8359\\\\295.0653\\\\343.01\\\\209.1797\\\\294.4177\\\\343.01\\\\211.5234\\\\292.3443\\\\343.01\\\\213.8672\\\\291.6227\\\\343.01\\\\214.5661\\\\290.9672\\\\343.01\\\\216.2109\\\\289.7204\\\\343.01\\\\217.3604\\\\288.6234\\\\343.01\\\\219.4239\\\\286.2797\\\\343.01\\\\221.9183\\\\283.9359\\\\343.01\\\\225.5859\\\\280.0836\\\\343.01\\\\227.9297\\\\277.9224\\\\343.01\\\\230.2734\\\\275.331\\\\343.01\\\\231.0524\\\\274.5609\\\\343.01\\\\232.6172\\\\273.2719\\\\343.01\\\\233.6719\\\\272.2172\\\\343.01\\\\235.7042\\\\269.8734\\\\343.01\\\\238.2833\\\\267.5297\\\\343.01\\\\240.4319\\\\265.1859\\\\343.01\\\\244.3359\\\\261.4106\\\\343.01\\\\246.6797\\\\258.8701\\\\343.01\\\\249.0234\\\\256.7692\\\\343.01\\\\249.948\\\\255.8109\\\\343.01\\\\251.3672\\\\254.1198\\\\343.01\\\\254.5945\\\\251.1234\\\\343.01\\\\255.6744\\\\248.7797\\\\343.01\\\\257.489\\\\246.4359\\\\343.01\\\\259.8584\\\\244.0922\\\\343.01\\\\260.9205\\\\241.7484\\\\343.01\\\\261.8858\\\\239.4047\\\\343.01\\\\261.2413\\\\237.0609\\\\343.01\\\\259.3759\\\\234.7172\\\\343.01\\\\258.3984\\\\233.7629\\\\343.01\\\\256.0547\\\\232.9203\\\\343.01\\\\251.3672\\\\232.8406\\\\343.01\\\\249.0234\\\\232.6981\\\\343.01\\\\241.9922\\\\232.8407\\\\343.01\\\\234.9609\\\\232.8642\\\\343.01\\\\227.9297\\\\232.8407\\\\343.01\\\\218.5547\\\\232.8757\\\\343.01\\\\213.8672\\\\232.8525\\\\343.01\\\\206.8359\\\\232.8642\\\\343.01\\\\202.1484\\\\232.7003\\\\343.01\\\\197.4609\\\\232.7406\\\\343.01\\\\190.4297\\\\232.7537\\\\343.01\\\\174.0234\\\\232.7537\\\\343.01\\\\157.6172\\\\232.7666\\\\343.01\\\\155.2734\\\\232.792\\\\343.01\\\\141.2109\\\\232.7794\\\\343.01\\\\136.5234\\\\232.7406\\\\343.01\\\\124.8047\\\\232.7537\\\\343.01\\\\122.4609\\\\232.7794\\\\343.01\\\\103.7109\\\\232.7794\\\\343.01\\\\96.67969\\\\232.9126\\\\343.01\\\\54.49219\\\\232.9126\\\\343.01\\\\45.11719\\\\232.8902\\\\343.01\\\\42.77344\\\\232.8044\\\\343.01\\\\38.08594\\\\232.7794\\\\343.01\\\\36.64153\\\\232.3734\\\\343.01\\\\38.08594\\\\231.9234\\\\343.01\\\\40.42969\\\\229.497\\\\343.01\\\\42.77344\\\\229.0768\\\\343.01\\\\45.11719\\\\228.3851\\\\343.01\\\\47.46094\\\\226.9828\\\\343.01\\\\49.80469\\\\226.2342\\\\343.01\\\\52.14844\\\\224.5333\\\\343.01\\\\54.49219\\\\223.435\\\\343.01\\\\54.94049\\\\222.9984\\\\343.01\\\\56.83594\\\\221.7432\\\\343.01\\\\58.32031\\\\220.6547\\\\343.01\\\\59.17969\\\\219.8756\\\\343.01\\\\61.52344\\\\219.154\\\\343.01\\\\64.5971\\\\215.9672\\\\343.01\\\\66.21094\\\\214.6717\\\\343.01\\\\67.30957\\\\213.6234\\\\343.01\\\\69.44305\\\\211.2797\\\\343.01\\\\71.93533\\\\208.9359\\\\343.01\\\\75.58594\\\\204.9751\\\\343.01\\\\76.34663\\\\204.2484\\\\343.01\\\\77.07796\\\\201.9047\\\\343.01\\\\79.4009\\\\199.5609\\\\343.01\\\\80.80936\\\\197.2172\\\\343.01\\\\82.61719\\\\194.8027\\\\343.01\\\\83.81027\\\\192.5297\\\\343.01\\\\84.70689\\\\190.1859\\\\343.01\\\\86.036\\\\187.8422\\\\343.01\\\\87.30469\\\\185.8838\\\\343.01\\\\87.68246\\\\185.4984\\\\343.01\\\\88.88221\\\\180.8109\\\\343.01\\\\89.11252\\\\178.4672\\\\343.01\\\\89.64844\\\\177.7911\\\\343.01\\\\90.63388\\\\176.1234\\\\343.01\\\\90.79569\\\\173.7797\\\\343.01\\\\91.10613\\\\171.4359\\\\343.01\\\\92.53577\\\\169.0922\\\\343.01\\\\92.74599\\\\166.7484\\\\343.01\\\\92.91211\\\\162.0609\\\\343.01\\\\93.33398\\\\159.7172\\\\343.01\\\\93.3525\\\\150.3422\\\\343.01\\\\92.9874\\\\147.9984\\\\343.01\\\\92.81616\\\\143.3109\\\\343.01\\\\92.65137\\\\140.9672\\\\343.01\\\\92.28917\\\\138.6234\\\\343.01\\\\91.99219\\\\138.295\\\\343.01\\\\90.88379\\\\136.2797\\\\343.01\\\\90.72434\\\\133.9359\\\\343.01\\\\89.63928\\\\131.5922\\\\343.01\\\\89.00118\\\\129.2484\\\\343.01\\\\88.44993\\\\126.9047\\\\343.01\\\\88.13577\\\\124.5609\\\\343.01\\\\87.30469\\\\123.6708\\\\343.01\\\\86.17751\\\\122.2172\\\\343.01\\\\85.52595\\\\119.8734\\\\343.01\\\\84.34743\\\\117.5297\\\\343.01\\\\83.591\\\\115.1859\\\\343.01\\\\81.36161\\\\112.8422\\\\343.01\\\\80.40365\\\\110.4984\\\\343.01\\\\77.92969\\\\107.4057\\\\343.01\\\\76.88994\\\\105.8109\\\\343.01\\\\75.58594\\\\104.1483\\\\343.01\\\\72.53094\\\\101.1234\\\\343.01\\\\70.89844\\\\99.3583\\\\343.01\\\\68.03966\\\\96.43594\\\\343.01\\\\66.21094\\\\94.85841\\\\343.01\\\\63.2178\\\\91.74844\\\\343.01\\\\59.9375\\\\89.40469\\\\343.01\\\\56.83594\\\\86.85168\\\\343.01\\\\54.49219\\\\85.89411\\\\343.01\\\\52.14844\\\\83.75636\\\\343.01\\\\49.80469\\\\83.0305\\\\343.01\\\\47.46094\\\\81.82986\\\\343.01\\\\45.11719\\\\81.13115\\\\343.01\\\\43.61636\\\\80.02969\\\\343.01\\\\42.77344\\\\79.26299\\\\343.01\\\\40.42969\\\\78.94232\\\\343.01\\\\38.08594\\\\78.44798\\\\343.01\\\\35.74219\\\\77.71319\\\\343.01\\\\33.39844\\\\76.62014\\\\343.01\\\\31.05469\\\\76.45058\\\\343.01\\\\28.99255\\\\75.34219\\\\343.01\\\\28.71094\\\\75.08814\\\\343.01\\\\26.36719\\\\74.78448\\\\343.01\\\\24.02344\\\\74.61057\\\\343.01\\\\21.67969\\\\74.54632\\\\343.01\\\\19.33594\\\\74.09106\\\\343.01\\\\9.960938\\\\74.04196\\\\343.01\\\\5.273438\\\\74.09106\\\\343.01\\\\2.929688\\\\74.55463\\\\343.01\\\\0.5859375\\\\74.63775\\\\343.01\\\\-4.101563\\\\75.10286\\\\343.01\\\\-6.445313\\\\76.49429\\\\343.01\\\\-8.789063\\\\76.67171\\\\343.01\\\\-10.42799\\\\77.68594\\\\343.01\\\\-11.13281\\\\78.2649\\\\343.01\\\\-13.47656\\\\78.54448\\\\343.01\\\\-15.82031\\\\79.10727\\\\343.01\\\\-18.16406\\\\79.86597\\\\343.01\\\\-18.33274\\\\80.02969\\\\343.01\\\\-22.37745\\\\82.37344\\\\343.01\\\\-22.85156\\\\82.77675\\\\343.01\\\\-25.19531\\\\83.57171\\\\343.01\\\\-27.22969\\\\84.71719\\\\343.01\\\\-27.53906\\\\84.99342\\\\343.01\\\\-29.88281\\\\86.06935\\\\343.01\\\\-32.22656\\\\87.99243\\\\343.01\\\\-34.57031\\\\90.32504\\\\343.01\\\\-36.91406\\\\91.11633\\\\343.01\\\\-37.53859\\\\91.74844\\\\343.01\\\\-41.60156\\\\95.4947\\\\343.01\\\\-44.75509\\\\98.77969\\\\343.01\\\\-46.28906\\\\100.0988\\\\343.01\\\\-47.29911\\\\101.1234\\\\343.01\\\\-48.63281\\\\102.8523\\\\343.01\\\\-51.72631\\\\105.8109\\\\343.01\\\\-52.41647\\\\108.1547\\\\343.01\\\\-53.32031\\\\109.1202\\\\343.01\\\\-54.39367\\\\110.4984\\\\343.01\\\\-55.85938\\\\112.8422\\\\343.01\\\\-57.07632\\\\115.1859\\\\343.01\\\\-58.80323\\\\117.5297\\\\343.01\\\\-59.511\\\\119.8734\\\\343.01\\\\-60.86516\\\\122.2172\\\\343.01\\\\-61.57744\\\\124.5609\\\\343.01\\\\-61.88775\\\\126.9047\\\\343.01\\\\-63.71547\\\\129.2484\\\\343.01\\\\-64.02027\\\\131.5922\\\\343.01\\\\-64.8298\\\\133.9359\\\\343.01\\\\-65.21739\\\\136.2797\\\\343.01\\\\-66.03065\\\\138.6234\\\\343.01\\\\-66.32159\\\\145.6547\\\\343.01\\\\-66.52577\\\\147.9984\\\\343.01\\\\-68.15379\\\\150.3422\\\\343.01\\\\-68.19411\\\\152.6859\\\\343.01\\\\-68.12855\\\\159.7172\\\\343.01\\\\-67.38281\\\\160.578\\\\343.01\\\\-66.33832\\\\162.0609\\\\343.01\\\\-66.11034\\\\169.0922\\\\343.01\\\\-65.01181\\\\173.7797\\\\343.01\\\\-64.67431\\\\176.1234\\\\343.01\\\\-63.80735\\\\178.4672\\\\343.01\\\\-63.54492\\\\180.8109\\\\343.01\\\\-61.65305\\\\183.1547\\\\343.01\\\\-61.42054\\\\185.4984\\\\343.01\\\\-60.49805\\\\187.8422\\\\343.01\\\\-58.00781\\\\191.825\\\\343.01\\\\-57.387\\\\192.5297\\\\343.01\\\\-56.74913\\\\194.8734\\\\343.01\\\\-55.66406\\\\195.936\\\\343.01\\\\-54.58899\\\\197.2172\\\\343.01\\\\-53.60318\\\\199.5609\\\\343.01\\\\-50.97656\\\\203.258\\\\343.01\\\\-48.00071\\\\206.5922\\\\343.01\\\\-45.5013\\\\208.9359\\\\343.01\\\\-43.50982\\\\211.2797\\\\343.01\\\\-41.60156\\\\212.84\\\\343.01\\\\-39.25781\\\\215.3739\\\\343.01\\\\-35.93977\\\\218.3109\\\\343.01\\\\-34.57031\\\\219.3768\\\\343.01\\\\-32.52378\\\\220.6547\\\\343.01\\\\-32.22656\\\\220.9395\\\\343.01\\\\-29.88281\\\\221.9077\\\\343.01\\\\-28.61672\\\\222.9984\\\\343.01\\\\-27.53906\\\\224.1391\\\\343.01\\\\-25.19531\\\\224.7669\\\\343.01\\\\-24.54516\\\\225.3422\\\\343.01\\\\-20.89297\\\\227.6859\\\\343.01\\\\-20.50781\\\\228.0531\\\\343.01\\\\-18.16406\\\\228.8638\\\\343.01\\\\-15.82031\\\\229.1627\\\\343.01\\\\-14.73801\\\\230.0297\\\\343.01\\\\-13.47656\\\\231.7823\\\\343.01\\\\-11.13281\\\\231.9774\\\\343.01\\\\-10.70429\\\\232.3734\\\\343.01\\\\-11.13281\\\\232.7537\\\\343.01\\\\-15.82031\\\\232.8044\\\\343.01\\\\-20.50781\\\\232.9015\\\\343.01\\\\-34.57031\\\\232.9126\\\\343.01\\\\-41.60156\\\\232.9345\\\\343.01\\\\-58.00781\\\\232.9453\\\\343.01\\\\-60.35156\\\\232.9345\\\\343.01\\\\-62.69531\\\\233.5567\\\\343.01\\\\-65.03906\\\\232.8672\\\\343.01\\\\-83.78906\\\\232.8788\\\\343.01\\\\-93.16406\\\\232.9126\\\\343.01\\\\-95.50781\\\\233.5393\\\\343.01\\\\-97.85156\\\\233.7035\\\\343.01\\\\-102.5391\\\\233.7288\\\\343.01\\\\-121.2891\\\\233.7406\\\\343.01\\\\-137.6953\\\\233.7202\\\\343.01\\\\-142.3828\\\\233.7337\\\\343.01\\\\-163.4766\\\\233.7135\\\\343.01\\\\-168.1641\\\\233.7435\\\\343.01\\\\-189.2578\\\\233.76\\\\343.01\\\\-200.9766\\\\233.788\\\\343.01\\\\-208.0078\\\\233.7738\\\\343.01\\\\-222.0703\\\\233.8144\\\\343.01\\\\-243.1641\\\\233.72\\\\343.01\\\\-247.8516\\\\233.5974\\\\343.01\\\\-250.1953\\\\233.592\\\\343.01\\\\-252.5391\\\\233.6761\\\\343.01\\\\-254.8828\\\\233.8458\\\\343.01\\\\-257.2266\\\\233.8853\\\\343.01\\\\-258.0715\\\\234.7172\\\\343.01\\\\-260.9406\\\\237.0609\\\\343.01\\\\-261.9141\\\\237.9809\\\\343.01\\\\-263.0811\\\\239.4047\\\\343.01\\\\-263.0952\\\\241.7484\\\\343.01\\\\-262.8482\\\\244.0922\\\\343.01\\\\-261.1393\\\\246.4359\\\\343.01\\\\-259.5703\\\\248.77\\\\343.01\\\\-257.2359\\\\251.1234\\\\343.01\\\\-256.1725\\\\253.4672\\\\343.01\\\\-254.8828\\\\254.541\\\\343.01\\\\-253.6187\\\\255.8109\\\\343.01\\\\-252.5391\\\\257.1211\\\\343.01\\\\-248.9935\\\\260.4984\\\\343.01\\\\-246.7643\\\\262.8422\\\\343.01\\\\-243.1641\\\\266.3453\\\\343.01\\\\-239.7483\\\\269.8734\\\\343.01\\\\-238.4766\\\\270.9653\\\\343.01\\\\-237.2486\\\\272.2172\\\\343.01\\\\-236.1328\\\\273.5273\\\\343.01\\\\-235.0992\\\\274.5609\\\\343.01\\\\-233.7891\\\\275.6767\\\\343.01\\\\-232.5432\\\\276.9047\\\\343.01\\\\-231.4453\\\\278.1825\\\\343.01\\\\-227.9297\\\\281.5922\\\\343.01\\\\-224.4141\\\\285.1934\\\\343.01\\\\-222.0703\\\\287.4207\\\\343.01\\\\-218.7154\\\\290.9672\\\\343.01\\\\-217.3828\\\\291.9945\\\\343.01\\\\-215.0391\\\\293.3382\\\\343.01\\\\-212.6953\\\\294.5551\\\\343.01\\\\-210.3516\\\\295.3438\\\\343.01\\\\-209.9888\\\\295.6547\\\\343.01\\\\-208.0078\\\\296.7353\\\\343.01\\\\-203.3203\\\\296.9163\\\\343.01\\\\-198.6328\\\\296.9163\\\\343.01\\\\-193.9453\\\\296.8423\\\\343.01\\\\-179.8828\\\\296.8319\\\\343.01\\\\-168.1641\\\\296.8428\\\\343.01\\\\-161.1328\\\\296.8266\\\\343.01\\\\-147.0703\\\\296.8376\\\\343.01\\\\-121.2891\\\\296.8209\\\\343.01\\\\-111.9141\\\\296.8435\\\\343.01\\\\-97.85156\\\\296.855\\\\343.01\\\\-90.82031\\\\296.838\\\\343.01\\\\-65.03906\\\\296.8668\\\\343.01\\\\-53.32031\\\\296.8553\\\\343.01\\\\-51.43964\\\\297.9984\\\\343.01\\\\-50.97656\\\\298.3864\\\\343.01\\\\-48.63281\\\\297.5516\\\\343.01\\\\-46.28906\\\\298.5331\\\\343.01\\\\-43.94531\\\\298.5331\\\\343.01\\\\-41.60156\\\\298.2598\\\\343.01\\\\-39.25781\\\\296.8499\\\\343.01\\\\-36.91406\\\\297.9681\\\\343.01\\\\-32.22656\\\\298.5331\\\\343.01\\\\-29.88281\\\\296.8741\\\\343.01\\\\-27.53906\\\\296.8383\\\\343.01\\\\-25.19531\\\\296.9201\\\\343.01\\\\-22.85156\\\\298.5331\\\\343.01\\\\-20.50781\\\\296.8441\\\\343.01\\\\-18.16406\\\\298.1147\\\\343.01\\\\-15.82031\\\\297.8174\\\\343.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"10\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,002a\" : {\n"
"               \"Name\" : \"ROIDisplayColor\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"255\\\\0\\\\255\"\n"
"            },\n"
"            \"3006,0040\" : {\n"
"               \"Name\" : \"ContourSequence\",\n"
"               \"Type\" : \"Sequence\",\n"
"               \"Value\" : [\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699844642358900001.492074456493\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"0\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"13.63579\\\\147.5839\\\\274.01\\\\11.63579\\\\147.882\\\\274.01\\\\10.13579\\\\148.3365\\\\274.01\\\\9.135789\\\\148.7989\\\\274.01\\\\7.635788\\\\149.7859\\\\274.01\\\\6.037574\\\\151.4199\\\\274.01\\\\5.095466\\\\152.9199\\\\274.01\\\\4.507\\\\154.4199\\\\274.01\\\\4.299851\\\\155.4199\\\\274.01\\\\4.099024\\\\157.4199\\\\274.01\\\\4.240627\\\\158.9199\\\\274.01\\\\4.522152\\\\160.4199\\\\274.01\\\\5.437513\\\\162.4199\\\\274.01\\\\6.109003\\\\163.4199\\\\274.01\\\\7.635788\\\\164.9824\\\\274.01\\\\9.135789\\\\165.9763\\\\274.01\\\\10.63579\\\\166.5941\\\\274.01\\\\12.13579\\\\166.9426\\\\274.01\\\\13.63579\\\\167.0941\\\\274.01\\\\15.13579\\\\166.988\\\\274.01\\\\17.13579\\\\166.537\\\\274.01\\\\19.13579\\\\165.5782\\\\274.01\\\\20.63579\\\\164.3655\\\\274.01\\\\22.19224\\\\162.4199\\\\274.01\\\\22.7061\\\\161.4199\\\\274.01\\\\23.21912\\\\159.9199\\\\274.01\\\\23.42485\\\\158.9199\\\\274.01\\\\23.5186\\\\156.9199\\\\274.01\\\\23.23427\\\\154.9199\\\\274.01\\\\22.72173\\\\153.4199\\\\274.01\\\\22.22746\\\\152.4199\\\\274.01\\\\21.13579\\\\150.9318\\\\274.01\\\\20.09829\\\\149.9199\\\\274.01\\\\18.63579\\\\148.8949\\\\274.01\\\\17.13579\\\\148.2402\\\\274.01\\\\15.63579\\\\147.8214\\\\274.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845076383700001.470610289105\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"47\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"1\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-2.854718\\\\157.4199\\\\277.01\\\\-2.472545\\\\160.9199\\\\277.01\\\\-1.487775\\\\163.9199\\\\277.01\\\\-0.4315192\\\\165.9199\\\\277.01\\\\0.9787372\\\\167.9199\\\\277.01\\\\2.406915\\\\169.4199\\\\277.01\\\\3.635788\\\\170.4986\\\\277.01\\\\5.135788\\\\171.5571\\\\277.01\\\\6.135788\\\\172.111\\\\277.01\\\\8.135789\\\\173.0158\\\\277.01\\\\10.13579\\\\173.6004\\\\277.01\\\\12.63579\\\\174.0313\\\\277.01\\\\15.13579\\\\174.0013\\\\277.01\\\\17.63579\\\\173.571\\\\277.01\\\\19.13579\\\\173.1137\\\\277.01\\\\21.63579\\\\172.0419\\\\277.01\\\\23.13579\\\\171.1081\\\\277.01\\\\24.63579\\\\169.9456\\\\277.01\\\\26.19738\\\\168.4199\\\\277.01\\\\27.74516\\\\166.4199\\\\277.01\\\\28.32924\\\\165.4199\\\\277.01\\\\29.29383\\\\163.4199\\\\277.01\\\\29.81079\\\\161.9199\\\\277.01\\\\30.28139\\\\159.9199\\\\277.01\\\\30.45172\\\\157.4199\\\\277.01\\\\30.29068\\\\154.9199\\\\277.01\\\\29.82399\\\\152.9199\\\\277.01\\\\28.86814\\\\150.4199\\\\277.01\\\\27.79128\\\\148.4199\\\\277.01\\\\26.27724\\\\146.4199\\\\277.01\\\\24.63579\\\\144.8084\\\\277.01\\\\22.63579\\\\143.3281\\\\277.01\\\\21.63579\\\\142.7463\\\\277.01\\\\19.63579\\\\141.8135\\\\277.01\\\\18.13579\\\\141.3103\\\\277.01\\\\15.63579\\\\140.7907\\\\277.01\\\\13.63579\\\\140.6754\\\\277.01\\\\12.13579\\\\140.7704\\\\277.01\\\\9.635789\\\\141.2671\\\\277.01\\\\8.135789\\\\141.7522\\\\277.01\\\\5.135788\\\\143.2278\\\\277.01\\\\3.635788\\\\144.2585\\\\277.01\\\\2.350577\\\\145.4199\\\\277.01\\\\0.5107885\\\\147.4199\\\\277.01\\\\-0.5040925\\\\148.9199\\\\277.01\\\\-1.520461\\\\150.9199\\\\277.01\\\\-2.489212\\\\153.9199\\\\277.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699845095384800001.485181436634\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"52\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"2\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n";
const char* k_rtStruct_json06 =
"                        \"Value\" : \"-5.041041\\\\149.9199\\\\280.01\\\\-6.018211\\\\152.9199\\\\280.01\\\\-6.511849\\\\155.9199\\\\280.01\\\\-6.502212\\\\158.9199\\\\280.01\\\\-5.99526\\\\161.9199\\\\280.01\\\\-5.006773\\\\164.9199\\\\280.01\\\\-4.038581\\\\166.9199\\\\280.01\\\\-3.442042\\\\167.9199\\\\280.01\\\\-2.039212\\\\169.9199\\\\280.01\\\\-1.135438\\\\170.9199\\\\280.01\\\\1.135789\\\\173.1176\\\\280.01\\\\3.135788\\\\174.5681\\\\280.01\\\\6.135788\\\\176.1244\\\\280.01\\\\8.635789\\\\177.0288\\\\280.01\\\\11.13579\\\\177.5844\\\\280.01\\\\13.63579\\\\177.7594\\\\280.01\\\\16.63579\\\\177.5678\\\\280.01\\\\18.63579\\\\177.1104\\\\280.01\\\\21.63579\\\\176.0678\\\\280.01\\\\23.63579\\\\175.0686\\\\280.01\\\\25.13579\\\\174.1177\\\\280.01\\\\27.63579\\\\172.1109\\\\280.01\\\\29.24222\\\\170.4199\\\\280.01\\\\30.74459\\\\168.4199\\\\280.01\\\\31.88371\\\\166.4199\\\\280.01\\\\32.81119\\\\164.4199\\\\280.01\\\\33.32179\\\\162.9199\\\\280.01\\\\33.82031\\\\160.9199\\\\280.01\\\\34.18623\\\\157.4199\\\\280.01\\\\34.07163\\\\155.9199\\\\280.01\\\\33.74713\\\\153.4199\\\\280.01\\\\32.83294\\\\150.4199\\\\280.01\\\\32.18903\\\\148.9199\\\\280.01\\\\30.80605\\\\146.4199\\\\280.01\\\\29.31239\\\\144.4199\\\\280.01\\\\28.13579\\\\143.1796\\\\280.01\\\\26.63579\\\\141.776\\\\280.01\\\\24.63579\\\\140.3194\\\\280.01\\\\23.63579\\\\139.7211\\\\280.01\\\\21.63579\\\\138.7252\\\\280.01\\\\19.13579\\\\137.8053\\\\280.01\\\\16.63579\\\\137.2421\\\\280.01\\\\13.63579\\\\137.0479\\\\280.01\\\\10.63579\\\\137.2928\\\\280.01\\\\8.635789\\\\137.7512\\\\280.01\\\\7.135788\\\\138.2404\\\\280.01\\\\4.635788\\\\139.3442\\\\280.01\\\\3.135788\\\\140.2151\\\\280.01\\\\1.635789\\\\141.2464\\\\280.01\\\\-0.8642115\\\\143.5646\\\\280.01\\\\-2.084666\\\\144.9199\\\\280.01\\\\-3.518559\\\\146.9199\\\\280.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846442461900001.533642576430\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"35\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"3\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-36.86421\\\\165.9127\\\\283.01\\\\-33.86421\\\\165.5801\\\\283.01\\\\-32.36421\\\\165.0618\\\\283.01\\\\-31.36421\\\\164.5518\\\\283.01\\\\-29.86421\\\\163.5032\\\\283.01\\\\-28.76573\\\\162.4199\\\\283.01\\\\-27.6906\\\\160.9199\\\\283.01\\\\-26.75524\\\\158.9199\\\\283.01\\\\-26.24242\\\\156.9199\\\\283.01\\\\-26.08132\\\\155.4199\\\\283.01\\\\-26.27046\\\\153.4199\\\\283.01\\\\-26.69974\\\\151.9199\\\\283.01\\\\-27.3034\\\\150.4199\\\\283.01\\\\-28.22451\\\\148.9199\\\\283.01\\\\-29.07254\\\\147.9199\\\\283.01\\\\-30.86421\\\\146.4123\\\\283.01\\\\-32.86421\\\\145.387\\\\283.01\\\\-34.86421\\\\144.8136\\\\283.01\\\\-36.36421\\\\144.637\\\\283.01\\\\-38.86421\\\\144.8261\\\\283.01\\\\-40.36421\\\\145.2685\\\\283.01\\\\-41.86421\\\\145.9129\\\\283.01\\\\-43.36421\\\\146.839\\\\283.01\\\\-45.00815\\\\148.4199\\\\283.01\\\\-46.03782\\\\149.9199\\\\283.01\\\\-46.92191\\\\151.9199\\\\283.01\\\\-47.13395\\\\152.9199\\\\283.01\\\\-47.39546\\\\155.4199\\\\283.01\\\\-47.01165\\\\158.4199\\\\283.01\\\\-46.47908\\\\159.9199\\\\283.01\\\\-45.95449\\\\160.9199\\\\283.01\\\\-44.36421\\\\162.8921\\\\283.01\\\\-42.36421\\\\164.4824\\\\283.01\\\\-41.36421\\\\165.0054\\\\283.01\\\\-39.86421\\\\165.5288\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846477463900001.467218939844\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"4\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"6.135788\\\\112.0556\\\\283.01\\\\8.135789\\\\113.058\\\\283.01\\\\9.635789\\\\113.5261\\\\283.01\\\\11.13579\\\\113.7211\\\\283.01\\\\13.13579\\\\113.7083\\\\283.01\\\\14.63579\\\\113.4775\\\\283.01\\\\17.13579\\\\112.4942\\\\283.01\\\\18.63579\\\\111.5302\\\\283.01\\\\20.27973\\\\109.9199\\\\283.01\\\\21.29551\\\\108.4199\\\\283.01\\\\22.15553\\\\106.4199\\\\283.01\\\\22.49763\\\\104.4199\\\\283.01\\\\22.56146\\\\102.9199\\\\283.01\\\\22.45816\\\\101.4199\\\\283.01\\\\22.20454\\\\99.91986\\\\283.01\\\\21.35107\\\\97.91986\\\\283.01\\\\20.75722\\\\96.91986\\\\283.01\\\\19.13579\\\\95.12147\\\\283.01\\\\18.13579\\\\94.30956\\\\283.01\\\\17.13579\\\\93.71847\\\\283.01\\\\15.13579\\\\92.88696\\\\283.01\\\\13.63579\\\\92.61065\\\\283.01\\\\11.63579\\\\92.5317\\\\283.01\\\\9.135789\\\\92.82371\\\\283.01\\\\7.635788\\\\93.33433\\\\283.01\\\\6.635788\\\\93.82958\\\\283.01\\\\5.135788\\\\94.8808\\\\283.01\\\\4.049851\\\\95.91986\\\\283.01\\\\2.948288\\\\97.41986\\\\283.01\\\\1.984473\\\\99.41986\\\\283.01\\\\1.529539\\\\100.9199\\\\283.01\\\\1.326578\\\\103.4199\\\\283.01\\\\1.542039\\\\105.4199\\\\283.01\\\\2.010788\\\\106.9199\\\\283.01\\\\3.000074\\\\108.9199\\\\283.01\\\\4.135788\\\\110.3525\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846504465400001.571321740640\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"60\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"5\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"9.635789\\\\134.821\\\\283.01\\\\6.635788\\\\135.7311\\\\283.01\\\\5.135788\\\\136.3564\\\\283.01\\\\3.135788\\\\137.3443\\\\283.01\\\\1.635789\\\\138.235\\\\283.01\\\\0.1357885\\\\139.3028\\\\283.01\\\\-1.698245\\\\140.9199\\\\283.01\\\\-4.057226\\\\143.4199\\\\283.01\\\\-5.492074\\\\145.4199\\\\283.01\\\\-6.94348\\\\147.9199\\\\283.01\\\\-8.519453\\\\151.4199\\\\283.01\\\\-9.020194\\\\152.9199\\\\283.01\\\\-9.438021\\\\154.9199\\\\283.01\\\\-9.607633\\\\157.4199\\\\283.01\\\\-9.515528\\\\158.9199\\\\283.01\\\\-8.990245\\\\161.4199\\\\283.01\\\\-7.952794\\\\164.4199\\\\283.01\\\\-7.072832\\\\166.4199\\\\283.01\\\\-6.031465\\\\168.4199\\\\283.01\\\\-5.07724\\\\169.9199\\\\283.01\\\\-3.571932\\\\171.9199\\\\283.01\\\\-2.141343\\\\173.4199\\\\283.01\\\\-0.3642115\\\\175.0795\\\\283.01\\\\1.635789\\\\176.5706\\\\283.01\\\\3.135788\\\\177.5459\\\\283.01\\\\5.635788\\\\178.9883\\\\283.01\\\\7.635788\\\\180.0143\\\\283.01\\\\9.135789\\\\180.633\\\\283.01\\\\12.13579\\\\181.4936\\\\283.01\\\\13.63579\\\\181.6266\\\\283.01\\\\15.13579\\\\181.5587\\\\283.01\\\\17.63579\\\\181.0121\\\\283.01\\\\20.13579\\\\179.997\\\\283.01\\\\23.63579\\\\178.0965\\\\283.01\\\\26.63579\\\\176.1115\\\\283.01\\\\28.63579\\\\174.5219\\\\283.01\\\\30.30579\\\\172.9199\\\\283.01\\\\32.35387\\\\170.4199\\\\283.01\\\\33.34441\\\\168.9199\\\\283.01\\\\34.6741\\\\166.4199\\\\283.01\\\\35.77129\\\\163.9199\\\\283.01\\\\36.76814\\\\160.9199\\\\283.01\\\\37.28946\\\\158.4199\\\\283.01\\\\37.36674\\\\156.4199\\\\283.01\\\\37.27414\\\\154.9199\\\\283.01\\\\36.83626\\\\152.9199\\\\283.01\\\\36.33173\\\\151.4199\\\\283.01\\\\35.21079\\\\148.9199\\\\283.01\\\\33.79925\\\\146.4199\\\\283.01\\\\32.80308\\\\144.9199\\\\283.01\\\\30.83086\\\\142.4199\\\\283.01\\\\29.13579\\\\140.7037\\\\283.01\\\\26.63579\\\\138.7033\\\\283.01\\\\25.13579\\\\137.7264\\\\283.01\\\\23.13579\\\\136.6915\\\\283.01\\\\21.13579\\\\135.8365\\\\283.01\\\\19.63579\\\\135.3254\\\\283.01\\\\17.63579\\\\134.7876\\\\283.01\\\\14.63579\\\\134.3352\\\\283.01\\\\12.63579\\\\134.3453\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699846528466800001.465226999207\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"35\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"6\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"11.63579\\\\193.3493\\\\283.01\\\\9.135789\\\\194.3455\\\\283.01\\\\7.635788\\\\195.3096\\\\283.01\\\\5.991849\\\\196.9199\\\\283.01\\\\4.976066\\\\198.4199\\\\283.01\\\\4.116052\\\\200.4199\\\\283.01\\\\3.773946\\\\202.4199\\\\283.01\\\\3.710113\\\\203.9199\\\\283.01\\\\3.81342\\\\205.4199\\\\283.01\\\\4.067039\\\\206.9199\\\\283.01\\\\4.920511\\\\208.9199\\\\283.01\\\\5.51436\\\\209.9199\\\\283.01\\\\7.135788\\\\211.7182\\\\283.01\\\\8.135789\\\\212.5302\\\\283.01\\\\9.135789\\\\213.1212\\\\283.01\\\\11.13579\\\\213.9528\\\\283.01\\\\12.63579\\\\214.2291\\\\283.01\\\\14.63579\\\\214.308\\\\283.01\\\\17.13579\\\\214.016\\\\283.01\\\\18.63579\\\\213.5054\\\\283.01\\\\19.63579\\\\213.0101\\\\283.01\\\\21.13579\\\\211.9589\\\\283.01\\\\22.22173\\\\210.9199\\\\283.01\\\\23.32329\\\\209.4199\\\\283.01\\\\24.2871\\\\207.4199\\\\283.01\\\\24.74204\\\\205.9199\\\\283.01\\\\24.945\\\\203.4199\\\\283.01\\\\24.72954\\\\201.4199\\\\283.01\\\\24.26079\\\\199.9199\\\\283.01\\\\23.2715\\\\197.9199\\\\283.01\\\\22.13579\\\\196.4872\\\\283.01\\\\20.13579\\\\194.774\\\\283.01\\\\18.13579\\\\193.7817\\\\283.01\\\\16.63579\\\\193.3109\\\\283.01\\\\14.13579\\\\193.0843\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848817597700001.509440095881\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"39\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"7\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"73.62865\\\\152.4199\\\\283.01\\\\73.25758\\\\149.9199\\\\283.01\\\\72.72131\\\\148.4199\\\\283.01\\\\72.19829\\\\147.4199\\\\283.01\\\\70.63579\\\\145.5002\\\\283.01\\\\69.13579\\\\144.2413\\\\283.01\\\\67.63579\\\\143.3861\\\\283.01\\\\66.13579\\\\142.8237\\\\283.01\\\\64.63579\\\\142.5712\\\\283.01\\\\63.13579\\\\142.4671\\\\283.01\\\\60.13579\\\\142.7724\\\\283.01\\\\58.63579\\\\143.2817\\\\283.01\\\\57.63579\\\\143.7984\\\\283.01\\\\56.13579\\\\144.8652\\\\283.01\\\\55.05246\\\\145.9199\\\\283.01\\\\53.9715\\\\147.4199\\\\283.01\\\\53.02681\\\\149.4199\\\\283.01\\\\52.51399\\\\151.4199\\\\283.01\\\\52.35289\\\\152.9199\\\\283.01\\\\52.53963\\\\154.9199\\\\283.01\\\\52.945\\\\156.4199\\\\283.01\\\\53.53444\\\\157.9199\\\\283.01\\\\54.11373\\\\158.9199\\\\283.01\\\\55.13579\\\\160.2145\\\\283.01\\\\56.13579\\\\161.186\\\\283.01\\\\57.13579\\\\161.9566\\\\283.01\\\\59.13579\\\\163.0032\\\\283.01\\\\60.63579\\\\163.4761\\\\283.01\\\\62.13579\\\\163.6955\\\\283.01\\\\64.13579\\\\163.6699\\\\283.01\\\\65.13579\\\\163.5261\\\\283.01\\\\66.63579\\\\163.0843\\\\283.01\\\\68.13579\\\\162.4546\\\\283.01\\\\69.63579\\\\161.5154\\\\283.01\\\\70.63579\\\\160.6282\\\\283.01\\\\71.68726\\\\159.4199\\\\283.01\\\\72.3094\\\\158.4199\\\\283.01\\\\73.20454\\\\156.4199\\\\283.01\\\\73.48447\\\\154.9199\\\\283.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848849599600001.538291679804\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"46\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"8\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-17.14754\\\\156.4199\\\\286.01\\\\-17.54129\\\\154.9199\\\\286.01\\\\-18.18997\\\\153.4199\\\\286.01\\\\-19.27567\\\\151.4199\\\\286.01\\\\-22.23767\\\\146.4199\\\\286.01\\\\-23.71109\\\\144.4199\\\\286.01\\\\-25.86421\\\\142.2542\\\\286.01\\\\-27.86421\\\\140.7934\\\\286.01\\\\-29.86421\\\\139.7504\\\\286.01\\\\-32.36421\\\\138.8496\\\\286.01\\\\-34.86421\\\\138.3014\\\\286.01\\\\-37.86421\\\\138.2204\\\\286.01\\\\-38.86421\\\\138.315\\\\286.01\\\\-40.86421\\\\138.7523\\\\286.01\\\\-42.36421\\\\139.2324\\\\286.01\\\\-44.86421\\\\140.3181\\\\286.01\\\\-46.36421\\\\141.2324\\\\286.01\\\\-48.36421\\\\142.7916\\\\286.01\\\\-49.47421\\\\143.9199\\\\286.01\\\\-51.016\\\\145.9199\\\\286.01\\\\-51.87383\\\\147.4199\\\\286.01\\\\-52.97685\\\\149.9199\\\\286.01\\\\-53.49453\\\\151.9199\\\\286.01\\\\-53.73249\\\\155.4199\\\\286.01\\\\-53.47443\\\\158.9199\\\\286.01\\\\-52.93605\\\\160.9199\\\\286.01\\\\-52.55032\\\\161.9199\\\\286.01\\\\-51.54154\\\\163.9199\\\\286.01\\\\-50.53799\\\\165.4199\\\\286.01\\\\-49.36421\\\\166.7567\\\\286.01\\\\-46.86421\\\\169.0795\\\\286.01\\\\-45.36421\\\\170.0664\\\\286.01\\\\-43.36421\\\\171.06\\\\286.01\\\\-41.86421\\\\171.5856\\\\286.01\\\\-39.86421\\\\172.0751\\\\286.01\\\\-36.86421\\\\172.2774\\\\286.01\\\\-33.36421\\\\172.0395\\\\286.01\\\\-31.36421\\\\171.5115\\\\286.01\\\\-30.36421\\\\171.1171\\\\286.01\\\\-28.36421\\\\170.1204\\\\286.01\\\\-26.86421\\\\169.1277\\\\286.01\\\\-25.43843\\\\167.9199\\\\286.01\\\\-24.36421\\\\166.8613\\\\286.01\\\\-22.68564\\\\164.9199\\\\286.01\\\\-20.18366\\\\161.4199\\\\286.01\\\\-18.24384\\\\158.4199\\\\286.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848866600500001.541849142008\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"143\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"9\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-14.57135\\\\153.4199\\\\286.01\\\\-15.49516\\\\155.4199\\\\286.01\\\\-15.75707\\\\156.4199\\\\286.01\\\\-15.78467\\\\157.4199\\\\286.01\\\\-15.0439\\\\158.9199\\\\286.01\\\\-14.02238\\\\160.4199\\\\286.01\\\\-11.75342\\\\163.4199\\\\286.01\\\\-10.05119\\\\165.9199\\\\286.01\\\\-9.266294\\\\166.9199\\\\286.01\\\\-6.534666\\\\170.9199\\\\286.01\\\\-4.989212\\\\172.9199\\\\286.01\\\\-2.364212\\\\175.6552\\\\286.01\\\\0.6357885\\\\178.3645\\\\286.01\\\\3.162897\\\\180.9199\\\\286.01\\\\4.831301\\\\182.9199\\\\286.01\\\\5.793481\\\\184.4199\\\\286.01\\\\6.209064\\\\185.4199\\\\286.01\\\\6.251529\\\\186.4199\\\\286.01\\\\5.84561\\\\187.4199\\\\286.01\\\\5.229538\\\\188.4199\\\\286.01\\\\3.904728\\\\189.9199\\\\286.01\\\\1.509165\\\\192.4199\\\\286.01\\\\-0.01421149\\\\194.4199\\\\286.01\\\\-1.566989\\\\197.4199\\\\286.01\\\\-1.955878\\\\198.4199\\\\286.01\\\\-2.473254\\\\200.4199\\\\286.01\\\\-2.691339\\\\203.4199\\\\286.01\\\\-2.664743\\\\204.9199\\\\286.01\\\\-2.487896\\\\206.9199\\\\286.01\\\\-1.985179\\\\208.9199\\\\286.01\\\\-0.9445686\\\\211.4199\\\\286.01\\\\-0.07303502\\\\212.9199\\\\286.01\\\\0.9783811\\\\214.4199\\\\286.01\\\\1.885789\\\\215.4199\\\\286.01\\\\3.635788\\\\217.0773\\\\286.01\\\\5.135788\\\\218.1228\\\\286.01\\\\6.635788\\\\218.9891\\\\286.01\\\\9.135789\\\\220.027\\\\286.01\\\\11.13579\\\\220.5289\\\\286.01\\\\14.13579\\\\220.731\\\\286.01\\\\17.13579\\\\220.5555\\\\286.01\\\\19.13579\\\\220.0785\\\\286.01\\\\20.63579\\\\219.56\\\\286.01\\\\22.63579\\\\218.5779\\\\286.01\\\\24.13579\\\\217.5997\\\\286.01\\\\26.63579\\\\215.3391\\\\286.01\\\\28.25469\\\\213.4199\\\\286.01\\\\29.69829\\\\210.9199\\\\286.01\\\\30.32329\\\\209.4199\\\\286.01\\\\30.81052\\\\207.9199\\\\286.01\\\\31.25947\\\\205.9199\\\\286.01\\\\31.36474\\\\202.9199\\\\286.01\\\\31.25015\\\\201.4199\\\\286.01\\\\30.79802\\\\199.4199\\\\286.01\\\\30.30698\\\\197.9199\\\\286.01\\\\29.65141\\\\196.4199\\\\286.01\\\\28.84218\\\\194.9199\\\\286.01\\\\27.82176\\\\193.4199\\\\286.01\\\\26.13579\\\\191.552\\\\286.01\\\\23.59412\\\\188.9199\\\\286.01\\\\22.45183\\\\187.4199\\\\286.01\\\\21.87136\\\\185.9199\\\\286.01\\\\22.07775\\\\184.9199\\\\286.01\\\\22.51447\\\\183.9199\\\\286.01\\\\23.47013\\\\182.4199\\\\286.01\\\\25.57\\\\179.9199\\\\286.01\\\\27.5517\\\\177.9199\\\\286.01\\\\30.13579\\\\175.5383\\\\286.01\\\\32.1768\\\\173.4199\\\\286.01\\\\33.78976\\\\171.4199\\\\286.01\\\\35.44261\\\\168.9199\\\\286.01\\\\37.52156\\\\165.9199\\\\286.01\\\\40.48853\\\\161.9199\\\\286.01\\\\42.13579\\\\160.0113\\\\286.01\\\\43.13579\\\\159.0058\\\\286.01\\\\44.13579\\\\158.355\\\\286.01\\\\45.63579\\\\159.3214\\\\286.01\\\\47.13579\\\\160.8254\\\\286.01\\\\48.51277\\\\162.4199\\\\286.01\\\\50.37894\\\\164.4199\\\\286.01\\\\52.63579\\\\166.5075\\\\286.01\\\\54.13579\\\\167.5623\\\\286.01\\\\57.13579\\\\169.0584\\\\286.01\\\\58.63579\\\\169.5623\\\\286.01\\\\60.63579\\\\170.0095\\\\286.01\\\\62.13579\\\\170.1383\\\\286.01\\\\65.13579\\\\170.0383\\\\286.01\\\\67.13579\\\\169.5946\\\\286.01\\\\68.63579\\\\169.1182\\\\286.01\\\\71.13579\\\\168.0377\\\\286.01\\\\72.63579\\\\167.1193\\\\286.01\\\\74.63579\\\\165.5595\\\\286.01\\\\75.7575\\\\164.4199\\\\286.01\\\\77.29167\\\\162.4199\\\\286.01\\\\78.15794\\\\160.9199\\\\286.01\\\\79.25264\\\\158.4199\\\\286.01\\\\79.77\\\\156.4199\\\\286.01\\\\80.00407\\\\152.9199\\\\286.01\\\\79.79632\\\\149.9199\\\\286.01\\\\79.30515\\\\147.9199\\\\286.01\\\\78.7759\\\\146.4199\\\\286.01\\\\77.78234\\\\144.4199\\\\286.01\\\\76.78518\\\\142.9199\\\\286.01\\\\75.44829\\\\141.4199\\\\286.01\\\\73.63579\\\\139.7192\\\\286.01\\\\71.63579\\\\138.3489\\\\286.01\\\\69.63579\\\\137.3365\\\\286.01\\\\68.13579\\\\136.7758\\\\286.01\\\\66.13579\\\\136.2751\\\\286.01\\\\62.63579\\\\136.0838\\\\286.01\\\\59.63579\\\\136.315\\\\286.01\\\\57.63579\\\\136.8432\\\\286.01\\\\56.63579\\\\137.2282\\\\286.01\\\\54.63579\\\\138.2251\\\\286.01\\\\53.13579\\\\139.2217\\\\286.01\\\\51.73048\\\\140.4199\\\\286.01\\\\50.63579\\\\141.5006\\\\286.01\\\\49.01391\\\\143.4199\\\\286.01\\\\45.63579\\\\148.6563\\\\286.01\\\\44.13579\\\\150.5612\\\\286.01\\\\43.13579\\\\151.2219\\\\286.01\\\\42.63579\\\\151.0217\\\\286.01\\\\40.13579\\\\149.1523\\\\286.01\\\\35.63579\\\\144.8964\\\\286.01\\\\31.34298\\\\140.4199\\\\286.01\\\\29.63579\\\\138.8365\\\\286.01\\\\27.63579\\\\137.2758\\\\286.01\\\\26.13579\\\\136.2875\\\\286.01\\\\21.63579\\\\133.7337\\\\286.01\\\\18.63579\\\\132.2451\\\\286.01\\\\16.13579\\\\131.3449\\\\286.01\\\\14.13579\\\\130.8667\\\\286.01\\\\12.63579\\\\130.8597\\\\286.01\\\\10.63579\\\\131.2148\\\\286.01\\\\9.135789\\\\131.787\\\\286.01\\\\7.135788\\\\132.7495\\\\286.01\\\\3.635788\\\\134.8391\\\\286.01\\\\0.6357885\\\\136.7923\\\\286.01\\\\-1.364211\\\\138.2802\\\\286.01\\\\-3.166659\\\\139.9199\\\\286.01\\\\-5.078728\\\\141.9199\\\\286.01\\\\-8.743597\\\\146.4199\\\\286.01\\\\-10.50432\\\\148.4199\\\\286.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848890601900001.533619501923\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"50\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"10\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"26.28579\\\\112.4199\\\\286.01\\\\27.83857\\\\109.4199\\\\286.01\\\\28.22746\\\\108.4199\\\\286.01\\\\28.74483\\\\106.4199\\\\286.01\\\\28.96292\\\\103.4199\\\\286.01\\\\28.93632\\\\101.9199\\\\286.01\\\\28.75947\\\\99.91986\\\\286.01\\\\28.25676\\\\97.91986\\\\286.01\\\\27.21615\\\\95.41986\\\\286.01\\\\26.34461\\\\93.91986\\\\286.01\\\\25.2932\\\\92.41986\\\\286.01\\\\24.38579\\\\91.41986\\\\286.01\\\\22.63579\\\\89.76245\\\\286.01\\\\21.13579\\\\88.71692\\\\286.01\\\\19.63579\\\\87.85058\\\\286.01\\\\17.13579\\\\86.81271\\\\286.01\\\\15.13579\\\\86.31081\\\\286.01\\\\12.13579\\\\86.10869\\\\286.01\\\\9.135789\\\\86.28422\\\\286.01\\\\7.135788\\\\86.76125\\\\286.01\\\\5.635788\\\\87.27975\\\\286.01\\\\3.635788\\\\88.26181\\\\286.01\\\\2.135788\\\\89.23998\\\\286.01\\\\-0.3642115\\\\91.50063\\\\286.01\\\\-1.983114\\\\93.41986\\\\286.01\\\\-3.426712\\\\95.91986\\\\286.01\\\\-4.051712\\\\97.41986\\\\286.01\\\\-4.538943\\\\98.91986\\\\286.01\\\\-4.987895\\\\100.9199\\\\286.01\\\\-5.093159\\\\103.9199\\\\286.01\\\\-4.978573\\\\105.4199\\\\286.01\\\\-4.526445\\\\107.4199\\\\286.01\\\\-4.035407\\\\108.9199\\\\286.01\\\\-3.379837\\\\110.4199\\\\286.01\\\\-2.570607\\\\111.9199\\\\286.01\\\\-1.120541\\\\113.9199\\\\286.01\\\\-0.1875448\\\\114.9199\\\\286.01\\\\1.635789\\\\116.6058\\\\286.01\\\\3.135788\\\\117.6287\\\\286.01\\\\4.635788\\\\118.4723\\\\286.01\\\\7.135788\\\\119.5478\\\\286.01\\\\8.635789\\\\120.032\\\\286.01\\\\11.13579\\\\120.4815\\\\286.01\\\\13.63579\\\\120.4539\\\\286.01\\\\15.63579\\\\120.0365\\\\286.01\\\\17.13579\\\\119.5128\\\\286.01\\\\20.13579\\\\118.0816\\\\286.01\\\\21.63579\\\\117.1312\\\\286.01\\\\23.63579\\\\115.5595\\\\286.01\\\\24.74579\\\\114.4199\\\\286.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848911603100001.549938975401\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"245\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"11\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-24.36421\\\\171.6106\\\\289.01\\\\-19.86421\\\\167.7245\\\\289.01\\\\-18.36421\\\\166.7742\\\\289.01\\\\-16.86421\\\\166.2171\\\\289.01\\\\-15.86421\\\\166.136\\\\289.01\\\\-14.86421\\\\166.3039\\\\289.01\\\\-12.86421\\\\167.2141\\\\289.01\\\\-11.36421\\\\168.2115\\\\289.01\\\\-8.864211\\\\170.4133\\\\289.01\\\\-7.364212\\\\172.015\\\\289.01\\\\-3.364212\\\\176.4838\\\\289.01\\\\-1.698104\\\\178.4199\\\\289.01\\\\-0.6444128\\\\179.9199\\\\289.01\\\\0.2081026\\\\181.4199\\\\289.01\\\\0.8369604\\\\182.9199\\\\289.01\\\\1.181462\\\\184.4199\\\\289.01\\\\1.159226\\\\185.4199\\\\289.01\\\\0.7715028\\\\186.9199\\\\289.01\\\\0.3016764\\\\187.9199\\\\289.01\\\\-0.6509087\\\\189.4199\\\\289.01\\\\-2.53016\\\\191.9199\\\\289.01\\\\-3.495677\\\\193.4199\\\\289.01\\\\-4.539615\\\\195.4199\\\\289.01\\\\-5.487195\\\\197.9199\\\\289.01\\\\-6.045509\\\\200.4199\\\\289.01\\\\-6.221633\\\\203.9199\\\\289.01\\\\-5.981275\\\\207.4199\\\\289.01\\\\-5.507912\\\\209.4199\\\\289.01\\\\-4.566212\\\\211.9199\\\\289.01\\\\-3.55636\\\\213.9199\\\\289.01\\\\-2.941314\\\\214.9199\\\\289.01\\\\-1.4598\\\\216.9199\\\\289.01\\\\0.1357885\\\\218.6212\\\\289.01\\\\1.135789\\\\219.5154\\\\289.01\\\\3.135788\\\\220.9883\\\\289.01\\\\4.135788\\\\221.6079\\\\289.01\\\\6.135788\\\\222.6179\\\\289.01\\\\8.635789\\\\223.5528\\\\289.01\\\\11.13579\\\\224.0987\\\\289.01\\\\14.13579\\\\224.2564\\\\289.01\\\\17.63579\\\\224.0546\\\\289.01\\\\19.63579\\\\223.5962\\\\289.01\\\\21.13579\\\\223.1108\\\\289.01\\\\23.63579\\\\222.0024\\\\289.01\\\\25.13579\\\\221.1236\\\\289.01\\\\26.63579\\\\220.0783\\\\289.01\\\\29.13579\\\\217.7342\\\\289.01\\\\30.33269\\\\216.4199\\\\289.01\\\\31.75305\\\\214.4199\\\\289.01\\\\33.30979\\\\211.4199\\\\289.01\\\\34.3199\\\\208.4199\\\\289.01\\\\34.75285\\\\206.4199\\\\289.01\\\\34.88579\\\\204.9199\\\\289.01\\\\34.88197\\\\202.4199\\\\289.01\\\\34.74179\\\\200.9199\\\\289.01\\\\34.31215\\\\198.9199\\\\289.01\\\\33.2965\\\\195.9199\\\\289.01\\\\32.31141\\\\193.9199\\\\289.01\\\\30.31499\\\\190.9199\\\\289.01\\\\29.46377\\\\189.9199\\\\289.01\\\\28.09009\\\\187.9199\\\\289.01\\\\27.5465\\\\186.9199\\\\289.01\\\\27.02657\\\\185.4199\\\\289.01\\\\26.95302\\\\183.9199\\\\289.01\\\\27.49712\\\\181.9199\\\\289.01\\\\28.4963\\\\179.9199\\\\289.01\\\\29.48924\\\\178.4199\\\\289.01\\\\31.52524\\\\175.9199\\\\289.01\\\\33.74527\\\\173.4199\\\\289.01\\\\35.40354\\\\171.4199\\\\289.01\\\\37.13579\\\\169.6051\\\\289.01\\\\39.13579\\\\167.7864\\\\289.01\\\\40.63579\\\\166.7655\\\\289.01\\\\41.63579\\\\166.2524\\\\289.01\\\\43.13579\\\\165.754\\\\289.01\\\\44.63579\\\\165.7558\\\\289.01\\\\46.13579\\\\166.2658\\\\289.01\\\\47.13579\\\\166.7949\\\\289.01\\\\48.63579\\\\167.8024\\\\289.01\\\\51.63579\\\\170.0929\\\\289.01\\\\53.13579\\\\171.0158\\\\289.01\\\\55.13579\\\\172.0168\\\\289.01\\\\58.13579\\\\173.0807\\\\289.01\\\\60.13579\\\\173.508\\\\289.01\\\\63.13579\\\\173.6928\\\\289.01\\\\65.63579\\\\173.5299\\\\289.01\\\\67.63579\\\\173.1064\\\\289.01\\\\69.63579\\\\172.491\\\\289.01\\\\71.63579\\\\171.6557\\\\289.01\\\\73.63579\\\\170.6136\\\\289.01\\\\75.13579\\\\169.622\\\\289.01\\\\77.13579\\\\167.9501\\\\289.01\\\\78.14953\\\\166.9199\\\\289.01\\\\79.80318\\\\164.9199\\\\289.01\\\\80.78197\\\\163.4199\\\\289.01\\\\81.81926\\\\161.4199\\\\289.01\\\\82.77949\\\\158.9199\\\\289.01\\\\83.26665\\\\156.9199\\\\289.01\\\\83.42233\\\\155.4199\\\\289.01\\\\83.54694\\\\152.9199\\\\289.01\\\\83.298\\\\149.4199\\\\289.01\\\\82.82378\\\\147.4199\\\\289.01\\\\82.33067\\\\145.9199\\\\289.01\\\\81.20625\\\\143.4199\\\\289.01\\\\80.32435\\\\141.9199\\\\289.01\\\\78.83173\\\\139.9199\\\\289.01\\\\77.63579\\\\138.6628\\\\289.01\\\\76.13579\\\\137.254\\\\289.01\\\\74.13579\\\\135.7893\\\\289.01\\\\73.13579\\\\135.2029\\\\289.01\\\\70.13579\\\\133.7565\\\\289.01\\\\68.63579\\\\133.2538\\\\289.01\\\\66.63579\\\\132.7745\\\\289.01\\\\63.13579\\\\132.5449\\\\289.01\\\\59.63579\\\\132.7386\\\\289.01\\\\57.13579\\\\133.3031\\\\289.01\\\\55.63579\\\\133.8401\\\\289.01\\\\53.63579\\\\134.7649\\\\289.01\\\\50.63579\\\\136.7378\\\\289.01\\\\48.79829\\\\138.4199\\\\289.01\\\\45.63579\\\\141.6464\\\\289.01\\\\44.63579\\\\142.51\\\\289.01\\\\42.63579\\\\143.6619\\\\289.01\\\\41.13579\\\\144.0638\\\\289.01\\\\40.13579\\\\143.9958\\\\289.01\\\\38.63579\\\\143.6008\\\\289.01\\\\35.63579\\\\142.1031\\\\289.01\\\\34.13579\\\\141.0644\\\\289.01\\\\29.63579\\\\137.31\\\\289.01\\\\26.63579\\\\135.0555\\\\289.01\\\\23.55246\\\\132.4199\\\\289.01\\\\22.0726\\\\130.9199\\\\289.01\\\\20.46226\\\\128.9199\\\\289.01\\\\19.39348\\\\126.9199\\\\289.01\\\\19.14968\\\\125.9199\\\\289.01\\\\19.44511\\\\124.9199\\\\289.01\\\\19.99907\\\\123.9199\\\\289.01\\\\20.82414\\\\122.9199\\\\289.01\\\\22.27133\\\\121.4199\\\\289.01\\\\26.13579\\\\117.9661\\\\289.01\\\\27.14412\\\\116.9199\\\\289.01\\\\28.79449\\\\114.9199\\\\289.01\\\\29.76725\\\\113.4199\\\\289.01\\\\30.81119\\\\111.4199\\\\289.01\\\\31.75877\\\\108.9199\\\\289.01\\\\32.31709\\\\106.4199\\\\289.01\\\\32.49321\\\\102.9199\\\\289.01\\\\32.25285\\\\99.41986\\\\289.01\\\\31.77949\\\\97.41986\\\\289.01\\\\30.83779\\\\94.91986\\\\289.01\\\\29.82794\\\\92.91986\\\\289.01\\\\29.21289\\\\91.91986\\\\289.01\\\\27.73138\\\\89.91986\\\\289.01\\\\26.13579\\\\88.21847\\\\289.01\\\\25.13579\\\\87.32427\\\\289.01\\\\23.13579\\\\85.85146\\\\289.01\\\\22.13579\\\\85.23184\\\\289.01\\\\20.13579\\\\84.22186\\\\289.01\\\\17.63579\\\\83.28586\\\\289.01\\\\15.63579\\\\82.81703\\\\289.01\\\\14.13579\\\\82.65841\\\\289.01\\\\12.13579\\\\82.58332\\\\289.01\\\\8.635789\\\\82.78226\\\\289.01\\\\6.635788\\\\83.2435\\\\289.01\\\\5.135788\\\\83.72891\\\\289.01\\\\2.635788\\\\84.83727\\\\289.01\\\\1.135789\\\\85.71608\\\\289.01\\\\-0.3642115\\\\86.76138\\\\289.01\\\\-2.864212\\\\89.1055\\\\289.01\\\\-4.061114\\\\90.41986\\\\289.01\\\\-5.481468\\\\92.41986\\\\289.01\\\\-7.038211\\\\95.41986\\\\289.01\\\\-8.04832\\\\98.41986\\\\289.01\\\\-8.481275\\\\100.4199\\\\289.01\\\\-8.614211\\\\101.9199\\\\289.01\\\\-8.610394\\\\104.4199\\\\289.01\\\\-8.470211\\\\105.9199\\\\289.01\\\\-8.040568\\\\107.9199\\\\289.01\\\\-7.024926\\\\110.9199\\\\289.01\\\\-6.039831\\\\112.9199\\\\289.01\\\\-5.436641\\\\113.9199\\\\289.01\\\\-4.016484\\\\115.9199\\\\289.01\\\\-2.364212\\\\117.6864\\\\289.01\\\\-0.8642115\\\\119.0858\\\\289.01\\\\2.635788\\\\121.7363\\\\289.01\\\\4.936513\\\\123.9199\\\\289.01\\\\5.739955\\\\124.9199\\\\289.01\\\\6.243547\\\\125.9199\\\\289.01\\\\6.389576\\\\126.9199\\\\289.01\\\\5.898776\\\\128.4199\\\\289.01\\\\5.36544\\\\129.4199\\\\289.01\\\\4.344122\\\\130.9199\\\\289.01\\\\2.635788\\\\132.901\\\\289.01\\\\0.6357885\\\\134.9343\\\\289.01\\\\-3.291989\\\\138.4199\\\\289.01\\\\-6.454596\\\\141.4199\\\\289.01\\\\-8.364211\\\\143.1276\\\\289.01\\\\-10.36421\\\\144.5659\\\\289.01\\\\-12.36421\\\\145.6168\\\\289.01\\\\-13.86421\\\\146.1154\\\\289.01\\\\-14.86421\\\\146.2075\\\\289.01\\\\-15.86421\\\\146.0664\\\\289.01\\\\-17.86421\\\\145.0818\\\\289.01\\\\-19.36421\\\\143.7907\\\\289.01\\\\-22.36421\\\\140.7383\\\\289.01\\\\-23.36421\\\\139.8224\\\\289.01\\\\-25.36421\\\\138.2655\\\\289.01\\\\-27.86421\\\\136.8177\\\\289.01\\\\-30.36421\\\\135.775\\\\289.01\\\\-31.86421\\\\135.3379\\\\289.01\\\\-34.36421\\\\134.8099\\\\289.01\\\\-36.36421\\\\134.6622\\\\289.01\\\\-39.36421\\\\134.8203\\\\289.01\\\\-41.36421\\\\135.2396\\\\289.01\\\\-44.36421\\\\136.2353\\\\289.01\\\\-47.36421\\\\137.7303\\\\289.01\\\\-48.86421\\\\138.7237\\\\289.01\\\\-50.31903\\\\139.9199\\\\289.01\\\\-51.86699\\\\141.4199\\\\289.01\\\\-53.52726\\\\143.4199\\\\289.01\\\\-54.5031\\\\144.9199\\\\289.01\\\\-55.54621\\\\146.9199\\\\289.01\\\\-56.50508\\\\149.4199\\\\289.01\\\\-56.99216\\\\151.4199\\\\289.01\\\\-57.2638\\\\154.9199\\\\289.01\\\\-57.13921\\\\157.9199\\\\289.01\\\\-56.96703\\\\159.4199\\\\289.01\\\\-56.47692\\\\161.4199\\\\289.01\\\\-55.93595\\\\162.9199\\\\289.01\\\\-55.00497\\\\164.9199\\\\289.01\\\\-54.08002\\\\166.4199\\\\289.01\\\\-53.01329\\\\167.9199\\\\289.01\\\\-51.64692\\\\169.4199\\\\289.01\\\\-49.36421\\\\171.5437\\\\289.01\\\\-47.86421\\\\172.6127\\\\289.01\\\\-46.36421\\\\173.4903\\\\289.01\\\\-43.86421\\\\174.6123\\\\289.01\\\\-42.36421\\\\175.104\\\\289.01\\\\-40.36421\\\\175.5783\\\\289.01\\\\-36.86421\\\\175.813\\\\289.01\\\\-34.36421\\\\175.6949\\\\289.01\\\\-32.86421\\\\175.5299\\\\289.01\\\\-30.86421\\\\175.0428\\\\289.01\\\\-29.36421\\\\174.5075\\\\289.01\\\\-27.36421\\\\173.5845\\\\289.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848933604400001.474954755728\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"237\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"12\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-50.86421\\\\137.2415\\\\292.01\\\\-53.36421\\\\139.566\\\\292.01\\\\-54.99971\\\\141.4199\\\\292.01\\\\-56.06249\\\\142.9199\\\\292.01\\\\-57.48386\\\\145.4199\\\\292.01\\\\-58.5421\\\\147.9199\\\\292.01\\\\-59.00731\\\\149.4199\\\\292.01\\\\-59.52402\\\\151.9199\\\\292.01\\\\-59.68875\\\\155.4199\\\\292.01\\\\-59.50454\\\\158.9199\\\\292.01\\\\-58.98414\\\\161.4199\\\\292.01\\\\-58.51487\\\\162.9199\\\\292.01\\\\-57.42337\\\\165.4199\\\\292.01\\\\-55.9553\\\\167.9199\\\\292.01\\\\-54.43354\\\\169.9199\\\\292.01\\\\-53.06614\\\\171.4199\\\\292.01\\\\-51.36421\\\\172.969\\\\292.01\\\\-49.86421\\\\174.1299\\\\292.01\\\\-48.36421\\\\175.1141\\\\292.01\\\\-45.36421\\\\176.6118\\\\292.01\\\\-42.36421\\\\177.607\\\\292.01\\\\-39.86421\\\\178.0984\\\\292.01\\\\-36.86421\\\\178.2351\\\\292.01\\\\-33.36421\\\\178.0634\\\\292.01\\\\-30.86421\\\\177.5482\\\\292.01\\\\-29.36421\\\\177.0816\\\\292.01\\\\-26.86421\\\\176.0001\\\\292.01\\\\-24.36421\\\\174.5449\\\\292.01\\\\-22.36421\\\\173.0784\\\\292.01\\\\-21.36421\\\\172.2305\\\\292.01\\\\-19.36421\\\\170.8463\\\\292.01\\\\-18.36421\\\\170.289\\\\292.01\\\\-16.86421\\\\169.7166\\\\292.01\\\\-15.36421\\\\169.4899\\\\292.01\\\\-13.36421\\\\169.7316\\\\292.01\\\\-10.86421\\\\170.7901\\\\292.01\\\\-9.364211\\\\171.7313\\\\292.01\\\\-7.874132\\\\172.9199\\\\292.01\\\\-5.819569\\\\174.9199\\\\292.01\\\\-4.197077\\\\176.9199\\\\292.01\\\\-3.215368\\\\178.4199\\\\292.01\\\\-2.222842\\\\180.4199\\\\292.01\\\\-1.726866\\\\181.9199\\\\292.01\\\\-1.545029\\\\183.9199\\\\292.01\\\\-1.7261\\\\185.4199\\\\292.01\\\\-2.267621\\\\186.9199\\\\292.01\\\\-3.670156\\\\189.4199\\\\292.01\\\\-5.063191\\\\191.4199\\\\292.01\\\\-6.47314\\\\193.9199\\\\292.01\\\\-7.528172\\\\196.4199\\\\292.01\\\\-7.99005\\\\197.9199\\\\292.01\\\\-8.496565\\\\200.4199\\\\292.01\\\\-8.660795\\\\203.9199\\\\292.01\\\\-8.561417\\\\206.4199\\\\292.01\\\\-8.006369\\\\209.4199\\\\292.01\\\\-6.968822\\\\212.4199\\\\292.01\\\\-6.01766\\\\214.4199\\\\292.01\\\\-4.087803\\\\217.4199\\\\292.01\\\\-1.925022\\\\219.9199\\\\292.01\\\\0.6357885\\\\222.1419\\\\292.01\\\\3.635788\\\\224.0674\\\\292.01\\\\5.635788\\\\225.0181\\\\292.01\\\\8.635789\\\\226.0507\\\\292.01\\\\11.63579\\\\226.6077\\\\292.01\\\\14.13579\\\\226.7023\\\\292.01\\\\17.13579\\\\226.5765\\\\292.01\\\\19.63579\\\\226.0975\\\\292.01\\\\21.63579\\\\225.4732\\\\292.01\\\\23.63579\\\\224.6535\\\\292.01\\\\25.63579\\\\223.6229\\\\292.01\\\\26.63579\\\\222.9888\\\\292.01\\\\28.63579\\\\221.508\\\\292.01\\\\30.13579\\\\220.1511\\\\292.01\\\\31.76374\\\\218.4199\\\\292.01\\\\33.28978\\\\216.4199\\\\292.01\\\\34.22413\\\\214.9199\\\\292.01\\\\35.26341\\\\212.9199\\\\292.01\\\\36.23893\\\\210.4199\\\\292.01\\\\36.81548\\\\208.4199\\\\292.01\\\\37.31635\\\\205.4199\\\\292.01\\\\37.31125\\\\201.9199\\\\292.01\\\\36.80509\\\\198.9199\\\\292.01\\\\36.21972\\\\196.9199\\\\292.01\\\\35.23758\\\\194.4199\\\\292.01\\\\34.185\\\\192.4199\\\\292.01\\\\33.25797\\\\190.9199\\\\292.01\\\\31.47586\\\\188.4199\\\\292.01\\\\30.39613\\\\186.4199\\\\292.01\\\\29.98475\\\\185.4199\\\\292.01\\\\29.70282\\\\183.9199\\\\292.01\\\\29.65647\\\\182.9199\\\\292.01\\\\30.01312\\\\180.9199\\\\292.01\\\\30.59115\\\\179.4199\\\\292.01\\\\31.96314\\\\176.9199\\\\292.01\\\\33.04444\\\\175.4199\\\\292.01\\\\34.63579\\\\173.6154\\\\292.01\\\\36.13579\\\\172.1795\\\\292.01\\\\37.13579\\\\171.3688\\\\292.01\\\\39.63579\\\\169.8149\\\\292.01\\\\41.13579\\\\169.2102\\\\292.01\\\\42.63579\\\\168.8687\\\\292.01\\\\43.63579\\\\168.8372\\\\292.01\\\\45.63579\\\\169.3281\\\\292.01\\\\47.63579\\\\170.2854\\\\292.01\\\\51.13579\\\\172.6065\\\\292.01\\\\53.63579\\\\173.9453\\\\292.01\\\\55.13579\\\\174.6138\\\\292.01\\\\58.13579\\\\175.5708\\\\292.01\\\\61.13579\\\\176.0839\\\\292.01\\\\64.63579\\\\176.0973\\\\292.01\\\\67.63579\\\\175.5995\\\\292.01\\\\69.63579\\\\175.0346\\\\292.01\\\\72.13579\\\\174.0871\\\\292.01\\\\74.13579\\\\173.0916\\\\292.01\\\\77.13579\\\\171.1028\\\\292.01\\\\79.63579\\\\168.7767\\\\292.01\\\\81.27783\\\\166.9199\\\\292.01\\\\82.33613\\\\165.4199\\\\292.01\\\\83.23394\\\\163.9199\\\\292.01\\\\84.21426\\\\161.9199\\\\292.01\\\\84.81573\\\\160.4199\\\\292.01\\\\85.28121\\\\158.9199\\\\292.01\\\\85.79774\\\\156.4199\\\\292.01\\\\85.96235\\\\152.9199\\\\292.01\\\\85.82057\\\\149.9199\\\\292.01\\\\85.32604\\\\147.4199\\\\292.01\\\\84.3297\\\\144.4199\\\\292.01\\\\82.82874\\\\141.4199\\\\292.01\\\\81.84238\\\\139.9199\\\\292.01\\\\79.77893\\\\137.4199\\\\292.01\\\\78.13579\\\\135.8536\\\\292.01\\\\76.13579\\\\134.271\\\\292.01\\\\73.63579\\\\132.7315\\\\292.01\\\\71.63579\\\\131.7812\\\\292.01\\\\68.63579\\\\130.7527\\\\292.01\\\\66.13579\\\\130.2516\\\\292.01\\\\63.13579\\\\130.112\\\\292.01\\\\59.63579\\\\130.2875\\\\292.01\\\\57.13579\\\\130.7999\\\\292.01\\\\55.63579\\\\131.2636\\\\292.01\\\\53.13579\\\\132.3465\\\\292.01\\\\50.63579\\\\133.8043\\\\292.01\\\\48.63579\\\\135.3059\\\\292.01\\\\45.13579\\\\138.5364\\\\292.01\\\\43.63579\\\\139.6493\\\\292.01\\\\42.13579\\\\140.4601\\\\292.01\\\\40.13579\\\\141.0791\\\\292.01\\\\38.63579\\\\141.1097\\\\292.01\\\\36.63579\\\\140.6416\\\\292.01\\\\35.13579\\\\140.0553\\\\292.01\\\\32.63579\\\\138.6373\\\\292.01\\\\31.13579\\\\137.5738\\\\292.01\\\\29.72654\\\\136.4199\\\\292.01\\\\28.07417\\\\134.9199\\\\292.01\\\\25.91139\\\\132.4199\\\\292.01\\\\24.93418\\\\130.9199\\\\292.01\\\\24.00985\\\\128.9199\\\\292.01\\\\23.59784\\\\126.9199\\\\292.01\\\\24.00117\\\\124.9199\\\\292.01\\\\24.45341\\\\123.9199\\\\292.01\\\\25.412\\\\122.4199\\\\292.01\\\\27.1188\\\\120.4199\\\\292.01\\\\30.26747\\\\116.9199\\\\292.01\\\\31.33271\\\\115.4199\\\\292.01\\\\32.74472\\\\112.9199\\\\292.01\\\\33.80082\\\\110.4199\\\\292.01\\\\34.26079\\\\108.9199\\\\292.01\\\\34.76814\\\\106.4199\\\\292.01\\\\34.93237\\\\102.9199\\\\292.01\\\\34.83299\\\\100.4199\\\\292.01\\\\34.27795\\\\97.41986\\\\292.01\\\\33.2404\\\\94.41986\\\\292.01\\\\32.28924\\\\92.41986\\\\292.01\\\\30.35938\\\\89.41986\\\\292.01\\\\28.13579\\\\86.85905\\\\292.01\\\\25.63579\\\\84.69783\\\\292.01\\\\22.63579\\\\82.77126\\\\292.01\\\\20.63579\\\\81.82164\\\\292.01\\\\17.63579\\\\80.78906\\\\292.01\\\\14.63579\\\\80.23197\\\\292.01\\\\12.13579\\\\80.13745\\\\292.01\\\\9.135789\\\\80.26321\\\\292.01\\\\6.635788\\\\80.74219\\\\292.01\\\\4.635788\\\\81.36616\\\\292.01\\\\2.635788\\\\82.1862\\\\292.01\\\\0.6357885\\\\83.21684\\\\292.01\\\\-0.3642115\\\\83.85096\\\\292.01\\\\-2.364212\\\\85.33102\\\\292.01\\\\-3.864212\\\\86.68865\\\\292.01\\\\-5.492164\\\\88.41986\\\\292.01\\\\-7.018197\\\\90.41986\\\\292.01\\\\-7.953227\\\\91.91986\\\\292.01\\\\-8.991834\\\\93.91986\\\\292.01\\\\-9.966642\\\\96.41986\\\\292.01\\\\-10.5439\\\\98.41986\\\\292.01\\\\-11.04477\\\\101.4199\\\\292.01\\\\-11.03968\\\\104.9199\\\\292.01\\\\-10.53351\\\\107.9199\\\\292.01\\\\-9.947545\\\\109.9199\\\\292.01\\\\-8.965997\\\\112.4199\\\\292.01\\\\-7.913424\\\\114.4199\\\\292.01\\\\-6.97299\\\\115.9199\\\\292.01\\\\-5.421904\\\\117.9199\\\\292.01\\\\-3.864212\\\\119.5885\\\\292.01\\\\-1.366711\\\\121.9199\\\\292.01\\\\-0.3642115\\\\123.0152\\\\292.01\\\\0.7380612\\\\124.4199\\\\292.01\\\\1.307276\\\\125.4199\\\\292.01\\\\1.837008\\\\126.9199\\\\292.01\\\\1.945404\\\\127.9199\\\\292.01\\\\1.824169\\\\129.4199\\\\292.01\\\\1.341957\\\\130.9199\\\\292.01\\\\0.3438723\\\\132.9199\\\\292.01\\\\-0.6656821\\\\134.4199\\\\292.01\\\\-2.354596\\\\136.4199\\\\292.01\\\\-3.364212\\\\137.4545\\\\292.01\\\\-6.364212\\\\140.0493\\\\292.01\\\\-7.864212\\\\141.0763\\\\292.01\\\\-9.864211\\\\142.1287\\\\292.01\\\\-11.36421\\\\142.6533\\\\292.01\\\\-12.86421\\\\142.9596\\\\292.01\\\\-13.86421\\\\142.9695\\\\292.01\\\\-15.36421\\\\142.6443\\\\292.01\\\\-16.86421\\\\142.0094\\\\292.01\\\\-18.36421\\\\141.0572\\\\292.01\\\\-20.86421\\\\138.869\\\\292.01\\\\-23.36421\\\\136.7725\\\\292.01\\\\-25.86421\\\\135.1882\\\\292.01\\\\-28.86421\\\\133.7659\\\\292.01\\\\-30.36421\\\\133.2484\\\\292.01\\\\-32.36421\\\\132.7296\\\\292.01\\\\-35.36421\\\\132.2475\\\\292.01\\\\-38.36421\\\\132.2506\\\\292.01\\\\-41.36421\\\\132.7474\\\\292.01\\\\-43.36421\\\\133.3174\\\\292.01\\\\-45.86421\\\\134.2638\\\\292.01\\\\-47.86421\\\\135.2561\\\\292.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848957605700001.539833993627\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"247\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"13\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.13579\\\\228.4295\\\\295.01\\\\18.13579\\\\228.1286\\\\295.01\\\\20.63579\\\\227.5699\\\\295.01\\\\23.63579\\\\226.5032\\\\295.01\\\\25.63579\\\\225.5385\\\\295.01\\\\27.13579\\\\224.6467\\\\295.01\\\\28.63579\\\\223.6208\\\\295.01\\\\30.63579\\\\221.9606\\\\295.01\\\\32.63579\\\\219.9176\\\\295.01\\\\34.26079\\\\217.9199\\\\295.01\\\\35.85596\\\\215.4199\\\\295.01\\\\36.86065\\\\213.4199\\\\295.01\\\\37.84481\\\\210.9199\\\\295.01\\\\38.30011\\\\209.4199\\\\295.01\\\\38.83393\\\\206.9199\\\\295.01\\\\39.02689\\\\203.9199\\\\295.01\\\\38.82799\\\\200.4199\\\\295.01\\\\38.28665\\\\197.9199\\\\295.01\\\\37.83388\\\\196.4199\\\\295.01\\\\36.84829\\\\193.9199\\\\295.01\\\\35.83665\\\\191.9199\\\\295.01\\\\34.2616\\\\189.4199\\\\295.01\\\\32.91469\\\\187.4199\\\\295.01\\\\31.91341\\\\185.4199\\\\295.01\\\\31.41753\\\\183.9199\\\\295.01\\\\31.21638\\\\182.4199\\\\295.01\\\\31.22183\\\\181.4199\\\\295.01\\\\31.4365\\\\179.9199\\\\295.01\\\\31.9365\\\\178.4199\\\\295.01\\\\32.96956\\\\176.4199\\\\295.01\\\\33.63579\\\\175.5072\\\\295.01\\\\35.13579\\\\173.8535\\\\295.01\\\\37.13579\\\\172.3393\\\\295.01\\\\39.13579\\\\171.3338\\\\295.01\\\\41.13579\\\\170.7382\\\\295.01\\\\42.13579\\\\170.6482\\\\295.01\\\\43.63579\\\\170.7407\\\\295.01\\\\45.63579\\\\171.3626\\\\295.01\\\\47.63579\\\\172.3561\\\\295.01\\\\51.13579\\\\174.5457\\\\295.01\\\\53.13579\\\\175.5831\\\\295.01\\\\55.63579\\\\176.5908\\\\295.01\\\\57.13579\\\\177.05\\\\295.01\\\\60.13579\\\\177.6526\\\\295.01\\\\63.13579\\\\177.7993\\\\295.01\\\\66.13579\\\\177.6153\\\\295.01\\\\68.63579\\\\177.0842\\\\295.01\\\\71.63579\\\\176.1077\\\\295.01\\\\73.13579\\\\175.4511\\\\295.01\\\\75.63579\\\\174.1397\\\\295.01\\\\76.63579\\\\173.5072\\\\295.01\\\\78.63579\\\\172.0237\\\\295.01\\\\79.63579\\\\171.1478\\\\295.01\\\\81.34893\\\\169.4199\\\\295.01\\\\83.33125\\\\166.9199\\\\295.01\\\\84.85596\\\\164.4199\\\\295.01\\\\85.83382\\\\162.4199\\\\295.01\\\\86.74227\\\\159.9199\\\\295.01\\\\87.34301\\\\157.4199\\\\295.01\\\\87.67046\\\\154.4199\\\\295.01\\\\87.69308\\\\151.9199\\\\295.01\\\\87.28692\\\\148.4199\\\\295.01\\\\86.80479\\\\146.4199\\\\295.01\\\\85.72182\\\\143.4199\\\\295.01\\\\84.74316\\\\141.4199\\\\295.01\\\\83.8507\\\\139.9199\\\\295.01\\\\82.79869\\\\138.4199\\\\295.01\\\\81.13579\\\\136.4793\\\\295.01\\\\80.09653\\\\135.4199\\\\295.01\\\\78.13579\\\\133.6971\\\\295.01\\\\76.13579\\\\132.2705\\\\295.01\\\\73.63579\\\\130.872\\\\295.01\\\\71.13579\\\\129.7844\\\\295.01\\\\69.63579\\\\129.2868\\\\295.01\\\\67.13579\\\\128.7073\\\\295.01\\\\64.13579\\\\128.4028\\\\295.01\\\\62.13579\\\\128.3921\\\\295.01\\\\58.63579\\\\128.7287\\\\295.01\\\\56.13579\\\\129.3365\\\\295.01\\\\53.63579\\\\130.2527\\\\295.01\\\\50.63579\\\\131.8282\\\\295.01\\\\48.63579\\\\133.1938\\\\295.01\\\\46.13579\\\\135.2852\\\\295.01\\\\44.13579\\\\137.0396\\\\295.01\\\\42.63579\\\\138.0648\\\\295.01\\\\40.63579\\\\138.9977\\\\295.01\\\\38.63579\\\\139.4802\\\\295.01\\\\37.13579\\\\139.5112\\\\295.01\\\\35.13579\\\\139.0861\\\\295.01\\\\32.63579\\\\138.0112\\\\295.01\\\\31.13579\\\\137.0381\\\\295.01\\\\29.3328\\\\135.4199\\\\295.01\\\\28.06653\\\\133.9199\\\\295.01\\\\27.42027\\\\132.9199\\\\295.01\\\\26.43855\\\\130.9199\\\\295.01\\\\25.97402\\\\129.4199\\\\295.01\\\\25.83393\\\\127.9199\\\\295.01\\\\25.95701\\\\126.4199\\\\295.01\\\\26.43787\\\\124.9199\\\\295.01\\\\27.49988\\\\122.9199\\\\295.01\\\\28.62579\\\\121.4199\\\\295.01\\\\31.1932\\\\118.4199\\\\295.01\\\\32.32444\\\\116.9199\\\\295.01\\\\33.85028\\\\114.4199\\\\295.01\\\\34.81903\\\\112.4199\\\\295.01\\\\35.7245\\\\109.9199\\\\295.01\\\\36.32799\\\\107.4199\\\\295.01\\\\36.61153\\\\104.4199\\\\295.01\\\\36.68043\\\\102.9199\\\\295.01\\\\36.33499\\\\98.91986\\\\295.01\\\\35.74227\\\\96.41986\\\\295.01\\\\34.84829\\\\93.91986\\\\295.01\\\\33.88437\\\\91.91986\\\\295.01\\\\32.7434\\\\89.91986\\\\295.01\\\\31.31367\\\\87.91986\\\\295.01\\\\29.63579\\\\86.04859\\\\295.01\\\\27.13579\\\\83.74197\\\\295.01\\\\25.13579\\\\82.31819\\\\295.01\\\\23.13579\\\\81.17551\\\\295.01\\\\21.13579\\\\80.21014\\\\295.01\\\\18.63579\\\\79.3208\\\\295.01\\\\16.13579\\\\78.72765\\\\295.01\\\\12.13579\\\\78.41024\\\\295.01\\\\8.135789\\\\78.70996\\\\295.01\\\\5.635788\\\\79.269\\\\295.01\\\\2.635788\\\\80.33652\\\\295.01\\\\0.6357885\\\\81.29802\\\\295.01\\\\-2.364212\\\\83.21894\\\\295.01\\\\-4.364212\\\\84.87916\\\\295.01\\\\-6.364212\\\\86.92207\\\\295.01\\\\-7.991695\\\\88.91986\\\\295.01\\\\-9.58564\\\\91.41986\\\\295.01\\\\-10.58907\\\\93.41986\\\\295.01\\\\-11.57323\\\\95.91986\\\\295.01\\\\-12.02854\\\\97.41986\\\\295.01\\\\-12.56235\\\\99.91986\\\\295.01\\\\-12.75532\\\\102.9199\\\\295.01\\\\-12.55642\\\\106.4199\\\\295.01\\\\-12.01507\\\\108.9199\\\\295.01\\\\-11.5623\\\\110.4199\\\\295.01\\\\-10.57671\\\\112.9199\\\\295.01\\\\-8.962544\\\\115.9199\\\\295.01\\\\-7.569458\\\\117.9199\\\\295.01\\\\-5.366278\\\\120.4199\\\\295.01\\\\-3.364212\\\\122.4839\\\\295.01\\\\-2.140774\\\\123.9199\\\\295.01\\\\-1.179429\\\\125.4199\\\\295.01\\\\-0.712649\\\\126.4199\\\\295.01\\\\-0.2422979\\\\127.9199\\\\295.01\\\\-0.09468023\\\\129.4199\\\\295.01\\\\-0.234851\\\\130.9199\\\\295.01\\\\-0.6829615\\\\132.4199\\\\295.01\\\\-1.630643\\\\134.4199\\\\295.01\\\\-2.241431\\\\135.4199\\\\295.01\\\\-3.364212\\\\136.8065\\\\295.01\\\\-4.479952\\\\137.9199\\\\295.01\\\\-5.864212\\\\139.0335\\\\295.01\\\\-6.864212\\\\139.6417\\\\295.01\\\\-8.864211\\\\140.5979\\\\295.01\\\\-10.36421\\\\141.0637\\\\295.01\\\\-11.86421\\\\141.2292\\\\295.01\\\\-13.86421\\\\141.0486\\\\295.01\\\\-15.36421\\\\140.5495\\\\295.01\\\\-16.36421\\\\140.0745\\\\295.01\\\\-17.86421\\\\139.1365\\\\295.01\\\\-21.36421\\\\136.2233\\\\295.01\\\\-23.36421\\\\134.7404\\\\295.01\\\\-24.86421\\\\133.8202\\\\295.01\\\\-26.86421\\\\132.7895\\\\295.01\\\\-29.36421\\\\131.8086\\\\295.01\\\\-31.36421\\\\131.2283\\\\295.01\\\\-33.86421\\\\130.726\\\\295.01\\\\-35.86421\\\\130.5824\\\\295.01\\\\-37.86421\\\\130.5851\\\\295.01\\\\-39.86421\\\\130.732\\\\295.01\\\\-42.36421\\\\131.2641\\\\295.01\\\\-45.36421\\\\132.2405\\\\295.01\\\\-46.86421\\\\132.9028\\\\295.01\\\\-49.36421\\\\134.2029\\\\295.01\\\\-50.36421\\\\134.8394\\\\295.01\\\\-52.36421\\\\136.3216\\\\295.01\\\\-53.36421\\\\137.1936\\\\295.01\\\\-55.51793\\\\139.4199\\\\295.01\\\\-57.05593\\\\141.4199\\\\295.01\\\\-58.58438\\\\143.9199\\\\295.01\\\\-59.55771\\\\145.9199\\\\295.01\\\\-60.46825\\\\148.4199\\\\295.01\\\\-61.06986\\\\150.9199\\\\295.01\\\\-61.39546\\\\153.9199\\\\295.01\\\\-61.46327\\\\155.4199\\\\295.01\\\\-61.38126\\\\156.9199\\\\295.01\\\\-61.0591\\\\159.9199\\\\295.01\\\\-60.44754\\\\162.4199\\\\295.01\\\\-59.52451\\\\164.9199\\\\295.01\\\\-58.53138\\\\166.9199\\\\295.01\\\\-56.56637\\\\169.9199\\\\295.01\\\\-54.86625\\\\171.9199\\\\295.01\\\\-53.86421\\\\172.9499\\\\295.01\\\\-51.36421\\\\175.0986\\\\295.01\\\\-49.86421\\\\176.1393\\\\295.01\\\\-48.36421\\\\177.0304\\\\295.01\\\\-46.36421\\\\178.0032\\\\295.01\\\\-43.36421\\\\179.0823\\\\295.01\\\\-40.86421\\\\179.6404\\\\295.01\\\\-37.86421\\\\179.9579\\\\295.01\\\\-35.86421\\\\179.9677\\\\295.01\\\\-32.36421\\\\179.618\\\\295.01\\\\-29.86421\\\\179.0112\\\\295.01\\\\-27.36421\\\\178.0918\\\\295.01\\\\-25.36421\\\\177.1055\\\\295.01\\\\-22.86421\\\\175.5296\\\\295.01\\\\-19.86421\\\\173.325\\\\295.01\\\\-18.86421\\\\172.7049\\\\295.01\\\\-16.86421\\\\171.7768\\\\295.01\\\\-14.86421\\\\171.2971\\\\295.01\\\\-13.36421\\\\171.3134\\\\295.01\\\\-11.36421\\\\171.8039\\\\295.01\\\\-9.364211\\\\172.7201\\\\295.01\\\\-8.364211\\\\173.3311\\\\295.01\\\\-7.018403\\\\174.4199\\\\295.01\\\\-5.208806\\\\176.4199\\\\295.01\\\\-4.272865\\\\177.9199\\\\295.01\\\\-3.310016\\\\180.4199\\\\295.01\\\\-3.060024\\\\182.4199\\\\295.01\\\\-3.162389\\\\183.9199\\\\295.01\\\\-3.778172\\\\185.9199\\\\295.01\\\\-4.226866\\\\186.9199\\\\295.01\\\\-5.705407\\\\189.4199\\\\295.01\\\\-7.020076\\\\191.4199\\\\295.01\\\\-8.54745\\\\194.4199\\\\295.01\\\\-9.452921\\\\196.9199\\\\295.01\\\\-10.05642\\\\199.4199\\\\295.01\\\\-10.37757\\\\202.9199\\\\295.01\\\\-10.35085\\\\204.9199\\\\295.01\\\\-10.06341\\\\207.9199\\\\295.01\\\\-9.470693\\\\210.4199\\\\295.01\\\\-8.575538\\\\212.9199\\\\295.01\\\\-7.082783\\\\215.9199\\\\295.01\\\\-6.471828\\\\216.9199\\\\295.01\\\\-5.042096\\\\218.9199\\\\295.01\\\\-3.364212\\\\220.7911\\\\295.01\\\\-0.8642115\\\\223.0957\\\\295.01\\\\1.135789\\\\224.5215\\\\295.01\\\\3.135788\\\\225.6642\\\\295.01\\\\5.135788\\\\226.6284\\\\295.01\\\\7.635788\\\\227.5189\\\\295.01\\\\10.13579\\\\228.111\\\\295.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699848978606900001.507522632828\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"254\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-17.36421\\\\138.0734\\\\298.01\\\\-21.36421\\\\134.8208\\\\298.01\\\\-22.86421\\\\133.7527\\\\298.01\\\\-25.36421\\\\132.2841\\\\298.01\\\\-27.36421\\\\131.3688\\\\298.01\\\\-30.36421\\\\130.3311\\\\298.01\\\\-32.86421\\\\129.7536\\\\298.01\\\\-35.86421\\\\129.4102\\\\298.01\\\\-37.86421\\\\129.4179\\\\298.01\\\\-40.86421\\\\129.7645\\\\298.01\\\\-42.86421\\\\130.2402\\\\298.01\\\\-45.86421\\\\131.2459\\\\298.01\\\\-48.36421\\\\132.3956\\\\298.01\\\\-50.86421\\\\133.8477\\\\298.01\\\\-52.86421\\\\135.3082\\\\298.01\\\\-55.36421\\\\137.6335\\\\298.01\\\\-56.93636\\\\139.4199\\\\298.01\\\\-58.04616\\\\140.9199\\\\298.01\\\\-59.57426\\\\143.4199\\\\298.01\\\\-60.54849\\\\145.4199\\\\298.01\\\\-61.49141\\\\147.9199\\\\298.01\\\\-62.02512\\\\149.9199\\\\298.01\\\\-62.42456\\\\152.4199\\\\298.01\\\\-62.59074\\\\155.4199\\\\298.01\\\\-62.47311\\\\157.9199\\\\298.01\\\\-62.01004\\\\160.9199\\\\298.01\\\\-61.47311\\\\162.9199\\\\298.01\\\\-60.51775\\\\165.4199\\\\298.01\\\\-58.92757\\\\168.4199\\\\298.01\\\\-57.56282\\\\170.4199\\\\298.01\\\\-55.90886\\\\172.4199\\\\298.01\\\\-53.86421\\\\174.4545\\\\298.01\\\\-51.86421\\\\176.0956\\\\298.01\\\\-50.36421\\\\177.126\\\\298.01\\\\-48.86421\\\\178.0214\\\\298.01\\\\-46.86421\\\\179.0059\\\\298.01\\\\-43.86421\\\\180.1004\\\\298.01\\\\-41.86421\\\\180.6074\\\\298.01\\\\-39.36421\\\\181.0032\\\\298.01\\\\-36.86421\\\\181.1393\\\\298.01\\\\-34.86421\\\\181.0696\\\\298.01\\\\-31.36421\\\\180.5696\\\\298.01\\\\-29.36421\\\\180.0358\\\\298.01\\\\-26.86421\\\\179.0826\\\\298.01\\\\-24.86421\\\\178.0915\\\\298.01\\\\-22.36421\\\\176.5441\\\\298.01\\\\-19.86421\\\\174.7606\\\\298.01\\\\-17.36421\\\\173.3134\\\\298.01\\\\-15.86421\\\\172.7165\\\\298.01\\\\-14.36421\\\\172.3535\\\\298.01\\\\-12.86421\\\\172.2821\\\\298.01\\\\-10.86421\\\\172.8134\\\\298.01\\\\-9.864211\\\\173.2612\\\\298.01\\\\-8.364211\\\\174.2009\\\\298.01\\\\-6.864212\\\\175.5768\\\\298.01\\\\-5.747024\\\\176.9199\\\\298.01\\\\-4.668733\\\\178.9199\\\\298.01\\\\-4.188285\\\\180.4199\\\\298.01\\\\-4.035754\\\\181.9199\\\\298.01\\\\-4.183949\\\\183.4199\\\\298.01\\\\-4.643007\\\\184.9199\\\\298.01\\\\-5.294955\\\\186.4199\\\\298.01\\\\-6.132928\\\\187.9199\\\\298.01\\\\-8.012517\\\\190.9199\\\\298.01\\\\-9.077596\\\\192.9199\\\\298.01\\\\-9.952921\\\\194.9199\\\\298.01\\\\-10.47785\\\\196.4199\\\\298.01\\\\-11.01005\\\\198.4199\\\\298.01\\\\-11.45025\\\\201.4199\\\\298.01\\\\-11.55772\\\\203.9199\\\\298.01\\\\-11.46073\\\\205.9199\\\\298.01\\\\-11.01962\\\\208.9199\\\\298.01\\\\-10.49571\\\\210.9199\\\\298.01\\\\-9.571923\\\\213.4199\\\\298.01\\\\-8.606749\\\\215.4199\\\\298.01\\\\-7.463268\\\\217.4199\\\\298.01\\\\-6.421503\\\\218.9199\\\\298.01\\\\-5.364212\\\\220.2352\\\\298.01\\\\-3.29047\\\\222.4199\\\\298.01\\\\-0.8642115\\\\224.4741\\\\298.01\\\\1.635789\\\\226.1212\\\\298.01\\\\4.635788\\\\227.6221\\\\298.01\\\\7.135788\\\\228.5449\\\\298.01\\\\9.135789\\\\229.0657\\\\298.01\\\\12.13579\\\\229.4949\\\\298.01\\\\14.13579\\\\229.5963\\\\298.01\\\\16.13579\\\\229.5164\\\\298.01\\\\19.13579\\\\229.0929\\\\298.01\\\\21.13579\\\\228.5912\\\\298.01\\\\24.13579\\\\227.5032\\\\298.01\\\\26.13579\\\\226.5287\\\\298.01\\\\29.13579\\\\224.6157\\\\298.01\\\\31.13579\\\\222.9954\\\\298.01\\\\32.76643\\\\221.4199\\\\298.01\\\\33.67046\\\\220.4199\\\\298.01\\\\35.25476\\\\218.4199\\\\298.01\\\\36.84434\\\\215.9199\\\\298.01\\\\37.8587\\\\213.9199\\\\298.01\\\\38.85302\\\\211.4199\\\\298.01\\\\39.31954\\\\209.9199\\\\298.01\\\\39.80026\\\\207.9199\\\\298.01\\\\40.16704\\\\204.9199\\\\298.01\\\\40.16004\\\\202.4199\\\\298.01\\\\39.79305\\\\199.4199\\\\298.01\\\\39.31053\\\\197.4199\\\\298.01\\\\38.84\\\\195.9199\\\\298.01\\\\37.8401\\\\193.4199\\\\298.01\\\\36.82063\\\\191.4199\\\\298.01\\\\35.26744\\\\188.9199\\\\298.01\\\\33.94211\\\\186.9199\\\\298.01\\\\32.91801\\\\184.9199\\\\298.01\\\\32.53175\\\\183.9199\\\\298.01\\\\32.02689\\\\181.4199\\\\298.01\\\\32.4123\\\\179.4199\\\\298.01\\\\32.97419\\\\177.9199\\\\298.01\\\\33.5138\\\\176.9199\\\\298.01\\\\34.61511\\\\175.4199\\\\298.01\\\\35.63579\\\\174.3649\\\\298.01\\\\37.13579\\\\173.2065\\\\298.01\\\\39.13579\\\\172.2018\\\\298.01\\\\40.63579\\\\171.7608\\\\298.01\\\\42.63579\\\\171.7324\\\\298.01\\\\44.63579\\\\172.2536\\\\298.01\\\\47.63579\\\\173.7016\\\\298.01\\\\50.63579\\\\175.5381\\\\298.01\\\\52.63579\\\\176.5734\\\\298.01\\\\55.13579\\\\177.5946\\\\298.01\\\\56.63579\\\\178.0771\\\\298.01\\\\58.63579\\\\178.5676\\\\298.01\\\\61.63579\\\\178.9218\\\\298.01\\\\64.13579\\\\178.9405\\\\298.01\\\\67.13579\\\\178.5826\\\\298.01\\\\69.13579\\\\178.1077\\\\298.01\\\\72.13579\\\\177.1048\\\\298.01\\\\73.63579\\\\176.4441\\\\298.01\\\\76.13579\\\\175.128\\\\298.01\\\\77.13579\\\\174.5032\\\\298.01\\\\79.13579\\\\173.037\\\\298.01\\\\80.41171\\\\171.9199\\\\298.01\\\\82.80591\\\\169.4199\\\\298.01\\\\84.32225\\\\167.4199\\\\298.01\\\\85.84733\\\\164.9199\\\\298.01\\\\86.82425\\\\162.9199\\\\298.01\\\\87.76729\\\\160.4199\\\\298.01\\\\88.29848\\\\158.4199\\\\298.01\\\\88.69914\\\\155.9199\\\\298.01\\\\88.86338\\\\152.9199\\\\298.01\\\\88.73231\\\\150.4199\\\\298.01\\\\88.32969\\\\147.9199\\\\298.01\\\\87.82111\\\\145.9199\\\\298.01\\\\86.72449\\\\142.9199\\\\298.01\\\\85.73485\\\\140.9199\\\\298.01\\\\84.83606\\\\139.4199\\\\298.01\\\\83.80003\\\\137.9199\\\\298.01\\\\81.67046\\\\135.4199\\\\298.01\\\\80.63579\\\\134.4028\\\\298.01\\\\78.13579\\\\132.3134\\\\298.01\\\\76.63579\\\\131.2847\\\\298.01\\\\74.18953\\\\129.9199\\\\298.01\\\\71.63579\\\\128.778\\\\298.01\\\\70.13579\\\\128.2663\\\\298.01\\\\68.13579\\\\127.7468\\\\298.01\\\\65.63579\\\\127.3565\\\\298.01\\\\64.13579\\\\127.2386\\\\298.01\\\\60.63579\\\\127.3285\\\\298.01\\\\57.63579\\\\127.776\\\\298.01\\\\55.63579\\\\128.311\\\\298.01\\\\53.13579\\\\129.2608\\\\298.01\\\\51.13579\\\\130.2564\\\\298.01\\\\48.63579\\\\131.8259\\\\298.01\\\\46.63579\\\\133.3886\\\\298.01\\\\43.63579\\\\135.9802\\\\298.01\\\\42.13579\\\\137.0449\\\\298.01\\\\40.13579\\\\138.0715\\\\298.01\\\\38.63579\\\\138.5637\\\\298.01\\\\36.63579\\\\138.7519\\\\298.01\\\\35.13579\\\\138.5808\\\\298.01\\\\33.63579\\\\138.0696\\\\298.01\\\\31.63579\\\\136.9511\\\\298.01\\\\29.8448\\\\135.4199\\\\298.01\\\\28.96778\\\\134.4199\\\\298.01\\\\27.9767\\\\132.9199\\\\298.01\\\\27.488\\\\131.9199\\\\298.01\\\\26.98038\\\\130.4199\\\\298.01\\\\26.80886\\\\128.9199\\\\298.01\\\\26.93039\\\\127.4199\\\\298.01\\\\27.55797\\\\125.4199\\\\298.01\\\\28.02931\\\\124.4199\\\\298.01\\\\28.95004\\\\122.9199\\\\298.01\\\\30.51157\\\\120.9199\\\\298.01\\\\31.80976\\\\119.4199\\\\298.01\\\\33.31496\\\\117.4199\\\\298.01\\\\34.83657\\\\114.9199\\\\298.01\\\\35.81157\\\\112.9199\\\\298.01\\\\36.74942\\\\110.4199\\\\298.01\\\\37.28162\\\\108.4199\\\\298.01\\\\37.72183\\\\105.4199\\\\298.01\\\\37.8293\\\\102.9199\\\\298.01\\\\37.73231\\\\100.9199\\\\298.01\\\\37.29119\\\\97.91986\\\\298.01\\\\36.76729\\\\95.91986\\\\298.01\\\\35.8435\\\\93.41986\\\\298.01\\\\34.34129\\\\90.41986\\\\298.01\\\\32.69308\\\\87.91986\\\\298.01\\\\31.63579\\\\86.6045\\\\298.01\\\\30.08003\\\\84.91986\\\\298.01\\\\27.63579\\\\82.74211\\\\298.01\\\\25.63579\\\\81.32591\\\\298.01\\\\23.63579\\\\80.17981\\\\298.01\\\\21.63579\\\\79.2176\\\\298.01\\\\19.13579\\\\78.29486\\\\298.01\\\\17.13579\\\\77.77402\\\\298.01\\\\14.13579\\\\77.34486\\\\298.01\\\\12.13579\\\\77.24346\\\\298.01\\\\10.13579\\\\77.32334\\\\298.01\\\\7.135788\\\\77.74678\\\\298.01\\\\5.135788\\\\78.24847\\\\298.01\\\\2.135788\\\\79.33382\\\\298.01\\\\0.1357885\\\\80.31096\\\\298.01\\\\-2.864212\\\\82.22402\\\\298.01\\\\-4.864212\\\\83.84435\\\\298.01\\\\-6.494857\\\\85.41986\\\\298.01\\\\-7.398883\\\\86.41986\\\\298.01\\\\-8.986199\\\\88.41986\\\\298.01\\\\-10.57276\\\\90.91986\\\\298.01\\\\-11.58712\\\\92.91986\\\\298.01\\\\-12.58144\\\\95.41986\\\\298.01\\\\-13.04796\\\\96.91986\\\\298.01\\\\-13.52869\\\\98.91986\\\\298.01\\\\-13.89546\\\\101.9199\\\\298.01\\\\-13.88846\\\\104.4199\\\\298.01\\\\-13.52147\\\\107.4199\\\\298.01\\\\-13.03896\\\\109.4199\\\\298.01\\\\-12.56842\\\\110.9199\\\\298.01\\\\-11.56853\\\\113.4199\\\\298.01\\\\-10.54737\\\\115.4199\\\\298.01\\\\-8.942026\\\\117.9199\\\\298.01\\\\-6.441266\\\\120.9199\\\\298.01\\\\-4.864212\\\\122.5615\\\\298.01\\\\-3.257069\\\\124.4199\\\\298.01\\\\-2.292064\\\\125.9199\\\\298.01\\\\-1.797885\\\\126.9199\\\\298.01\\\\-1.141081\\\\128.9199\\\\298.01\\\\-0.9914045\\\\130.4199\\\\298.01\\\\-1.169235\\\\131.9199\\\\298.01\\\\-1.678251\\\\133.4199\\\\298.01\\\\-2.778172\\\\135.4199\\\\298.01\\\\-3.864212\\\\136.7395\\\\298.01\\\\-5.864212\\\\138.492\\\\298.01\\\\-7.864212\\\\139.5963\\\\298.01\\\\-9.364211\\\\140.1104\\\\298.01\\\\-11.36421\\\\140.3688\\\\298.01\\\\-13.36421\\\\140.0734\\\\298.01\\\\-14.86421\\\\139.5239\\\\298.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851016723500001.512636853846\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"246\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"15\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n";
const char* k_rtStruct_json07 =
"                        \"Value\" : \"41.63579\\\\136.0798\\\\301.01\\\\39.63579\\\\137.1339\\\\301.01\\\\38.13579\\\\137.6336\\\\301.01\\\\36.63579\\\\137.8285\\\\301.01\\\\35.13579\\\\137.6244\\\\301.01\\\\33.63579\\\\137.0851\\\\301.01\\\\32.63579\\\\136.5518\\\\301.01\\\\31.13579\\\\135.4333\\\\301.01\\\\30.12233\\\\134.4199\\\\301.01\\\\29.0038\\\\132.9199\\\\301.01\\\\28.47056\\\\131.9199\\\\301.01\\\\27.93124\\\\130.4199\\\\301.01\\\\27.72713\\\\128.9199\\\\301.01\\\\27.9259\\\\127.4199\\\\301.01\\\\28.43386\\\\125.9199\\\\301.01\\\\29.51482\\\\123.9199\\\\301.01\\\\31.77887\\\\120.9199\\\\301.01\\\\33.79488\\\\117.9199\\\\301.01\\\\35.27357\\\\115.4199\\\\301.01\\\\36.26941\\\\113.4199\\\\301.01\\\\37.23983\\\\110.9199\\\\301.01\\\\37.80718\\\\108.9199\\\\301.01\\\\38.26298\\\\106.4199\\\\301.01\\\\38.42556\\\\102.9199\\\\301.01\\\\38.32632\\\\100.4199\\\\301.01\\\\37.81704\\\\97.41986\\\\301.01\\\\37.26298\\\\95.41986\\\\301.01\\\\36.30376\\\\92.91986\\\\301.01\\\\35.31862\\\\90.91986\\\\301.01\\\\33.79669\\\\88.41986\\\\301.01\\\\32.69613\\\\86.91986\\\\301.01\\\\31.40617\\\\85.41986\\\\301.01\\\\30.13579\\\\84.11847\\\\301.01\\\\28.13579\\\\82.36256\\\\301.01\\\\26.63579\\\\81.26077\\\\301.01\\\\24.13579\\\\79.73861\\\\301.01\\\\22.13579\\\\78.75716\\\\301.01\\\\19.63579\\\\77.80389\\\\301.01\\\\17.63579\\\\77.24678\\\\301.01\\\\15.13579\\\\76.79932\\\\301.01\\\\12.13579\\\\76.64858\\\\301.01\\\\9.635789\\\\76.72783\\\\301.01\\\\6.635788\\\\77.21915\\\\301.01\\\\4.635788\\\\77.74511\\\\301.01\\\\3.135788\\\\78.25896\\\\301.01\\\\0.5678061\\\\79.41986\\\\301.01\\\\-1.864211\\\\80.78835\\\\301.01\\\\-3.364212\\\\81.83115\\\\301.01\\\\-4.710485\\\\82.91986\\\\301.01\\\\-6.864212\\\\84.94054\\\\301.01\\\\-9.004042\\\\87.41986\\\\301.01\\\\-11.00404\\\\90.41986\\\\301.01\\\\-12.04224\\\\92.41986\\\\301.01\\\\-13.0635\\\\94.91986\\\\301.01\\\\-13.55475\\\\96.41986\\\\301.01\\\\-14.05919\\\\98.41986\\\\301.01\\\\-14.46577\\\\100.9199\\\\301.01\\\\-14.56064\\\\103.9199\\\\301.01\\\\-14.45556\\\\105.4199\\\\301.01\\\\-14.05171\\\\107.9199\\\\301.01\\\\-13.54386\\\\109.9199\\\\301.01\\\\-12.47311\\\\112.9199\\\\301.01\\\\-11.52147\\\\114.9199\\\\301.01\\\\-10.0356\\\\117.4199\\\\301.01\\\\-7.977848\\\\120.4199\\\\301.01\\\\-6.493406\\\\122.4199\\\\301.01\\\\-4.729596\\\\124.4199\\\\301.01\\\\-3.644515\\\\125.9199\\\\301.01\\\\-2.642783\\\\127.9199\\\\301.01\\\\-2.184044\\\\129.4199\\\\301.01\\\\-2.190483\\\\131.4199\\\\301.01\\\\-2.659926\\\\132.9199\\\\301.01\\\\-3.746865\\\\134.9199\\\\301.01\\\\-4.864212\\\\136.2112\\\\301.01\\\\-5.864212\\\\137.1478\\\\301.01\\\\-7.364212\\\\138.139\\\\301.01\\\\-8.364211\\\\138.6184\\\\301.01\\\\-9.864211\\\\139.0841\\\\301.01\\\\-10.86421\\\\139.1837\\\\301.01\\\\-12.36421\\\\139.0851\\\\301.01\\\\-13.86421\\\\138.6368\\\\301.01\\\\-15.36421\\\\137.9369\\\\301.01\\\\-16.86421\\\\137.0216\\\\301.01\\\\-19.36421\\\\135.1894\\\\301.01\\\\-22.36421\\\\133.2626\\\\301.01\\\\-25.86421\\\\131.3565\\\\301.01\\\\-28.36421\\\\130.3086\\\\301.01\\\\-29.86421\\\\129.8259\\\\301.01\\\\-32.36421\\\\129.2191\\\\301.01\\\\-34.86421\\\\128.8338\\\\301.01\\\\-37.86421\\\\128.7519\\\\301.01\\\\-41.36421\\\\129.2324\\\\301.01\\\\-43.36421\\\\129.7339\\\\301.01\\\\-46.36421\\\\130.7721\\\\301.01\\\\-49.36421\\\\132.2284\\\\301.01\\\\-51.86421\\\\133.7884\\\\301.01\\\\-53.86421\\\\135.3506\\\\301.01\\\\-56.47069\\\\137.9199\\\\301.01\\\\-58.44481\\\\140.4199\\\\301.01\\\\-60.00607\\\\142.9199\\\\301.01\\\\-61.00807\\\\144.9199\\\\301.01\\\\-61.98475\\\\147.4199\\\\301.01\\\\-62.55171\\\\149.4199\\\\301.01\\\\-62.94481\\\\151.4199\\\\301.01\\\\-63.10673\\\\152.9199\\\\301.01\\\\-63.19798\\\\155.4199\\\\301.01\\\\-63.00807\\\\158.9199\\\\301.01\\\\-62.53729\\\\161.4199\\\\301.01\\\\-61.96073\\\\163.4199\\\\301.01\\\\-60.97069\\\\165.9199\\\\301.01\\\\-60.48248\\\\166.9199\\\\301.01\\\\-59.01394\\\\169.4199\\\\301.01\\\\-57.95292\\\\170.9199\\\\301.01\\\\-56.86421\\\\172.2435\\\\301.01\\\\-55.29496\\\\173.9199\\\\301.01\\\\-53.67131\\\\175.4199\\\\301.01\\\\-52.36421\\\\176.4802\\\\301.01\\\\-49.86421\\\\178.1377\\\\301.01\\\\-47.36421\\\\179.4369\\\\301.01\\\\-45.86421\\\\180.0861\\\\301.01\\\\-42.86421\\\\181.0311\\\\301.01\\\\-40.36421\\\\181.5311\\\\301.01\\\\-36.86421\\\\181.7386\\\\301.01\\\\-33.36421\\\\181.5637\\\\301.01\\\\-30.86421\\\\181.0963\\\\301.01\\\\-28.86421\\\\180.5239\\\\301.01\\\\-26.36421\\\\179.5404\\\\301.01\\\\-24.36421\\\\178.5404\\\\301.01\\\\-20.36421\\\\176.3956\\\\301.01\\\\-17.86421\\\\175.2373\\\\301.01\\\\-14.86421\\\\174.2269\\\\301.01\\\\-13.36421\\\\174.0709\\\\301.01\\\\-11.86421\\\\174.2958\\\\301.01\\\\-9.864211\\\\175.2389\\\\301.01\\\\-8.441711\\\\176.4199\\\\301.01\\\\-7.16156\\\\177.9199\\\\301.01\\\\-6.177148\\\\179.9199\\\\301.01\\\\-5.842727\\\\181.4199\\\\301.01\\\\-5.813838\\\\182.4199\\\\301.01\\\\-6.169591\\\\184.4199\\\\301.01\\\\-7.196132\\\\187.4199\\\\301.01\\\\-8.770263\\\\190.9199\\\\301.01\\\\-10.00404\\\\193.4199\\\\301.01\\\\-10.97069\\\\195.9199\\\\301.01\\\\-11.5356\\\\197.9199\\\\301.01\\\\-12.04705\\\\200.9199\\\\301.01\\\\-12.15398\\\\203.9199\\\\301.01\\\\-12.05475\\\\206.4199\\\\301.01\\\\-11.54546\\\\209.4199\\\\301.01\\\\-10.9914\\\\211.4199\\\\301.01\\\\-10.03218\\\\213.9199\\\\301.01\\\\-9.047048\\\\215.9199\\\\301.01\\\\-7.525116\\\\218.4199\\\\301.01\\\\-6.424556\\\\219.9199\\\\301.01\\\\-5.133131\\\\221.4199\\\\301.01\\\\-3.864212\\\\222.7213\\\\301.01\\\\-1.864211\\\\224.4771\\\\301.01\\\\-0.3642115\\\\225.5771\\\\301.01\\\\2.135788\\\\227.0995\\\\301.01\\\\4.135788\\\\228.0826\\\\301.01\\\\6.635788\\\\229.0358\\\\301.01\\\\8.635789\\\\229.5929\\\\301.01\\\\11.13579\\\\230.0381\\\\301.01\\\\14.13579\\\\230.1911\\\\301.01\\\\16.63579\\\\230.1119\\\\301.01\\\\19.63579\\\\229.6206\\\\301.01\\\\21.63579\\\\229.0929\\\\301.01\\\\23.13579\\\\228.5808\\\\301.01\\\\25.70377\\\\227.4199\\\\301.01\\\\28.13579\\\\226.0514\\\\301.01\\\\29.63579\\\\225.0086\\\\301.01\\\\30.98206\\\\223.9199\\\\301.01\\\\33.13579\\\\221.8956\\\\301.01\\\\35.27562\\\\219.4199\\\\301.01\\\\37.27562\\\\216.4199\\\\301.01\\\\38.31382\\\\214.4199\\\\301.01\\\\39.33508\\\\211.9199\\\\301.01\\\\39.82632\\\\210.4199\\\\301.01\\\\40.33076\\\\208.4199\\\\301.01\\\\40.73735\\\\205.9199\\\\301.01\\\\40.83222\\\\202.9199\\\\301.01\\\\40.72713\\\\201.4199\\\\301.01\\\\40.32329\\\\198.9199\\\\301.01\\\\39.81544\\\\196.9199\\\\301.01\\\\39.32329\\\\195.4199\\\\301.01\\\\38.29488\\\\192.9199\\\\301.01\\\\35.95219\\\\188.4199\\\\301.01\\\\34.43608\\\\184.9199\\\\301.01\\\\33.9686\\\\183.4199\\\\301.01\\\\33.73262\\\\181.4199\\\\301.01\\\\33.94788\\\\179.9199\\\\301.01\\\\34.52448\\\\178.4199\\\\301.01\\\\35.44645\\\\176.9199\\\\301.01\\\\36.36579\\\\175.9199\\\\301.01\\\\37.63579\\\\174.8116\\\\301.01\\\\39.63579\\\\173.747\\\\301.01\\\\41.13579\\\\173.3669\\\\301.01\\\\42.13579\\\\173.3279\\\\301.01\\\\44.13579\\\\173.7036\\\\301.01\\\\46.13579\\\\174.3009\\\\301.01\\\\48.63579\\\\175.2884\\\\301.01\\\\51.13579\\\\176.5471\\\\301.01\\\\54.63579\\\\178.0789\\\\301.01\\\\56.13579\\\\178.5789\\\\301.01\\\\58.13579\\\\179.0963\\\\301.01\\\\60.63579\\\\179.4977\\\\301.01\\\\62.13579\\\\179.6058\\\\301.01\\\\65.13579\\\\179.5138\\\\301.01\\\\67.63579\\\\179.1134\\\\301.01\\\\69.63579\\\\178.6148\\\\301.01\\\\72.63579\\\\177.5771\\\\301.01\\\\75.63579\\\\176.1191\\\\301.01\\\\78.13579\\\\174.5576\\\\301.01\\\\80.13579\\\\172.992\\\\301.01\\\\81.80482\\\\171.4199\\\\301.01\\\\83.17384\\\\169.9199\\\\301.01\\\\84.72182\\\\167.9199\\\\301.01\\\\86.27964\\\\165.4199\\\\301.01\\\\87.28358\\\\163.4199\\\\301.01\\\\88.26079\\\\160.9199\\\\301.01\\\\88.82632\\\\158.9199\\\\301.01\\\\89.22182\\\\156.9199\\\\301.01\\\\89.3794\\\\155.4199\\\\301.01\\\\89.46956\\\\152.9199\\\\301.01\\\\89.26079\\\\149.4199\\\\301.01\\\\88.75857\\\\146.9199\\\\301.01\\\\87.80548\\\\143.9199\\\\301.01\\\\87.16004\\\\142.4199\\\\301.01\\\\85.85093\\\\139.9199\\\\301.01\\\\84.18044\\\\137.4199\\\\301.01\\\\82.63579\\\\135.578\\\\301.01\\\\80.45245\\\\133.4199\\\\301.01\\\\79.13579\\\\132.3039\\\\301.01\\\\76.13579\\\\130.259\\\\301.01\\\\73.13579\\\\128.7278\\\\301.01\\\\70.63579\\\\127.7663\\\\301.01\\\\68.63579\\\\127.2192\\\\301.01\\\\66.63579\\\\126.8285\\\\301.01\\\\63.13579\\\\126.6148\\\\301.01\\\\59.63579\\\\126.7884\\\\301.01\\\\57.13579\\\\127.2502\\\\301.01\\\\55.13579\\\\127.8208\\\\301.01\\\\52.63579\\\\128.8062\\\\301.01\\\\51.63579\\\\129.2905\\\\301.01\\\\49.13579\\\\130.7368\\\\301.01\\\\46.13579\\\\132.7185\\\\301.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851035724600001.533642609670\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"234\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"16\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-7.002701\\\\124.9199\\\\304.01\\\\-5.332349\\\\127.4199\\\\304.01\\\\-4.657244\\\\128.9199\\\\304.01\\\\-4.339658\\\\130.4199\\\\304.01\\\\-4.672063\\\\131.9199\\\\304.01\\\\-5.731034\\\\133.9199\\\\304.01\\\\-7.364212\\\\135.553\\\\304.01\\\\-9.364211\\\\136.612\\\\304.01\\\\-10.86421\\\\136.9399\\\\304.01\\\\-12.86421\\\\136.6037\\\\304.01\\\\-14.36421\\\\135.9559\\\\304.01\\\\-16.36421\\\\134.8809\\\\304.01\\\\-19.36421\\\\134.0734\\\\304.01\\\\-20.86421\\\\133.5753\\\\304.01\\\\-23.36421\\\\132.4405\\\\304.01\\\\-26.86421\\\\130.722\\\\304.01\\\\-29.36421\\\\129.7841\\\\304.01\\\\-31.36421\\\\129.2435\\\\304.01\\\\-33.86421\\\\128.7862\\\\304.01\\\\-36.86421\\\\128.6288\\\\304.01\\\\-39.86421\\\\128.8016\\\\304.01\\\\-42.36421\\\\129.274\\\\304.01\\\\-44.36421\\\\129.8477\\\\304.01\\\\-46.86421\\\\130.78\\\\304.01\\\\-48.86421\\\\131.7485\\\\304.01\\\\-51.36421\\\\133.2519\\\\304.01\\\\-52.86421\\\\134.3259\\\\304.01\\\\-54.71862\\\\135.9199\\\\304.01\\\\-56.66553\\\\137.9199\\\\304.01\\\\-57.90558\\\\139.4199\\\\304.01\\\\-58.98475\\\\140.9199\\\\304.01\\\\-59.89888\\\\142.4199\\\\304.01\\\\-60.96327\\\\144.4199\\\\304.01\\\\-62.00404\\\\146.9199\\\\304.01\\\\-62.49141\\\\148.4199\\\\304.01\\\\-62.99141\\\\150.4199\\\\304.01\\\\-63.42456\\\\153.4199\\\\304.01\\\\-63.54705\\\\155.4199\\\\304.01\\\\-63.39888\\\\157.4199\\\\304.01\\\\-63.05474\\\\159.9199\\\\304.01\\\\-62.44481\\\\162.4199\\\\304.01\\\\-61.95556\\\\163.9199\\\\304.01\\\\-60.87383\\\\166.4199\\\\304.01\\\\-59.51394\\\\168.9199\\\\304.01\\\\-58.50607\\\\170.4199\\\\304.01\\\\-56.92757\\\\172.4199\\\\304.01\\\\-55.52116\\\\173.9199\\\\304.01\\\\-53.86421\\\\175.4709\\\\304.01\\\\-51.86421\\\\177.0449\\\\304.01\\\\-49.36421\\\\178.6177\\\\304.01\\\\-46.36421\\\\180.0843\\\\304.01\\\\-43.36421\\\\181.1058\\\\304.01\\\\-41.36421\\\\181.5843\\\\304.01\\\\-38.36421\\\\181.9891\\\\304.01\\\\-35.86421\\\\182.0426\\\\304.01\\\\-32.36421\\\\181.6119\\\\304.01\\\\-29.86421\\\\181.0164\\\\304.01\\\\-28.36421\\\\180.5311\\\\304.01\\\\-25.86421\\\\179.4802\\\\304.01\\\\-23.36421\\\\178.2136\\\\304.01\\\\-21.36421\\\\177.3016\\\\304.01\\\\-19.86421\\\\176.7993\\\\304.01\\\\-17.36421\\\\176.4579\\\\304.01\\\\-15.36421\\\\176.7158\\\\304.01\\\\-13.36421\\\\177.2203\\\\304.01\\\\-11.36421\\\\178.2083\\\\304.01\\\\-9.70184\\\\179.9199\\\\304.01\\\\-8.743179\\\\181.9199\\\\304.01\\\\-8.174695\\\\183.9199\\\\304.01\\\\-7.905579\\\\185.9199\\\\304.01\\\\-8.208806\\\\188.4199\\\\304.01\\\\-9.245948\\\\191.4199\\\\304.01\\\\-9.968248\\\\192.9199\\\\304.01\\\\-10.97069\\\\195.4199\\\\304.01\\\\-11.4448\\\\196.9199\\\\304.01\\\\-12.03729\\\\199.4199\\\\304.01\\\\-12.38126\\\\202.4199\\\\304.01\\\\-12.46825\\\\203.9199\\\\304.01\\\\-12.05171\\\\207.9199\\\\304.01\\\\-11.47785\\\\210.4199\\\\304.01\\\\-10.41841\\\\213.4199\\\\304.01\\\\-9.501996\\\\215.4199\\\\304.01\\\\-8.038957\\\\217.9199\\\\304.01\\\\-6.995714\\\\219.4199\\\\304.01\\\\-5.364212\\\\221.372\\\\304.01\\\\-4.364212\\\\222.4369\\\\304.01\\\\-2.749882\\\\223.9199\\\\304.01\\\\-1.364211\\\\225.0471\\\\304.01\\\\0.1357885\\\\226.0878\\\\304.01\\\\2.635788\\\\227.5471\\\\304.01\\\\3.635788\\\\228.0311\\\\304.01\\\\6.135788\\\\229.0404\\\\304.01\\\\7.635788\\\\229.5059\\\\304.01\\\\10.13579\\\\230.0826\\\\304.01\\\\13.63579\\\\230.4369\\\\304.01\\\\15.13579\\\\230.4102\\\\304.01\\\\18.13579\\\\230.1177\\\\304.01\\\\20.63579\\\\229.5789\\\\304.01\\\\23.63579\\\\228.5771\\\\304.01\\\\25.63579\\\\227.6478\\\\304.01\\\\27.63579\\\\226.5696\\\\304.01\\\\29.13579\\\\225.5808\\\\304.01\\\\30.63579\\\\224.4476\\\\304.01\\\\32.13579\\\\223.1389\\\\304.01\\\\34.72183\\\\220.4199\\\\304.01\\\\36.23735\\\\218.4199\\\\304.01\\\\37.77357\\\\215.9199\\\\304.01\\\\38.76079\\\\213.9199\\\\304.01\\\\39.71638\\\\211.4199\\\\304.01\\\\40.29669\\\\209.4199\\\\304.01\\\\40.83928\\\\206.4199\\\\304.01\\\\40.95776\\\\203.9199\\\\304.01\\\\40.76941\\\\200.4199\\\\304.01\\\\40.27357\\\\197.9199\\\\304.01\\\\39.32481\\\\194.9199\\\\304.01\\\\38.71079\\\\193.4199\\\\304.01\\\\37.00217\\\\189.9199\\\\304.01\\\\35.97488\\\\186.9199\\\\304.01\\\\35.81218\\\\184.9199\\\\304.01\\\\36.02215\\\\182.9199\\\\304.01\\\\36.53948\\\\180.9199\\\\304.01\\\\36.95037\\\\179.9199\\\\304.01\\\\37.94024\\\\178.4199\\\\304.01\\\\39.13579\\\\177.2874\\\\304.01\\\\40.13579\\\\176.6719\\\\304.01\\\\42.13579\\\\175.8343\\\\304.01\\\\44.13579\\\\175.2645\\\\304.01\\\\45.63579\\\\175.1192\\\\304.01\\\\47.13579\\\\175.259\\\\304.01\\\\49.13579\\\\175.872\\\\304.01\\\\54.13579\\\\178.0657\\\\304.01\\\\57.13579\\\\179.0164\\\\304.01\\\\59.63579\\\\179.5214\\\\304.01\\\\63.13579\\\\179.7263\\\\304.01\\\\66.13579\\\\179.5597\\\\304.01\\\\68.63579\\\\179.0826\\\\304.01\\\\70.63579\\\\178.5164\\\\304.01\\\\73.13579\\\\177.5753\\\\304.01\\\\75.13579\\\\176.6043\\\\304.01\\\\77.63579\\\\175.0995\\\\304.01\\\\79.13579\\\\174.0263\\\\304.01\\\\81.00275\\\\172.4199\\\\304.01\\\\82.94763\\\\170.4199\\\\304.01\\\\84.19308\\\\168.9199\\\\304.01\\\\85.26515\\\\167.4199\\\\304.01\\\\86.74468\\\\164.9199\\\\304.01\\\\87.67715\\\\162.9199\\\\304.01\\\\88.76941\\\\159.9199\\\\304.01\\\\89.26729\\\\157.9199\\\\304.01\\\\89.69914\\\\154.9199\\\\304.01\\\\89.77764\\\\151.9199\\\\304.01\\\\89.31544\\\\148.4199\\\\304.01\\\\88.83076\\\\146.4199\\\\304.01\\\\88.18044\\\\144.4199\\\\304.01\\\\87.34991\\\\142.4199\\\\304.01\\\\86.33366\\\\140.4199\\\\304.01\\\\84.75405\\\\137.9199\\\\304.01\\\\83.16004\\\\135.9199\\\\304.01\\\\80.63579\\\\133.3449\\\\304.01\\\\78.13579\\\\131.3449\\\\304.01\\\\75.63579\\\\129.7572\\\\304.01\\\\73.63579\\\\128.7402\\\\304.01\\\\71.13579\\\\127.7324\\\\304.01\\\\69.63579\\\\127.2572\\\\304.01\\\\67.63579\\\\126.778\\\\304.01\\\\64.13579\\\\126.3338\\\\304.01\\\\61.63579\\\\126.3752\\\\304.01\\\\58.63579\\\\126.7435\\\\304.01\\\\56.13579\\\\127.3449\\\\304.01\\\\53.63579\\\\128.2206\\\\304.01\\\\51.13579\\\\129.3886\\\\304.01\\\\48.13579\\\\131.1148\\\\304.01\\\\45.13579\\\\132.5771\\\\304.01\\\\42.63579\\\\133.274\\\\304.01\\\\41.63579\\\\133.6847\\\\304.01\\\\40.13579\\\\134.6251\\\\304.01\\\\38.13579\\\\135.5657\\\\304.01\\\\36.63579\\\\135.7968\\\\304.01\\\\35.13579\\\\135.5526\\\\304.01\\\\33.13579\\\\134.5317\\\\304.01\\\\32.13579\\\\133.6744\\\\304.01\\\\31.02395\\\\132.4199\\\\304.01\\\\30.0031\\\\130.4199\\\\304.01\\\\29.75883\\\\128.9199\\\\304.01\\\\29.99374\\\\127.4199\\\\304.01\\\\30.93457\\\\125.4199\\\\304.01\\\\31.87468\\\\123.9199\\\\304.01\\\\32.28358\\\\122.9199\\\\304.01\\\\32.97309\\\\120.4199\\\\304.01\\\\34.10454\\\\117.9199\\\\304.01\\\\36.20794\\\\113.9199\\\\304.01\\\\37.24227\\\\111.4199\\\\304.01\\\\37.71638\\\\109.9199\\\\304.01\\\\38.30886\\\\107.4199\\\\304.01\\\\38.70211\\\\103.9199\\\\304.01\\\\38.67046\\\\101.9199\\\\304.01\\\\38.32329\\\\98.91986\\\\304.01\\\\37.74942\\\\96.41986\\\\304.01\\\\36.68998\\\\93.41986\\\\304.01\\\\35.77357\\\\91.41986\\\\304.01\\\\34.31053\\\\88.91986\\\\304.01\\\\33.26729\\\\87.41986\\\\304.01\\\\32.13579\\\\86.02943\\\\304.01\\\\30.13383\\\\83.91986\\\\304.01\\\\27.63579\\\\81.79266\\\\304.01\\\\26.13579\\\\80.75189\\\\304.01\\\\23.63579\\\\79.29266\\\\304.01\\\\20.13579\\\\77.79707\\\\304.01\\\\18.63579\\\\77.33382\\\\304.01\\\\16.13579\\\\76.75716\\\\304.01\\\\12.63579\\\\76.39918\\\\304.01\\\\11.13579\\\\76.42567\\\\304.01\\\\8.135789\\\\76.72199\\\\304.01\\\\5.635788\\\\77.25896\\\\304.01\\\\2.635788\\\\78.2626\\\\304.01\\\\0.6357885\\\\79.19189\\\\304.01\\\\-1.364211\\\\80.27013\\\\304.01\\\\-2.864212\\\\81.25896\\\\304.01\\\\-4.364212\\\\82.39208\\\\304.01\\\\-6.864212\\\\84.67407\\\\304.01\\\\-8.452921\\\\86.41986\\\\304.01\\\\-9.968248\\\\88.41986\\\\304.01\\\\-11.50404\\\\90.91986\\\\304.01\\\\-12.48921\\\\92.91986\\\\304.01\\\\-13.4448\\\\95.41986\\\\304.01\\\\-14.02512\\\\97.41986\\\\304.01\\\\-14.56907\\\\100.4199\\\\304.01\\\\-14.68618\\\\102.9199\\\\304.01\\\\-14.49783\\\\106.4199\\\\304.01\\\\-14.002\\\\108.9199\\\\304.01\\\\-13.05324\\\\111.9199\\\\304.01\\\\-11.97069\\\\114.4199\\\\304.01\\\\-8.813155\\\\119.9199\\\\304.01\\\\-7.750575\\\\122.4199\\\\304.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851068726500001.546012551665\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"248\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"17\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.13579\\\\230.4862\\\\307.01\\\\18.13579\\\\230.1234\\\\307.01\\\\20.63579\\\\229.5843\\\\307.01\\\\23.63579\\\\228.5826\\\\307.01\\\\25.63579\\\\227.6544\\\\307.01\\\\27.63579\\\\226.5734\\\\307.01\\\\29.13579\\\\225.5843\\\\307.01\\\\30.63579\\\\224.4545\\\\307.01\\\\32.13579\\\\223.1497\\\\307.01\\\\34.73231\\\\220.4199\\\\307.01\\\\36.24707\\\\218.4199\\\\307.01\\\\37.78162\\\\215.9199\\\\307.01\\\\38.76729\\\\213.9199\\\\307.01\\\\39.7245\\\\211.4199\\\\307.01\\\\40.30202\\\\209.4199\\\\307.01\\\\40.84336\\\\206.4199\\\\307.01\\\\40.96271\\\\203.9199\\\\307.01\\\\40.77357\\\\200.4199\\\\307.01\\\\40.27764\\\\197.9199\\\\307.01\\\\39.3293\\\\194.9199\\\\307.01\\\\38.71912\\\\193.4199\\\\307.01\\\\37.00643\\\\189.9199\\\\307.01\\\\36.42688\\\\188.4199\\\\307.01\\\\35.98225\\\\186.9199\\\\307.01\\\\35.84065\\\\184.9199\\\\307.01\\\\36.01079\\\\183.4199\\\\307.01\\\\36.51752\\\\181.9199\\\\307.01\\\\37.00217\\\\180.9199\\\\307.01\\\\37.99393\\\\179.4199\\\\307.01\\\\39.13579\\\\178.1658\\\\307.01\\\\40.13579\\\\177.278\\\\307.01\\\\41.63579\\\\176.2862\\\\307.01\\\\42.63579\\\\175.8016\\\\307.01\\\\44.13579\\\\175.2949\\\\307.01\\\\45.63579\\\\175.1247\\\\307.01\\\\47.13579\\\\175.2645\\\\307.01\\\\48.63579\\\\175.7045\\\\307.01\\\\54.13579\\\\178.0715\\\\307.01\\\\57.13579\\\\179.0214\\\\307.01\\\\59.63579\\\\179.5239\\\\307.01\\\\63.13579\\\\179.7308\\\\307.01\\\\65.63579\\\\179.6192\\\\307.01\\\\68.63579\\\\179.0896\\\\307.01\\\\70.63579\\\\178.5263\\\\307.01\\\\72.13579\\\\177.9862\\\\307.01\\\\74.13579\\\\177.1148\\\\307.01\\\\76.13579\\\\176.0637\\\\307.01\\\\79.13579\\\\174.0311\\\\307.01\\\\81.0086\\\\172.4199\\\\307.01\\\\82.9529\\\\170.4199\\\\307.01\\\\84.19914\\\\168.9199\\\\307.01\\\\85.26941\\\\167.4199\\\\307.01\\\\86.75175\\\\164.9199\\\\307.01\\\\87.68684\\\\162.9199\\\\307.01\\\\88.77357\\\\159.9199\\\\307.01\\\\89.27562\\\\157.9199\\\\307.01\\\\89.71638\\\\154.9199\\\\307.01\\\\89.82782\\\\152.9199\\\\307.01\\\\89.68684\\\\150.9199\\\\307.01\\\\89.32175\\\\148.4199\\\\307.01\\\\88.83649\\\\146.4199\\\\307.01\\\\88.18999\\\\144.4199\\\\307.01\\\\87.35246\\\\142.4199\\\\307.01\\\\86.33649\\\\140.4199\\\\307.01\\\\84.75857\\\\137.9199\\\\307.01\\\\83.16704\\\\135.9199\\\\307.01\\\\80.63579\\\\133.3393\\\\307.01\\\\79.13579\\\\132.1011\\\\307.01\\\\76.63579\\\\130.3393\\\\307.01\\\\73.63579\\\\128.7339\\\\307.01\\\\71.13579\\\\127.7278\\\\307.01\\\\69.63579\\\\127.2502\\\\307.01\\\\67.63579\\\\126.7701\\\\307.01\\\\64.63579\\\\126.372\\\\307.01\\\\63.13579\\\\126.2701\\\\307.01\\\\61.63579\\\\126.3565\\\\307.01\\\\58.63579\\\\126.7386\\\\307.01\\\\56.13579\\\\127.3393\\\\307.01\\\\54.63579\\\\127.8233\\\\307.01\\\\52.13579\\\\128.8818\\\\307.01\\\\46.13579\\\\132.1365\\\\307.01\\\\45.13579\\\\132.5696\\\\307.01\\\\43.13579\\\\133.1104\\\\307.01\\\\41.13579\\\\133.1027\\\\307.01\\\\39.13579\\\\132.5188\\\\307.01\\\\38.63579\\\\132.4971\\\\307.01\\\\37.13579\\\\134.4706\\\\307.01\\\\35.63579\\\\135.5464\\\\307.01\\\\34.13579\\\\136.0158\\\\307.01\\\\32.63579\\\\135.5587\\\\307.01\\\\31.13579\\\\134.5006\\\\307.01\\\\30.00924\\\\132.9199\\\\307.01\\\\29.57402\\\\131.4199\\\\307.01\\\\30.02434\\\\129.9199\\\\307.01\\\\31.13579\\\\128.4078\\\\307.01\\\\33.07329\\\\126.9199\\\\307.01\\\\33.03827\\\\126.4199\\\\307.01\\\\32.45295\\\\124.4199\\\\307.01\\\\32.44526\\\\122.4199\\\\307.01\\\\32.98225\\\\120.4199\\\\307.01\\\\34.13579\\\\117.8923\\\\307.01\\\\36.2136\\\\113.9199\\\\307.01\\\\37.24707\\\\111.4199\\\\307.01\\\\37.72183\\\\109.9199\\\\307.01\\\\38.31382\\\\107.4199\\\\307.01\\\\38.67046\\\\104.4199\\\\307.01\\\\38.75405\\\\102.9199\\\\307.01\\\\38.3293\\\\98.91986\\\\307.01\\\\37.75405\\\\96.41986\\\\307.01\\\\36.69613\\\\93.41986\\\\307.01\\\\35.77964\\\\91.41986\\\\307.01\\\\34.31382\\\\88.91986\\\\307.01\\\\33.2715\\\\87.41986\\\\307.01\\\\32.13579\\\\86.02634\\\\307.01\\\\30.13579\\\\83.91791\\\\307.01\\\\27.63579\\\\81.7905\\\\307.01\\\\26.13579\\\\80.74847\\\\307.01\\\\23.63579\\\\79.28835\\\\307.01\\\\20.13579\\\\77.79486\\\\307.01\\\\18.63579\\\\77.32851\\\\307.01\\\\16.13579\\\\76.75017\\\\307.01\\\\12.13579\\\\76.35353\\\\307.01\\\\8.135789\\\\76.71637\\\\307.01\\\\5.635788\\\\77.25362\\\\307.01\\\\2.635788\\\\78.25716\\\\307.01\\\\0.6357885\\\\79.18528\\\\307.01\\\\-1.364211\\\\80.26633\\\\307.01\\\\-2.864212\\\\81.25539\\\\307.01\\\\-4.364212\\\\82.38519\\\\307.01\\\\-5.864212\\\\83.69002\\\\307.01\\\\-8.460731\\\\86.41986\\\\307.01\\\\-9.975492\\\\88.41986\\\\307.01\\\\-11.51005\\\\90.91986\\\\307.01\\\\-12.49571\\\\92.91986\\\\307.01\\\\-13.45292\\\\95.41986\\\\307.01\\\\-14.03044\\\\97.41986\\\\307.01\\\\-14.57178\\\\100.4199\\\\307.01\\\\-14.69113\\\\102.9199\\\\307.01\\\\-14.502\\\\106.4199\\\\307.01\\\\-14.00607\\\\108.9199\\\\307.01\\\\-13.05772\\\\111.9199\\\\307.01\\\\-11.97785\\\\114.4199\\\\307.01\\\\-8.843534\\\\119.9199\\\\307.01\\\\-8.178251\\\\121.4199\\\\307.01\\\\-7.722358\\\\122.9199\\\\307.01\\\\-7.57178\\\\124.4199\\\\307.01\\\\-7.64881\\\\125.4199\\\\307.01\\\\-7.944804\\\\126.4199\\\\307.01\\\\-7.864212\\\\127.2299\\\\307.01\\\\-5.864212\\\\128.2935\\\\307.01\\\\-4.207962\\\\129.9199\\\\307.01\\\\-3.203497\\\\131.9199\\\\307.01\\\\-3.045334\\\\132.9199\\\\307.01\\\\-3.208599\\\\133.9199\\\\307.01\\\\-4.270838\\\\135.9199\\\\307.01\\\\-5.364212\\\\137.0193\\\\307.01\\\\-7.364212\\\\138.0806\\\\307.01\\\\-8.364211\\\\138.2472\\\\307.01\\\\-9.364211\\\\138.0891\\\\307.01\\\\-11.36421\\\\137.0818\\\\307.01\\\\-12.99477\\\\135.4199\\\\307.01\\\\-13.86421\\\\133.7608\\\\307.01\\\\-14.36421\\\\133.5952\\\\307.01\\\\-15.86421\\\\134.0404\\\\307.01\\\\-17.36421\\\\134.2123\\\\307.01\\\\-18.86421\\\\134.0696\\\\307.01\\\\-21.86421\\\\133.1043\\\\307.01\\\\-26.86421\\\\130.7191\\\\307.01\\\\-29.36421\\\\129.78\\\\307.01\\\\-31.36421\\\\129.2402\\\\307.01\\\\-34.36421\\\\128.7263\\\\307.01\\\\-36.86421\\\\128.6261\\\\307.01\\\\-39.86421\\\\128.7971\\\\307.01\\\\-42.36421\\\\129.2701\\\\307.01\\\\-44.36421\\\\129.8393\\\\307.01\\\\-46.86421\\\\130.776\\\\307.01\\\\-48.86421\\\\131.7451\\\\307.01\\\\-51.36421\\\\133.2485\\\\307.01\\\\-52.86421\\\\134.3233\\\\307.01\\\\-54.72322\\\\135.9199\\\\307.01\\\\-56.66919\\\\137.9199\\\\307.01\\\\-57.91208\\\\139.4199\\\\307.01\\\\-58.98921\\\\140.9199\\\\307.01\\\\-59.90558\\\\142.4199\\\\307.01\\\\-60.96825\\\\144.4199\\\\307.01\\\\-62.00807\\\\146.9199\\\\307.01\\\\-62.49783\\\\148.4199\\\\307.01\\\\-62.99993\\\\150.4199\\\\307.01\\\\-63.44202\\\\153.4199\\\\307.01\\\\-63.55772\\\\155.4199\\\\307.01\\\\-63.41841\\\\157.4199\\\\307.01\\\\-63.06064\\\\159.9199\\\\307.01\\\\-62.45292\\\\162.4199\\\\307.01\\\\-61.96073\\\\163.9199\\\\307.01\\\\-60.87757\\\\166.4199\\\\307.01\\\\-59.51585\\\\168.9199\\\\307.01\\\\-58.50807\\\\170.4199\\\\307.01\\\\-56.93054\\\\172.4199\\\\307.01\\\\-55.36421\\\\174.0796\\\\307.01\\\\-53.86421\\\\175.4771\\\\307.01\\\\-51.86421\\\\177.0492\\\\307.01\\\\-49.36421\\\\178.6206\\\\307.01\\\\-47.36421\\\\179.634\\\\307.01\\\\-45.36421\\\\180.4612\\\\307.01\\\\-43.36421\\\\181.1119\\\\307.01\\\\-41.36421\\\\181.5929\\\\307.01\\\\-38.36421\\\\182.0059\\\\307.01\\\\-35.86421\\\\182.0576\\\\307.01\\\\-32.36421\\\\181.6177\\\\307.01\\\\-29.86421\\\\181.0239\\\\307.01\\\\-28.36421\\\\180.5358\\\\307.01\\\\-25.86421\\\\179.4891\\\\307.01\\\\-23.36421\\\\178.2178\\\\307.01\\\\-21.36421\\\\177.3183\\\\307.01\\\\-19.36421\\\\176.722\\\\307.01\\\\-17.86421\\\\176.5826\\\\307.01\\\\-16.36421\\\\176.7435\\\\307.01\\\\-14.86421\\\\177.2485\\\\307.01\\\\-12.86421\\\\178.3477\\\\307.01\\\\-11.86421\\\\179.138\\\\307.01\\\\-10.58232\\\\180.4199\\\\307.01\\\\-9.792064\\\\181.4199\\\\307.01\\\\-8.69282\\\\183.4199\\\\307.01\\\\-8.187816\\\\184.9199\\\\307.01\\\\-8.02691\\\\186.4199\\\\307.01\\\\-8.145048\\\\187.9199\\\\307.01\\\\-8.676711\\\\189.9199\\\\307.01\\\\-9.252931\\\\191.4199\\\\307.01\\\\-9.970693\\\\192.9199\\\\307.01\\\\-10.97549\\\\195.4199\\\\307.01\\\\-11.45025\\\\196.9199\\\\307.01\\\\-12.04224\\\\199.4199\\\\307.01\\\\-12.39888\\\\202.4199\\\\307.01\\\\-12.48248\\\\203.9199\\\\307.01\\\\-12.05772\\\\207.9199\\\\307.01\\\\-11.48475\\\\210.4199\\\\307.01\\\\-10.42456\\\\213.4199\\\\307.01\\\\-9.508066\\\\215.4199\\\\307.01\\\\-8.042242\\\\217.9199\\\\307.01\\\\-6.999926\\\\219.4199\\\\307.01\\\\-4.870026\\\\221.9199\\\\307.01\\\\-2.752931\\\\223.9199\\\\307.01\\\\-1.364211\\\\225.0492\\\\307.01\\\\0.1357885\\\\226.0912\\\\307.01\\\\2.635788\\\\227.5514\\\\307.01\\\\6.135788\\\\229.0449\\\\307.01\\\\7.635788\\\\229.5112\\\\307.01\\\\10.13579\\\\230.0896\\\\307.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851092727900001.507332422180\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"270\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"18\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.13579\\\\230.4862\\\\310.01\\\\18.13579\\\\230.1234\\\\310.01\\\\20.63579\\\\229.5843\\\\310.01\\\\23.63579\\\\228.5826\\\\310.01\\\\25.63579\\\\227.6544\\\\310.01\\\\27.63579\\\\226.5734\\\\310.01\\\\29.13579\\\\225.5843\\\\310.01\\\\30.63579\\\\224.4545\\\\310.01\\\\32.13579\\\\223.1497\\\\310.01\\\\34.73231\\\\220.4199\\\\310.01\\\\36.24707\\\\218.4199\\\\310.01\\\\37.78162\\\\215.9199\\\\310.01\\\\38.76729\\\\213.9199\\\\310.01\\\\39.7245\\\\211.4199\\\\310.01\\\\40.30202\\\\209.4199\\\\310.01\\\\40.84336\\\\206.4199\\\\310.01\\\\40.96271\\\\203.9199\\\\310.01\\\\40.77357\\\\200.4199\\\\310.01\\\\40.27764\\\\197.9199\\\\310.01\\\\39.3293\\\\194.9199\\\\310.01\\\\38.71912\\\\193.4199\\\\310.01\\\\37.00643\\\\189.9199\\\\310.01\\\\36.42688\\\\188.4199\\\\310.01\\\\35.98225\\\\186.9199\\\\310.01\\\\35.83789\\\\184.9199\\\\310.01\\\\35.988\\\\183.4199\\\\310.01\\\\36.26941\\\\182.4199\\\\310.01\\\\36.26491\\\\181.4199\\\\310.01\\\\34.99829\\\\179.9199\\\\310.01\\\\34.38188\\\\178.9199\\\\310.01\\\\33.88579\\\\177.4199\\\\310.01\\\\34.07719\\\\176.4199\\\\310.01\\\\34.63116\\\\175.4199\\\\310.01\\\\35.63579\\\\174.4152\\\\310.01\\\\36.63579\\\\173.8545\\\\310.01\\\\37.63579\\\\173.6662\\\\310.01\\\\39.13579\\\\174.166\\\\310.01\\\\40.13579\\\\174.7764\\\\310.01\\\\41.13579\\\\175.6614\\\\310.01\\\\42.13579\\\\175.9128\\\\310.01\\\\44.13579\\\\175.2927\\\\310.01\\\\45.63579\\\\175.1247\\\\310.01\\\\47.13579\\\\175.2645\\\\310.01\\\\48.63579\\\\175.7045\\\\310.01\\\\54.13579\\\\178.0715\\\\310.01\\\\57.13579\\\\179.0214\\\\310.01\\\\59.63579\\\\179.5239\\\\310.01\\\\63.13579\\\\179.7308\\\\310.01\\\\65.63579\\\\179.6192\\\\310.01\\\\68.63579\\\\179.0896\\\\310.01\\\\70.63579\\\\178.5263\\\\310.01\\\\72.13579\\\\177.9862\\\\310.01\\\\74.13579\\\\177.1148\\\\310.01\\\\76.13579\\\\176.0637\\\\310.01\\\\79.13579\\\\174.0311\\\\310.01\\\\81.0086\\\\172.4199\\\\310.01\\\\82.9529\\\\170.4199\\\\310.01\\\\84.19914\\\\168.9199\\\\310.01\\\\85.26941\\\\167.4199\\\\310.01\\\\86.75175\\\\164.9199\\\\310.01\\\\87.68684\\\\162.9199\\\\310.01\\\\88.77357\\\\159.9199\\\\310.01\\\\89.27562\\\\157.9199\\\\310.01\\\\89.71638\\\\154.9199\\\\310.01\\\\89.82782\\\\152.9199\\\\310.01\\\\89.68684\\\\150.9199\\\\310.01\\\\89.32175\\\\148.4199\\\\310.01\\\\88.83649\\\\146.4199\\\\310.01\\\\88.18999\\\\144.4199\\\\310.01\\\\87.35246\\\\142.4199\\\\310.01\\\\86.33649\\\\140.4199\\\\310.01\\\\84.75857\\\\137.9199\\\\310.01\\\\83.16704\\\\135.9199\\\\310.01\\\\80.63579\\\\133.3393\\\\310.01\\\\79.13579\\\\132.1011\\\\310.01\\\\76.63579\\\\130.3393\\\\310.01\\\\73.63579\\\\128.7339\\\\310.01\\\\71.13579\\\\127.7278\\\\310.01\\\\69.63579\\\\127.2502\\\\310.01\\\\67.63579\\\\126.7701\\\\310.01\\\\64.63579\\\\126.372\\\\310.01\\\\63.13579\\\\126.2701\\\\310.01\\\\61.63579\\\\126.3565\\\\310.01\\\\58.63579\\\\126.7386\\\\310.01\\\\56.13579\\\\127.3393\\\\310.01\\\\54.63579\\\\127.8233\\\\310.01\\\\52.13579\\\\128.8818\\\\310.01\\\\46.13579\\\\132.1365\\\\310.01\\\\45.13579\\\\132.5696\\\\310.01\\\\42.13579\\\\133.3956\\\\310.01\\\\41.55696\\\\133.9199\\\\310.01\\\\40.21622\\\\135.9199\\\\310.01\\\\38.63579\\\\137.4791\\\\310.01\\\\37.13579\\\\138.4632\\\\310.01\\\\35.63579\\\\139.1069\\\\310.01\\\\34.13579\\\\139.3742\\\\310.01\\\\32.13579\\\\139.0896\\\\310.01\\\\30.13579\\\\138.1211\\\\310.01\\\\28.68742\\\\136.9199\\\\310.01\\\\27.43292\\\\135.4199\\\\310.01\\\\26.47601\\\\133.4199\\\\310.01\\\\26.25527\\\\131.4199\\\\310.01\\\\26.50186\\\\129.9199\\\\310.01\\\\27.44195\\\\127.9199\\\\310.01\\\\28.58762\\\\126.4199\\\\310.01\\\\29.65523\\\\125.4199\\\\310.01\\\\31.63579\\\\124.0459\\\\310.01\\\\32.17715\\\\123.4199\\\\310.01\\\\32.98225\\\\120.4199\\\\310.01\\\\34.13579\\\\117.8923\\\\310.01\\\\36.2136\\\\113.9199\\\\310.01\\\\37.24707\\\\111.4199\\\\310.01\\\\37.72183\\\\109.9199\\\\310.01\\\\38.31382\\\\107.4199\\\\310.01\\\\38.67046\\\\104.4199\\\\310.01\\\\38.75405\\\\102.9199\\\\310.01\\\\38.3293\\\\98.91986\\\\310.01\\\\37.75405\\\\96.41986\\\\310.01\\\\36.69613\\\\93.41986\\\\310.01\\\\35.77964\\\\91.41986\\\\310.01\\\\34.31382\\\\88.91986\\\\310.01\\\\33.2715\\\\87.41986\\\\310.01\\\\32.13579\\\\86.02634\\\\310.01\\\\30.13579\\\\83.91791\\\\310.01\\\\27.63579\\\\81.7905\\\\310.01\\\\26.13579\\\\80.74847\\\\310.01\\\\23.63579\\\\79.28835\\\\310.01\\\\20.13579\\\\77.79486\\\\310.01\\\\18.63579\\\\77.32851\\\\310.01\\\\16.13579\\\\76.75017\\\\310.01\\\\12.13579\\\\76.35353\\\\310.01\\\\8.135789\\\\76.71637\\\\310.01\\\\5.635788\\\\77.25362\\\\310.01\\\\2.635788\\\\78.25716\\\\310.01\\\\0.6357885\\\\79.18528\\\\310.01\\\\-1.364211\\\\80.26633\\\\310.01\\\\-2.864212\\\\81.25539\\\\310.01\\\\-4.364212\\\\82.38519\\\\310.01\\\\-5.864212\\\\83.69002\\\\310.01\\\\-8.460731\\\\86.41986\\\\310.01\\\\-9.975492\\\\88.41986\\\\310.01\\\\-11.51005\\\\90.91986\\\\310.01\\\\-12.49571\\\\92.91986\\\\310.01\\\\-13.45292\\\\95.41986\\\\310.01\\\\-14.03044\\\\97.41986\\\\310.01\\\\-14.57178\\\\100.4199\\\\310.01\\\\-14.69113\\\\102.9199\\\\310.01\\\\-14.502\\\\106.4199\\\\310.01\\\\-14.00607\\\\108.9199\\\\310.01\\\\-13.05772\\\\111.9199\\\\310.01\\\\-11.97785\\\\114.4199\\\\310.01\\\\-8.843534\\\\119.9199\\\\310.01\\\\-8.175187\\\\121.4199\\\\310.01\\\\-7.470693\\\\123.9199\\\\310.01\\\\-6.864212\\\\124.6244\\\\310.01\\\\-5.364212\\\\125.3065\\\\310.01\\\\-3.864212\\\\126.3094\\\\310.01\\\\-2.211956\\\\127.9199\\\\310.01\\\\-1.176711\\\\129.4199\\\\310.01\\\\-0.6816457\\\\130.4199\\\\310.01\\\\-0.188405\\\\131.9199\\\\310.01\\\\-0.08336733\\\\132.9199\\\\310.01\\\\-0.247305\\\\134.4199\\\\310.01\\\\-0.8017115\\\\135.9199\\\\310.01\\\\-1.679097\\\\137.4199\\\\310.01\\\\-2.581154\\\\138.4199\\\\310.01\\\\-3.864212\\\\139.5973\\\\310.01\\\\-5.364212\\\\140.4625\\\\310.01\\\\-6.864212\\\\141.0252\\\\310.01\\\\-8.364211\\\\141.2053\\\\310.01\\\\-9.364211\\\\141.101\\\\310.01\\\\-10.86421\\\\140.6143\\\\310.01\\\\-11.86421\\\\140.1195\\\\310.01\\\\-13.36421\\\\139.0945\\\\310.01\\\\-15.00626\\\\137.4199\\\\310.01\\\\-16.86421\\\\134.6526\\\\310.01\\\\-17.36421\\\\134.3886\\\\310.01\\\\-18.86421\\\\134.0912\\\\310.01\\\\-21.86421\\\\133.1043\\\\310.01\\\\-26.86421\\\\130.7191\\\\310.01\\\\-29.36421\\\\129.78\\\\310.01\\\\-31.36421\\\\129.2402\\\\310.01\\\\-34.36421\\\\128.7263\\\\310.01\\\\-36.86421\\\\128.6261\\\\310.01\\\\-39.86421\\\\128.7971\\\\310.01\\\\-42.36421\\\\129.2701\\\\310.01\\\\-44.36421\\\\129.8393\\\\310.01\\\\-46.86421\\\\130.776\\\\310.01\\\\-48.86421\\\\131.7451\\\\310.01\\\\-51.36421\\\\133.2485\\\\310.01\\\\-52.86421\\\\134.3233\\\\310.01\\\\-54.72322\\\\135.9199\\\\310.01\\\\-56.66919\\\\137.9199\\\\310.01\\\\-57.91208\\\\139.4199\\\\310.01\\\\-58.98921\\\\140.9199\\\\310.01\\\\-59.90558\\\\142.4199\\\\310.01\\\\-60.96825\\\\144.4199\\\\310.01\\\\-62.00807\\\\146.9199\\\\310.01\\\\-62.49783\\\\148.4199\\\\310.01\\\\-62.99993\\\\150.4199\\\\310.01\\\\-63.44202\\\\153.4199\\\\310.01\\\\-63.55772\\\\155.4199\\\\310.01\\\\-63.41841\\\\157.4199\\\\310.01\\\\-63.06064\\\\159.9199\\\\310.01\\\\-62.45292\\\\162.4199\\\\310.01\\\\-61.96073\\\\163.9199\\\\310.01\\\\-60.87757\\\\166.4199\\\\310.01\\\\-59.51585\\\\168.9199\\\\310.01\\\\-58.50807\\\\170.4199\\\\310.01\\\\-56.93054\\\\172.4199\\\\310.01\\\\-55.36421\\\\174.0796\\\\310.01\\\\-53.86421\\\\175.4771\\\\310.01\\\\-51.86421\\\\177.0492\\\\310.01\\\\-49.36421\\\\178.6206\\\\310.01\\\\-47.36421\\\\179.634\\\\310.01\\\\-45.36421\\\\180.4612\\\\310.01\\\\-43.36421\\\\181.1119\\\\310.01\\\\-41.36421\\\\181.5929\\\\310.01\\\\-38.36421\\\\182.0059\\\\310.01\\\\-35.86421\\\\182.0576\\\\310.01\\\\-32.36421\\\\181.6177\\\\310.01\\\\-29.86421\\\\181.0239\\\\310.01\\\\-28.36421\\\\180.5358\\\\310.01\\\\-25.86421\\\\179.4891\\\\310.01\\\\-23.36421\\\\178.2178\\\\310.01\\\\-21.36421\\\\177.3183\\\\310.01\\\\-19.36421\\\\176.722\\\\310.01\\\\-17.86421\\\\176.5789\\\\310.01\\\\-15.86421\\\\176.8016\\\\310.01\\\\-14.86421\\\\177.0947\\\\310.01\\\\-13.86421\\\\177.1754\\\\310.01\\\\-13.38972\\\\176.9199\\\\310.01\\\\-12.40512\\\\175.4199\\\\310.01\\\\-10.86421\\\\174.216\\\\310.01\\\\-9.864211\\\\173.802\\\\310.01\\\\-8.864211\\\\173.8091\\\\310.01\\\\-7.864212\\\\174.2572\\\\310.01\\\\-6.864212\\\\175.0914\\\\310.01\\\\-6.193576\\\\175.9199\\\\310.01\\\\-5.746355\\\\176.9199\\\\310.01\\\\-5.739212\\\\177.9199\\\\310.01\\\\-6.144981\\\\178.9199\\\\310.01\\\\-7.364212\\\\180.5032\\\\310.01\\\\-8.680878\\\\181.4199\\\\310.01\\\\-8.937325\\\\181.9199\\\\310.01\\\\-8.250575\\\\184.4199\\\\310.01\\\\-8.025116\\\\186.4199\\\\310.01\\\\-8.145048\\\\187.9199\\\\310.01\\\\-8.676711\\\\189.9199\\\\310.01\\\\-9.252931\\\\191.4199\\\\310.01\\\\-9.970693\\\\192.9199\\\\310.01\\\\-10.97549\\\\195.4199\\\\310.01\\\\-11.45025\\\\196.9199\\\\310.01\\\\-12.04224\\\\199.4199\\\\310.01\\\\-12.39888\\\\202.4199\\\\310.01\\\\-12.48248\\\\203.9199\\\\310.01\\\\-12.05772\\\\207.9199\\\\310.01\\\\-11.48475\\\\210.4199\\\\310.01\\\\-10.42456\\\\213.4199\\\\310.01\\\\-9.508066\\\\215.4199\\\\310.01\\\\-8.042242\\\\217.9199\\\\310.01\\\\-6.999926\\\\219.4199\\\\310.01\\\\-4.870026\\\\221.9199\\\\310.01\\\\-2.752931\\\\223.9199\\\\310.01\\\\-1.364211\\\\225.0492\\\\310.01\\\\0.1357885\\\\226.0912\\\\310.01\\\\2.635788\\\\227.5514\\\\310.01\\\\6.135788\\\\229.0449\\\\310.01\\\\7.635788\\\\229.5112\\\\310.01\\\\10.13579\\\\230.0896\\\\310.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851113729100001.470312254320\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"272\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"19\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"45.13579\\\\174.0365\\\\313.01\\\\49.63579\\\\175.8393\\\\313.01\\\\53.13579\\\\177.5358\\\\313.01\\\\56.13579\\\\178.6134\\\\313.01\\\\57.63579\\\\179.0189\\\\313.01\\\\60.63579\\\\179.5311\\\\313.01\\\\62.13579\\\\179.634\\\\313.01\\\\64.13579\\\\179.6288\\\\313.01\\\\65.63579\\\\179.4949\\\\313.01\\\\68.13579\\\\179.0826\\\\313.01\\\\70.13579\\\\178.5597\\\\313.01\\\\71.63579\\\\178.0576\\\\313.01\\\\74.13579\\\\176.992\\\\313.01\\\\76.63579\\\\175.6117\\\\313.01\\\\77.63579\\\\174.9709\\\\313.01\\\\79.63579\\\\173.4802\\\\313.01\\\\81.13579\\\\172.141\\\\313.01\\\\83.24468\\\\169.9199\\\\313.01\\\\84.78358\\\\167.9199\\\\313.01\\\\86.33118\\\\165.4199\\\\313.01\\\\87.33508\\\\163.4199\\\\313.01\\\\88.32481\\\\160.9199\\\\313.01\\\\88.79119\\\\159.4199\\\\313.01\\\\89.33366\\\\156.9199\\\\313.01\\\\89.63774\\\\153.9199\\\\313.01\\\\89.66704\\\\152.4199\\\\313.01\\\\89.30026\\\\148.9199\\\\313.01\\\\88.71638\\\\146.4199\\\\313.01\\\\88.25405\\\\144.9199\\\\313.01\\\\87.24943\\\\142.4199\\\\313.01\\\\86.76299\\\\141.4199\\\\313.01\\\\85.29488\\\\138.9199\\\\313.01\\\\84.24227\\\\137.4199\\\\313.01\\\\83.13579\\\\136.0748\\\\313.01\\\\81.58791\\\\134.4199\\\\313.01\\\\79.95832\\\\132.9199\\\\313.01\\\\78.63579\\\\131.8477\\\\313.01\\\\77.13579\\\\130.7949\\\\313.01\\\\75.63579\\\\129.9028\\\\313.01\\\\73.63579\\\\128.8657\\\\313.01\\\\71.13579\\\\127.8535\\\\313.01\\\\69.13579\\\\127.2418\\\\313.01\\\\66.63579\\\\126.722\\\\313.01\\\\63.63579\\\\126.4741\\\\313.01\\\\62.63579\\\\126.4677\\\\313.01\\\\59.13579\\\\126.7663\\\\313.01\\\\56.63579\\\\127.3183\\\\313.01\\\\55.13579\\\\127.7721\\\\313.01\\\\52.63579\\\\128.7645\\\\313.01\\\\50.63579\\\\129.776\\\\313.01\\\\48.13579\\\\131.2884\\\\313.01\\\\45.13579\\\\133.2862\\\\313.01\\\\43.81487\\\\134.4199\\\\313.01\\\\42.44948\\\\135.9199\\\\313.01\\\\40.76783\\\\137.9199\\\\313.01\\\\39.63579\\\\138.9596\\\\313.01\\\\38.13579\\\\140.0828\\\\313.01\\\\36.13579\\\\141.1221\\\\313.01\\\\34.63579\\\\141.6103\\\\313.01\\\\33.13579\\\\141.7315\\\\313.01\\\\32.13579\\\\141.6267\\\\313.01\\\\30.63579\\\\141.1365\\\\313.01\\\\28.63579\\\\140.0659\\\\313.01\\\\27.2121\\\\138.9199\\\\313.01\\\\26.13579\\\\137.8833\\\\313.01\\\\24.93796\\\\136.4199\\\\313.01\\\\23.94562\\\\134.4199\\\\313.01\\\\23.68623\\\\132.9199\\\\313.01\\\\23.94523\\\\130.9199\\\\313.01\\\\24.50558\\\\129.4199\\\\313.01\\\\25.96752\\\\126.9199\\\\313.01\\\\27.13579\\\\125.5354\\\\313.01\\\\28.23917\\\\124.4199\\\\313.01\\\\30.50511\\\\122.4199\\\\313.01\\\\32.27399\\\\120.4199\\\\313.01\\\\33.26515\\\\118.9199\\\\313.01\\\\34.76941\\\\116.4199\\\\313.01\\\\35.8293\\\\114.4199\\\\313.01\\\\36.74227\\\\112.4199\\\\313.01\\\\37.74227\\\\109.4199\\\\313.01\\\\38.34469\\\\106.4199\\\\313.01\\\\38.54975\\\\103.4199\\\\313.01\\\\38.35495\\\\99.91986\\\\313.01\\\\37.76941\\\\96.91986\\\\313.01\\\\36.78162\\\\93.91986\\\\313.01\\\\35.36021\\\\90.91986\\\\313.01\\\\33.83302\\\\88.41986\\\\313.01\\\\32.73983\\\\86.91986\\\\313.01\\\\31.13579\\\\85.08799\\\\313.01\\\\29.96766\\\\83.91986\\\\313.01\\\\28.13579\\\\82.3208\\\\313.01\\\\25.13579\\\\80.25896\\\\313.01\\\\22.13579\\\\78.72199\\\\313.01\\\\19.63579\\\\77.75362\\\\313.01\\\\18.13579\\\\77.31338\\\\313.01\\\\15.63579\\\\76.78624\\\\313.01\\\\12.13579\\\\76.54264\\\\313.01\\\\9.135789\\\\76.69946\\\\313.01\\\\6.135788\\\\77.24847\\\\313.01\\\\4.135788\\\\77.83652\\\\313.01\\\\1.635789\\\\78.81096\\\\313.01\\\\0.6357885\\\\79.28624\\\\313.01\\\\-1.864211\\\\80.72861\\\\313.01\\\\-3.364212\\\\81.75896\\\\313.01\\\\-5.864212\\\\83.83652\\\\313.01\\\\-7.433468\\\\85.41986\\\\313.01\\\\-9.477848\\\\87.91986\\\\313.01\\\\-11.07731\\\\90.41986\\\\313.01\\\\-12.56631\\\\93.41986\\\\313.01\\\\-13.49357\\\\95.91986\\\\313.01\\\\-14.02512\\\\97.91986\\\\313.01\\\\-14.45556\\\\100.4199\\\\313.01\\\\-14.60266\\\\102.4199\\\\313.01\\\\-14.50807\\\\105.4199\\\\313.01\\\\-14.00404\\\\108.4199\\\\313.01\\\\-13.45556\\\\110.4199\\\\313.01\\\\-12.93347\\\\111.9199\\\\313.01\\\\-12.06961\\\\113.9199\\\\313.01\\\\-11.03044\\\\115.9199\\\\313.01\\\\-9.447545\\\\118.4199\\\\313.01\\\\-7.028274\\\\121.9199\\\\313.01\\\\-5.864212\\\\123.1258\\\\313.01\\\\-3.364212\\\\124.8657\\\\313.01\\\\-1.605229\\\\126.4199\\\\313.01\\\\-0.2548365\\\\127.9199\\\\313.01\\\\0.7884731\\\\129.4199\\\\313.01\\\\1.803042\\\\131.4199\\\\313.01\\\\2.263741\\\\132.9199\\\\313.01\\\\2.37004\\\\133.9199\\\\313.01\\\\2.269263\\\\134.9199\\\\313.01\\\\1.749628\\\\136.4199\\\\313.01\\\\0.8712052\\\\137.9199\\\\313.01\\\\-0.3263327\\\\139.4199\\\\313.01\\\\-1.364211\\\\140.4421\\\\313.01\\\\-2.864212\\\\141.6427\\\\313.01\\\\-4.364212\\\\142.5469\\\\313.01\\\\-5.864212\\\\143.1195\\\\313.01\\\\-7.364212\\\\143.3308\\\\313.01\\\\-9.364211\\\\143.0816\\\\313.01\\\\-11.86421\\\\142.0038\\\\313.01\\\\-13.86421\\\\140.6465\\\\313.01\\\\-15.36421\\\\139.234\\\\313.01\\\\-16.5498\\\\137.9199\\\\313.01\\\\-17.74925\\\\136.4199\\\\313.01\\\\-18.36421\\\\135.8188\\\\313.01\\\\-19.86421\\\\134.709\\\\313.01\\\\-23.86421\\\\132.3233\\\\313.01\\\\-26.86421\\\\130.8365\\\\313.01\\\\-29.86421\\\\129.7536\\\\313.01\\\\-31.86421\\\\129.2502\\\\313.01\\\\-34.36421\\\\128.8535\\\\313.01\\\\-36.86421\\\\128.6958\\\\313.01\\\\-38.86421\\\\128.7993\\\\313.01\\\\-41.86421\\\\129.278\\\\313.01\\\\-43.86421\\\\129.8016\\\\313.01\\\\-45.36421\\\\130.3039\\\\313.01\\\\-47.86421\\\\131.3785\\\\313.01\\\\-50.36421\\\\132.7435\\\\313.01\\\\-51.86421\\\\133.7382\\\\313.01\\\\-53.36421\\\\134.8785\\\\313.01\\\\-55.58069\\\\136.9199\\\\313.01\\\\-56.96327\\\\138.4199\\\\313.01\\\\-58.502\\\\140.4199\\\\313.01\\\\-60.05171\\\\142.9199\\\\313.01\\\\-61.05772\\\\144.9199\\\\313.01\\\\-62.04862\\\\147.4199\\\\313.01\\\\-62.51394\\\\148.9199\\\\313.01\\\\-63.05919\\\\151.4199\\\\313.01\\\\-63.42757\\\\155.4199\\\\313.01\\\\-63.03729\\\\159.4199\\\\313.01\\\\-62.47549\\\\161.9199\\\\313.01\\\\-61.44202\\\\164.9199\\\\313.01\\\\-60.51775\\\\166.9199\\\\313.01\\\\-59.04568\\\\169.4199\\\\313.01\\\\-57.99572\\\\170.9199\\\\313.01\\\\-56.36421\\\\172.8595\\\\313.01\\\\-55.36421\\\\173.9179\\\\313.01\\\\-53.73056\\\\175.4199\\\\313.01\\\\-52.36421\\\\176.5381\\\\313.01\\\\-50.86421\\\\177.5843\\\\313.01\\\\-48.36421\\\\179.0449\\\\313.01\\\\-47.36421\\\\179.5287\\\\313.01\\\\-44.86421\\\\180.5263\\\\313.01\\\\-42.86421\\\\181.1177\\\\313.01\\\\-40.86421\\\\181.5676\\\\313.01\\\\-37.36421\\\\181.9065\\\\313.01\\\\-36.36421\\\\181.914\\\\313.01\\\\-33.36421\\\\181.6486\\\\313.01\\\\-30.36421\\\\181.0404\\\\313.01\\\\-28.86421\\\\180.5826\\\\313.01\\\\-26.36421\\\\179.5878\\\\313.01\\\\-24.36421\\\\178.5929\\\\313.01\\\\-20.86421\\\\176.7294\\\\313.01\\\\-18.36421\\\\175.6257\\\\313.01\\\\-17.36421\\\\175.0934\\\\313.01\\\\-16.58718\\\\174.4199\\\\313.01\\\\-13.86421\\\\171.7129\\\\313.01\\\\-12.36421\\\\170.7226\\\\313.01\\\\-11.36421\\\\170.2449\\\\313.01\\\\-9.864211\\\\169.8028\\\\313.01\\\\-8.364211\\\\169.7928\\\\313.01\\\\-6.864212\\\\170.2438\\\\313.01\\\\-5.864212\\\\170.7449\\\\313.01\\\\-4.364212\\\\171.9052\\\\313.01\\\\-2.711557\\\\173.9199\\\\313.01\\\\-2.21085\\\\174.9199\\\\313.01\\\\-1.762128\\\\176.4199\\\\313.01\\\\-1.727673\\\\177.9199\\\\313.01\\\\-2.167647\\\\179.4199\\\\313.01\\\\-3.207707\\\\181.4199\\\\313.01\\\\-5.876966\\\\184.4199\\\\313.01\\\\-6.53835\\\\185.4199\\\\313.01\\\\-7.7156\\\\188.4199\\\\313.01\\\\-10.04546\\\\193.4199\\\\313.01\\\\-11.02147\\\\195.9199\\\\313.01\\\\-11.47069\\\\197.4199\\\\313.01\\\\-12.012\\\\199.9199\\\\313.01\\\\-12.28921\\\\203.9199\\\\313.01\\\\-12.08461\\\\206.9199\\\\313.01\\\\-11.49571\\\\209.9199\\\\313.01\\\\-10.51005\\\\212.9199\\\\313.01\\\\-9.600636\\\\214.9199\\\\313.01\\\\-8.53218\\\\216.9199\\\\313.01\\\\-6.468249\\\\219.9199\\\\313.01\\\\-4.864212\\\\221.7517\\\\313.01\\\\-3.695094\\\\222.9199\\\\313.01\\\\-1.864211\\\\224.5189\\\\313.01\\\\1.135789\\\\226.5789\\\\313.01\\\\4.135788\\\\228.1177\\\\313.01\\\\6.635788\\\\229.0843\\\\313.01\\\\8.135789\\\\229.5239\\\\313.01\\\\10.63579\\\\230.0514\\\\313.01\\\\14.13579\\\\230.2971\\\\313.01\\\\17.13579\\\\230.1403\\\\313.01\\\\20.13579\\\\229.5878\\\\313.01\\\\22.13579\\\\229.0004\\\\313.01\\\\24.63579\\\\228.0287\\\\313.01\\\\25.63579\\\\227.5514\\\\313.01\\\\28.13579\\\\226.1111\\\\313.01\\\\29.63579\\\\225.0808\\\\313.01\\\\32.13579\\\\223.0032\\\\313.01\\\\33.70211\\\\221.4199\\\\313.01\\\\35.74942\\\\218.9199\\\\313.01\\\\37.34888\\\\216.4199\\\\313.01\\\\38.83649\\\\213.4199\\\\313.01\\\\39.76298\\\\210.9199\\\\313.01\\\\40.29488\\\\208.9199\\\\313.01\\\\40.7245\\\\206.4199\\\\313.01\\\\40.87313\\\\204.4199\\\\313.01\\\\40.84863\\\\202.4199\\\\313.01\\\\40.70794\\\\200.9199\\\\313.01\\\\40.27562\\\\198.4199\\\\313.01\\\\39.72713\\\\196.4199\\\\313.01\\\\39.20504\\\\194.9199\\\\313.01\\\\38.34256\\\\192.9199\\\\313.01\\\\36.03673\\\\188.4199\\\\313.01\\\\35.18366\\\\186.4199\\\\313.01\\\\34.37849\\\\184.9199\\\\313.01\\\\31.46317\\\\181.4199\\\\313.01\\\\30.60703\\\\179.9199\\\\313.01\\\\29.99819\\\\178.4199\\\\313.01\\\\29.80047\\\\176.9199\\\\313.01\\\\29.96704\\\\175.4199\\\\313.01\\\\30.57026\\\\173.9199\\\\313.01\\\\31.56785\\\\172.4199\\\\313.01\\\\32.63579\\\\171.3258\\\\313.01\\\\34.13579\\\\170.2914\\\\313.01\\\\35.63579\\\\169.7194\\\\313.01\\\\36.63579\\\\169.5831\\\\313.01\\\\38.63579\\\\169.7668\\\\313.01\\\\40.13579\\\\170.3782\\\\313.01\\\\41.63579\\\\171.2379\\\\313.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851133730200001.543478973601\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"277\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"20\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"25.13579\\\\125.7391\\\\316.01\\\\29.49653\\\\121.9199\\\\316.01\\\\31.13579\\\\120.2483\\\\316.01\\\\32.28399\\\\118.9199\\\\316.01\\\\33.71638\\\\116.9199\\\\316.01\\\\35.1454\\\\114.4199\\\\316.01\\\\36.28551\\\\111.9199\\\\316.01\\\\37.23983\\\\108.9199\\\\316.01\\\\37.85425\\\\105.4199\\\\316.01\\\\37.94677\\\\102.9199\\\\316.01\\\\37.75405\\\\99.91986\\\\316.01\\\\37.26729\\\\97.41986\\\\316.01\\\\36.3198\\\\94.41986\\\\316.01\\\\35.22183\\\\91.91986\\\\316.01\\\\33.79257\\\\89.41986\\\\316.01\\\\32.75317\\\\87.91986\\\\316.01\\\\31.13579\\\\85.97396\\\\316.01\\\\30.13579\\\\84.91024\\\\316.01\\\\28.50211\\\\83.41986\\\\316.01\\\\27.13579\\\\82.31029\\\\316.01\\\\25.63579\\\\81.27071\\\\316.01\\\\23.13579\\\\79.8506\\\\316.01\\\\20.63579\\\\78.75362\\\\316.01\\\\17.63579\\\\77.8183\\\\316.01\\\\15.13579\\\\77.34204\\\\316.01\\\\12.13579\\\\77.14066\\\\316.01\\\\9.135789\\\\77.29932\\\\316.01\\\\6.635788\\\\77.75716\\\\316.01\\\\4.635788\\\\78.30858\\\\316.01\\\\2.135788\\\\79.25539\\\\316.01\\\\0.1357885\\\\80.2317\\\\316.01\\\\-2.364212\\\\81.76813\\\\316.01\\\\-4.364212\\\\83.29879\\\\316.01\\\\-6.115665\\\\84.91986\\\\316.01\\\\-7.947545\\\\86.91986\\\\316.01\\\\-9.447545\\\\88.91986\\\\316.01\\\\-10.96327\\\\91.41986\\\\316.01\\\\-11.91527\\\\93.41986\\\\316.01\\\\-13.00807\\\\96.41986\\\\316.01\\\\-13.50807\\\\98.41986\\\\316.01\\\\-13.93921\\\\101.4199\\\\316.01\\\\-14.03218\\\\102.9199\\\\316.01\\\\-13.97549\\\\104.4199\\\\316.01\\\\-13.48699\\\\107.9199\\\\316.01\\\\-12.97549\\\\109.9199\\\\316.01\\\\-12.48018\\\\111.4199\\\\316.01\\\\-11.40558\\\\113.9199\\\\316.01\\\\-10.02949\\\\116.4199\\\\316.01\\\\-9.019734\\\\117.9199\\\\316.01\\\\-7.430538\\\\119.9199\\\\316.01\\\\-5.864212\\\\121.586\\\\316.01\\\\-4.864212\\\\122.5021\\\\316.01\\\\-2.864212\\\\124.1116\\\\316.01\\\\-1.364211\\\\125.2054\\\\316.01\\\\1.135789\\\\127.2049\\\\316.01\\\\2.936209\\\\128.9199\\\\316.01\\\\4.82263\\\\130.9199\\\\316.01\\\\6.264465\\\\132.9199\\\\316.01\\\\6.784093\\\\133.9199\\\\316.01\\\\7.031243\\\\134.9199\\\\316.01\\\\6.765253\\\\135.9199\\\\316.01\\\\5.798487\\\\137.4199\\\\316.01\\\\4.889261\\\\138.4199\\\\316.01\\\\2.635788\\\\140.5555\\\\316.01\\\\1.135789\\\\141.7674\\\\316.01\\\\-1.364211\\\\144.0751\\\\316.01\\\\-3.364212\\\\145.6282\\\\316.01\\\\-5.364212\\\\146.6321\\\\316.01\\\\-6.364212\\\\146.802\\\\316.01\\\\-7.864212\\\\146.5771\\\\316.01\\\\-9.864211\\\\145.615\\\\316.01\\\\-11.36421\\\\144.6044\\\\316.01\\\\-13.36421\\\\142.8379\\\\316.01\\\\-15.36421\\\\140.7688\\\\316.01\\\\-17.86421\\\\137.9523\\\\316.01\\\\-18.86421\\\\136.9179\\\\316.01\\\\-21.36421\\\\134.7479\\\\316.01\\\\-24.36421\\\\132.7573\\\\316.01\\\\-26.36421\\\\131.7247\\\\316.01\\\\-28.86421\\\\130.7191\\\\316.01\\\\-30.36421\\\\130.2485\\\\316.01\\\\-32.36421\\\\129.7721\\\\316.01\\\\-35.86421\\\\129.3365\\\\316.01\\\\-37.86421\\\\129.3449\\\\316.01\\\\-41.36421\\\\129.7971\\\\316.01\\\\-43.36421\\\\130.2884\\\\316.01\\\\-46.36421\\\\131.3565\\\\316.01\\\\-48.36421\\\\132.2884\\\\316.01\\\\-50.86421\\\\133.767\\\\316.01\\\\-52.86421\\\\135.2392\\\\316.01\\\\-54.72205\\\\136.9199\\\\316.01\\\\-57.00942\\\\139.4199\\\\316.01\\\\-58.45816\\\\141.4199\\\\316.01\\\\-59.07794\\\\142.4199\\\\316.01\\\\-60.38489\\\\144.9199\\\\316.01\\\\-61.03985\\\\146.4199\\\\316.01\\\\-62.01394\\\\149.4199\\\\316.01\\\\-62.58369\\\\152.4199\\\\316.01\\\\-62.75058\\\\155.4199\\\\316.01\\\\-62.49993\\\\158.9199\\\\316.01\\\\-61.97785\\\\161.4199\\\\316.01\\\\-61.53218\\\\162.9199\\\\316.01\\\\-60.56345\\\\165.4199\\\\316.01\\\\-59.56343\\\\167.4199\\\\316.01\\\\-57.98846\\\\169.9199\\\\316.01\\\\-56.41841\\\\171.9199\\\\316.01\\\\-54.86421\\\\173.5871\\\\316.01\\\\-53.36421\\\\174.9645\\\\316.01\\\\-51.36421\\\\176.5263\\\\316.01\\\\-48.86421\\\\178.0915\\\\316.01\\\\-46.86421\\\\179.087\\\\316.01\\\\-44.36421\\\\180.0535\\\\316.01\\\\-42.36421\\\\180.6162\\\\316.01\\\\-39.36421\\\\181.1487\\\\316.01\\\\-36.86421\\\\181.28\\\\316.01\\\\-33.36421\\\\181.0535\\\\316.01\\\\-30.86421\\\\180.5426\\\\316.01\\\\-29.36421\\\\180.1004\\\\316.01\\\\-26.86421\\\\179.1441\\\\316.01\\\\-24.86421\\\\178.1536\\\\316.01\\\\-22.36421\\\\176.6336\\\\316.01\\\\-20.86421\\\\175.584\\\\316.01\\\\-18.86421\\\\173.9379\\\\316.01\\\\-15.36421\\\\170.4052\\\\316.01\\\\-13.36421\\\\168.7274\\\\316.01\\\\-11.86421\\\\167.7328\\\\316.01\\\\-9.864211\\\\166.7353\\\\316.01\\\\-8.364211\\\\166.2789\\\\316.01\\\\-7.364212\\\\166.2522\\\\316.01\\\\-5.864212\\\\166.7301\\\\316.01\\\\-4.864212\\\\167.2843\\\\316.01\\\\-3.864212\\\\168.1102\\\\316.01\\\\-2.364212\\\\169.5607\\\\316.01\\\\-0.7082709\\\\171.4199\\\\316.01\\\\0.3049062\\\\172.9199\\\\316.01\\\\0.8081186\\\\173.9199\\\\316.01\\\\1.284373\\\\175.4199\\\\316.01\\\\1.291771\\\\176.9199\\\\316.01\\\\0.8586567\\\\178.4199\\\\316.01\\\\0.1957059\\\\179.9199\\\\316.01\\\\-0.6775677\\\\181.4199\\\\316.01\\\\-2.661191\\\\183.9199\\\\316.01\\\\-5.007315\\\\186.4199\\\\316.01\\\\-6.500715\\\\188.4199\\\\316.01\\\\-8.073847\\\\190.9199\\\\316.01\\\\-8.87757\\\\192.4199\\\\316.01\\\\-10.01585\\\\194.9199\\\\316.01\\\\-10.96825\\\\197.9199\\\\316.01\\\\-11.58267\\\\201.4199\\\\316.01\\\\-11.67519\\\\203.9199\\\\316.01\\\\-11.48248\\\\206.9199\\\\316.01\\\\-10.99571\\\\209.4199\\\\316.01\\\\-10.04661\\\\212.4199\\\\316.01\\\\-8.950251\\\\214.9199\\\\316.01\\\\-7.520991\\\\217.4199\\\\316.01\\\\-6.481589\\\\218.9199\\\\316.01\\\\-4.364212\\\\221.414\\\\316.01\\\\-3.354596\\\\222.4199\\\\316.01\\\\-0.8642115\\\\224.5294\\\\316.01\\\\0.6357885\\\\225.569\\\\316.01\\\\3.135788\\\\226.9891\\\\316.01\\\\5.635788\\\\228.0878\\\\316.01\\\\8.635789\\\\229.0214\\\\316.01\\\\11.13579\\\\229.4977\\\\316.01\\\\14.13579\\\\229.6991\\\\316.01\\\\17.13579\\\\229.5404\\\\316.01\\\\19.63579\\\\229.0808\\\\316.01\\\\21.63579\\\\228.5287\\\\316.01\\\\24.13579\\\\227.5826\\\\316.01\\\\26.13579\\\\226.6064\\\\316.01\\\\28.63579\\\\225.0716\\\\316.01\\\\30.63579\\\\223.5409\\\\316.01\\\\32.38724\\\\221.9199\\\\316.01\\\\34.21912\\\\219.9199\\\\316.01\\\\35.71912\\\\217.9199\\\\316.01\\\\37.23231\\\\215.4199\\\\316.01\\\\37.74227\\\\214.4199\\\\316.01\\\\38.78743\\\\211.9199\\\\316.01\\\\39.27964\\\\210.4199\\\\316.01\\\\39.77964\\\\208.4199\\\\316.01\\\\40.21079\\\\205.4199\\\\316.01\\\\40.30376\\\\203.9199\\\\316.01\\\\40.24707\\\\202.4199\\\\316.01\\\\39.75857\\\\198.9199\\\\316.01\\\\39.24707\\\\196.9199\\\\316.01\\\\38.75175\\\\195.4199\\\\316.01\\\\37.67715\\\\192.9199\\\\316.01\\\\36.30844\\\\190.4199\\\\316.01\\\\35.32992\\\\188.9199\\\\316.01\\\\33.83548\\\\186.9199\\\\316.01\\\\30.59809\\\\183.4199\\\\316.01\\\\28.97444\\\\181.4199\\\\316.01\\\\27.49209\\\\178.9199\\\\316.01\\\\26.92882\\\\177.4199\\\\316.01\\\\26.67292\\\\175.9199\\\\316.01\\\\27.0272\\\\173.9199\\\\316.01\\\\27.47647\\\\172.9199\\\\316.01\\\\28.42746\\\\171.4199\\\\316.01\\\\30.0573\\\\169.4199\\\\316.01\\\\31.13579\\\\168.3365\\\\316.01\\\\32.63579\\\\167.1923\\\\316.01\\\\34.63579\\\\166.3017\\\\316.01\\\\36.13579\\\\166.2524\\\\316.01\\\\38.13579\\\\166.8033\\\\316.01\\\\40.13579\\\\167.7816\\\\316.01\\\\41.63579\\\\168.748\\\\316.01\\\\43.63579\\\\170.3422\\\\316.01\\\\46.63579\\\\172.9895\\\\316.01\\\\48.13579\\\\174.0761\\\\316.01\\\\50.63579\\\\175.6084\\\\316.01\\\\52.63579\\\\176.6375\\\\316.01\\\\55.13579\\\\177.647\\\\316.01\\\\56.63579\\\\178.1235\\\\316.01\\\\58.63579\\\\178.6027\\\\316.01\\\\61.13579\\\\178.9332\\\\316.01\\\\63.13579\\\\179.0715\\\\316.01\\\\64.63579\\\\178.9771\\\\316.01\\\\67.63579\\\\178.5617\\\\316.01\\\\69.63579\\\\178.0715\\\\316.01\\\\72.63579\\\\177.0086\\\\316.01\\\\74.63579\\\\176.0676\\\\316.01\\\\77.13579\\\\174.5898\\\\316.01\\\\78.63579\\\\173.5138\\\\316.01\\\\81.01079\\\\171.4199\\\\316.01\\\\82.63579\\\\169.6953\\\\316.01\\\\83.67384\\\\168.4199\\\\316.01\\\\85.35615\\\\165.9199\\\\316.01\\\\86.67046\\\\163.4199\\\\316.01\\\\87.31818\\\\161.9199\\\\316.01\\\\88.28932\\\\158.9199\\\\316.01\\\\88.85658\\\\155.9199\\\\316.01\\\\89.01982\\\\152.9199\\\\316.01\\\\88.76299\\\\149.4199\\\\316.01\\\\88.34512\\\\147.4199\\\\316.01\\\\87.78162\\\\145.4199\\\\316.01\\\\87.23485\\\\143.9199\\\\316.01\\\\85.80917\\\\140.9199\\\\316.01\\\\84.23231\\\\138.4199\\\\316.01\\\\82.65283\\\\136.4199\\\\316.01\\\\81.26814\\\\134.9199\\\\316.01\\\\79.62587\\\\133.4199\\\\316.01\\\\78.13579\\\\132.2224\\\\316.01\\\\75.13579\\\\130.2905\\\\316.01\\\\73.13579\\\\129.2905\\\\316.01\\\\70.63579\\\\128.3183\\\\316.01\\\\68.63579\\\\127.7468\\\\316.01\\\\65.63579\\\\127.2051\\\\316.01\\\\63.13579\\\\127.0843\\\\316.01\\\\59.63579\\\\127.3134\\\\316.01\\\\57.13579\\\\127.8183\\\\316.01\\\\55.63579\\\\128.2519\\\\316.01\\\\53.13579\\\\129.2097\\\\316.01\\\\51.13579\\\\130.2003\\\\316.01\\\\48.63579\\\\131.7489\\\\316.01\\\\47.13579\\\\132.8785\\\\316.01\\\\45.13579\\\\134.6568\\\\316.01\\\\43.91469\\\\135.9199\\\\316.01\\\\42.24456\\\\137.9199\\\\316.01\\\\40.13579\\\\140.2911\\\\316.01\\\\39.13579\\\\141.291\\\\316.01\\\\37.13579\\\\143.0427\\\\316.01\\\\35.63579\\\\144.0175\\\\316.01\\\\34.63579\\\\144.5213\\\\316.01\\\\33.13579\\\\144.9878\\\\316.01\\\\32.13579\\\\145.0095\\\\316.01\\\\30.63579\\\\144.5631\\\\316.01\\\\29.63579\\\\144.0704\\\\316.01\\\\28.13579\\\\143.0939\\\\316.01\\\\25.13579\\\\140.7603\\\\316.01\\\\24.13579\\\\140.097\\\\316.01\\\\22.13579\\\\138.5449\\\\316.01\\\\20.45722\\\\136.9199\\\\316.01\\\\19.40894\\\\135.4199\\\\316.01\\\\19.02304\\\\134.4199\\\\316.01\\\\19.41152\\\\132.9199\\\\316.01\\\\20.52696\\\\130.9199\\\\316.01\\\\22.4984\\\\128.4199\\\\316.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851156731500001.532752123275\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"62\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"21\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-12.89888\\\\102.9199\\\\319.01\\\\-12.55271\\\\106.9199\\\\319.01\\\\-11.97069\\\\109.4199\\\\319.01\\\\-11.4907\\\\110.9199\\\\319.01\\\\-10.43921\\\\113.4199\\\\319.01\\\\-9.063915\\\\115.9199\\\\319.01\\\\-8.047024\\\\117.4199\\\\319.01\\\\-6.419136\\\\119.4199\\\\319.01\\\\-4.864212\\\\121.0275\\\\319.01\\\\-2.364212\\\\123.1128\\\\319.01\\\\-0.8642115\\\\124.1393\\\\319.01\\\\1.635789\\\\125.6274\\\\319.01\\\\6.135788\\\\128.0177\\\\319.01\\\\8.635789\\\\129.0637\\\\319.01\\\\10.63579\\\\129.5962\\\\319.01\\\\12.63579\\\\129.8757\\\\319.01\\\\14.63579\\\\129.5984\\\\319.01\\\\16.13579\\\\129.1345\\\\319.01\\\\18.63579\\\\127.9669\\\\319.01\\\\21.13579\\\\126.5592\\\\319.01\\\\25.13579\\\\123.9956\\\\319.01\\\\27.13579\\\\122.544\\\\319.01\\\\28.96026\\\\120.9199\\\\319.01\\\\30.87045\\\\118.9199\\\\319.01\\\\32.74672\\\\116.4199\\\\319.01\\\\34.18366\\\\113.9199\\\\319.01\\\\35.30482\\\\111.4199\\\\319.01\\\\35.81496\\\\109.9199\\\\319.01\\\\36.33659\\\\107.9199\\\\319.01\\\\36.74227\\\\105.4199\\\\319.01\\\\36.87132\\\\102.9199\\\\319.01\\\\36.75857\\\\100.9199\\\\319.01\\\\36.24469\\\\97.91986\\\\319.01\\\\35.34095\\\\94.91986\\\\319.01\\\\34.72183\\\\93.41986\\\\319.01\\\\33.72296\\\\91.41986\\\\319.01\\\\32.8289\\\\89.91986\\\\319.01\\\\31.77888\\\\88.41986\\\\319.01\\\\30.13579\\\\86.49043\\\\319.01\\\\29.10066\\\\85.41986\\\\319.01\\\\26.63579\\\\83.28575\\\\319.01\\\\25.13579\\\\82.23956\\\\319.01\\\\22.63579\\\\80.81954\\\\319.01\\\\20.13579\\\\79.73236\\\\319.01\\\\18.63579\\\\79.23167\\\\319.01\\\\16.63579\\\\78.72334\\\\319.01\\\\14.13579\\\\78.33926\\\\319.01\\\\12.13579\\\\78.20802\\\\319.01\\\\10.13579\\\\78.30858\\\\319.01\\\\7.135788\\\\78.77843\\\\319.01\\\\5.135788\\\\79.30858\\\\319.01\\\\2.635788\\\\80.22572\\\\319.01\\\\0.6357885\\\\81.20093\\\\319.01\\\\-1.864211\\\\82.73734\\\\319.01\\\\-3.864212\\\\84.28801\\\\319.01\\\\-5.595103\\\\85.91986\\\\319.01\\\\-6.961729\\\\87.41986\\\\319.01\\\\-8.481761\\\\89.41986\\\\319.01\\\\-9.996199\\\\91.91986\\\\319.01\\\\-10.94203\\\\93.91986\\\\319.01\\\\-12.00149\\\\96.91986\\\\319.01\\\\-12.56767\\\\99.41986\\\\319.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851179732800001.515857303411\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"61\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"22\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-7.780878\\\\157.9199\\\\319.01\\\\-7.557069\\\\155.9199\\\\319.01\\\\-7.641239\\\\155.4199\\\\319.01\\\\-8.680249\\\\152.9199\\\\319.01\\\\-10.14019\\\\150.4199\\\\319.01\\\\-11.96758\\\\147.9199\\\\319.01\\\\-14.06594\\\\144.9199\\\\319.01\\\\-15.72495\\\\142.4199\\\\319.01\\\\-17.18427\\\\140.4199\\\\319.01\\\\-19.36421\\\\137.9739\\\\319.01\\\\-21.86421\\\\135.7308\\\\319.01\\\\-24.86421\\\\133.7219\\\\319.01\\\\-26.86421\\\\132.7002\\\\319.01\\\\-29.36421\\\\131.7077\\\\319.01\\\\-30.86421\\\\131.2518\\\\319.01\\\\-33.36421\\\\130.7201\\\\319.01\\\\-36.86421\\\\130.4545\\\\319.01\\\\-40.36421\\\\130.7374\\\\319.01\\\\-42.86421\\\\131.2971\\\\319.01\\\\-44.36421\\\\131.7584\\\\319.01\\\\-46.86421\\\\132.7666\\\\319.01\\\\-48.86421\\\\133.8061\\\\319.01\\\\-50.36421\\\\134.7354\\\\319.01\\\\-52.36421\\\\136.2252\\\\319.01\\\\-54.36421\\\\138.0987\\\\319.01\\\\-55.60049\\\\139.4199\\\\319.01\\\\-57.48921\\\\141.9199\\\\319.01\\\\-58.94202\\\\144.4199\\\\319.01\\\\-60.06339\\\\146.9199\\\\319.01\\\\-60.99572\\\\149.9199\\\\319.01\\\\-61.47311\\\\152.4199\\\\319.01\\\\-61.65237\\\\155.4199\\\\319.01\\\\-61.57079\\\\157.4199\\\\319.01\\\\-61.0741\\\\160.4199\\\\319.01\\\\-60.5406\\\\162.4199\\\\319.01\\\\-59.58504\\\\164.9199\\\\319.01\\\\-58.59007\\\\166.9199\\\\319.01\\\\-57.01646\\\\169.4199\\\\319.01\\\\-55.41872\\\\171.4199\\\\319.01\\\\-53.86421\\\\173.0574\\\\319.01\\\\-52.86421\\\\173.9672\\\\319.01\\\\-50.86421\\\\175.5555\\\\319.01\\\\-48.36421\\\\177.127\\\\319.01\\\\-46.36421\\\\178.1151\\\\319.01\\\\-43.86421\\\\179.0579\\\\319.01\\\\-41.86421\\\\179.606\\\\319.01\\\\-38.86421\\\\180.1074\\\\319.01\\\\-36.86421\\\\180.191\\\\319.01\\\\-34.86421\\\\180.1185\\\\319.01\\\\-31.36421\\\\179.5214\\\\319.01\\\\-28.36421\\\\178.5899\\\\319.01\\\\-25.86421\\\\177.4476\\\\319.01\\\\-24.36421\\\\176.6231\\\\319.01\\\\-22.86421\\\\175.6373\\\\319.01\\\\-21.36421\\\\174.4941\\\\319.01\\\\-19.86421\\\\173.1573\\\\319.01\\\\-17.74542\\\\170.9199\\\\319.01\\\\-15.68287\\\\168.4199\\\\319.01\\\\-14.53537\\\\166.9199\\\\319.01\\\\-11.19653\\\\162.9199\\\\319.01\\\\-9.668378\\\\160.9199\\\\319.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.3.46.670589.33.1.63686699851194733700001.468081152243\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"156\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"23\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"33.13579\\\\158.2413\\\\319.01\\\\35.63579\\\\160.3188\\\\319.01\\\\37.63579\\\\162.1973\\\\319.01\\\\41.63579\\\\166.1942\\\\319.01\\\\45.15037\\\\169.9199\\\\319.01\\\\46.17537\\\\170.9199\\\\319.01\\\\48.13579\\\\172.6269\\\\319.01\\\\50.13579\\\\174.0326\\\\319.01\\\\53.13579\\\\175.6531\\\\319.01\\\\55.13579\\\\176.4862\\\\319.01\\\\57.13579\\\\177.1228\\\\319.01\\\\59.13579\\\\177.5749\\\\319.01\\\\63.13579\\\\177.9218\\\\319.01\\\\66.63579\\\\177.6196\\\\319.01\\\\69.13579\\\\177.067\\\\319.01\\\\72.13579\\\\176.0294\\\\319.01\\\\74.13579\\\\175.1044\\\\319.01\\\\76.63579\\\\173.6219\\\\319.01\\\\78.13579\\\\172.5346\\\\319.01\\\\79.96026\\\\170.9199\\\\319.01\\\\82.30837\\\\168.4199\\\\319.01\\\\83.772\\\\166.4199\\\\319.01\\\\84.67715\\\\164.9199\\\\319.01\\\\85.70212\\\\162.9199\\\\319.01\\\\86.34207\\\\161.4199\\\\319.01\\\\87.27515\\\\158.4199\\\\319.01\\\\87.74707\\\\155.9199\\\\319.01\\\\87.92132\\\\152.9199\\\\319.01\\\\87.83974\\\\150.9199\\\\319.01\\\\87.33659\\\\147.9199\\\\319.01\\\\86.78665\\\\145.9199\\\\319.01\\\\85.83718\\\\143.4199\\\\319.01\\\\84.84293\\\\141.4199\\\\319.01\\\\83.26244\\\\138.9199\\\\319.01\\\\81.65337\\\\136.9199\\\\319.01\\\\80.13579\\\\135.3188\\\\319.01\\\\77.63579\\\\133.1981\\\\319.01\\\\74.63579\\\\131.2557\\\\319.01\\\\72.63579\\\\130.2625\\\\319.01\\\\70.13579\\\\129.3158\\\\319.01\\\\68.13579\\\\128.7592\\\\319.01\\\\65.63579\\\\128.3183\\\\319.01\\\\63.13579\\\\128.1659\\\\319.01\\\\60.63579\\\\128.3016\\\\319.01\\\\58.13579\\\\128.7191\\\\319.01\\\\56.13579\\\\129.2453\\\\319.01\\\\53.63579\\\\130.192\\\\319.01\\\\50.63579\\\\131.7338\\\\319.01\\\\49.13579\\\\132.7196\\\\319.01\\\\47.13579\\\\134.2836\\\\319.01\\\\45.41303\\\\135.9199\\\\319.01\\\\44.05789\\\\137.4199\\\\319.01\\\\42.53205\\\\139.4199\\\\319.01\\\\40.22859\\\\142.9199\\\\319.01\\\\37.76569\\\\146.4199\\\\319.01\\\\34.63579\\\\150.5317\\\\319.01\\\\33.63579\\\\151.6949\\\\319.01\\\\32.63579\\\\152.5969\\\\319.01\\\\32.13579\\\\152.8053\\\\319.01\\\\31.63579\\\\152.5761\\\\319.01\\\\30.63579\\\\151.7115\\\\319.01\\\\29.63579\\\\150.6442\\\\319.01\\\\27.79562\\\\148.4199\\\\319.01\\\\25.45295\\\\145.9199\\\\319.01\\\\24.13579\\\\144.7099\\\\319.01\\\\22.13579\\\\143.3042\\\\319.01\\\\20.13579\\\\142.2827\\\\319.01\\\\17.13579\\\\141.2436\\\\319.01\\\\14.13579\\\\140.6942\\\\319.01\\\\12.13579\\\\140.8091\\\\319.01\\\\10.13579\\\\141.2746\\\\319.01\\\\8.635789\\\\141.7564\\\\319.01\\\\6.635788\\\\142.6667\\\\319.01\\\\4.635788\\\\143.8113\\\\319.01\\\\2.690873\\\\145.4199\\\\319.01\\\\1.635789\\\\146.4907\\\\319.01\\\\0.0003718426\\\\148.4199\\\\319.01\\\\-1.019515\\\\149.9199\\\\319.01\\\\-2.529212\\\\152.4199\\\\319.01\\\\-3.476054\\\\154.4199\\\\319.01\\\\-4.022545\\\\155.9199\\\\319.01\\\\-3.918899\\\\158.4199\\\\319.01\\\\-3.595462\\\\159.4199\\\\319.01\\\\-1.940824\\\\162.9199\\\\319.01\\\\-0.5122378\\\\165.4199\\\\319.01\\\\0.4943411\\\\166.9199\\\\319.01\\\\2.635788\\\\169.3352\\\\319.01\\\\5.151413\\\\171.9199\\\\319.01\\\\6.24749\\\\173.4199\\\\319.01\\\\6.578497\\\\174.4199\\\\319.01\\\\6.298288\\\\175.9199\\\\319.01\\\\5.779171\\\\176.9199\\\\319.01\\\\4.722327\\\\178.4199\\\\319.01\\\\3.883184\\\\179.4199\\\\319.01\\\\1.506402\\\\181.9199\\\\319.01\\\\-2.864212\\\\186.0801\\\\319.01\\\\-5.025976\\\\188.4199\\\\319.01\\\\-6.477698\\\\190.4199\\\\319.01\\\\-7.915268\\\\192.9199\\\\319.01\\\\-9.033246\\\\195.4199\\\\319.01\\\\-9.545151\\\\196.9199\\\\319.01\\\\-10.06501\\\\198.9199\\\\319.01\\\\-10.47069\\\\201.4199\\\\319.01\\\\-10.59974\\\\203.9199\\\\319.01\\\\-10.48699\\\\205.9199\\\\319.01\\\\-9.975492\\\\208.9199\\\\319.01\\\\-9.069374\\\\211.9199\\\\319.01\\\\-8.450251\\\\213.4199\\\\319.01\\\\-7.448093\\\\215.4199\\\\319.01\\\\-6.554331\\\\216.9199\\\\319.01\\\\-5.50494\\\\218.4199\\\\319.01\\\\-3.364212\\\\220.8809\\\\319.01\\\\-2.289615\\\\221.9199\\\\319.01\\\\-0.3642115\\\\223.5549\\\\319.01\\\\1.135789\\\\224.6032\\\\319.01\\\\2.635788\\\\225.492\\\\319.01\\\\4.635788\\\\226.4862\\\\319.01\\\\7.635788\\\\227.608\\\\319.01\\\\9.635789\\\\228.1153\\\\319.01\\\\12.13579\\\\228.5004\\\\319.01\\\\14.13579\\\\228.6302\\\\319.01\\\\16.13579\\\\228.5311\\\\319.01\\\\19.13579\\\\228.0613\\\\319.01\\\\21.13579\\\\227.5287\\\\319.01\\\\23.63579\\\\226.6123\\\\319.01\\\\25.63579\\\\225.6388\\\\319.01\\\\28.13579\\\\224.1024\\\\319.01\\\\30.13579\\\\222.5501\\\\319.01\\\\31.86496\\\\220.9199\\\\319.01\\\\33.23043\\\\219.4199\\\\319.01\\\\34.75003\\\\217.4199\\\\319.01\\\\36.26778\\\\214.9199\\\\319.01\\\\37.2136\\\\212.9199\\\\319.01\\\\38.27096\\\\209.9199\\\\319.01\\\\38.83924\\\\207.4199\\\\319.01\\\\39.17046\\\\203.9199\\\\319.01\\\\38.82429\\\\199.9199\\\\319.01\\\\38.24227\\\\197.4199\\\\319.01\\\\37.76228\\\\195.9199\\\\319.01\\\\36.7136\\\\193.4199\\\\319.01\\\\35.33726\\\\190.9199\\\\319.01\\\\34.32057\\\\189.4199\\\\319.01\\\\32.69406\\\\187.4199\\\\319.01\\\\30.13579\\\\184.8861\\\\319.01\\\\28.13579\\\\183.1373\\\\319.01\\\\26.33371\\\\181.4199\\\\319.01\\\\23.47009\\\\178.4199\\\\319.01\\\\22.04972\\\\176.4199\\\\319.01\\\\21.53857\\\\175.4199\\\\319.01\\\\21.30416\\\\174.4199\\\\319.01\\\\21.59775\\\\173.4199\\\\319.01\\\\22.58987\\\\171.9199\\\\319.01\\\\26.76079\\\\167.4199\\\\319.01\\\\28.76607\\\\164.4199\\\\319.01\\\\30.52865\\\\161.4199\\\\319.01\\\\32.13579\\\\159.0449\\\\319.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n";
const char* k_rtStruct_json08 =
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"61\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"24\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-22.86421\\\\173.6167\\\\322.01\\\\-20.36421\\\\171.3328\\\\322.01\\\\-18.64568\\\\169.4199\\\\322.01\\\\-17.21133\\\\167.4199\\\\322.01\\\\-15.76229\\\\164.9199\\\\322.01\\\\-14.17866\\\\161.4199\\\\322.01\\\\-13.68593\\\\159.9199\\\\322.01\\\\-13.19968\\\\157.9199\\\\322.01\\\\-13.03158\\\\155.4199\\\\322.01\\\\-13.19686\\\\153.4199\\\\322.01\\\\-13.74929\\\\150.9199\\\\322.01\\\\-14.23635\\\\149.4199\\\\322.01\\\\-15.71317\\\\145.9199\\\\322.01\\\\-16.80026\\\\143.9199\\\\322.01\\\\-17.74579\\\\142.4199\\\\322.01\\\\-19.29129\\\\140.4199\\\\322.01\\\\-20.86421\\\\138.7435\\\\322.01\\\\-21.86421\\\\137.8386\\\\322.01\\\\-23.86421\\\\136.282\\\\322.01\\\\-26.36421\\\\134.764\\\\322.01\\\\-28.36421\\\\133.8425\\\\322.01\\\\-29.86421\\\\133.2735\\\\322.01\\\\-31.86421\\\\132.7164\\\\322.01\\\\-34.36421\\\\132.2486\\\\322.01\\\\-36.86421\\\\132.13\\\\322.01\\\\-39.36421\\\\132.2642\\\\322.01\\\\-41.86421\\\\132.7382\\\\322.01\\\\-43.86421\\\\133.3401\\\\322.01\\\\-46.36421\\\\134.3583\\\\322.01\\\\-48.86421\\\\135.7135\\\\322.01\\\\-50.36421\\\\136.728\\\\322.01\\\\-52.36421\\\\138.4039\\\\322.01\\\\-53.86421\\\\139.9274\\\\322.01\\\\-55.50681\\\\141.9199\\\\322.01\\\\-56.51492\\\\143.4199\\\\322.01\\\\-58.07415\\\\146.4199\\\\322.01\\\\-59.00964\\\\148.9199\\\\322.01\\\\-59.5293\\\\150.9199\\\\322.01\\\\-59.89546\\\\153.4199\\\\322.01\\\\-60.03242\\\\154.9199\\\\322.01\\\\-59.93835\\\\156.9199\\\\322.01\\\\-59.49086\\\\159.9199\\\\322.01\\\\-58.95904\\\\161.9199\\\\322.01\\\\-58.02025\\\\164.4199\\\\322.01\\\\-57.0033\\\\166.4199\\\\322.01\\\\-56.06999\\\\167.9199\\\\322.01\\\\-54.56957\\\\169.9199\\\\322.01\\\\-52.65075\\\\171.9199\\\\322.01\\\\-51.36421\\\\173.1195\\\\322.01\\\\-49.36421\\\\174.6134\\\\322.01\\\\-47.86421\\\\175.5312\\\\322.01\\\\-45.86421\\\\176.5396\\\\322.01\\\\-42.86421\\\\177.6258\\\\322.01\\\\-40.86421\\\\178.1171\\\\322.01\\\\-38.36421\\\\178.4579\\\\322.01\\\\-36.86421\\\\178.5724\\\\322.01\\\\-35.36421\\\\178.4741\\\\322.01\\\\-32.36421\\\\178.0522\\\\322.01\\\\-30.36421\\\\177.5304\\\\322.01\\\\-27.86421\\\\176.6086\\\\322.01\\\\-25.86421\\\\175.6103\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"57\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"25\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"12.13579\\\\79.89561\\\\322.01\\\\8.135789\\\\80.26321\\\\322.01\\\\6.135788\\\\80.73197\\\\322.01\\\\3.135788\\\\81.80273\\\\322.01\\\\1.135789\\\\82.78688\\\\322.01\\\\-0.3642115\\\\83.70047\\\\322.01\\\\-1.864211\\\\84.7538\\\\322.01\\\\-4.364212\\\\87.02203\\\\322.01\\\\-6.084282\\\\88.91986\\\\322.01\\\\-7.526799\\\\90.91986\\\\322.01\\\\-8.904288\\\\93.41986\\\\322.01\\\\-9.95048\\\\95.91986\\\\322.01\\\\-10.55831\\\\97.91986\\\\322.01\\\\-11.04673\\\\100.4199\\\\322.01\\\\-11.16636\\\\102.4199\\\\322.01\\\\-11.03347\\\\105.9199\\\\322.01\\\\-10.53559\\\\108.4199\\\\322.01\\\\-9.545301\\\\111.4199\\\\322.01\\\\-8.064543\\\\114.4199\\\\322.01\\\\-5.994056\\\\117.4199\\\\322.01\\\\-4.364212\\\\119.1962\\\\322.01\\\\-2.364212\\\\121.063\\\\322.01\\\\-0.3642115\\\\122.5134\\\\322.01\\\\1.635789\\\\123.6633\\\\322.01\\\\3.635788\\\\124.6195\\\\322.01\\\\6.135788\\\\125.5152\\\\322.01\\\\8.135789\\\\126.0422\\\\322.01\\\\11.13579\\\\126.4622\\\\322.01\\\\13.13579\\\\126.4859\\\\322.01\\\\16.13579\\\\126.0393\\\\322.01\\\\19.13579\\\\125.1215\\\\322.01\\\\20.63579\\\\124.5159\\\\322.01\\\\22.63579\\\\123.5519\\\\322.01\\\\25.63579\\\\121.6349\\\\322.01\\\\28.13579\\\\119.4801\\\\322.01\\\\29.13579\\\\118.4173\\\\322.01\\\\30.76925\\\\116.4199\\\\322.01\\\\32.34138\\\\113.9199\\\\322.01\\\\33.32649\\\\111.9199\\\\322.01\\\\34.23682\\\\109.4199\\\\322.01\\\\34.83486\\\\106.9199\\\\322.01\\\\35.12617\\\\104.4199\\\\322.01\\\\35.21572\\\\102.9199\\\\322.01\\\\34.76814\\\\98.91986\\\\322.01\\\\34.2723\\\\96.91986\\\\322.01\\\\33.77169\\\\95.41986\\\\322.01\\\\32.64571\\\\92.91986\\\\322.01\\\\31.84079\\\\91.41986\\\\322.01\\\\30.85131\\\\89.91986\\\\322.01\\\\28.81709\\\\87.41986\\\\322.01\\\\27.13579\\\\85.78797\\\\322.01\\\\25.13579\\\\84.21124\\\\322.01\\\\23.63579\\\\83.22486\\\\322.01\\\\21.63579\\\\82.17953\\\\322.01\\\\19.63579\\\\81.30779\\\\322.01\\\\18.13579\\\\80.81271\\\\322.01\\\\15.63579\\\\80.22696\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"30\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"26\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.13579\\\\148.5344\\\\322.01\\\\11.63579\\\\148.8526\\\\322.01\\\\10.13579\\\\149.3299\\\\322.01\\\\9.135789\\\\149.8438\\\\322.01\\\\7.635788\\\\151.0171\\\\322.01\\\\6.442606\\\\152.4199\\\\322.01\\\\5.485788\\\\154.4199\\\\322.01\\\\5.087712\\\\156.4199\\\\322.01\\\\5.087712\\\\158.4199\\\\322.01\\\\5.505788\\\\160.4199\\\\322.01\\\\6.510788\\\\162.4199\\\\322.01\\\\7.333157\\\\163.4199\\\\322.01\\\\8.635789\\\\164.5903\\\\322.01\\\\10.63579\\\\165.6099\\\\322.01\\\\12.13579\\\\165.9872\\\\322.01\\\\13.63579\\\\166.1299\\\\322.01\\\\15.63579\\\\165.9679\\\\322.01\\\\17.13579\\\\165.5553\\\\322.01\\\\19.13579\\\\164.4994\\\\322.01\\\\20.74293\\\\162.9199\\\\322.01\\\\21.68787\\\\161.4199\\\\322.01\\\\22.26579\\\\159.9199\\\\322.01\\\\22.5597\\\\158.4199\\\\322.01\\\\22.58144\\\\156.9199\\\\322.01\\\\22.26579\\\\154.9199\\\\322.01\\\\21.72579\\\\153.4199\\\\322.01\\\\20.79055\\\\151.9199\\\\322.01\\\\19.13579\\\\150.2949\\\\322.01\\\\17.13579\\\\149.2219\\\\322.01\\\\16.13579\\\\148.9099\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"58\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"27\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"14.13579\\\\226.9441\\\\322.01\\\\18.13579\\\\226.5733\\\\322.01\\\\20.13579\\\\226.1058\\\\322.01\\\\23.13579\\\\225.037\\\\322.01\\\\25.13579\\\\224.0554\\\\322.01\\\\26.63579\\\\223.1393\\\\322.01\\\\28.13579\\\\222.0859\\\\322.01\\\\30.02057\\\\220.4199\\\\322.01\\\\32.35234\\\\217.9199\\\\322.01\\\\33.796\\\\215.9199\\\\322.01\\\\35.17205\\\\213.4199\\\\322.01\\\\36.22206\\\\210.9199\\\\322.01\\\\36.82989\\\\208.9199\\\\322.01\\\\37.3183\\\\206.4199\\\\322.01\\\\37.43793\\\\204.4199\\\\322.01\\\\37.30504\\\\200.9199\\\\322.01\\\\36.80717\\\\198.4199\\\\322.01\\\\35.81688\\\\195.4199\\\\322.01\\\\34.33746\\\\192.4199\\\\322.01\\\\33.71524\\\\191.4199\\\\322.01\\\\32.26848\\\\189.4199\\\\322.01\\\\30.63579\\\\187.6435\\\\322.01\\\\28.63579\\\\185.7795\\\\322.01\\\\26.63579\\\\184.2958\\\\322.01\\\\25.13579\\\\183.3219\\\\322.01\\\\21.63579\\\\181.3373\\\\322.01\\\\19.13579\\\\180.2194\\\\322.01\\\\17.63579\\\\179.7097\\\\322.01\\\\16.13579\\\\179.3674\\\\322.01\\\\14.13579\\\\179.1844\\\\322.01\\\\12.63579\\\\179.2658\\\\322.01\\\\10.63579\\\\179.7215\\\\322.01\\\\9.135789\\\\180.2507\\\\322.01\\\\6.135788\\\\181.6909\\\\322.01\\\\2.635788\\\\183.7795\\\\322.01\\\\0.6357885\\\\185.1998\\\\322.01\\\\-1.864211\\\\187.3557\\\\322.01\\\\-2.866484\\\\188.4199\\\\322.01\\\\-4.500405\\\\190.4199\\\\322.01\\\\-6.073093\\\\192.9199\\\\322.01\\\\-7.054916\\\\194.9199\\\\322.01\\\\-7.965239\\\\197.4199\\\\322.01\\\\-8.563286\\\\199.9199\\\\322.01\\\\-8.854596\\\\202.4199\\\\322.01\\\\-8.944143\\\\203.9199\\\\322.01\\\\-8.496565\\\\207.9199\\\\322.01\\\\-8.000725\\\\209.9199\\\\322.01\\\\-7.083763\\\\212.4199\\\\322.01\\\\-5.565878\\\\215.4199\\\\322.01\\\\-4.578253\\\\216.9199\\\\322.01\\\\-2.545509\\\\219.4199\\\\322.01\\\\-0.8642115\\\\221.0499\\\\322.01\\\\1.135789\\\\222.6285\\\\322.01\\\\2.635788\\\\223.6162\\\\322.01\\\\4.635788\\\\224.66\\\\322.01\\\\6.635788\\\\225.5312\\\\322.01\\\\8.135789\\\\226.0243\\\\322.01\\\\10.63579\\\\226.6116\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"59\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"28\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"49.13579\\\\134.7441\\\\322.01\\\\46.63579\\\\137.0307\\\\322.01\\\\44.92775\\\\138.9199\\\\322.01\\\\43.50623\\\\140.9199\\\\322.01\\\\42.38579\\\\142.9199\\\\322.01\\\\40.97799\\\\145.9199\\\\322.01\\\\39.93071\\\\148.9199\\\\322.01\\\\39.43028\\\\150.9199\\\\322.01\\\\39.09673\\\\153.4199\\\\322.01\\\\39.41588\\\\156.4199\\\\322.01\\\\39.95945\\\\158.4199\\\\322.01\\\\40.52195\\\\159.9199\\\\322.01\\\\41.48458\\\\161.9199\\\\322.01\\\\42.9492\\\\164.4199\\\\322.01\\\\43.96608\\\\165.9199\\\\322.01\\\\45.53619\\\\167.9199\\\\322.01\\\\46.44071\\\\168.9199\\\\322.01\\\\48.13579\\\\170.5235\\\\322.01\\\\50.13579\\\\172.0799\\\\322.01\\\\51.63579\\\\173.0376\\\\322.01\\\\53.63579\\\\174.1012\\\\322.01\\\\56.13579\\\\175.1018\\\\322.01\\\\57.63579\\\\175.5449\\\\322.01\\\\60.13579\\\\176.0546\\\\322.01\\\\63.13579\\\\176.222\\\\322.01\\\\65.63579\\\\176.0973\\\\322.01\\\\68.13579\\\\175.614\\\\322.01\\\\70.13579\\\\175.0277\\\\322.01\\\\72.63579\\\\174.0123\\\\322.01\\\\75.13579\\\\172.6378\\\\322.01\\\\76.63579\\\\171.6285\\\\322.01\\\\78.63579\\\\169.9578\\\\322.01\\\\80.14716\\\\168.4199\\\\322.01\\\\81.7909\\\\166.4199\\\\322.01\\\\82.79488\\\\164.9199\\\\322.01\\\\84.34893\\\\161.9199\\\\322.01\\\\85.28902\\\\159.4199\\\\322.01\\\\85.8061\\\\157.4199\\\\322.01\\\\86.23142\\\\154.4199\\\\322.01\\\\86.31414\\\\152.9199\\\\322.01\\\\86.13579\\\\150.9318\\\\322.01\\\\85.74485\\\\148.4199\\\\322.01\\\\85.19829\\\\146.4199\\\\322.01\\\\84.26165\\\\143.9199\\\\322.01\\\\83.2471\\\\141.9199\\\\322.01\\\\82.32372\\\\140.4199\\\\322.01\\\\80.81777\\\\138.4199\\\\322.01\\\\78.13579\\\\135.695\\\\322.01\\\\75.63579\\\\133.7706\\\\322.01\\\\74.13579\\\\132.8641\\\\322.01\\\\72.13579\\\\131.8482\\\\322.01\\\\69.13579\\\\130.7308\\\\322.01\\\\67.13579\\\\130.2393\\\\322.01\\\\63.13579\\\\129.7981\\\\322.01\\\\61.63579\\\\129.9028\\\\322.01\\\\58.63579\\\\130.3108\\\\322.01\\\\56.63579\\\\130.8307\\\\322.01\\\\54.13579\\\\131.7483\\\\322.01\\\\52.13579\\\\132.7481\\\\322.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"55\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"29\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-36.86421\\\\134.5039\\\\325.01\\\\-40.36421\\\\134.8251\\\\325.01\\\\-42.36421\\\\135.3162\\\\325.01\\\\-44.86421\\\\136.2328\\\\325.01\\\\-46.86421\\\\137.2308\\\\325.01\\\\-47.86421\\\\137.8358\\\\325.01\\\\-49.86421\\\\139.2767\\\\325.01\\\\-50.86421\\\\140.1812\\\\325.01\\\\-52.55669\\\\141.9199\\\\325.01\\\\-54.06213\\\\143.9199\\\\325.01\\\\-54.98921\\\\145.4199\\\\325.01\\\\-55.96295\\\\147.4199\\\\325.01\\\\-56.98823\\\\150.4199\\\\325.01\\\\-57.53526\\\\153.4199\\\\325.01\\\\-57.62727\\\\155.4199\\\\325.01\\\\-57.52279\\\\157.4199\\\\325.01\\\\-57.06078\\\\159.9199\\\\325.01\\\\-56.49021\\\\161.9199\\\\325.01\\\\-55.41194\\\\164.4199\\\\325.01\\\\-54.57702\\\\165.9199\\\\325.01\\\\-53.57396\\\\167.4199\\\\325.01\\\\-51.89916\\\\169.4199\\\\325.01\\\\-50.86421\\\\170.4544\\\\325.01\\\\-48.86421\\\\172.1211\\\\325.01\\\\-47.36421\\\\173.1227\\\\325.01\\\\-45.86421\\\\173.9317\\\\325.01\\\\-43.36421\\\\175.006\\\\325.01\\\\-41.36421\\\\175.596\\\\325.01\\\\-38.86421\\\\176.0515\\\\325.01\\\\-36.86421\\\\176.1623\\\\325.01\\\\-34.86421\\\\176.0684\\\\325.01\\\\-32.36421\\\\175.6225\\\\325.01\\\\-30.36421\\\\175.073\\\\325.01\\\\-27.86421\\\\174.0279\\\\325.01\\\\-25.36421\\\\172.5758\\\\325.01\\\\-23.36421\\\\171.0328\\\\325.01\\\\-21.68302\\\\169.4199\\\\325.01\\\\-19.67303\\\\166.9199\\\\325.01\\\\-18.72346\\\\165.4199\\\\325.01\\\\-17.71021\\\\163.4199\\\\325.01\\\\-16.66229\\\\160.4199\\\\325.01\\\\-16.19818\\\\158.4199\\\\325.01\\\\-15.96381\\\\155.4199\\\\325.01\\\\-16.25821\\\\151.9199\\\\325.01\\\\-16.73232\\\\149.9199\\\\325.01\\\\-17.64964\\\\147.4199\\\\325.01\\\\-19.19963\\\\144.4199\\\\325.01\\\\-20.24376\\\\142.9199\\\\325.01\\\\-21.86421\\\\141.089\\\\325.01\\\\-24.36421\\\\138.787\\\\325.01\\\\-25.86421\\\\137.7401\\\\325.01\\\\-27.36421\\\\136.9034\\\\325.01\\\\-29.86421\\\\135.7551\\\\325.01\\\\-31.36421\\\\135.2615\\\\325.01\\\\-33.36421\\\\134.7939\\\\325.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"55\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"30\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"10.13579\\\\183.3021\\\\325.01\\\\7.135788\\\\184.2407\\\\325.01\\\\5.135788\\\\185.1719\\\\325.01\\\\3.135788\\\\186.2872\\\\325.01\\\\1.135789\\\\187.7495\\\\325.01\\\\-1.559387\\\\190.4199\\\\325.01\\\\-3.057961\\\\192.4199\\\\325.01\\\\-4.487128\\\\194.9199\\\\325.01\\\\-5.510696\\\\197.4199\\\\325.01\\\\-6.051712\\\\199.4199\\\\325.01\\\\-6.519062\\\\202.4199\\\\325.01\\\\-6.565339\\\\203.9199\\\\325.01\\\\-6.408329\\\\205.9199\\\\325.01\\\\-5.967034\\\\208.4199\\\\325.01\\\\-4.985113\\\\211.4199\\\\325.01\\\\-4.035583\\\\213.4199\\\\325.01\\\\-3.447545\\\\214.4199\\\\325.01\\\\-2.060332\\\\216.4199\\\\325.01\\\\-0.8642115\\\\217.754\\\\325.01\\\\1.635789\\\\220.1117\\\\325.01\\\\3.635788\\\\221.491\\\\325.01\\\\4.635788\\\\222.0804\\\\325.01\\\\6.635788\\\\223.0186\\\\325.01\\\\8.135789\\\\223.5795\\\\325.01\\\\10.13579\\\\224.1074\\\\325.01\\\\12.63579\\\\224.4875\\\\325.01\\\\14.13579\\\\224.5909\\\\325.01\\\\16.13579\\\\224.4519\\\\325.01\\\\18.63579\\\\224.0641\\\\325.01\\\\20.63579\\\\223.4818\\\\325.01\\\\23.13579\\\\222.4497\\\\325.01\\\\25.63579\\\\221.0292\\\\325.01\\\\27.63579\\\\219.5015\\\\325.01\\\\29.76198\\\\217.4199\\\\325.01\\\\31.33117\\\\215.4199\\\\325.01\\\\32.29767\\\\213.9199\\\\325.01\\\\33.31492\\\\211.9199\\\\325.01\\\\34.25379\\\\209.4199\\\\325.01\\\\34.83692\\\\206.9199\\\\325.01\\\\35.10346\\\\203.9199\\\\325.01\\\\34.88203\\\\200.9199\\\\325.01\\\\34.73055\\\\199.9199\\\\325.01\\\\34.20932\\\\197.9199\\\\325.01\\\\33.27197\\\\195.4199\\\\325.01\\\\32.22284\\\\193.4199\\\\325.01\\\\31.25856\\\\191.9199\\\\325.01\\\\29.63579\\\\189.9463\\\\325.01\\\\28.11496\\\\188.4199\\\\325.01\\\\26.13579\\\\186.7882\\\\325.01\\\\23.63579\\\\185.2499\\\\325.01\\\\21.63579\\\\184.3438\\\\325.01\\\\20.13579\\\\183.7888\\\\325.01\\\\18.13579\\\\183.2512\\\\325.01\\\\15.13579\\\\182.8438\\\\325.01\\\\13.13579\\\\182.8685\\\\325.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"54\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"31\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"12.13579\\\\123.9516\\\\325.01\\\\15.63579\\\\123.596\\\\325.01\\\\17.63579\\\\123.0987\\\\325.01\\\\19.13579\\\\122.5923\\\\325.01\\\\21.13579\\\\121.6678\\\\325.01\\\\23.13579\\\\120.5568\\\\325.01\\\\25.13579\\\\119.0902\\\\325.01\\\\26.89038\\\\117.4199\\\\325.01\\\\28.2372\\\\115.9199\\\\325.01\\\\29.32537\\\\114.4199\\\\325.01\\\\30.75554\\\\111.9199\\\\325.01\\\\31.77949\\\\109.4199\\\\325.01\\\\32.32329\\\\107.4199\\\\325.01\\\\32.79064\\\\104.4199\\\\325.01\\\\32.83692\\\\102.9199\\\\325.01\\\\32.6799\\\\100.9199\\\\325.01\\\\32.23861\\\\98.41986\\\\325.01\\\\31.25669\\\\95.41986\\\\325.01\\\\30.30979\\\\93.41986\\\\325.01\\\\29.72363\\\\92.41986\\\\325.01\\\\28.33361\\\\90.41986\\\\325.01\\\\25.96829\\\\87.91986\\\\325.01\\\\24.63579\\\\86.72805\\\\325.01\\\\21.63579\\\\84.76335\\\\325.01\\\\19.63579\\\\83.82532\\\\325.01\\\\18.13579\\\\83.26024\\\\325.01\\\\16.13579\\\\82.73236\\\\325.01\\\\13.63579\\\\82.35223\\\\325.01\\\\11.13579\\\\82.27512\\\\325.01\\\\7.635788\\\\82.77291\\\\325.01\\\\5.635788\\\\83.35789\\\\325.01\\\\3.135788\\\\84.39439\\\\325.01\\\\0.6357885\\\\85.814\\\\325.01\\\\-1.364211\\\\87.34236\\\\325.01\\\\-3.033193\\\\88.91986\\\\325.01\\\\-5.063791\\\\91.41986\\\\325.01\\\\-6.030195\\\\92.91986\\\\325.01\\\\-7.047282\\\\94.91986\\\\325.01\\\\-7.982212\\\\97.41986\\\\325.01\\\\-8.565339\\\\99.91986\\\\325.01\\\\-8.831883\\\\102.9199\\\\325.01\\\\-8.610452\\\\105.9199\\\\325.01\\\\-8.458969\\\\106.9199\\\\325.01\\\\-7.937741\\\\108.9199\\\\325.01\\\\-6.997408\\\\111.4199\\\\325.01\\\\-5.947545\\\\113.4199\\\\325.01\\\\-4.983581\\\\114.9199\\\\325.01\\\\-3.364212\\\\116.8934\\\\325.01\\\\-1.843378\\\\118.4199\\\\325.01\\\\0.1357885\\\\120.0548\\\\325.01\\\\1.635789\\\\121.0273\\\\325.01\\\\3.635788\\\\122.0806\\\\325.01\\\\6.135788\\\\123.0276\\\\325.01\\\\8.135789\\\\123.5526\\\\325.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"54\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"32\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"63.13579\\\\173.8627\\\\325.01\\\\66.63579\\\\173.5429\\\\325.01\\\\68.63579\\\\173.0517\\\\325.01\\\\71.13579\\\\172.1226\\\\325.01\\\\74.13579\\\\170.5327\\\\325.01\\\\76.13579\\\\169.0828\\\\325.01\\\\77.39513\\\\167.9199\\\\325.01\\\\79.25494\\\\165.9199\\\\325.01\\\\80.34204\\\\164.4199\\\\325.01\\\\81.26814\\\\162.9199\\\\325.01\\\\82.24203\\\\160.9199\\\\325.01\\\\83.26665\\\\157.9199\\\\325.01\\\\83.75102\\\\155.4199\\\\325.01\\\\83.89706\\\\152.9199\\\\325.01\\\\83.7918\\\\150.9199\\\\325.01\\\\83.32329\\\\148.4199\\\\325.01\\\\82.73289\\\\146.4199\\\\325.01\\\\81.65215\\\\143.9199\\\\325.01\\\\80.83456\\\\142.4199\\\\325.01\\\\79.83023\\\\140.9199\\\\325.01\\\\78.13579\\\\138.9287\\\\325.01\\\\77.1152\\\\137.9199\\\\325.01\\\\75.13579\\\\136.2468\\\\325.01\\\\73.63579\\\\135.2457\\\\325.01\\\\70.63579\\\\133.7382\\\\325.01\\\\67.63579\\\\132.7589\\\\325.01\\\\64.63579\\\\132.2613\\\\325.01\\\\63.13579\\\\132.1962\\\\325.01\\\\61.13579\\\\132.3016\\\\325.01\\\\58.63579\\\\132.7324\\\\325.01\\\\56.63579\\\\133.2899\\\\325.01\\\\54.13579\\\\134.3387\\\\325.01\\\\51.63579\\\\135.7905\\\\325.01\\\\49.63579\\\\137.3374\\\\325.01\\\\47.52657\\\\139.4199\\\\325.01\\\\45.95454\\\\141.4199\\\\325.01\\\\45.00655\\\\142.9199\\\\325.01\\\\43.99264\\\\144.9199\\\\325.01\\\\42.93922\\\\147.9199\\\\325.01\\\\42.47358\\\\149.9199\\\\325.01\\\\42.22876\\\\152.9199\\\\325.01\\\\42.5039\\\\156.4199\\\\325.01\\\\42.97881\\\\158.4199\\\\325.01\\\\43.47368\\\\159.9199\\\\325.01\\\\44.61928\\\\162.4199\\\\325.01\\\\45.45341\\\\163.9199\\\\325.01\\\\46.4907\\\\165.4199\\\\325.01\\\\48.77973\\\\167.9199\\\\325.01\\\\50.63579\\\\169.5769\\\\325.01\\\\52.13579\\\\170.6161\\\\325.01\\\\53.63579\\\\171.4762\\\\325.01\\\\56.13579\\\\172.604\\\\325.01\\\\57.63579\\\\173.1012\\\\325.01\\\\59.63579\\\\173.5834\\\\325.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"44\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"33\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-36.86421\\\\137.8837\\\\328.01\\\\-40.36421\\\\138.2949\\\\328.01\\\\-43.36421\\\\139.2683\\\\328.01\\\\-45.36421\\\\140.256\\\\328.01\\\\-46.86421\\\\141.2251\\\\328.01\\\\-49.36421\\\\143.4318\\\\328.01\\\\-51.02226\\\\145.4199\\\\328.01\\\\-52.4865\\\\147.9199\\\\328.01\\\\-53.44845\\\\150.4199\\\\328.01\\\\-54.01472\\\\152.9199\\\\328.01\\\\-54.17799\\\\155.4199\\\\328.01\\\\-53.99431\\\\157.9199\\\\328.01\\\\-53.53869\\\\159.9199\\\\328.01\\\\-53.04579\\\\161.4199\\\\328.01\\\\-51.54679\\\\164.4199\\\\328.01\\\\-50.51004\\\\165.9199\\\\328.01\\\\-48.86421\\\\167.6923\\\\328.01\\\\-47.36421\\\\169.0581\\\\328.01\\\\-45.86421\\\\170.0912\\\\328.01\\\\-42.86421\\\\171.5804\\\\328.01\\\\-41.36421\\\\172.0771\\\\328.01\\\\-39.36421\\\\172.5214\\\\328.01\\\\-36.86421\\\\172.7081\\\\328.01\\\\-34.36421\\\\172.5449\\\\328.01\\\\-32.36421\\\\172.1048\\\\328.01\\\\-30.86421\\\\171.6225\\\\328.01\\\\-29.36421\\\\170.9925\\\\328.01\\\\-26.86421\\\\169.5144\\\\328.01\\\\-24.36421\\\\167.2983\\\\328.01\\\\-22.72177\\\\165.4199\\\\328.01\\\\-21.74773\\\\163.9199\\\\328.01\\\\-20.7528\\\\161.9199\\\\328.01\\\\-20.20369\\\\160.4199\\\\328.01\\\\-19.68638\\\\158.4199\\\\328.01\\\\-19.35579\\\\155.4199\\\\328.01\\\\-19.73277\\\\151.9199\\\\328.01\\\\-20.68263\\\\148.9199\\\\328.01\\\\-22.23921\\\\145.9199\\\\328.01\\\\-23.76356\\\\143.9199\\\\328.01\\\\-25.36421\\\\142.3077\\\\328.01\\\\-27.36421\\\\140.775\\\\328.01\\\\-29.36421\\\\139.6808\\\\328.01\\\\-31.36421\\\\138.8302\\\\328.01\\\\-33.36421\\\\138.2729\\\\328.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"48\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"34\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"19.13579\\\\87.28422\\\\328.01\\\\16.13579\\\\86.24017\\\\328.01\\\\13.63579\\\\85.78097\\\\328.01\\\\12.13579\\\\85.70812\\\\328.01\\\\10.13579\\\\85.80904\\\\328.01\\\\7.635788\\\\86.28326\\\\328.01\\\\6.135788\\\\86.7646\\\\328.01\\\\3.135788\\\\88.22263\\\\328.01\\\\1.635789\\\\89.22868\\\\328.01\\\\0.2878155\\\\90.41986\\\\328.01\\\\-2.01651\\\\92.91986\\\\328.01\\\\-3.005878\\\\94.41986\\\\328.01\\\\-4.015807\\\\96.41986\\\\328.01\\\\-5.004518\\\\99.41986\\\\328.01\\\\-5.367085\\\\101.9199\\\\328.01\\\\-5.40542\\\\103.9199\\\\328.01\\\\-4.976191\\\\106.9199\\\\328.01\\\\-4.533483\\\\108.4199\\\\328.01\\\\-3.493332\\\\110.9199\\\\328.01\\\\-2.909943\\\\111.9199\\\\328.01\\\\-1.527902\\\\113.9199\\\\328.01\\\\-0.3642115\\\\115.1828\\\\328.01\\\\1.135789\\\\116.5895\\\\328.01\\\\3.135788\\\\117.9891\\\\328.01\\\\5.135788\\\\119.0462\\\\328.01\\\\8.135789\\\\120.0565\\\\328.01\\\\10.63579\\\\120.4504\\\\328.01\\\\13.13579\\\\120.4817\\\\328.01\\\\15.63579\\\\120.0977\\\\328.01\\\\17.63579\\\\119.5301\\\\328.01\\\\20.63579\\\\118.1171\\\\328.01\\\\22.13579\\\\117.1346\\\\328.01\\\\23.61563\\\\115.9199\\\\328.01\\\\24.63579\\\\114.9158\\\\328.01\\\\26.28404\\\\112.9199\\\\328.01\\\\27.71817\\\\110.4199\\\\328.01\\\\28.33316\\\\108.9199\\\\328.01\\\\28.81362\\\\107.4199\\\\328.01\\\\29.22263\\\\105.4199\\\\328.01\\\\29.37814\\\\102.9199\\\\328.01\\\\29.24661\\\\100.9199\\\\328.01\\\\28.70996\\\\98.41986\\\\328.01\\\\28.19545\\\\96.91986\\\\328.01\\\\27.28689\\\\94.91986\\\\328.01\\\\26.34556\\\\93.41986\\\\328.01\\\\25.22345\\\\91.91986\\\\328.01\\\\23.63579\\\\90.26862\\\\328.01\\\\21.63579\\\\88.71584\\\\328.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"47\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"35\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"31.24777\\\\199.9199\\\\328.01\\\\30.23194\\\\196.9199\\\\328.01\\\\29.18762\\\\194.9199\\\\328.01\\\\27.80543\\\\192.9199\\\\328.01\\\\25.8825\\\\190.9199\\\\328.01\\\\25.13579\\\\190.2482\\\\328.01\\\\23.13579\\\\188.8506\\\\328.01\\\\21.13579\\\\187.7935\\\\328.01\\\\18.13579\\\\186.7796\\\\328.01\\\\15.63579\\\\186.3893\\\\328.01\\\\13.13579\\\\186.358\\\\328.01\\\\10.63579\\\\186.7369\\\\328.01\\\\8.635789\\\\187.3055\\\\328.01\\\\5.635788\\\\188.7204\\\\328.01\\\\4.135788\\\\189.7018\\\\328.01\\\\2.65595\\\\190.9199\\\\328.01\\\\1.632165\\\\191.9199\\\\328.01\\\\-0.01651034\\\\193.9199\\\\328.01\\\\-1.45228\\\\196.4199\\\\328.01\\\\-2.066843\\\\197.9199\\\\328.01\\\\-2.542047\\\\199.4199\\\\328.01\\\\-2.951977\\\\201.4199\\\\328.01\\\\-3.091252\\\\204.4199\\\\328.01\\\\-2.975036\\\\205.9199\\\\328.01\\\\-2.438387\\\\208.4199\\\\328.01\\\\-1.923871\\\\209.9199\\\\328.01\\\\-1.013668\\\\211.9199\\\\328.01\\\\-0.07398161\\\\213.4199\\\\328.01\\\\1.482664\\\\215.4199\\\\328.01\\\\2.635788\\\\216.573\\\\328.01\\\\4.635788\\\\218.1181\\\\328.01\\\\7.135788\\\\219.5555\\\\328.01\\\\10.13579\\\\220.5977\\\\328.01\\\\12.63579\\\\221.0551\\\\328.01\\\\14.13579\\\\221.1316\\\\328.01\\\\16.13579\\\\221.0307\\\\328.01\\\\18.63579\\\\220.5513\\\\328.01\\\\20.13579\\\\220.0735\\\\328.01\\\\23.13579\\\\218.6193\\\\328.01\\\\24.63579\\\\217.6051\\\\328.01\\\\26.63579\\\\215.7863\\\\328.01\\\\28.28404\\\\213.9199\\\\328.01\\\\29.27745\\\\212.4199\\\\328.01\\\\30.28206\\\\210.4199\\\\328.01\\\\31.27239\\\\207.4199\\\\328.01\\\\31.63863\\\\204.9199\\\\328.01\\\\31.677\\\\202.9199\\\\328.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"47\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"36\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"45.63295\\\\152.9199\\\\328.01\\\\45.99919\\\\156.4199\\\\328.01\\\\46.56079\\\\158.4199\\\\328.01\\\\46.94164\\\\159.4199\\\\328.01\\\\48.4909\\\\162.4199\\\\328.01\\\\50.01237\\\\164.4199\\\\328.01\\\\51.63579\\\\166.0496\\\\328.01\\\\53.63579\\\\167.5761\\\\328.01\\\\55.13579\\\\168.4353\\\\328.01\\\\57.63579\\\\169.5475\\\\328.01\\\\59.63579\\\\170.0908\\\\328.01\\\\62.13579\\\\170.4227\\\\328.01\\\\64.13579\\\\170.4052\\\\328.01\\\\66.63579\\\\170.0602\\\\328.01\\\\69.63579\\\\169.0909\\\\328.01\\\\71.63579\\\\168.1004\\\\328.01\\\\73.13579\\\\167.1263\\\\328.01\\\\75.13579\\\\165.4452\\\\328.01\\\\77.29958\\\\162.9199\\\\328.01\\\\78.76894\\\\160.4199\\\\328.01\\\\79.72987\\\\157.9199\\\\328.01\\\\80.2863\\\\155.4199\\\\328.01\\\\80.43733\\\\153.4199\\\\328.01\\\\80.26079\\\\150.4199\\\\328.01\\\\79.80331\\\\148.4199\\\\328.01\\\\79.30334\\\\146.9199\\\\328.01\\\\77.80714\\\\143.9199\\\\328.01\\\\76.75469\\\\142.4199\\\\328.01\\\\75.63579\\\\141.1699\\\\328.01\\\\73.63579\\\\139.3292\\\\328.01\\\\72.13579\\\\138.2726\\\\328.01\\\\69.13579\\\\136.7804\\\\328.01\\\\67.63579\\\\136.2818\\\\328.01\\\\65.13579\\\\135.7744\\\\328.01\\\\63.13579\\\\135.652\\\\328.01\\\\60.63579\\\\135.8225\\\\328.01\\\\58.63579\\\\136.2454\\\\328.01\\\\57.13579\\\\136.731\\\\328.01\\\\55.63579\\\\137.3752\\\\328.01\\\\54.13579\\\\138.2036\\\\328.01\\\\52.63579\\\\139.2228\\\\328.01\\\\51.28782\\\\140.4199\\\\328.01\\\\49.00781\\\\142.9199\\\\328.01\\\\48.03405\\\\144.4199\\\\328.01\\\\47.02981\\\\146.4199\\\\328.01\\\\46.48053\\\\147.9199\\\\328.01\\\\45.95977\\\\149.9199\\\\328.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"39\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"37\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"-35.86421\\\\143.7896\\\\331.01\\\\-38.36421\\\\143.8818\\\\331.01\\\\-40.36421\\\\144.3699\\\\331.01\\\\-42.36421\\\\145.2396\\\\331.01\\\\-43.36421\\\\145.8187\\\\331.01\\\\-44.67136\\\\146.9199\\\\331.01\\\\-46.04104\\\\148.4199\\\\331.01\\\\-46.98353\\\\149.9199\\\\331.01\\\\-47.44573\\\\150.9199\\\\331.01\\\\-47.93336\\\\152.4199\\\\331.01\\\\-48.16312\\\\153.9199\\\\331.01\\\\-48.25866\\\\155.4199\\\\331.01\\\\-48.0823\\\\157.4199\\\\331.01\\\\-47.55443\\\\159.4199\\\\331.01\\\\-46.57254\\\\161.4199\\\\331.01\\\\-45.48601\\\\162.9199\\\\331.01\\\\-44.36421\\\\164.0417\\\\331.01\\\\-42.86421\\\\165.1163\\\\331.01\\\\-40.86421\\\\166.0921\\\\331.01\\\\-39.36421\\\\166.5422\\\\331.01\\\\-36.86421\\\\166.784\\\\331.01\\\\-35.36421\\\\166.6911\\\\331.01\\\\-33.86421\\\\166.4476\\\\331.01\\\\-31.36421\\\\165.5051\\\\331.01\\\\-29.86421\\\\164.5511\\\\331.01\\\\-28.86421\\\\163.6834\\\\331.01\\\\-27.69546\\\\162.4199\\\\331.01\\\\-26.71886\\\\160.9199\\\\331.01\\\\-25.70996\\\\158.4199\\\\331.01\\\\-25.27567\\\\156.4199\\\\331.01\\\\-25.21317\\\\155.4199\\\\331.01\\\\-25.32698\\\\153.9199\\\\331.01\\\\-25.78442\\\\151.9199\\\\331.01\\\\-26.63747\\\\149.9199\\\\331.01\\\\-27.21177\\\\148.9199\\\\331.01\\\\-28.36421\\\\147.5032\\\\331.01\\\\-30.36421\\\\145.7552\\\\331.01\\\\-31.36421\\\\145.1815\\\\331.01\\\\-33.36421\\\\144.3188\\\\331.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"38\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"38\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"4.534598\\\\209.9199\\\\331.01\\\\6.188819\\\\211.9199\\\\331.01\\\\8.135789\\\\213.5071\\\\331.01\\\\10.13579\\\\214.5014\\\\331.01\\\\12.13579\\\\215.0422\\\\331.01\\\\14.13579\\\\215.197\\\\331.01\\\\16.63579\\\\215.0084\\\\331.01\\\\18.13579\\\\214.5883\\\\331.01\\\\19.63579\\\\213.9377\\\\331.01\\\\21.13579\\\\213.0235\\\\331.01\\\\22.13579\\\\212.1699\\\\331.01\\\\23.30454\\\\210.9199\\\\331.01\\\\24.30439\\\\209.4199\\\\331.01\\\\25.19557\\\\207.4199\\\\331.01\\\\25.71232\\\\205.4199\\\\331.01\\\\25.81808\\\\203.9199\\\\331.01\\\\25.67383\\\\201.9199\\\\331.01\\\\25.30427\\\\200.4199\\\\331.01\\\\24.75245\\\\198.9199\\\\331.01\\\\24.23462\\\\197.9199\\\\331.01\\\\23.19348\\\\196.4199\\\\331.01\\\\21.63579\\\\194.8493\\\\331.01\\\\20.13579\\\\193.8005\\\\331.01\\\\18.13579\\\\192.881\\\\331.01\\\\16.13579\\\\192.3417\\\\331.01\\\\14.63579\\\\192.2231\\\\331.01\\\\12.63579\\\\192.3105\\\\331.01\\\\10.63579\\\\192.7975\\\\331.01\\\\8.635789\\\\193.6931\\\\331.01\\\\7.635788\\\\194.2918\\\\331.01\\\\5.818221\\\\195.9199\\\\331.01\\\\4.971154\\\\196.9199\\\\331.01\\\\4.039197\\\\198.4199\\\\331.01\\\\3.596016\\\\199.4199\\\\331.01\\\\3.00558\\\\201.4199\\\\331.01\\\\2.826006\\\\203.9199\\\\331.01\\\\3.086875\\\\206.4199\\\\331.01\\\\3.532528\\\\207.9199\\\\331.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"39\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"39\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"21.73698\\\\96.91986\\\\331.01\\\\20.08276\\\\94.91986\\\\331.01\\\\18.13579\\\\93.33265\\\\331.01\\\\16.13579\\\\92.33833\\\\331.01\\\\14.13579\\\\91.79752\\\\331.01\\\\12.13579\\\\91.64268\\\\331.01\\\\9.635789\\\\91.83131\\\\331.01\\\\8.135789\\\\92.25138\\\\331.01\\\\6.635788\\\\92.902\\\\331.01\\\\5.135788\\\\93.8162\\\\331.01\\\\4.135788\\\\94.66986\\\\331.01\\\\2.967038\\\\95.91986\\\\331.01\\\\1.967184\\\\97.41986\\\\331.01\\\\1.076006\\\\99.41986\\\\331.01\\\\0.5592579\\\\101.4199\\\\331.01\\\\0.4534968\\\\102.9199\\\\331.01\\\\0.597745\\\\104.9199\\\\331.01\\\\0.9673102\\\\106.4199\\\\331.01\\\\1.519122\\\\107.9199\\\\331.01\\\\2.036951\\\\108.9199\\\\331.01\\\\3.078096\\\\110.4199\\\\331.01\\\\4.635788\\\\111.9904\\\\331.01\\\\6.135788\\\\113.0392\\\\331.01\\\\8.135789\\\\113.9587\\\\331.01\\\\10.13579\\\\114.498\\\\331.01\\\\11.63579\\\\114.6167\\\\331.01\\\\13.63579\\\\114.5292\\\\331.01\\\\15.63579\\\\114.0422\\\\331.01\\\\17.63579\\\\113.1466\\\\331.01\\\\18.63579\\\\112.5479\\\\331.01\\\\20.45336\\\\110.9199\\\\331.01\\\\21.30042\\\\109.9199\\\\331.01\\\\22.23238\\\\108.4199\\\\331.01\\\\22.67556\\\\107.4199\\\\331.01\\\\23.266\\\\105.4199\\\\331.01\\\\23.41296\\\\103.9199\\\\331.01\\\\23.39122\\\\101.9199\\\\331.01\\\\23.1847\\\\100.4199\\\\331.01\\\\22.73905\\\\98.91986\\\\331.01\"\n"
"                     }\n"
"                  },\n"
"                  {\n"
"                     \"3006,0016\" : {\n"
"                        \"Name\" : \"ContourImageSequence\",\n"
"                        \"Type\" : \"Sequence\",\n"
"                        \"Value\" : [\n"
"                           {\n"
"                              \"0008,1150\" : {\n"
"                                 \"Name\" : \"ReferencedSOPClassUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"1.2.840.10008.5.1.4.1.1.2\"\n"
"                              },\n"
"                              \"0008,1155\" : {\n"
"                                 \"Name\" : \"ReferencedSOPInstanceUID\",\n"
"                                 \"Type\" : \"String\",\n"
"                                 \"Value\" : \"\"\n"
"                              }\n"
"                           }\n"
"                        ]\n"
"                     },\n"
"                     \"3006,0042\" : {\n"
"                        \"Name\" : \"ContourGeometricType\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"CLOSED_PLANAR\"\n"
"                     },\n"
"                     \"3006,0046\" : {\n"
"                        \"Name\" : \"NumberOfContourPoints\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"37\"\n"
"                     },\n"
"                     \"3006,0048\" : {\n"
"                        \"Name\" : \"ContourNumber\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"40\"\n"
"                     },\n"
"                     \"3006,0050\" : {\n"
"                        \"Name\" : \"ContourData\",\n"
"                        \"Type\" : \"String\",\n"
"                        \"Value\" : \"74.53024\\\\152.9199\\\\331.01\\\\74.26877\\\\150.4199\\\\331.01\\\\73.81514\\\\148.9199\\\\331.01\\\\72.83221\\\\146.9199\\\\331.01\\\\71.73194\\\\145.4199\\\\331.01\\\\70.63579\\\\144.3343\\\\331.01\\\\69.13579\\\\143.2472\\\\331.01\\\\67.13579\\\\142.2623\\\\331.01\\\\65.63579\\\\141.8188\\\\331.01\\\\63.13579\\\\141.581\\\\331.01\\\\60.63579\\\\141.7975\\\\331.01\\\\59.13579\\\\142.2188\\\\331.01\\\\57.63579\\\\142.8559\\\\331.01\\\\56.13579\\\\143.804\\\\331.01\\\\55.40785\\\\144.4199\\\\331.01\\\\53.99204\\\\145.9199\\\\331.01\\\\53.00511\\\\147.4199\\\\331.01\\\\51.98905\\\\149.9199\\\\331.01\\\\51.54905\\\\151.9199\\\\331.01\\\\51.52641\\\\153.9199\\\\331.01\\\\52.04536\\\\156.4199\\\\331.01\\\\52.89742\\\\158.4199\\\\331.01\\\\53.47115\\\\159.4199\\\\331.01\\\\54.63579\\\\160.8652\\\\331.01\\\\56.63579\\\\162.6044\\\\331.01\\\\58.13579\\\\163.4489\\\\331.01\\\\59.63579\\\\164.0449\\\\331.01\\\\61.63579\\\\164.5168\\\\331.01\\\\63.13579\\\\164.5917\\\\331.01\\\\64.63579\\\\164.4964\\\\331.01\\\\66.63579\\\\164.0014\\\\331.01\\\\68.63579\\\\163.1234\\\\331.01\\\\70.40923\\\\161.9199\\\\331.01\\\\71.63579\\\\160.736\\\\331.01\\\\72.67847\\\\159.4199\\\\331.01\\\\73.72818\\\\157.4199\\\\331.01\\\\74.21391\\\\155.9199\\\\331.01\"\n"
"                     }\n"
"                  }\n"
"               ]\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"11\"\n"
"            }\n"
"         }\n"
"      ]\n"
"   },\n"
"   \"3006,0080\" : {\n"
"      \"Name\" : \"RTROIObservationsSequence\",\n"
"      \"Type\" : \"Sequence\",\n"
"      \"Value\" : [\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"1\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"LN300\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"ORGAN\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"2\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"2\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Cortical Bone\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"ORGAN\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"3\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"3\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Adipose\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"ORGAN\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"4\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"4\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"CB2-50%\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"ORGAN\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"5\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"5\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"Water\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"ORGAN\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"6\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"10\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"External\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"EXTERNAL\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         },\n"
"         {\n"
"            \"3006,0082\" : {\n"
"               \"Name\" : \"ObservationNumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"7\"\n"
"            },\n"
"            \"3006,0084\" : {\n"
"               \"Name\" : \"ReferencedROINumber\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"11\"\n"
"            },\n"
"            \"3006,0085\" : {\n"
"               \"Name\" : \"ROIObservationLabel\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"PTV\"\n"
"            },\n"
"            \"3006,00a4\" : {\n"
"               \"Name\" : \"RTROIInterpretedType\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"PTV\"\n"
"            },\n"
"            \"3006,00a6\" : {\n"
"               \"Name\" : \"ROIInterpreter\",\n"
"               \"Type\" : \"String\",\n"
"               \"Value\" : \"\"\n"
"            }\n"
"         }\n"
"      ]\n"
"   },\n"
"   \"300e,0002\" : {\n"
"      \"Name\" : \"ApprovalStatus\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"APPROVED\"\n"
"   },\n"
"   \"300e,0004\" : {\n"
"      \"Name\" : \"ReviewDate\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"20190318\"\n"
"   },\n"
"   \"300e,0005\" : {\n"
"      \"Name\" : \"ReviewTime\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"182558\"\n"
"   },\n"
"   \"300e,0008\" : {\n"
"      \"Name\" : \"ReviewerName\",\n"
"      \"Type\" : \"String\",\n"
"      \"Value\" : \"ONC_jwulff2\"\n"
"   }\n"
"}\n"
"";

#ifdef _MSC_VER
#pragma endregion
#endif
// _MSC_VER


#define STONE_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

static void CheckGroundTruth(
  const std::vector<DicomStructure2>& structures,
  const size_t structureIndex,
  const size_t sliceIndex,
  std::vector<double> groundTruth)
{
  const std::vector<DicomStructurePolygon2>& polygonsForThisStruct = structures.at(structureIndex).GetPolygons();
  const DicomStructurePolygon2& polygon = polygonsForThisStruct.at(sliceIndex);

  //double groundTruth[] = { 7.657838, 108.2725, 304.01, 6.826687, 107.4413, 304.01, 6.152492, 106.4785, 304.01, 5.655735, 105.4132, 304.01, 5.351513, 104.2778, 304.01, 5.249068, 103.1069, 304.01, 5.351513, 101.9359, 304.01, 5.655735, 100.8005, 304.01, 6.152492, 99.73524, 304.01, 6.826687, 98.77239, 304.01, 7.657838, 97.94124, 304.01, 8.620689, 97.26704, 304.01, 9.685987, 96.77029, 304.01, 10.82136, 96.46606, 304.01, 11.99231, 96.36362, 304.01, 13.16326, 96.46606, 304.01, 14.29864, 96.77029, 304.01, 15.36393, 97.26704, 304.01, 16.32678, 97.94124, 304.01, 17.15794, 98.77239, 304.01, 17.83213, 99.73524, 304.01, 18.32889, 100.8005, 304.01, 18.63311, 101.9359, 304.01, 18.73555, 103.1069, 304.01, 18.63311, 104.2778, 304.01, 18.32889, 105.4132, 304.01, 17.83213, 106.4785, 304.01, 17.15794, 107.4413, 304.01, 16.32678, 108.2725, 304.01, 15.36393, 108.9467, 304.01, 14.29864, 109.4434, 304.01, 13.16326, 109.7477, 304.01, 11.99231, 109.8501, 304.01, 10.82136, 109.7477, 304.01, 9.685987, 109.4434, 304.01, 8.620689, 108.9467, 304.01 };
  size_t groundTruthItems = groundTruth.size();

  size_t pointCount = 3 * polygon.GetPointCount();

  EXPECT_EQ(groundTruthItems, pointCount);

  for (size_t i = 0; i < polygon.GetPointCount(); ++i)
  {
    const Point3D& point = polygon.GetPoint(i);

    // loop over X, Y then Z.
    for (size_t j = 0; j < 3; ++j)
    {
      size_t index = 3 * i + j;
      ASSERT_LT(index, groundTruthItems);
      bool isNear = LinearAlgebra::IsNear(groundTruth[index], point[j]);
      EXPECT_TRUE(isNear);
    }
  }
}


#include <Toolbox.h>

TEST(StructureSet, ReadFromJsonThatsAll)
{
  DicomStructureSet2 structureSet;

  Json::Value test;
  Orthanc::Toolbox::ReadJson(test, k_rtStruct_json00);

  FullOrthancDataset dicom(test);
  //loader.content_.reset(new DicomStructureSet(dicom));
  structureSet.Clear();

  structureSet.FillStructuresFromDataset(dicom);
  structureSet.ComputeDependentProperties();

  const std::vector<DicomStructure2>& structures = structureSet.structures_;

  /*

  ██████╗  █████╗ ███████╗██╗ ██████╗     ██████╗██╗  ██╗███████╗ ██████╗██╗  ██╗███████╗
  ██╔══██╗██╔══██╗██╔════╝██║██╔════╝    ██╔════╝██║  ██║██╔════╝██╔════╝██║ ██╔╝██╔════╝
  ██████╔╝███████║███████╗██║██║         ██║     ███████║█████╗  ██║     █████╔╝ ███████╗
  ██╔══██╗██╔══██║╚════██║██║██║         ██║     ██╔══██║██╔══╝  ██║     ██╔═██╗ ╚════██║
  ██████╔╝██║  ██║███████║██║╚██████╗    ╚██████╗██║  ██║███████╗╚██████╗██║  ██╗███████║
  ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝ ╚═════╝     ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚══════╝
  http://patorjk.com/software/taag/#p=display&f=ANSI%20Shadow&t=BASIC%20CHECKS
  */

  // (0x3006, 0x0080) seq. size
  EXPECT_EQ(7, structures.size());

  // (0x3006, 0x0080)[i]/(0x3006, 0x00a4)
  for (size_t i = 0; i < 5; ++i)
  {
    EXPECT_EQ(std::string("ORGAN"), structures[i].interpretation_);
  }
  EXPECT_EQ(std::string("EXTERNAL"), structures[5].interpretation_);
  EXPECT_EQ(std::string("PTV"), structures[6].interpretation_);

  // (0x3006, 0x0020)[i]/(0x3006, 0x0026)
  EXPECT_EQ(std::string("LN300"), structures[0].name_);
  EXPECT_EQ(std::string("Cortical Bone"), structures[1].name_);
  EXPECT_EQ(std::string("Adipose"), structures[2].name_);
  EXPECT_EQ(std::string("CB2-50%"), structures[3].name_);
  EXPECT_EQ(std::string("Water"), structures[4].name_);
  EXPECT_EQ(std::string("External"), structures[5].name_);
  EXPECT_EQ(std::string("PTV"), structures[6].name_);

  // (0x3006, 0x0039)[i]/(0x3006, 0x002a)
  EXPECT_EQ(0xff, structures[0].red_);
  EXPECT_EQ(0x00, structures[0].green_);
  EXPECT_EQ(0x00, structures[0].blue_);

  EXPECT_EQ(0x00, structures[1].red_);
  EXPECT_EQ(0xff, structures[1].green_);
  EXPECT_EQ(0xff, structures[1].blue_);

  // ...

  EXPECT_EQ(0x00, structures[5].red_);
  EXPECT_EQ(0x80, structures[5].green_);
  EXPECT_EQ(0x00, structures[5].blue_);

  EXPECT_EQ(0xff, structures[6].red_);
  EXPECT_EQ(0x00, structures[6].green_);
  EXPECT_EQ(0xff, structures[6].blue_);

  /*

   ██████╗ ███████╗ ██████╗ ███╗   ███╗███████╗████████╗██████╗ ██╗   ██╗
  ██╔════╝ ██╔════╝██╔═══██╗████╗ ████║██╔════╝╚══██╔══╝██╔══██╗╚██╗ ██╔╝
  ██║  ███╗█████╗  ██║   ██║██╔████╔██║█████╗     ██║   ██████╔╝ ╚████╔╝
  ██║   ██║██╔══╝  ██║   ██║██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗  ╚██╔╝
  ╚██████╔╝███████╗╚██████╔╝██║ ╚═╝ ██║███████╗   ██║   ██║  ██║   ██║
   ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝   ╚═╝
  http://patorjk.com/software/taag/#p=display&f=ANSI%20Shadow&t=BASIC%20CHECKS
  */


  {
    double groundTruthRaw[] = { 7.657838, 108.2725, 304.01, 6.826687, 107.4413, 304.01, 6.152492, 106.4785, 304.01, 5.655735, 105.4132, 304.01, 5.351513, 104.2778, 304.01, 5.249068, 103.1069, 304.01, 5.351513, 101.9359, 304.01, 5.655735, 100.8005, 304.01, 6.152492, 99.73524, 304.01, 6.826687, 98.77239, 304.01, 7.657838, 97.94124, 304.01, 8.620689, 97.26704, 304.01, 9.685987, 96.77029, 304.01, 10.82136, 96.46606, 304.01, 11.99231, 96.36362, 304.01, 13.16326, 96.46606, 304.01, 14.29864, 96.77029, 304.01, 15.36393, 97.26704, 304.01, 16.32678, 97.94124, 304.01, 17.15794, 98.77239, 304.01, 17.83213, 99.73524, 304.01, 18.32889, 100.8005, 304.01, 18.63311, 101.9359, 304.01, 18.73555, 103.1069, 304.01, 18.63311, 104.2778, 304.01, 18.32889, 105.4132, 304.01, 17.83213, 106.4785, 304.01, 17.15794, 107.4413, 304.01, 16.32678, 108.2725, 304.01, 15.36393, 108.9467, 304.01, 14.29864, 109.4434, 304.01, 13.16326, 109.7477, 304.01, 11.99231, 109.8501, 304.01, 10.82136, 109.7477, 304.01, 9.685987, 109.4434, 304.01, 8.620689, 108.9467, 304.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    CheckGroundTruth(structures, 0, 0, groundTruth);
  }
  {
    double groundTruthRaw[] = { 7.657838, 108.2725, 310.01, 6.826687, 107.4413, 310.01, 6.152492, 106.4785, 310.01, 5.655735, 105.4132, 310.01, 5.351513, 104.2778, 310.01, 5.249068, 103.1069, 310.01, 5.351513, 101.9359, 310.01, 5.655735, 100.8005, 310.01, 6.152492, 99.73524, 310.01, 6.826687, 98.77239, 310.01, 7.657838, 97.94124, 310.01, 8.620689, 97.26704, 310.01, 9.685987, 96.77029, 310.01, 10.82136, 96.46606, 310.01, 11.99231, 96.36362, 310.01, 13.16326, 96.46606, 310.01, 14.29864, 96.77029, 310.01, 15.36393, 97.26704, 310.01, 16.32678, 97.94124, 310.01, 17.15794, 98.77239, 310.01, 17.83213, 99.73524, 310.01, 18.32889, 100.8005, 310.01, 18.63311, 101.9359, 310.01, 18.73555, 103.1069, 310.01, 18.63311, 104.2778, 310.01, 18.32889, 105.4132, 310.01, 17.83213, 106.4785, 310.01, 17.15794, 107.4413, 310.01, 16.32678, 108.2725, 310.01, 15.36393, 108.9467, 310.01, 14.29864, 109.4434, 310.01, 13.16326, 109.7477, 310.01, 11.99231, 109.8501, 310.01, 10.82136, 109.7477, 310.01, 9.685987, 109.4434, 310.01, 8.620689, 108.9467, 310.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    CheckGroundTruth(structures, 0, 2, groundTruth);
  }
  {
    double groundTruthRaw[] = { -37.967, 161.9664, 304.01, -39.10237, 161.6622, 304.01, -40.16767, 161.1655, 304.01, -41.13052, 160.4913, 304.01, -41.96167, 159.6601, 304.01, -42.63587, 158.6973, 304.01, -43.13263, 157.632, 304.01, -43.43685, 156.4966, 304.01, -43.53929, 155.3257, 304.01, -43.43685, 154.1547, 304.01, -43.13263, 153.0193, 304.01, -42.63587, 151.954, 304.01, -41.96167, 150.9912, 304.01, -41.13052, 150.16, 304.01, -40.16767, 149.4858, 304.01, -39.10237, 148.9891, 304.01, -37.967, 148.6849, 304.01, -36.79605, 148.5824, 304.01, -35.6251, 148.6849, 304.01, -34.48972, 148.9891, 304.01, -33.42443, 149.4858, 304.01, -32.46157, 150.16, 304.01, -31.63042, 150.9912, 304.01, -30.95623, 151.954, 304.01, -30.45947, 153.0193, 304.01, -30.15525, 154.1547, 304.01, -30.0528, 155.3257, 304.01, -30.15525, 156.4966, 304.01, -30.45947, 157.632, 304.01, -30.95623, 158.6973, 304.01, -31.63042, 159.6601, 304.01, -32.46157, 160.4913, 304.01, -33.42443, 161.1655, 304.01, -34.48972, 161.6622, 304.01, -35.6251, 161.9664, 304.01, -36.79605, 162.0689, 304.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    CheckGroundTruth(structures, 1, 0, groundTruth);
  }
  {
    double groundTruthRaw[] = { 69.4042, 150.7324, 307.01, 69.70842, 151.8678, 307.01, 69.81087, 153.0387, 307.01, 69.70842, 154.2097, 307.01, 69.4042, 155.345, 307.01, 68.90745, 156.4103, 307.01, 68.23325, 157.3732, 307.01, 67.4021, 158.2043, 307.01, 66.43925, 158.8785, 307.01, 65.37395, 159.3753, 307.01, 64.23858, 159.6795, 307.01, 63.06762, 159.7819, 307.01, 61.89667, 159.6795, 307.01, 60.7613, 159.3753, 307.01, 59.696, 158.8785, 307.01, 58.73315, 158.2043, 307.01, 57.902, 157.3732, 307.01, 57.22781, 156.4103, 307.01, 56.73105, 155.345, 307.01, 56.42683, 154.2097, 307.01, 56.32438, 153.0387, 307.01, 56.42683, 151.8678, 307.01, 56.73105, 150.7324, 307.01, 57.22781, 149.6671, 307.01, 57.902, 148.7042, 307.01, 58.73315, 147.8731, 307.01, 59.696, 147.1989, 307.01, 60.7613, 146.7021, 307.01, 61.89667, 146.3979, 307.01, 63.06762, 146.2955, 307.01, 64.23858, 146.3979, 307.01, 65.37395, 146.7021, 307.01, 66.43925, 147.1989, 307.01, 67.4021, 147.8731, 307.01, 68.23325, 148.7042, 307.01, 68.90745, 149.6671, 307.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    CheckGroundTruth(structures, 2, 1, groundTruth);
  }

  {
    double groundTruthRaw[] = { 108.3984, 232.7406, 274.01, 106.0547, 231.7948, 274.01, 103.7109, 232.8407, 274.01, 96.67969, 232.8757, 274.01, 77.92969, 232.887, 274.01, 47.46094, 232.8902, 274.01, 38.08594, 232.7537, 274.01, 37.6668, 232.3734, 274.01, 38.08594, 231.9774, 274.01, 40.42969, 231.8475, 274.01, 41.76413, 230.0297, 274.01, 42.77344, 229.1388, 274.01, 45.11719, 228.5069, 274.01, 47.46094, 227.1533, 274.01, 49.80469, 226.3505, 274.01, 52.14844, 224.6564, 274.01, 54.49219, 223.923, 274.01, 56.83594, 222.0692, 274.01, 59.17969, 220.3438, 274.01, 61.52344, 219.3888, 274.01, 63.86719, 217.1287, 274.01, 65.83488, 215.9672, 274.01, 68.55469, 213.2383, 274.01, 70.89844, 211.2328, 274.01, 72.8125, 208.9359, 274.01, 75.58594, 206.3615, 274.01, 76.91445, 204.2484, 274.01, 78.89509, 201.9047, 274.01, 80.51276, 199.5609, 274.01, 81.51955, 197.2172, 274.01, 83.67448, 194.8734, 274.01, 84.60938, 192.5297, 274.01, 85.86986, 190.1859, 274.01, 86.57623, 187.8422, 274.01, 88.30051, 185.4984, 274.01, 88.94002, 183.1547, 274.01, 89.23261, 180.8109, 274.01, 89.64844, 180.3263, 274.01, 90.71885, 178.4672, 274.01, 90.97656, 176.1234, 274.01, 91.99219, 174.4794, 274.01, 92.56773, 173.7797, 274.01, 92.80016, 171.4359, 274.01, 93.23473, 169.0922, 274.01, 93.37606, 166.7484, 274.01, 93.60748, 157.3734, 274.01, 93.6341, 152.6859, 274.01, 93.35742, 140.9672, 274.01, 92.89317, 138.6234, 274.01, 92.7069, 136.2797, 274.01, 92.03726, 133.9359, 274.01, 90.84009, 131.5922, 274.01, 90.3769, 129.2484, 274.01, 89.09074, 126.9047, 274.01, 88.13225, 122.2172, 274.01, 86.17828, 119.8734, 274.01, 84.96094, 117.4163, 274.01, 83.99619, 115.1859, 274.01, 83.13079, 112.8422, 274.01, 82.61719, 112.2984, 274.01, 80.27344, 108.8454, 274.01, 79.64514, 108.1547, 274.01, 77.21497, 105.8109, 274.01, 76.47787, 103.4672, 274.01, 75.58594, 102.6177, 274.01, 73.24219, 100.0077, 274.01, 69.54492, 96.43594, 274.01, 67.34096, 94.09219, 274.01, 64.66306, 91.74844, 274.01, 63.86719, 90.92619, 274.01, 61.52344, 90.20454, 274.01, 59.17969, 87.78574, 274.01, 56.83594, 86.48566, 274.01, 54.49219, 84.31388, 274.01, 52.14844, 83.44438, 274.01, 49.80469, 82.75121, 274.01, 49.37617, 82.37344, 274.01, 47.46094, 81.26244, 274.01, 45.71391, 80.02969, 274.01, 45.11719, 79.45415, 274.01, 42.77344, 79.08185, 274.01, 40.42969, 78.51941, 274.01, 38.08594, 78.27534, 274.01, 37.36932, 77.68594, 274.01, 35.74219, 76.67624, 274.01, 33.39844, 76.49941, 274.01, 31.05469, 76.03495, 274.01, 28.71094, 74.83174, 274.01, 26.36719, 74.62859, 274.01, 24.02344, 74.55463, 274.01, 21.67969, 74.22861, 274.01, 19.33594, 74.05312, 274.01, 12.30469, 73.99397, 274.01, 5.273438, 74.0736, 274.01, 2.929688, 74.55463, 274.01, 0.5859375, 74.68513, 274.01, -1.757813, 74.914, 274.01, -2.319131, 75.34219, 274.01, -4.101563, 76.31516, 274.01, -8.789063, 76.74514, 274.01, -11.13281, 78.39038, 274.01, -13.47656, 78.6124, 274.01, -15.82031, 79.19784, 274.01, -18.16406, 81.11024, 274.01, -20.50781, 82.03296, 274.01, -22.85156, 83.13991, 274.01, -25.19531, 83.70732, 274.01, -27.53906, 85.85863, 274.01, -29.88281, 87.03368, 274.01, -32.22656, 88.3274, 274.01, -34.57031, 90.53674, 274.01, -36.91406, 92.5602, 274.01, -39.25781, 93.55952, 274.01, -41.60156, 95.74537, 274.01, -43.94531, 98.26609, 274.01, -46.28906, 100.3701, 274.01, -47.02621, 101.1234, 274.01, -47.86611, 103.4672, 274.01, -49.83594, 105.8109, 274.01, -51.98182, 108.1547, 274.01, -53.06448, 110.4984, 274.01, -53.32031, 110.7675, 274.01, -54.53804, 112.8422, 274.01, -55.66406, 114.273, 274.01, -56.55722, 115.1859, 274.01, -57.13953, 117.5297, 274.01, -58.29264, 119.8734, 274.01, -59.26869, 122.2172, 274.01, -60.35156, 124.0119, 274.01, -60.84229, 124.5609, 274.01, -61.54484, 126.9047, 274.01, -61.71691, 129.2484, 274.01, -63.62281, 131.5922, 274.01, -63.81256, 133.9359, 274.01, -64.12511, 136.2797, 274.01, -64.84515, 138.6234, 274.01, -65.13599, 140.9672, 274.01, -65.33604, 143.3109, 274.01, -65.87358, 145.6547, 274.01, -66.10577, 147.9984, 274.01, -66.17618, 155.0297, 274.01, -66.09933, 162.0609, 274.01, -65.40382, 164.4047, 274.01, -65.24833, 166.7484, 274.01, -64.71442, 171.4359, 274.01, -63.88171, 173.7797, 274.01, -63.69299, 176.1234, 274.01, -61.79081, 178.4672, 274.01, -61.59269, 180.8109, 274.01, -61.19405, 183.1547, 274.01, -60.35156, 185.2055, 274.01, -59.08288, 187.8422, 274.01, -58.00781, 189.3499, 274.01, -57.25858, 190.1859, 274.01, -56.64558, 192.5297, 274.01, -55.29191, 194.8734, 274.01, -54.28698, 197.2172, 274.01, -52.28595, 199.5609, 274.01, -51.47569, 201.9047, 274.01, -48.63281, 204.6417, 274.01, -47.10181, 206.5922, 274.01, -44.64154, 208.9359, 274.01, -42.38504, 211.2797, 274.01, -39.25781, 214.4025, 274.01, -37.42723, 215.9672, 274.01, -34.57031, 218.9107, 274.01, -32.22656, 219.7277, 274.01, -29.88281, 221.6934, 274.01, -27.53906, 222.852, 274.01, -25.19531, 224.5168, 274.01, -22.85156, 225.9419, 274.01, -20.50781, 226.7359, 274.01, -18.16406, 228.3332, 274.01, -15.82031, 229.065, 274.01, -13.47656, 229.267, 274.01, -12.63854, 230.0297, 274.01, -11.13281, 231.9201, 274.01, -10.65505, 232.3734, 274.01, -11.13281, 232.7794, 274.01, -15.82031, 232.792, 274.01, -18.16406, 232.8902, 274.01, -36.91406, 232.9015, 274.01, -39.25781, 232.8902, 274.01, -50.97656, 232.9236, 274.01, -60.35156, 232.9126, 274.01, -67.38281, 232.8407, 274.01, -72.07031, 232.8642, 274.01, -79.10156, 232.8555, 274.01, -83.78906, 232.8788, 274.01, -95.50781, 232.8902, 274.01, -97.85156, 233.4886, 274.01, -100.1953, 233.647, 274.01, -102.5391, 232.9858, 274.01, -104.8828, 233.6969, 274.01, -109.5703, 233.722, 274.01, -125.9766, 233.7086, 274.01, -128.3203, 233.2849, 274.01, -130.6641, 233.702, 274.01, -135.3516, 233.727, 274.01, -149.4141, 233.7135, 274.01, -156.4453, 233.727, 274.01, -163.4766, 233.7119, 274.01, -168.1641, 233.7643, 274.01, -191.6016, 233.7809, 274.01, -210.3516, 233.7716, 274.01, -224.4141, 233.7998, 274.01, -233.7891, 233.7647, 274.01, -243.1641, 233.7785, 274.01, -247.8516, 233.7378, 274.01, -254.8828, 233.8578, 274.01, -257.2266, 235.2519, 274.01, -259.5703, 236.0817, 274.01, -260.7617, 237.0609, 274.01, -261.9141, 238.2262, 274.01, -262.8989, 239.4047, 274.01, -262.9743, 241.7484, 274.01, -262.5977, 244.0922, 274.01, -260.6675, 246.4359, 274.01, -259.6161, 248.7797, 274.01, -257.2266, 251.0035, 274.01, -255.0361, 253.4672, 274.01, -252.5391, 256.0995, 274.01, -251.2277, 258.1547, 274.01, -246.7444, 262.8422, 274.01, -243.1641, 266.3515, 274.01, -239.7411, 269.8734, 274.01, -238.4766, 270.9495, 274.01, -237.2269, 272.2172, 274.01, -236.1328, 273.5215, 274.01, -235.0934, 274.5609, 274.01, -233.7891, 275.6655, 274.01, -232.5319, 276.9047, 274.01, -231.4453, 278.1693, 274.01, -227.917, 281.5922, 274.01, -224.4141, 285.1802, 274.01, -222.0703, 287.4025, 274.01, -218.6841, 290.9672, 274.01, -217.3828, 291.9709, 274.01, -215.0391, 293.1788, 274.01, -212.6953, 294.5138, 274.01, -210.3516, 295.2614, 274.01, -209.8994, 295.6547, 274.01, -208.0078, 296.7083, 274.01, -203.3203, 296.9372, 274.01, -196.2891, 296.9317, 274.01, -193.9453, 296.8988, 274.01, -172.8516, 296.8482, 274.01, -161.1328, 296.843, 274.01, -137.6953, 296.8542, 274.01, -130.6641, 296.8378, 274.01, -107.2266, 296.8379, 274.01, -93.16406, 296.8208, 274.01, -74.41406, 296.838, 274.01, -65.03906, 296.8609, 274.01, -50.97656, 296.8556, 274.01, -46.28906, 296.9051, 274.01, -41.60156, 298.5331, 274.01, -39.25781, 298.5624, 274.01, -36.91406, 297.1455, 274.01, -34.57031, 297.0498, 274.01, -32.22656, 298.5589, 274.01, -25.19531, 298.5624, 274.01, -22.85156, 297.2842, 274.01, -20.50781, 298.5624, 274.01, -1.757813, 298.5624, 274.01, 0.5859375, 297.2104, 274.01, 2.929688, 298.5624, 274.01, 5.273438, 297.6946, 274.01, 7.617188, 298.5168, 274.01, 9.960938, 298.5512, 274.01, 12.30469, 296.937, 274.01, 14.64844, 298.5478, 274.01, 16.99219, 298.5478, 274.01, 19.33594, 297.0782, 274.01, 21.67969, 296.844, 274.01, 23.54531, 297.9984, 274.01, 24.02344, 298.4023, 274.01, 24.50156, 297.9984, 274.01, 26.36719, 296.844, 274.01, 38.08594, 296.8381, 274.01, 52.14844, 296.8033, 274.01, 59.17969, 296.8033, 274.01, 73.24219, 296.7682, 274.01, 99.02344, 296.7566, 274.01, 117.7734, 296.7216, 274.01, 129.4922, 296.7152, 274.01, 131.8359, 295.9083, 274.01, 134.1797, 295.5245, 274.01, 138.8672, 295.4763, 274.01, 155.2734, 295.4763, 274.01, 176.3672, 295.3861, 274.01, 190.4297, 295.3718, 274.01, 197.4609, 295.4763, 274.01, 202.1484, 295.4454, 274.01, 204.4922, 295.3438, 274.01, 206.8359, 295.0757, 274.01, 209.1797, 294.4124, 274.01, 211.5234, 292.3133, 274.01, 213.8672, 291.0809, 274.01, 216.2109, 289.6743, 274.01, 217.3081, 288.6234, 274.01, 219.3558, 286.2797, 274.01, 221.8608, 283.9359, 274.01, 225.5859, 280.045, 274.01, 227.9297, 277.8885, 274.01, 230.2734, 275.2857, 274.01, 232.6172, 273.2225, 274.01, 233.6225, 272.2172, 274.01, 234.9609, 270.5822, 274.01, 238.2254, 267.5297, 274.01, 240.3691, 265.1859, 274.01, 244.3359, 261.3326, 274.01, 246.6797, 258.8034, 274.01, 249.0234, 256.7196, 274.01, 251.3672, 254.0746, 274.01, 254.5313, 251.1234, 274.01, 255.333, 248.7797, 274.01, 257.3723, 246.4359, 274.01, 259.7201, 244.0922, 274.01, 260.106, 241.7484, 274.01, 261.6423, 239.4047, 274.01, 261.0804, 237.0609, 274.01, 259.3552, 234.7172, 274.01, 258.3984, 233.7696, 274.01, 256.0547, 232.8757, 274.01, 253.7109, 232.792, 274.01, 251.3672, 232.8161, 274.01, 246.6797, 232.6981, 274.01, 244.3359, 232.725, 274.01, 239.6484, 232.9137, 274.01, 234.9609, 232.8525, 274.01, 225.5859, 232.8757, 274.01, 209.1797, 232.8757, 274.01, 204.4922, 232.7537, 274.01, 195.1172, 232.7794, 274.01, 171.6797, 232.792, 274.01, 164.6484, 232.7666, 274.01, 152.9297, 232.7666, 274.01, 148.2422, 232.792, 274.01, 138.8672, 232.7406, 274.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    EXPECT_EQ(340u * 3, groundTruth.size());
    CheckGroundTruth(structures, 5, 0, groundTruth);
  }

  {
    double groundTruthRaw[] = { -18.16406, 233.0632, 298.01, -27.53906, 233.1042, 298.01, -29.88281, 233.0819, 298.01, -34.57031, 233.131, 298.01, -43.94531, 233.1221, 298.01, -50.97656, 233.1736, 298.01, -62.69531, 233.1397, 298.01, -65.03906, 232.8376, 298.01, -69.72656, 232.9839, 298.01, -79.10156, 233.0245, 298.01, -90.82031, 233.0382, 298.01, -93.16406, 233.0859, 298.01, -109.5703, 233.1132, 298.01, -111.9141, 233.1791, 298.01, -114.2578, 233.7139, 298.01, -118.9453, 233.9793, 298.01, -128.3203, 234.0284, 298.01, -130.6641, 233.9793, 298.01, -135.3516, 234.0591, 298.01, -137.6953, 234.0284, 298.01, -142.3828, 234.0855, 298.01, -144.7266, 234.0284, 298.01, -151.7578, 234.002, 298.01, -158.7891, 234.0263, 298.01, -163.4766, 233.9784, 298.01, -165.8203, 234.0072, 298.01, -168.1641, 234.1756, 298.01, -170.5078, 234.2214, 298.01, -179.8828, 234.1934, 298.01, -186.9141, 234.2721, 298.01, -189.2578, 234.2289, 298.01, -193.9453, 234.2431, 298.01, -198.6328, 234.1692, 298.01, -200.9766, 234.2326, 298.01, -205.6641, 234.1271, 298.01, -212.6953, 234.2224, 298.01, -215.0391, 234.1992, 298.01, -222.0703, 234.3115, 298.01, -224.4141, 234.2224, 298.01, -226.7578, 234.2502, 298.01, -233.7891, 234.0906, 298.01, -238.4766, 234.0329, 298.01, -243.1641, 234.0283, 298.01, -247.8516, 233.7949, 298.01, -250.1953, 233.8681, 298.01, -252.5391, 234.7626, 298.01, -254.3469, 237.0609, 298.01, -255.6034, 239.4047, 298.01, -254.5181, 241.7484, 298.01, -254.2274, 244.0922, 298.01, -254.181, 248.7797, 298.01, -253.9355, 251.1234, 298.01, -253.5926, 253.4672, 298.01, -252.7483, 255.8109, 298.01, -250.8092, 258.1547, 298.01, -248.713, 260.4984, 298.01, -246.263, 262.8422, 298.01, -244.1406, 265.1859, 298.01, -241.6671, 267.5297, 298.01, -239.4754, 269.8734, 298.01, -237.0156, 272.2172, 298.01, -233.7891, 275.382, 298.01, -231.4453, 277.8249, 298.01, -229.1016, 279.9981, 298.01, -226.7578, 282.5281, 298.01, -224.4141, 284.6784, 298.01, -222.0703, 287.2355, 298.01, -220.5414, 288.6234, 298.01, -218.2745, 290.9672, 298.01, -217.3828, 291.6508, 298.01, -212.6953, 294.5949, 298.01, -210.3516, 295.3142, 298.01, -208.0078, 296.4674, 298.01, -205.6641, 296.8852, 298.01, -203.3203, 297.1563, 298.01, -196.2891, 297.1488, 298.01, -193.9453, 297.0597, 298.01, -182.2266, 296.9529, 298.01, -168.1641, 296.8576, 298.01, -154.1016, 296.9249, 298.01, -149.4141, 296.8921, 298.01, -128.3203, 296.9228, 298.01, -121.2891, 296.8623, 298.01, -111.9141, 296.8549, 298.01, -107.2266, 296.8266, 298.01, -102.5391, 296.8731, 298.01, -95.50781, 296.8453, 298.01, -88.47656, 296.9218, 298.01, -83.78906, 296.9016, 298.01, -69.72656, 296.979, 298.01, -67.38281, 296.9514, 298.01, -65.03906, 297.2199, 298.01, -62.69531, 296.9622, 298.01, -55.66406, 296.9926, 298.01, -50.97656, 296.9467, 298.01, -48.63281, 297.3652, 298.01, -46.28906, 297.0439, 298.01, -43.94531, 297.2875, 298.01, -39.25781, 297.0121, 298.01, -34.57031, 297.1564, 298.01, -32.22656, 297.3612, 298.01, -29.88281, 297.4229, 298.01, -27.53906, 297.1687, 298.01, -25.19531, 297.4334, 298.01, -18.16406, 297.3612, 298.01, -15.82031, 297.4441, 298.01, -13.47656, 297.4125, 298.01, -11.13281, 297.2468, 298.01, -8.789063, 297.4125, 298.01, -6.445313, 297.373, 298.01, -4.101563, 297.4195, 298.01, -1.757813, 297.077, 298.01, 0.5859375, 297.4229, 298.01, 2.929688, 297.4125, 298.01, 5.273438, 296.9489, 298.01, 7.617188, 297.3168, 298.01, 9.960938, 296.9377, 298.01, 12.30469, 296.8998, 298.01, 14.64844, 297.1975, 298.01, 16.99219, 296.8579, 298.01, 28.71094, 296.878, 298.01, 40.42969, 296.8163, 298.01, 42.77344, 296.8369, 298.01, 49.80469, 296.734, 298.01, 59.17969, 296.6906, 298.01, 61.52344, 296.6365, 298.01, 68.55469, 296.6278, 298.01, 73.24219, 296.5777, 298.01, 75.58594, 296.6191, 298.01, 84.96094, 296.5284, 298.01, 96.67969, 296.5538, 298.01, 103.7109, 296.479, 298.01, 115.4297, 296.4259, 298.01, 122.4609, 296.3434, 298.01, 129.4922, 296.3495, 298.01, 131.8359, 295.9141, 298.01, 136.5234, 296.2256, 298.01, 138.8672, 295.833, 298.01, 143.5547, 295.9857, 298.01, 145.8984, 295.8791, 298.01, 152.9297, 295.833, 298.01, 164.6484, 295.6819, 298.01, 171.6797, 295.6819, 298.01, 181.0547, 295.5401, 298.01, 185.7422, 295.5742, 298.01, 192.7734, 295.557, 298.01, 197.4609, 295.8012, 298.01, 202.1484, 295.6819, 298.01, 204.4922, 295.3698, 298.01, 206.8359, 294.803, 298.01, 209.1797, 294.3656, 298.01, 211.5234, 292.4764, 298.01, 213.8672, 291.1765, 298.01, 216.2109, 289.5873, 298.01, 217.229, 288.6234, 298.01, 218.5547, 287.0752, 298.01, 221.7097, 283.9359, 298.01, 225.5859, 279.8775, 298.01, 227.9297, 277.5633, 298.01, 230.2734, 275.0808, 298.01, 233.1989, 272.2172, 298.01, 234.9609, 270.2887, 298.01, 237.7384, 267.5297, 298.01, 241.9922, 263.0843, 298.01, 244.3359, 260.7643, 298.01, 246.788, 258.1547, 298.01, 249.0234, 255.451, 298.01, 250.3651, 253.4672, 298.01, 251.5297, 251.1234, 298.01, 252.1947, 248.7797, 298.01, 252.4915, 246.4359, 298.01, 252.5755, 241.7484, 298.01, 252.8592, 239.4047, 298.01, 252.9236, 237.0609, 298.01, 252.2924, 234.7172, 298.01, 251.3672, 233.4697, 298.01, 249.0234, 232.882, 298.01, 244.3359, 232.9048, 298.01, 241.9922, 233.0145, 298.01, 232.6172, 232.9048, 298.01, 227.9297, 233.0007, 298.01, 216.2109, 233.0632, 298.01, 211.5234, 233.0537, 298.01, 206.8359, 232.9699, 298.01, 204.4922, 232.7322, 298.01, 199.8047, 232.7186, 298.01, 190.4297, 232.7719, 298.01, 183.3984, 232.7719, 298.01, 181.0547, 232.7322, 298.01, 174.0234, 232.7048, 298.01, 171.6797, 232.7322, 298.01, 166.9922, 232.6908, 298.01, 157.6172, 232.7975, 298.01, 155.2734, 232.7588, 298.01, 148.2422, 232.7875, 298.01, 143.5547, 232.7614, 298.01, 138.8672, 232.6477, 298.01, 124.8047, 232.6179, 298.01, 122.4609, 232.6477, 298.01, 113.0859, 232.6027, 298.01, 110.7422, 232.4552, 298.01, 108.3984, 232.2192, 298.01, 106.0547, 231.6764, 298.01, 103.7109, 231.8559, 298.01, 102.8237, 232.3734, 298.01, 101.3672, 232.9839, 298.01, 99.02344, 233.0951, 298.01, 87.30469, 233.0819, 298.01, 84.96094, 233.1091, 298.01, 80.27344, 233.0726, 298.01, 77.92969, 233.1132, 298.01, 70.89844, 233.1397, 298.01, 68.55469, 233.1132, 298.01, 52.14844, 233.131, 298.01, 45.11719, 233.0859, 298.01, 44.16726, 232.3734, 298.01, 42.77344, 231.0206, 298.01, 42.04498, 230.0297, 298.01, 42.77344, 229.2462, 298.01, 45.11719, 228.5664, 298.01, 47.46094, 227.0695, 298.01, 49.80469, 226.0552, 298.01, 52.14844, 224.5723, 298.01, 54.49219, 223.6857, 298.01, 56.83594, 221.8519, 298.01, 59.17969, 220.2086, 298.01, 61.52344, 218.8854, 298.01, 64.94469, 215.9672, 298.01, 66.21094, 215.0191, 298.01, 67.72036, 213.6234, 298.01, 68.55469, 212.6986, 298.01, 70.89844, 210.5055, 298.01, 74.53191, 206.5922, 298.01, 76.54903, 204.2484, 298.01, 78.26105, 201.9047, 298.01, 80.27344, 198.9262, 298.01, 82.61719, 195.2822, 298.01, 82.98087, 194.8734, 298.01, 84.96094, 190.9255, 298.01, 85.43701, 190.1859, 298.01, 86.33423, 187.8422, 298.01, 87.78722, 185.4984, 298.01, 88.60233, 183.1547, 298.01, 89.10253, 180.8109, 298.01, 90.17504, 178.4672, 298.01, 90.88959, 176.1234, 298.01, 91.43783, 173.7797, 298.01, 92.39601, 171.4359, 298.01, 92.95762, 169.0922, 298.01, 93.55695, 159.7172, 298.01, 93.65527, 157.3734, 298.01, 93.67542, 152.6859, 298.01, 93.61213, 150.3422, 298.01, 93.22542, 143.3109, 298.01, 93.06345, 140.9672, 298.01, 92.77563, 138.6234, 298.01, 91.21714, 133.9359, 298.01, 90.67235, 131.5922, 298.01, 89.88776, 129.2484, 298.01, 88.8737, 126.9047, 298.01, 88.44087, 124.5609, 298.01, 86.09712, 119.8734, 298.01, 85.05786, 117.5297, 298.01, 83.87151, 115.1859, 298.01, 82.22388, 112.8422, 298.01, 81.09117, 110.4984, 298.01, 77.92969, 106.4052, 298.01, 77.3894, 105.8109, 298.01, 75.94332, 103.4672, 298.01, 71.71799, 98.77969, 298.01, 68.55469, 95.65721, 298.01, 63.86719, 91.54878, 298.01, 61.52344, 90.1121, 298.01, 59.17969, 88.15762, 298.01, 56.83594, 86.51503, 298.01, 54.49219, 85.42721, 298.01, 52.14844, 83.64907, 298.01, 49.80469, 82.89023, 298.01, 47.46094, 81.50237, 298.01, 45.11719, 80.62591, 298.01, 42.77344, 79.18153, 298.01, 40.42969, 78.7203, 298.01, 38.08594, 78.1349, 298.01, 35.74219, 77.11755, 298.01, 33.39844, 76.51949, 298.01, 31.05469, 76.07934, 298.01, 26.36719, 74.67744, 298.01, 24.02344, 74.42056, 298.01, 14.64844, 74.07317, 298.01, 9.960938, 74.11538, 298.01, 2.929688, 74.40105, 298.01, 0.5859375, 74.67952, 298.01, -1.757813, 75.31406, 298.01, -4.101563, 76.07065, 298.01, -6.445313, 76.49051, 298.01, -8.789063, 77.17276, 298.01, -11.13281, 78.20097, 298.01, -15.82031, 79.31967, 298.01, -18.16406, 80.76948, 298.01, -20.50781, 81.64266, 298.01, -22.85156, 83.0305, 298.01, -25.19531, 83.7937, 298.01, -27.53906, 85.63515, 298.01, -29.88281, 86.7363, 298.01, -32.22656, 88.36089, 298.01, -34.57031, 90.3302, 298.01, -36.56719, 91.74844, 298.01, -41.60156, 95.93605, 298.01, -46.58845, 101.1234, 298.01, -50.17995, 105.8109, 298.01, -52.10386, 108.1547, 298.01, -53.63992, 110.4984, 298.01, -54.95532, 112.8422, 298.01, -56.64794, 115.1859, 298.01, -57.4403, 117.5297, 298.01, -58.91927, 119.8734, 298.01, -59.78655, 122.2172, 298.01, -61.11754, 124.5609, 298.01, -61.58921, 126.9047, 298.01, -62.38012, 129.2484, 298.01, -63.49118, 131.5922, 298.01, -64.02599, 133.9359, 298.01, -64.3932, 136.2797, 298.01, -65.11897, 138.6234, 298.01, -65.64544, 140.9672, 298.01, -66.23938, 147.9984, 298.01, -66.46289, 152.6859, 298.01, -66.48911, 155.0297, 298.01, -66.34437, 159.7172, 298.01, -65.99894, 164.4047, 298.01, -65.49149, 169.0922, 298.01, -64.6875, 171.4359, 298.01, -63.7739, 176.1234, 298.01, -62.9398, 178.4672, 298.01, -61.86011, 180.8109, 298.01, -61.33423, 183.1547, 298.01, -60.43332, 185.4984, 298.01, -58.00781, 190.0632, 298.01, -56.85406, 192.5297, 298.01, -55.66406, 194.7283, 298.01, -54.11692, 197.2172, 298.01, -50.97656, 201.8369, 298.01, -47.36435, 206.5922, 298.01, -45.04395, 208.9359, 298.01, -42.83026, 211.2797, 298.01, -39.25781, 214.7435, 298.01, -34.57031, 218.4974, 298.01, -32.22656, 219.9595, 298.01, -28.02053, 222.9984, 298.01, -27.53906, 223.4238, 298.01, -25.19531, 224.4187, 298.01, -22.85156, 225.8252, 298.01, -20.50781, 226.9067, 298.01, -18.16406, 228.4286, 298.01, -15.82031, 229.1235, 298.01, -14.9447, 230.0297, 298.01, -15.82031, 231.3969, 298.01, -16.94484, 232.3734, 298.01 };
    size_t n = sizeof(groundTruthRaw) / sizeof(groundTruthRaw[0]); 
    std::vector<double> groundTruth(groundTruthRaw, groundTruthRaw+n);
    EXPECT_EQ(358u * 3, groundTruth.size());
    CheckGroundTruth(structures, 5, 8, groundTruth);
  }
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#if 0

TEST(StructureSet, ReadFromJsonAndCompute1)
{
  DicomStructureSet2 structureSet;

  OrthancPlugins::FullOrthancDataset dicom(GetTestJson());
  //loader.content_.reset(new DicomStructureSet(dicom));
  structureSet.Clear();

  structureSet.FillStructuresFromDataset(dicom);

  structureSet.ComputeDependentProperties();
}

TEST(StructureSet, ReadFromJsonAndCompute2)
{
  DicomStructureSet2 structureSet;

  OrthancPlugins::FullOrthancDataset dicom(GetTestJson());
  //loader.content_.reset(new DicomStructureSet(dicom));
  structureSet.Clear();

  structureSet.SetContents(dicom);
}
#endif

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

static bool CutStructureWithPlane(
  std::vector< std::pair<Point2D, Point2D> >& segments,
  const DicomStructure2& structure,
  const double originX, const double originY, const double originZ,
  const double axisX_X, const double axisX_Y, const double axisX_Z,
  const double axisY_X, const double axisY_Y, const double axisY_Z
)
{
  // create an AXIAL cutting plane, too far away from the volume 
  // (> sliceThickness/2)
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, originX, originY, originZ);
  LinearAlgebra::AssignVector(axisX, axisX_X, axisX_Y, axisX_Z);
  LinearAlgebra::AssignVector(axisY, axisY_X, axisY_Y, axisY_Z);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  // compute intersection
  bool ok = structure.Project(segments, cuttingPlane);
  return ok;
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2

static double pointsCoord1[] = { 2, 2, 3,  3,  6,  8, 8, 7, 8, 8, 6 };
static double pointsCoord2[] = { 2, 6, 8, 10, 12, 10, 8, 6, 4, 2, 4 };
static const size_t pointsCoord1Count = STONE_ARRAY_SIZE(pointsCoord1);
static const size_t pointsCoord2Count = STONE_ARRAY_SIZE(pointsCoord2);
const size_t POLYGON_POINT_COUNT = pointsCoord1Count;

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

static void CreateBasicStructure(DicomStructure2& structure)
{
  // see https://www.dropbox.com/s/1o1vg53hsbvx4cc/test-rtstruct-polygons.jpg?dl=0
  EXPECT_EQ(pointsCoord1Count, pointsCoord2Count);
  EXPECT_EQ(11, pointsCoord2Count);

  for (size_t slice = 0; slice < 3; ++slice)
  {
    DicomStructurePolygon2 polygon("Oblomptu", "CLOSED_PLANAR");
    for (size_t ip = 0; ip < pointsCoord1Count; ++ip)
    {
      Point3D pt;
      double pt0 = pointsCoord1[ip];
      double pt1 = pointsCoord2[ip];
      double pt2 = 4 * (static_cast<double>(slice) - 1); // -4, 0, 4 
      LinearAlgebra::AssignVector(pt, pt0, pt1, pt2);
      polygon.AddPoint(pt);
    }
    structure.AddPolygon(polygon);
  }
  structure.ComputeDependentProperties();
}


TEST(StructureSet, CutAxialOutsideTop)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an AXIAL cutting plane, too far away from the volume 
  // (> sliceThickness/2)
  bool ok = CutStructureWithPlane(segments, structure,
    0, 0, 7,
    1, 0, 0,
    0, 1, 0);
  EXPECT_FALSE(ok);
}


TEST(StructureSet, CutAxialOutsideBottom)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an AXIAL cutting plane, too far away from the volume 
  // (> sliceThickness/2)
  bool ok = CutStructureWithPlane(segments, structure,
    0, 0, -6.66,
    1, 0, 0,
    0, 1, 0);
  EXPECT_FALSE(ok);
}

TEST(StructureSet, CutAxialInsideClose)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an AXIAL cutting plane in the volume
  bool ok = CutStructureWithPlane(segments, structure,
    0, 0, 1.1,
    1, 0, 0,
    0, 1, 0);
  EXPECT_TRUE(ok);
  EXPECT_EQ(POLYGON_POINT_COUNT, segments.size());

  for (size_t i = 0; i < segments.size(); ++i)
  {
    EXPECT_LT(i, POLYGON_POINT_COUNT);
    EXPECT_LT(i, POLYGON_POINT_COUNT);

    const Point2D& pt = segments[i].first;

    // ...should be at the same location as the 3D coords since the plane 
    // is rooted at 0,0,0 with normal 0,0,1
    EXPECT_NEAR(pt.x, pointsCoord1[i], DELTA_MAX);
    EXPECT_NEAR(pt.y, pointsCoord2[i], DELTA_MAX);
  }
}


TEST(StructureSet, CutAxialInsideFar)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an AXIAL cutting plane
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, 0, 0, 0);
  LinearAlgebra::AssignVector(axisX, 1, 0, 0);
  LinearAlgebra::AssignVector(axisY, 0, 1, 0);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  // compute intersection
  bool ok = structure.Project(segments, cuttingPlane);
  EXPECT_TRUE(ok);

  EXPECT_EQ(11, segments.size());
  for (size_t i = 0; i < segments.size(); ++i)
  {
    EXPECT_LT(i, pointsCoord1Count);
    EXPECT_LT(i, pointsCoord2Count);

    // the 2D points of the projected polygon
    const Point2D& pt = segments[i].first;

    // ...should be at the same location as the 3D coords since the plane 
    // is rooted at 0,0,0 with normal 0,0,1
    EXPECT_NEAR(pt.x, pointsCoord1[i], DELTA_MAX);
    EXPECT_NEAR(pt.y, pointsCoord2[i], DELTA_MAX);
  }
}

TEST(StructureSet, CutCoronalOutsideClose)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an X,Z cutting plane, outside of the volume
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, 0, 0, 0);
  LinearAlgebra::AssignVector(axisX, 1, 0, 0);
  LinearAlgebra::AssignVector(axisY, 0, 0, 1);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  // compute intersection
  bool ok = structure.Project(segments, cuttingPlane);
  EXPECT_FALSE(ok);
}

TEST(StructureSet, CutCoronalInsideClose1DTest)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);

  // create an X,Z cutting plane, outside of the volume
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, 0, 3, 0);
  LinearAlgebra::AssignVector(axisX, 1, 0, 0);
  LinearAlgebra::AssignVector(axisY, 0, 0, 1);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  ASSERT_EQ(3u, structure.GetPolygons().size());

  for (int i = 0; i < 3; ++i)
  {
    double polygonZ = static_cast<double>(i - 1) * 4.0;

    const DicomStructurePolygon2& topSlab = structure.GetPolygons()[i];

    // let's compute the intersection between the polygon and the plane
    // intersections are in plane coords
    std::vector<Point2D> intersects;
    topSlab.ProjectOnConstantPlane(intersects, cuttingPlane);

    ASSERT_EQ(4u, intersects.size());

    EXPECT_NEAR(2, intersects[0].x, DELTA_MAX);
    EXPECT_NEAR(4, intersects[1].x, DELTA_MAX);
    EXPECT_NEAR(7, intersects[2].x, DELTA_MAX);
    EXPECT_NEAR(8, intersects[3].x, DELTA_MAX);

    for (size_t i = 0; i < 4u; ++i)
    {
      EXPECT_NEAR(polygonZ, intersects[i].y, DELTA_MAX);
    }
  }
}

TEST(StructureSet, CutCoronalInsideClose)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an X,Z cutting plane, outside of the volume
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, 0, 3, 0);
  LinearAlgebra::AssignVector(axisX, 1, 0, 0);
  LinearAlgebra::AssignVector(axisY, 0, 0, 1);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  // compute intersection
  ASSERT_TRUE(structure.Project(segments, cuttingPlane));
  EXPECT_EQ(24, segments.size());

  size_t numberOfVeryShortSegments = 0;
  for (size_t iSegment = 0; iSegment < segments.size(); ++iSegment)
  {
    // count the NON vertical very short segments 
    if (LinearAlgebra::IsNear(segments[iSegment].first.x, segments[iSegment].second.x))
    {
      if (LinearAlgebra::IsNear(segments[iSegment].first.y, segments[iSegment].second.y))
      {
        numberOfVeryShortSegments++;
      }
    }
  }
  EXPECT_EQ(8, numberOfVeryShortSegments);
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


TEST(DisjointDataSet, BasicTest)
{
  const size_t ITEM_COUNT = 10;
  DisjointDataSet ds(ITEM_COUNT);

  for (size_t i = 0; i < ITEM_COUNT; ++i)
  {
    EXPECT_EQ(i, ds.Find(i));
  }

  ds.Union(0, 4);
  EXPECT_EQ(0u, ds.Find(0));
  EXPECT_EQ(0u, ds.Find(4));

  ds.Union(4, 6);
  ds.Union(8, 9);
  ds.Union(0, 8);

  for (size_t i = 0; i < ITEM_COUNT; ++i)
  {
    size_t parent = ds.Find(i);
    EXPECT_TRUE(0 == parent || 1 == parent || 2 == parent || 3 == parent || 5 == parent || 7 == parent);
  }

  ds.Union(1, 2);
  ds.Union(1, 7);
  for (size_t i = 0; i < ITEM_COUNT; ++i)
  {
    size_t parent = ds.Find(i);
    EXPECT_TRUE(0 == parent || 1 == parent || 3 == parent || 5 == parent);
  }

  ds.Union(3, 5);
  for (size_t i = 0; i < ITEM_COUNT; ++i)
  {
    size_t parent = ds.Find(i);
    EXPECT_TRUE(0 == parent || 1 == parent || 3 == parent);
  }

  EXPECT_EQ(ds.Find(0), ds.Find(0));
  EXPECT_EQ(ds.Find(0), ds.Find(4));
  EXPECT_EQ(ds.Find(0), ds.Find(6));
  EXPECT_EQ(ds.Find(0), ds.Find(8));
  EXPECT_EQ(ds.Find(0), ds.Find(8));

  EXPECT_EQ(ds.Find(1), ds.Find(7));
  EXPECT_EQ(ds.Find(2), ds.Find(1));
  EXPECT_EQ(ds.Find(7), ds.Find(2));

  EXPECT_EQ(ds.Find(3), ds.Find(5));
  EXPECT_EQ(ds.Find(5), ds.Find(3));

  ds.Union(0, 1);
  ds.Union(3, 1);
  for (size_t i = 0; i < ITEM_COUNT; ++i)
  {
    EXPECT_EQ(ds.Find(0), ds.Find(i));
  }
}

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

TEST(StructureSet, CutSagittalInsideClose)
{
  DicomStructure2 structure;
  CreateBasicStructure(structure);
  std::vector< std::pair<Point2D, Point2D> > segments;

  // create an X,Z cutting plane, inside of the volume
  Point3D origin, axisX, axisY;
  LinearAlgebra::AssignVector(origin, 0, 3, 0);
  LinearAlgebra::AssignVector(axisX, 1, 0, 0);
  LinearAlgebra::AssignVector(axisY, 0, 0, 1);
  CoordinateSystem3D cuttingPlane(origin, axisX, axisY);

  // compute intersection
  bool ok = structure.Project(segments, cuttingPlane);
  EXPECT_TRUE(ok);
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


static size_t ConvertListOfSlabsToSegments_Add(RtStructRectanglesInSlab& rectangles, int row, double xmin, double xmax)
{
  double ymin = static_cast<double>(row) * 5.0;
  double ymax = static_cast<double>(row + 1) * 5.0;

  RtStructRectangleInSlab rectangle;
  rectangle.xmin = xmin;
  rectangle.xmax = xmax;
  rectangle.ymin = ymin;
  rectangle.ymax = ymax;

  rectangles.push_back(rectangle);
  
  return 1u;
}

static size_t FillTestRectangleList(std::vector< RtStructRectanglesInSlab >& rectanglesForEachSlab)
{
  // ConvertListOfSlabsToSegments
  size_t rectCount = 0;

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 0, 5, 31);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 0, 36, 50);

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 1, 20, 45);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 1, 52, 70);

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 2, 0, 32);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 2, 35, 44);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 2, 60, 75);

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 3, 10, 41);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 3, 46, 80);

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 4, 34, 42);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 4, 90, 96);

  rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 5, 1, 33);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 5, 40, 43);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 5, 51, 61);
  rectCount += ConvertListOfSlabsToSegments_Add(rectanglesForEachSlab.back(), 5, 76, 95);

  return rectCount;
}

/*
void AddSlabBoundaries(
  std::vector<std::pair<double, RectangleBoundaryKind> >& boundaries,
  const std::vector<RtStructRectanglesInSlab>& slabCuts, size_t iSlab)
*/


/*
void ProcessBoundaryList(
  std::vector< std::pair<Point2D, Point2D> >& segments,
  const std::vector<std::pair<double, RectangleBoundaryKind> >& boundaries,
  double y)
*/


TEST(StructureSet, ProcessBoundaryList_Empty)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;

  boundaries.clear();
  EXPECT_NO_THROW(AddSlabBoundaries(boundaries, slabCuts, 0));
  ASSERT_EQ(0u, boundaries.size());
}

TEST(StructureSet, ProcessBoundaryListTopRow)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
  FillTestRectangleList(slabCuts);

  boundaries.clear();
  AddSlabBoundaries(boundaries, slabCuts, 0);

  {
    size_t i = 0;
    ASSERT_EQ(4u, boundaries.size());

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(5, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(31, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(36, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(50, boundaries[i].first, DELTA_MAX);
    i++;
  }
}

TEST(StructureSet, ProcessBoundaryListRows_0_and_1)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
  FillTestRectangleList(slabCuts);

  boundaries.clear();
  AddSlabBoundaries(boundaries, slabCuts, 0);
  AddSlabBoundaries(boundaries, slabCuts, 1);

  ASSERT_EQ(8u, boundaries.size());

  {
    size_t i = 0;

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(5, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(20, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(31, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(36, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(45, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(50, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_Start, boundaries[i].second);
    ASSERT_NEAR(52, boundaries[i].first, DELTA_MAX);
    i++;

    ASSERT_EQ(RectangleBoundaryKind_End, boundaries[i].second);
    ASSERT_NEAR(70, boundaries[i].first, DELTA_MAX);
    i++;
  }
}

TEST(StructureSet, ConvertListOfSlabsToSegments_EmptyBoundaries)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
  FillTestRectangleList(slabCuts);
  boundaries.clear();
  std::vector< std::pair<Point2D, Point2D> > segments;
  ASSERT_NO_THROW(ProcessBoundaryList(segments, boundaries, 42.0));
  ASSERT_EQ(0u, segments.size());
}

TEST(StructureSet, ConvertListOfSlabsToSegments_TopRow_Horizontal)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  FillTestRectangleList(slabCuts);

  // top row
  {
    std::vector< std::pair<Point2D, Point2D> > segments;
    std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
    AddSlabBoundaries(boundaries, slabCuts, 0);
    ProcessBoundaryList(segments, boundaries, slabCuts[0][0].ymin);

    ASSERT_EQ(2u, segments.size());

    ASSERT_NEAR( 5.0, segments[0].first.x, DELTA_MAX);
    ASSERT_NEAR(31.0, segments[0].second.x, DELTA_MAX);
    ASSERT_NEAR( 0.0, segments[0].first.y, DELTA_MAX);
    ASSERT_NEAR( 0.0, segments[0].second.y, DELTA_MAX);

    ASSERT_NEAR(36.0, segments[1].first.x, DELTA_MAX);
    ASSERT_NEAR(50.0, segments[1].second.x, DELTA_MAX);
    ASSERT_NEAR( 0.0, segments[1].first.y, DELTA_MAX);
    ASSERT_NEAR( 0.0, segments[1].second.y, DELTA_MAX);
  }
}

TEST(StructureSet, ConvertListOfSlabsToSegments_All_Horizontal)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
  FillTestRectangleList(slabCuts);

  // top row
  {
    std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
    AddSlabBoundaries(boundaries, slabCuts, 0);
    std::vector< std::pair<Point2D, Point2D> > segments;
    ProcessBoundaryList(segments, boundaries, slabCuts[0][0].ymin);
  }

  // mids
  {
    std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
    AddSlabBoundaries(boundaries, slabCuts, 0);
    AddSlabBoundaries(boundaries, slabCuts, 1);
    std::vector< std::pair<Point2D, Point2D> > segments;
    ProcessBoundaryList(segments, boundaries, slabCuts[0][0].ymax);

    ASSERT_EQ(4u, segments.size());

    ASSERT_NEAR(05.0, segments[0].first.x, DELTA_MAX);
    ASSERT_NEAR(20.0, segments[0].second.x, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[0].first.y, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[0].second.y, DELTA_MAX);

    ASSERT_NEAR(31.0, segments[1].first.x, DELTA_MAX);
    ASSERT_NEAR(36.0, segments[1].second.x, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[1].first.y, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[1].second.y, DELTA_MAX);

    ASSERT_NEAR(45.0, segments[2].first.x, DELTA_MAX);
    ASSERT_NEAR(50.0, segments[2].second.x, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[2].first.y, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[2].second.y, DELTA_MAX);

    ASSERT_NEAR(52.0, segments[3].first.x, DELTA_MAX);
    ASSERT_NEAR(70.0, segments[3].second.x, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[3].first.y, DELTA_MAX);
    ASSERT_NEAR(05.0, segments[3].second.y, DELTA_MAX);
  }

  // bottom row
  {
    std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
    AddSlabBoundaries(boundaries, slabCuts, 1);
    std::vector< std::pair<Point2D, Point2D> > segments;
    ProcessBoundaryList(segments, boundaries, slabCuts[1][0].ymax);

    ASSERT_EQ(2u, segments.size());

    ASSERT_NEAR(20.0, segments[0].first.x, DELTA_MAX);
    ASSERT_NEAR(45.0, segments[0].second.x, DELTA_MAX);
    ASSERT_NEAR(10.0, segments[0].first.y, DELTA_MAX);
    ASSERT_NEAR(10.0, segments[0].second.y, DELTA_MAX);

    ASSERT_NEAR(52.0, segments[1].first.x, DELTA_MAX);
    ASSERT_NEAR(70.0, segments[1].second.x, DELTA_MAX);
    ASSERT_NEAR(10.0, segments[1].first.y, DELTA_MAX);
    ASSERT_NEAR(10.0, segments[1].second.y, DELTA_MAX);
  }

}

TEST(StructureSet, ConvertListOfSlabsToSegments_Complete_Empty)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;

  std::vector< std::pair<Point2D, Point2D> > segments;

  ASSERT_NO_THROW(ConvertListOfSlabsToSegments(segments, slabCuts, 0));
  ASSERT_EQ(0u, segments.size());
}

TEST(StructureSet, ConvertListOfSlabsToSegments_Complete_Regular)
{
  std::vector< RtStructRectanglesInSlab > slabCuts;
  std::vector<std::pair<double, RectangleBoundaryKind> > boundaries;
  size_t totalRectCount = FillTestRectangleList(slabCuts);

  std::vector< std::pair<Point2D, Point2D> > segments;

  ASSERT_NO_THROW(ConvertListOfSlabsToSegments(segments, slabCuts, totalRectCount));
  ASSERT_EQ(60u, segments.size());

  size_t i = 0;

  ASSERT_NEAR(segments[i].first.x, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 31.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 31.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 36.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 36.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 50.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 50.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 45.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 45.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 52.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 52.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 70.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 70.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 32.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 32.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 35.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 35.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 44.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 44.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 60.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 60.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 75.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 75.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 41.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 41.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 46.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 46.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 80.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 80.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 34.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 34.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 42.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 42.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 90.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 90.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 96.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 96.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 1.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 1.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 33.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 33.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 40.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 40.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 43.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 43.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 51.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 51.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 61.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 61.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 76.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 76.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 95.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 95.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 31.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 0.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 36.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 50.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 0.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 31.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 36.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 45.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 50.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 52.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 5.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 70.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 5.0000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 32.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 35.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 44.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 45.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 52.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 60.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 70.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 75.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 10.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 0.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 32.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 35.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 41.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 44.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 46.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 60.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 75.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 15.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 80.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 15.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 10.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 34.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 41.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 42.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 46.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 80.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 90.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 20.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 96.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 20.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 1.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 33.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 34.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 40.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 42.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 43.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 51.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 61.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 76.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 90.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 95.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 25.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 96.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 25.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 1.0000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 30.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 33.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 40.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 30.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 43.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 51.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 30.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 61.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
  i++;
  ASSERT_NEAR(segments[i].first.x, 76.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].first.y, 30.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.x, 95.000000000000000, DELTA_MAX);
  ASSERT_NEAR(segments[i].second.y, 30.000000000000000, DELTA_MAX);
}

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include <SystemToolbox.h>

TEST(StructureSet, ReadFromJsonPart2)
{
  DicomStructureSet2 structureSet;
  std::string jsonText;

  Orthanc::SystemToolbox::ReadFile(jsonText, "72c773ac-5059f2c4-2e6a9120-4fd4bca1-45701661.json");

  FullOrthancDataset dicom(jsonText);
  //loader.content_.reset(new DicomStructureSet(dicom));
  structureSet.Clear();

  structureSet.FillStructuresFromDataset(dicom);
  structureSet.ComputeDependentProperties();

  const std::vector<DicomStructure2>& structures = structureSet.structures_;
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2
