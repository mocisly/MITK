/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "MinimalView.h"
#include <ui_MinimalViewControls.h>

#include "org_mitk_example_gui_extensionpointdefinition_Activator.h"

#include <QPushButton>

const std::string MinimalView::VIEW_ID = "org.mitk.views.minimalview";

MinimalView::MinimalView()
  : m_Controls(new Ui::MinimalViewControls),
    m_Parent(nullptr)
{
}

void MinimalView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls->setupUi(parent);

  auto layout = new QVBoxLayout(m_Controls->m_ButtonContainer);
  //! [Collect extensions through registry]
  QList<ChangeTextDescriptor::Pointer> changeTexts = m_Registry.GetChangeTexts();
  foreach (const ChangeTextDescriptor::Pointer &changeText, changeTexts)
  {
    // Create a push button for each "changetext" descriptor
    QPushButton *button = new QPushButton(changeText->GetName(), m_Controls->m_ButtonContainer);
    button->setToolTip(changeText->GetDescription());
    button->setObjectName(changeText->GetID());
    layout->addWidget(button);

    connect(button, SIGNAL(clicked()), &m_SignalMapper, SLOT(map()));
    m_SignalMapper.setMapping(button, changeText->GetID());
  }
  layout->addStretch();

  connect(&m_SignalMapper, SIGNAL(mapped(QString)), this, SLOT(ChangeText(QString)));
  //! [Collect extensions through registry]
}

void MinimalView::SetFocus()
{
  m_Controls->m_InputText->setFocus();
}

//! [Use extended functionality to alter input text]
void MinimalView::ChangeText(const QString &id)
{
  ChangeTextDescriptor::Pointer changeTextDescr = m_Registry.Find(id);

  // lazily create an instance of IChangeText (the descriptor will cache it)
  IChangeText::Pointer changeText = changeTextDescr->CreateChangeText();
  m_Controls->m_OutputText->setText(changeText->ChangeText(m_Controls->m_InputText->text()));
}
//! [Use extended functionality to alter input text]
