/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <itksys/SystemTools.hxx>

//MITK
#include <mitkBaseApplication.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkIOUtil.h>

// Qmitk
#include "QmitkSegmentationFlowControlView.h"

// Qt
#include <QMessageBox>
#include <QDir>

#include <filesystem>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Parent(nullptr)
{
  auto notHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object"));

  m_SegmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::LabelSetImage>::New(),
    notHelperObject);

  m_SegmentationTaskPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::SegmentationTask>::New(),
    notHelperObject);
}

void QmitkSegmentationFlowControlView::SetFocus()
{
    m_Controls.btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  m_Parent = parent;

  using Self = QmitkSegmentationFlowControlView;

  connect(m_Controls.btnStoreAndAccept, &QPushButton::clicked, this, &Self::OnAcceptButtonPushed);
  connect(m_Controls.segmentationTaskWidget, &QmitkSegmentationTaskWidget::ActiveSubtaskChanged, this, &Self::OnActiveSubtaskChanged);
  connect(m_Controls.segmentationTaskWidget, &QmitkSegmentationTaskWidget::CurrentSubtaskChanged, this, &Self::OnCurrentSubtaskChanged);

  m_Controls.segmentationTaskWidget->setVisible(false);
  m_Controls.labelStored->setVisible(false);
  UpdateControls();

  m_OutputDir = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputdir", itksys::SystemTools::GetCurrentWorkingDirectory()));
  m_OutputDir = QDir::fromNativeSeparators(m_OutputDir);

  m_FileExtension = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputextension", "nrrd"));
}

void QmitkSegmentationFlowControlView::OnAcceptButtonPushed()
{
  if (m_Controls.segmentationTaskWidget->isVisible())
  {
    auto* task = m_Controls.segmentationTaskWidget->GetTask();

    if (task != nullptr)
    {
      auto activeSubtaskIndex = m_Controls.segmentationTaskWidget->GetActiveSubtaskIndex();

      if (activeSubtaskIndex.has_value())
      {
        auto segmentationNode = m_Controls.segmentationTaskWidget->GetSegmentationDataNode(activeSubtaskIndex.value());

        if (segmentationNode != nullptr)
        {
          auto path = task->GetAbsolutePath(task->GetResult(activeSubtaskIndex.value()));

          if (!path.empty())
          {
            QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

            try
            {
              mitk::IOUtil::Save(segmentationNode->GetData(), path.string());
              // TODO: Give temporarily displayed feedback to user
            }
            catch (const mitk::Exception&)
            {
            }

            QApplication::restoreOverrideCursor();
          }
        }
      }
    }
  }
  else
  {
    auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);

    for (auto node : *nodes)
    {
      QString outputpath = m_OutputDir + "/" + QString::fromStdString(node->GetName()) + "." + m_FileExtension;
      outputpath = QDir::toNativeSeparators(QDir::cleanPath(outputpath));
      mitk::IOUtil::Save(node->GetData(), outputpath.toStdString());
    }

    m_Controls.labelStored->setVisible(true);
  }
}

void QmitkSegmentationFlowControlView::OnActiveSubtaskChanged(const std::optional<size_t>&)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::OnCurrentSubtaskChanged(size_t)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto dataStorage = this->GetDataStorage();

  auto hasTask = !dataStorage->GetSubset(m_SegmentationTaskPredicate)->empty();
  m_Controls.segmentationTaskWidget->setVisible(hasTask);

  if (hasTask)
  {
    auto activeSubtaskIndex = m_Controls.segmentationTaskWidget->GetActiveSubtaskIndex();
    auto hasActiveSubtask = activeSubtaskIndex.has_value();

    auto isCurrentSubtask = hasActiveSubtask
      ? m_Controls.segmentationTaskWidget->GetCurrentSubtaskIndex() == activeSubtaskIndex.value()
      : false;

    m_Controls.btnStoreAndAccept->setEnabled(hasActiveSubtask && isCurrentSubtask);
  }
  else
  {
    auto hasSegmentation = !dataStorage->GetSubset(m_SegmentationPredicate)->empty();
    m_Controls.btnStoreAndAccept->setEnabled(hasSegmentation);
  }
}

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::NodeChanged(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::NodeRemoved(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}
