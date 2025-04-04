/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMimeTypeProvider.h"

#include "mitkLog.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <itksys/SystemTools.hxx>

#ifdef _MSC_VER
#pragma warning(disable : 4503) // decorated name length exceeded, name was truncated
#pragma warning(disable : 4355)
#endif

namespace mitk
{
  MimeTypeProvider::MimeTypeProvider() : m_Tracker(nullptr) {}
  MimeTypeProvider::~MimeTypeProvider() { delete m_Tracker; }
  void MimeTypeProvider::Start()
  {
    if (m_Tracker == nullptr)
    {
      m_Tracker = new us::ServiceTracker<CustomMimeType, MimeTypeTrackerTypeTraits>(us::GetModuleContext(), this);
    }
    m_Tracker->Open();
  }

  void MimeTypeProvider::Stop() { m_Tracker->Close(); }
  std::vector<MimeType> MimeTypeProvider::GetMimeTypes() const
  {
    std::vector<MimeType> result;
    for (const auto &elem : m_NameToMimeType)
    {
      result.push_back(elem.second);
    }
    return result;
  }

  std::vector<MimeType> MimeTypeProvider::GetMimeTypesForFile(const std::string &filePath) const
  {
    std::vector<MimeType> result;
    for (const auto &elem : m_NameToMimeType)
    {
      try
      {
        if (elem.second.AppliesTo(filePath))
          result.push_back(elem.second);
      }
      catch (...)
      {
      }
    }
    std::sort(result.begin(), result.end());
    std::reverse(result.begin(), result.end());
    return result;
  }

  std::vector<MimeType> MimeTypeProvider::GetMimeTypesForCategory(const std::string &category) const
  {
    std::vector<MimeType> result;
    for (const auto &elem : m_NameToMimeType)
    {
      if (elem.second.GetCategory() == category)
      {
        result.push_back(elem.second);
      }
    }
    return result;
  }

  MimeType MimeTypeProvider::GetMimeTypeForName(const std::string &name) const
  {
    auto iter = m_NameToMimeType.find(name);
    if (iter != m_NameToMimeType.end())
      return iter->second;
    return MimeType();
  }

  std::vector<std::string> MimeTypeProvider::GetCategories() const
  {
    std::vector<std::string> result;
    for (const auto &elem : m_NameToMimeType)
    {
      std::string category = elem.second.GetCategory();
      if (!category.empty())
      {
        result.push_back(category);
      }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
  }

  MimeTypeProvider::TrackedType MimeTypeProvider::AddingService(const ServiceReferenceType &reference)
  {
    MimeType result = this->GetMimeType(reference);
    if (result.IsValid())
    {
      std::string name = result.GetName();
      m_NameToMimeTypes[name].insert(result);

      // get the highest ranked mime-type
      m_NameToMimeType[name] = *(m_NameToMimeTypes[name].rbegin());
    }
    return result;
  }

  void MimeTypeProvider::ModifiedService(const ServiceReferenceType & /*reference*/, TrackedType /*mimetype*/)
  {
    // should we track changes in the ranking property?
  }

  void MimeTypeProvider::RemovedService(const ServiceReferenceType & /*reference*/, TrackedType mimeType)
  {
    std::string name = mimeType.GetName();
    std::set<MimeType> &mimeTypes = m_NameToMimeTypes[name];
    mimeTypes.erase(mimeType);
    if (mimeTypes.empty())
    {
      m_NameToMimeTypes.erase(name);
      m_NameToMimeType.erase(name);
    }
    else
    {
      // get the highest ranked mime-type
      m_NameToMimeType[name] = *(mimeTypes.rbegin());
    }
  }

  MimeType MimeTypeProvider::GetMimeType(const ServiceReferenceType &reference) const
  {
    MimeType result;
    if (!reference)
      return result;

    CustomMimeType *mimeType = us::GetModuleContext()->GetService(reference);
    if (mimeType != nullptr)
    {
      try
      {
        int rank = 0;
        us::Any rankProp = reference.GetProperty(us::ServiceConstants::SERVICE_RANKING());
        if (!rankProp.Empty())
        {
          rank = us::any_cast<int>(rankProp);
        }
        auto id = us::any_cast<long>(reference.GetProperty(us::ServiceConstants::SERVICE_ID()));
        result = MimeType(*mimeType, rank, id);
      }
      catch (const us::BadAnyCastException &e)
      {
        MITK_WARN << "Unexpected exception: " << e.what();
      }
      us::GetModuleContext()->UngetService(reference);
    }
    return result;
  }
}
