// @(#)root/gui:$Name:$:$Id:$
// Author: Abdelhalim Ssadik   07/07/04

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
/**************************************************************************

    This source is based on Xclass95, a Win95-looking GUI toolkit.
    Copyright (C) 1996, 1997 David Barth, Ricky Ralston, Hector Peraza.

    Xclass95 is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

**************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// A TGDockableFrame is a frame with handles that allow it to be        //
// undocked (i.e. put in a transient frame of its own) and to be docked //
// again or hidden and shown again. It uses the TGDockButton, which is  //
// a button with two vertical bars (||) and TGDockHideButton, which is  //
// a button with a small triangle. The TGUndockedFrame is a transient   //
// frame that on closure will put the frame back in the dock.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TColor.h"
#include "TGFrame.h"
#include "TMessage.h"
#include "TGWidget.h"
#include "TGButton.h"
#include "TGDockableFrame.h"
#include "TGWindow.h"
#include "TVirtualX.h"



ClassImp(TGDockButton)
ClassImp(TGDockHideButton)
ClassImp(TGUndockedFrame)
ClassImp(TGDockableFrame)

//______________________________________________________________________________
TGDockButton::TGDockButton(const TGCompositeFrame *p, int id) :
   TGButton (p, id, kChildFrame)
{
   // Create a dock button (i.e. button with two vertical bars).

   Resize(10, GetDefaultHeight());
   fMouseOn = kFALSE;

   fNormBg = fBackground;
   TColor c;
   Float_t r,g,b;
   c.Pixel2RGB(fNormBg, r, g, b);
   c.SetRGB(r, g, b);
   Float_t newL = c.GetLight() + (255 - c.GetLight()) * 45 / 100;
   Float_t newR = c.GetRed();
   Float_t newG = c.GetGreen();
   Float_t newB = c.GetBlue();
   c.HLStoRGB(c.GetHue(), newL, c.GetSaturation(), newR, newG, newB);
   c.SetRGB(newR, newG, newB);

   fHiBg = c.GetPixel();

   AddInput(kEnterWindowMask | kLeaveWindowMask);
}

//______________________________________________________________________________
TGDockButton::~TGDockButton()
{
   // Delete dock button.

   fClient->FreeColor(fHiBg);
}

//______________________________________________________________________________
Bool_t TGDockButton::HandleCrossing(Event_t *event)
{
   // Handle dock button crossing events.

   TGButton::HandleCrossing(event);
   if (event->fType == kLeaveNotify) {
      fMouseOn = kFALSE;
   } else if (event->fType == kEnterNotify) {
      fMouseOn = kTRUE;
   }
   if (IsEnabled()) fClient->NeedRedraw(this);

   return kTRUE;
}

//______________________________________________________________________________
void TGDockButton::DrawBorder()
{
   // Draw borders of dock button.

   int options = GetOptions();

   if (fState == kButtonDown || fState == kButtonEngaged)
      ;
   else if (fMouseOn == kTRUE && IsEnabled()) {
      SetBackgroundColor(fHiBg);
      ChangeOptions(kChildFrame);
   } else {
      SetBackgroundColor(fNormBg);
      ChangeOptions(kChildFrame);
   }
   gVirtualX->ClearWindow(fId);
   TGFrame::DrawBorder();

   ChangeOptions(options);
}

//______________________________________________________________________________
void TGDockButton::DoRedraw()
{
   // Draw the dock button, i.e. two vertical lines.

   int x = 1, y = 0;

   DrawBorder();
   if (fState == kButtonDown || fState == kButtonEngaged) { ++x; ++y; }

   for (int i = 0; i < 5; i +=4) {
      gVirtualX->DrawLine(fId, GetHilightGC()(), i+x,   y+1, i+x,   fHeight-y-3);
      gVirtualX->DrawLine(fId, GetShadowGC()(),  i+x+1, y+1, i+x+1, fHeight-y-3);
   }
}


//______________________________________________________________________________
TGDockHideButton::TGDockHideButton(const TGCompositeFrame *p) :
   TGDockButton (p, 2)
{
   // Create a dock hide button (i.e. button with small triangle).

   Resize(10, 8);
   fAspectRatio = 0;
}

//______________________________________________________________________________
void TGDockHideButton::DoRedraw()
{
   // Draw dock hide button.

   int x = 1, y = 0;

   DrawBorder();
   if (fState == kButtonDown || fState == kButtonEngaged) { ++x; ++y; }

   if (fAspectRatio) {
      gVirtualX->DrawLine(fId, GetBlackGC()(), x+1, y+1, x+5, y+3);
      gVirtualX->DrawLine(fId, GetBlackGC()(), x+1, y+5, x+5, y+3);
      gVirtualX->DrawLine(fId, GetHilightGC()(), x, y+1, x, y+5);
   } else {
      gVirtualX->DrawLine(fId, GetHilightGC()(), x+5, y+1, x+1, y+3);
      gVirtualX->DrawLine(fId, GetHilightGC()(), x+5, y+5, x+1, y+3);
      gVirtualX->DrawLine(fId, GetBlackGC()(), x+6, y+1, x+6, y+5);
   }
}


//______________________________________________________________________________
TGUndockedFrame::TGUndockedFrame(const TGWindow *p, TGDockableFrame *dockable) :
   TGTransientFrame(p, dockable->GetMainFrame(), 10, 10)
{
   // Create the undocked (transient) frame.

   SetWindowName("");
   fDockable = dockable;

   SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
                              kMWMDecorMinimize | kMWMDecorMenu,
               kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
                             kMWMFuncMinimize,
               kMWMInputModeless);
}

//______________________________________________________________________________
TGUndockedFrame::~TGUndockedFrame()
{
   // Delete undocked frame. Puts back dockable frame in its original container.

   fDockable->DockContainer(kFALSE);
}

//______________________________________________________________________________
void TGUndockedFrame::FixSize()
{
   // Fix the size of the undocked frame so it cannot be changed via the WM.

   SetWMSize(fWidth, fHeight);
   SetWMSizeHints(fWidth, fHeight, fWidth, fHeight, 0, 0);
}

//______________________________________________________________________________
void TGUndockedFrame::CloseWindow()
{
   // Close undocked frame (called via WM close button).

   DeleteWindow();
}


//______________________________________________________________________________
TGDockableFrame::TGDockableFrame(const TGWindow *p, int id, UInt_t options) :
   TGCompositeFrame(p, 10, 10, kHorizontalFrame),
   TGWidget(id)
{
   // Create a dockable frame widget.

   fMsgWindow = fParent;

   fCl = new TGLayoutHints(kLHintsExpandY | kLHintsExpandX);

   TGLayoutHints *l1 = new TGLayoutHints(kLHintsTop | kLHintsLeft);
   TGLayoutHints *l2 = new TGLayoutHints(kLHintsExpandY | kLHintsLeft);
   TGLayoutHints *lb = new TGLayoutHints(kLHintsExpandY | kLHintsLeft, 0, 2, 0, 0);
   TGLayoutHints *lc = new TGLayoutHints(kLHintsExpandY | kLHintsExpandX);

   SetCleanup();

   fButtons = new TGCompositeFrame(this, 10, 10, kVerticalFrame);
   fButtons->SetCleanup();
   fHideButton = new TGDockHideButton(fButtons);
   fButtons->AddFrame(fHideButton, l1);
   fDockButton = new TGDockButton(fButtons);
   fButtons->AddFrame(fDockButton, l2);

   TGCompositeFrame::AddFrame(fButtons, lb);

   fContainer = new TGCompositeFrame(this, 10, 10);

   TGCompositeFrame::AddFrame(fContainer, lc);

   fEnableHide   = kTRUE;
   fEnableUndock = kTRUE;
   fHidden       = kFALSE;
   fFrame        = 0;

   fDockButton->Associate(this);
   fHideButton->Associate(this);

   MapSubwindows();
   Resize(GetDefaultSize());
}

//______________________________________________________________________________
TGDockableFrame::~TGDockableFrame()
{
   // Cleanup dockable frame.

   if (fFrame)
      delete fFrame;
   delete fCl;
}

//______________________________________________________________________________
void TGDockableFrame::AddFrame(TGFrame *f, TGLayoutHints *hints)
{
   // Add frame to dockable frame container. Frame and hints are NOT adopted.

   f->ReparentWindow(fContainer);
   fContainer->AddFrame(f, fHints = hints);
   fContainer->Layout();
}

//______________________________________________________________________________
void TGDockableFrame::UndockContainer()
{
   // Undock container.

   int i, ax, ay;
   Window_t wdummy;

   if (fFrame || !fEnableUndock) return;

   fFrame = new TGUndockedFrame(fClient->GetRoot(), this);

   RemoveFrame(fContainer);
   fContainer->ReparentWindow(fFrame);
   fFrame->AddFrame(fContainer, fCl);  // fHints

   gVirtualX->TranslateCoordinates(GetId(), fClient->GetRoot()->GetId(), fX,
                                   fY + fFrame->GetHeight(), ax, ay, wdummy);

   if (fDockName) fFrame->SetWindowName(fDockName);

   fFrame->MapSubwindows();
   fFrame->Resize(fFrame->GetDefaultSize());
   fFrame->FixSize();
   fFrame->Move(ax, ay);
   fFrame->SetWMPosition(ax, ay);
   fFrame->MapWindow();

   if (((TGFrame *)fParent)->IsComposite())           // paranoia check
      ((TGCompositeFrame *)fParent)->HideFrame(this);

   Layout();

   SendMessage(fMsgWindow, MK_MSG(kC_DOCK, kDOCK_UNDOCK), fWidgetId, 0);
}

//______________________________________________________________________________
void TGDockableFrame::DockContainer(Int_t del)
{
   // Dock container back to TGDockableFrame.

   if (!fFrame) return;
   if (del) {
      delete fFrame;  // this will call DockContainer again with del = kFALSE
      return;
   }

   fFrame->RemoveFrame(fContainer);
   fContainer->ReparentWindow(this);
   TGCompositeFrame::AddFrame(fContainer, fCl);  // fHints

   // kludge! (for special case)
   fDockButton->Resize(fDockButton->GetDefaultWidth(), 1);

   Layout();
   if (((TGFrame *)fParent)->IsComposite())           // paranoia check
      ((TGCompositeFrame *)fParent)->ShowFrame(this);

   // fFrame is just being deleted (we're here called by TGUndockedFrame's
   // destructor) so just set it NULL below to avoid eventual problems in
   // TGDockableFrame's destructor.

   fFrame = 0;

   SendMessage(fMsgWindow, MK_MSG(kC_DOCK, kDOCK_DOCK), fWidgetId, 0);
}

//______________________________________________________________________________
void TGDockableFrame::ShowContainer()
{
   // Show dock container.

   if (!fHidden) return;

   ShowFrame(fContainer);
   if (fEnableUndock) fButtons->ShowFrame(fDockButton);
   fHideButton->SetAspectRatio(0);
   if (((TGFrame *)fParent)->IsComposite())           // paranoia check
      ((TGCompositeFrame *)fParent)->Layout();
   fHidden = kFALSE;

   SendMessage(fMsgWindow, MK_MSG(kC_DOCK, kDOCK_SHOW), fWidgetId, 0);
}

//______________________________________________________________________________
void TGDockableFrame::HideContainer()
{
   // Hide dock container.

   if (fHidden || !fEnableHide) return;

   HideFrame(fContainer);
   fButtons->HideFrame(fDockButton);
   fHideButton->SetAspectRatio(1);
   if (((TGFrame *)fParent)->IsComposite())           // paranoia check
      ((TGCompositeFrame *)fParent)->Layout();
   fHidden = kTRUE;

   SendMessage(fMsgWindow, MK_MSG(kC_DOCK, kDOCK_HIDE),fWidgetId, 0);
}

//______________________________________________________________________________
Bool_t TGDockableFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Process dockable frame messages.

   switch (GET_MSG(msg)) {
      case kC_COMMAND:
         switch (GET_SUBMSG(msg)) {
            case kCM_BUTTON:
               switch (parm1) {
                  case 1:
                     if (!fHidden) UndockContainer();
                     break;
                  case 2:
                     if (!fHidden)
                        HideContainer();
                     else
                        ShowContainer();
                     break;
               }
               break;
         }
         break;
   }

   return kTRUE;
}

//______________________________________________________________________________
void TGDockableFrame::EnableUndock(Bool_t onoff)
{
   // Enable undocking.

   fEnableUndock = onoff;
   if (onoff)
      fButtons->ShowFrame(fDockButton);
   else
      fButtons->HideFrame(fDockButton);
   Layout();
}

//______________________________________________________________________________
void TGDockableFrame::EnableHide(Bool_t onoff)
{
   // Enable hiding.

   fEnableHide = onoff;
   if (onoff)
      fButtons->ShowFrame(fHideButton);
   else
      fButtons->HideFrame(fHideButton);
   Layout();
}

//______________________________________________________________________________
void TGDockableFrame::SetWindowName(const char *name)
{
   // Set window name so it appear as title of the undock window.

   fDockName = "";
   if (name) {
      fDockName = name;
      if (fFrame) fFrame->SetWindowName(fDockName);
   }
}
