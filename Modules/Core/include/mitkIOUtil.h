/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOUtil_h
#define mitkIOUtil_h

#include <MitkCoreExports.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>

#include <mitkFileReaderSelector.h>
#include <mitkFileWriterSelector.h>
#include <mitkIFileReader.h>
#include <mitkIFileWriter.h>

#include <fstream>

#if !defined(MITK_WINDOWS_NO_UNDEF) && defined(GetTempPath)
  #undef GetTempPath
#endif

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  class PropertyList;

  /**
   * \ingroup IO
   *
   * \brief A utility class to load and save data from/to the local file system.
   *
   * \see QmitkIOUtil
   */
  class MITKCORE_EXPORT IOUtil
  {
  public:
    /**Struct that contains information regarding the current loading process. (e.g. Path that should be loaded,
    all found readers for the load path,...). It is set be IOUtil and used to pass information via the option callback
    in load operations.
    */
    struct MITKCORE_EXPORT LoadInfo
    {
      LoadInfo(const std::string &path);

      std::string m_Path;
      std::vector<BaseData::Pointer> m_Output;

      FileReaderSelector m_ReaderSelector;
      bool m_Cancel;

      const PropertyList* m_Properties;
    };

    /**Struct that is the base class for option callbacks used in load operations. The callback is used by IOUtil, if
    more than one suitable reader was found or the a reader contains options that can be set. The callback allows to
    change option settings and select the reader that should be used (via loadInfo).
    */
    struct MITKCORE_EXPORT ReaderOptionsFunctorBase
    {
      virtual bool operator()(LoadInfo &loadInfo) const = 0;
    };

    struct MITKCORE_EXPORT SaveInfo
    {
      SaveInfo(const BaseData *baseData, const MimeType &mimeType, const std::string &path);

      bool operator<(const SaveInfo &other) const;

      /// The BaseData object to save.
      const BaseData *m_BaseData;

      /// Contains a set of IFileWriter objects.
      FileWriterSelector m_WriterSelector;
      /// The selected mime-type, used to restrict results from FileWriterSelector.
      MimeType m_MimeType;
      /// The path to write the BaseData object to.
      std::string m_Path;
      /// Flag indicating if sub-sequent save operations are to be canceled.
      bool m_Cancel;
    };

    /**Struct that is the base class for option callbacks used in save operations. The callback is used by IOUtil, if
    more than one suitable writer was found or the a writer contains options that can be set. The callback allows to
    change option settings and select the writer that should be used (via saveInfo).
    */
    struct MITKCORE_EXPORT WriterOptionsFunctorBase
    {
      virtual bool operator()(SaveInfo &saveInfo) const = 0;
    };

    /**
     * Get the file system path where the running executable is located.
     *
     * @return The location of the currently running executable, without the filename.
     */
    static std::string GetProgramPath();

    /**
     * Get the default temporary path.
     *
     * @return The default path for temporary data.
     */
    static std::string GetTempPath();

    /**
    * Returns the Directory Separator for the current OS.
    *
    * @return the Directory Separator for the current OS, i.e. "\\" for Windows and "/" otherwise.
    */
    static char GetDirectorySeparator();

    /**
     * Create and open a temporary file.
     *
     * This method generates a unique temporary filename from \c templateName, creates
     * and opens the file using the output stream \c tmpStream and returns the name of
     * the newly create file.
     *
     * The \c templateName argument must contain six consecutive 'X' characters ("XXXXXX")
     * and these are replaced with a string that makes the filename unique.
     *
     * The file is created with read and write permissions for owner only.
     *
     * @param tmpStream The output stream for writing to the temporary file.
     * @param templateName An optional template for the filename.
     * @param path An optional path where the temporary file should be created. Defaults
     *        to the default temp path as returned by GetTempPath().
     * @return The filename of the created temporary file.
     *
     * @throw mitk::Exception if the temporary file could not be created.
     */
    static std::string CreateTemporaryFile(std::ofstream &tmpStream,
                                           const std::string &templateName = "XXXXXX",
                                           std::string path = std::string());

    /**
     * Create and open a temporary file.
     *
     * This method generates a unique temporary filename from \c templateName, creates
     * and opens the file using the output stream \c tmpStream and the specified open
     * mode \c mode and returns the name of the newly create file. The open mode is always
     * OR'd with <code>std::ios_base::out | std::ios_base::trunc</code>.
     *
     * The \c templateName argument must contain six consecutive 'X' characters ("XXXXXX")
     * and these are replaced with a string that makes the filename unique.
     *
     * The file is created with read and write permissions for owner only.
     *
     * @param tmpStream The output stream for writing to the temporary file.
     * @param mode The open mode for the temporary file stream.
     * @param templateName An optional template for the filename.
     * @param path An optional path where the temporary file should be created. Defaults
     *        to the default temp path as returned by GetTempPath().
     * @return The filename of the created temporary file.
     *
     * @throw mitk::Exception if the temporary file could not be created.
     */
    static std::string CreateTemporaryFile(std::ofstream &tmpStream,
                                           std::ios_base::openmode mode,
                                           const std::string &templateName = "XXXXXX",
                                           std::string path = std::string());

    /**
    * Creates an empty temporary file.
    *
    * This method generates a unique temporary filename from \c templateName and creates
    * this file.
    *
    * The file is created with read and write permissions for owner only.
    *
    * ---
    * This version is potentially unsafe because the created temporary file is not kept open
    * and could be used by another process between calling this method and opening the returned
    * file path for reading or writing.
    * ---
    *
    * @return The filename of the created temporary file.
    * @param templateName An optional template for the filename.
    * @param path An optional path where the temporary file should be created. Defaults
    *        to the default temp path as returned by GetTempPath().
    * @throw mitk::Exception if the temporary file could not be created.
    */
    static std::string CreateTemporaryFile(const std::string &templateName = "XXXXXX",
                                           std::string path = std::string());

    /**
     * Create a temporary directory.
     *
     * This method generates a uniquely named temporary directory from \c templateName.
     * The last set of six consecutive 'X' characters in \c templateName is replaced
     * with a string that makes the directory name unique.
     *
     * The directory is created with read, write and executable permissions for owner only.
     *
     * @param templateName An optional template for the directory name.
     * @param path An optional path where the temporary directory should be created. Defaults
     *        to the default temp path as returned by GetTempPath().
     * @return The filename of the created temporary file.
     *
     * @throw mitk::Exception if the temporary directory could not be created.
     */
    static std::string CreateTemporaryDirectory(const std::string &templateName = "XXXXXX",
                                                std::string path = std::string());

    /**
     * @brief Load a file into the given DataStorage.
     *
     * This method calls Load(const std::vector<std::string>&, DataStorage&) with a
     * one-element vector.
     *
     * @param path The absolute file name including the file extension.
     * @param storage A DataStorage object to which the loaded data will be added.
     * @param optionsCallback Pointer to a callback instance. The callback is used by
     * the load operation if more the suitable reader was found or the reader has options
     * that can be set.
     * @return The set of added DataNode objects.
     * @throws mitk::Exception if \c path could not be loaded.
     *
     * @sa Load(const std::vector<std::string>&, DataStorage&)
     */
    static DataStorage::SetOfObjects::Pointer Load(const std::string &path, DataStorage &storage,
                                                   const ReaderOptionsFunctorBase *optionsCallback = nullptr);

    /**
    * @brief Load a file into the given DataStorage given user defined IFileReader::Options.
    *
    * This method calls Load(const std::vector<std::string>&, DataStorage&) with a
    * one-element vector.
    *
    * @param path The absolute file name including the file extension.
    * @param options IFileReader option instance that should be used if selected reader
    * has options.
    * @param storage A DataStorage object to which the loaded data will be added.
    * @return The set of added DataNode objects.
    * @throws mitk::Exception if \c path could not be loaded.
    *
    * @sa Load(const std::vector<std::string>&, DataStorage&)
    */
    static DataStorage::SetOfObjects::Pointer Load(const std::string &path,
                                                   const IFileReader::Options &options,
                                                   DataStorage &storage);

    /**
    * @brief Load a file and return the loaded data.
    *
    * This method calls Load(const std::vector<std::string>&) with a
    * one-element vector.
    *
    * @param path The absolute file name including the file extension.
    * @param optionsCallback Pointer to a callback instance. The callback is used by
    * the load operation if more the suitable reader was found or the reader has options
    * that can be set.
    * @return The set of added DataNode objects.
    * @throws mitk::Exception if \c path could not be loaded.
    *
    * @sa Load(const std::vector<std::string>&, DataStorage&)
    */
    static std::vector<BaseData::Pointer> Load(const std::string &path,
                                               const ReaderOptionsFunctorBase *optionsCallback = nullptr);

    template <typename T>
    static typename T::Pointer Load(const std::string& path, const ReaderOptionsFunctorBase *optionsCallback = nullptr)
    {
      return dynamic_cast<T*>(Load(path, optionsCallback).at(0).GetPointer());
    }

    /**
    * @brief Load a file and return the loaded data.
    *
    * This method calls Load(const std::vector<std::string>&) with a
    * one-element vector.
    *
    * @param path The absolute file name including the file extension.
    * @param options IFileReader option instance that should be used if selected reader
    * has options.
    * @return The set of added DataNode objects.
    * @throws mitk::Exception if \c path could not be loaded.
    *
    * @sa Load(const std::vector<std::string>&, DataStorage&)
    */
    static std::vector<BaseData::Pointer> Load(const std::string &path, const IFileReader::Options &options);

    template <typename T>
    static typename T::Pointer Load(const std::string& path, const IFileReader::Options &options)
    {
      return dynamic_cast<T*>(Load(path, options).at(0).GetPointer());
    }

    /**
     * @brief Loads a list of file paths into the given DataStorage.
     *
     * If an entry in \c paths cannot be loaded, this method will continue to load
     * the remaining entries into \c storage and throw an exception afterwards.
     *
     * @param paths A list of absolute file names including the file extension.
     * @param storage A DataStorage object to which the loaded data will be added.
     * @param optionsCallback Pointer to a callback instance. The callback is used by
     * the load operation if more the suitable reader was found or the reader has options
     * that can be set.
     * @return The set of added DataNode objects.
     * @throws mitk::Exception if an entry in \c paths could not be loaded.
     */
    static DataStorage::SetOfObjects::Pointer Load(const std::vector<std::string> &paths, DataStorage &storage,
                                                   const ReaderOptionsFunctorBase *optionsCallback = nullptr);

    static std::vector<BaseData::Pointer> Load(const std::vector<std::string> &paths,
                                               const ReaderOptionsFunctorBase *optionsCallback = nullptr);

    /**
     * @brief Loads the contents of a us::ModuleResource and returns the corresponding mitk::BaseData
     * @param usResource a ModuleResource, representing a BaseData object
     * @param mode Optional parameter to set the openmode of the stream
     * @return The set of loaded BaseData objects. \c Should contain either one or zero elements, since a resource
     * stream
     * represents one object.
     * @throws mitk::Exception if no reader was found for the stream.
     */
    static std::vector<BaseData::Pointer> Load(const us::ModuleResource &usResource,
                                               std::ios_base::openmode mode = std::ios_base::in);

    template <typename T>
    static typename T::Pointer Load(const us::ModuleResource &usResource, std::ios_base::openmode mode = std::ios_base::in)
    {
      return dynamic_cast<T*>(Load(usResource, mode).at(0).GetPointer());
    }

    static BaseData::Pointer Load(const std::string& path, const PropertyList* properties);

    /**
     * @brief Save a mitk::BaseData instance.
     * @param data The data to save.
     * @param path The path to the image including file name and and optional file extension.
     *        If no extension is set, the default extension and mime-type for the
     *        BaseData type of \c data is used.
     * @param setPathProperty
     * @throws mitk::Exception if no writer for \c data is available or the writer
     *         is not able to write the image.
     */
    static void Save(const mitk::BaseData *data, const std::string &path, bool setPathProperty = false);

    /**
     * @brief Save a mitk::BaseData instance.
     * @param data The data to save.
     * @param path The path to the image including file name and an optional file extension.
     *        If no extension is set, the default extension and mime-type for the
     *        BaseData type of \c data is used.
     * @param options The IFileWriter options to use for the selected writer.
     * @param setPathProperty
     * @throws mitk::Exception if no writer for \c data is available or the writer
     *         is not able to write the image.
     */
    static void Save(const mitk::BaseData *data, const std::string &path, const IFileWriter::Options &options, bool setPathProperty = false);

    /**
     * @brief Save a mitk::BaseData instance.
     * @param data The data to save.
     * @param mimeType The mime-type to use for writing \c data.
     * @param path The path to the image including file name and an optional file extension.
     * @param addExtension If \c true, an extension according to the given \c mimeType
     *        is added to \c path if it does not contain one. If \c path already contains
     *        a file name extension, it is not checked for compatibility with \c mimeType.
     * @param setPathProperty
     *
     * @throws mitk::Exception if no writer for the combination of \c data and \c mimeType is
     *         available or the writer is not able to write the image.
     */
    static void Save(const mitk::BaseData *data,
                     const std::string &mimeType,
                     const std::string &path,
                     bool addExtension = true,
                     bool setPathProperty = false);

    /**
     * @brief Save a mitk::BaseData instance.
     * @param data The data to save.
     * @param mimeType The mime-type to use for writing \c data.
     * @param path The path to the image including file name and an optional file extension.
     * @param options Configuration data for the used IFileWriter instance.
     * @param addExtension If \c true, an extension according to the given \c mimeType
     *        is added to \c path if it does not contain one. If \c path already contains
     *        a file name extension, it is not checked for compatibility with \c mimeType.
     * @param setPathProperty
     *
     * @throws mitk::Exception if no writer for the combination of \c data and \c mimeType is
     *         available or the writer is not able to write the image.
     */
    static void Save(const mitk::BaseData *data,
                     const std::string &mimeType,
                     const std::string &path,
                     const mitk::IFileWriter::Options &options,
                     bool addExtension = true,
                     bool setPathProperty = false);

    /**
     * @brief Use SaveInfo objects to save BaseData instances.
     *
     * This is a low-level method for directly working with SaveInfo objects. Usually,
     * the Save() methods taking a BaseData object as an argument are more appropriate.
     *
     * @param saveInfos A list of SaveInfo objects for saving contained BaseData objects.
     * @param setPathProperty
     *
     * @see Save(const mitk::BaseData*, const std::string&)
     */
    static void Save(std::vector<SaveInfo> &saveInfos, bool setPathProperty = false);

  protected:
    static std::string Load(std::vector<LoadInfo> &loadInfos,
                            DataStorage::SetOfObjects *nodeResult,
                            DataStorage *ds,
                            const ReaderOptionsFunctorBase *optionsCallback);

    static std::string Save(const BaseData *data,
                            const std::string &mimeType,
                            const std::string &path,
                            WriterOptionsFunctorBase *optionsCallback,
                            bool addExtension,
                            bool setPathProperty);

    static std::string Save(std::vector<SaveInfo> &saveInfos,
                            WriterOptionsFunctorBase *optionsCallback,
                            bool setPathProperty);

  private:
    struct Impl;
  };
}

#endif
