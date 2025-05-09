/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagBasedSorter_h
#define mitkDICOMTagBasedSorter_h

#include "mitkDICOMDatasetSorter.h"
#include "mitkDICOMSortCriterion.h"

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief Sort DICOM datasets based on configurable tags.

  This class implements sorting of input DICOM datasets into multiple outputs
  as described in \ref DICOMITKSeriesGDCMReader_LoadingStrategy.

  The logic of sorting and splitting is most simple and most generic:

  1. Datasets will be put into different groups, if they differ in their value of specific tags (defined by AddDistinguishingTag())
    - there might be multiple distinguishing tags defined
    - tag values might be processed before comparison by means of TagValueProcessor (e.g. round to a number of decimal places)
  2. Each of the groups will be sorted by comparing their tag values using multiple DICOMSortCriterion
    - DICOMSortCriterion might evaluate a single tag (e.g. Instance Number) or multiple values (as in SortByImagePositionPatient)
    - only a single DICOMSortCriterion is defined for DICOMTagBasedSorter, because each DICOMSortCriterion holds a "secondary sort criterion", i.e. an application can define multiple tags for sorting by chaining \link DICOMSortCriterion DICOMSortCriteria \endlink
    - applications should make sure that sorting is always defined (to avoid problems with standard containers), e.g. by adding a comparison of filenames or instance UIDs as a last sorting fallback.

*/
class MITKDICOM_EXPORT DICOMTagBasedSorter : public DICOMDatasetSorter
{
  public:

    /**
      \brief Processes tag values before they are compared.
      These classes could do some kind of normalization such as rounding, lower case formatting, etc.
    */
    class MITKDICOM_EXPORT TagValueProcessor
    {
      public:
        /// \brief Implements the "processing".
        virtual std::string operator()(const std::string&) const = 0;
        virtual TagValueProcessor* Clone() const = 0;
        virtual ~TagValueProcessor() {}
    };

    /**
      \brief Cuts a number after configured number of decimal places.
      An instance of this class can be used to avoid errors when comparing minimally different image orientations.
    */
    class MITKDICOM_EXPORT CutDecimalPlaces : public TagValueProcessor
    {
      public:
        CutDecimalPlaces(unsigned int precision);
        CutDecimalPlaces(const CutDecimalPlaces& other);
        unsigned int GetPrecision() const;
        std::string operator()(const std::string&) const override;
        TagValueProcessor* Clone() const override;
      private:
        unsigned int m_Precision;
    };

    mitkClassMacro( DICOMTagBasedSorter, DICOMDatasetSorter );

    itkNewMacro( DICOMTagBasedSorter );

    /**
      \brief Datasets that differ in given tag's value will be sorted into separate outputs.
    */
    void AddDistinguishingTag( const DICOMTag&, TagValueProcessor* tagValueProcessor = nullptr );
    DICOMTagList GetDistinguishingTags() const;
    const TagValueProcessor* GetTagValueProcessorForDistinguishingTag(const DICOMTag&) const;

    /**
      \brief Define the sorting criterion (which holds seconardy criteria)
    */
    void SetSortCriterion( DICOMSortCriterion::ConstPointer criterion );
    DICOMSortCriterion::ConstPointer GetSortCriterion() const;

    /**
      \brief A list of all the tags needed for processing (facilitates scanning).
    */
    DICOMTagList GetTagsOfInterest() override;

    /**
      \brief Whether or not groups should be checked for consecutive tag values.

      When this flag is set (default in constructor=off), the sorter will
      not only sort in a way that the values of a configured tag are ascending
      BUT in addition the sorter will enforce a constant numerical distance
      between values.

      Having this flag is useful for handling of series with missing slices,
      e.g. Instance Numbers 1 2 3 5 6 7 8. With the flag set to true, the sorter
      would split this group into two, because the initial distance of 1 is
      not kept between Instance Numbers 3 and 5.

      A special case of this behavior can be configured by SetExpectDistanceOne().
      When this additional flag is set to true, the sorter will expect distance
      1 exactly. This can help if the second slice is missing already. Without
      this additional flag, we would "learn" about a wrong distance of 2 (or similar)
      and then sort completely wrong.
    */
    void SetStrictSorting(bool strict);
    bool GetStrictSorting() const;

    /**
      \brief Flag for a special case in "strict sorting".
      Please see documentation of SetStrictSorting().
      \sa SetStrictSorting
    */
    void SetExpectDistanceOne(bool strict);
    bool GetExpectDistanceOne() const;


    /**
      \brief Actually sort as described in the Detailed Description.
    */
    void Sort() override;

    /**
      \brief Print configuration details into given stream.
    */
    void PrintConfiguration(std::ostream& os, const std::string& indent = "") const override;

    bool operator==(const DICOMDatasetSorter& other) const override;

    static bool GetDefaultStrictSorting()
    {
      return m_DefaultStrictSorting;
    }

    static bool GetDefaultExpectDistanceOne()
    {
      return m_DefaultExpectDistanceOne;
    }

  protected:

    /**
      \brief Helper struct to feed into std::sort, configured via DICOMSortCriterion.
    */
    struct ParameterizedDatasetSort
    {
      ParameterizedDatasetSort(DICOMSortCriterion::ConstPointer);
      bool operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right);
      DICOMSortCriterion::ConstPointer m_SortCriterion;
    };


    DICOMTagBasedSorter();
    ~DICOMTagBasedSorter() override;

    DICOMTagBasedSorter(const DICOMTagBasedSorter& other);
    DICOMTagBasedSorter& operator=(const DICOMTagBasedSorter& other);

    /**
      \brief Helper for SplitInputGroups().
    */
    std::string BuildGroupID( DICOMDatasetAccess* dataset );

    using GroupIDToListType = std::map<std::string, DICOMDatasetList>;

    using SplitReasonListType = std::map<std::string, IOVolumeSplitReason::Pointer>;

    /**
      \brief Implements the "distiguishing tags".
      To sort datasets into different groups, a long string will be built for each dataset. The string concatenates all tags and their respective values.
      Datasets that match in all values will end up with the same string.
      @param splitReasons Reference to the split reasons vector. It will be also updated by the method to reflect the reasons for the returned groups.
    */
    GroupIDToListType SplitInputGroups(SplitReasonListType& splitReasons);

    /**
      \brief Implements the sorting step.
      Relatively simple implementation thanks to std::sort and a parameterization via DICOMSortCriterion.
      @param groups Argument that contains the groups that should be sorted.
      @param splitReasons Reference to the split reasons vector. It will be also updated by the method to reflect the reasons for the returned groups.
    */
    GroupIDToListType& SortGroups(GroupIDToListType& groups, SplitReasonListType& splitReasons);

    DICOMTagList m_DistinguishingTags;
    typedef std::map<const DICOMTag, TagValueProcessor*>  TagValueProcessorMap;
    TagValueProcessorMap m_TagValueProcessor;

    DICOMSortCriterion::ConstPointer m_SortCriterion;

    bool m_StrictSorting;
    bool m_ExpectDistanceOne;

    const static bool m_DefaultStrictSorting = false;
    const static bool m_DefaultExpectDistanceOne = false;
};

}

#endif
