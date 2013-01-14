//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef _LOMSE_BUTTON_CTRL_H__
#define _LOMSE_BUTTON_CTRL_H__

#include "lomse_control.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// A simple, clickable button
class ButtonCtrl : public Control
{
protected:
    LibraryScope& m_libraryScope;
    string m_label;
    ImoStyle* m_style;
    UPoint  m_pos;
    LUnits  m_width;
    LUnits  m_height;
    Color m_bgColor;

public:
    ButtonCtrl(LibraryScope& libScope, Control* pParent, Document* pDoc,
               const string& label, LUnits width=-1.0f, LUnits height=-1.0f,
               ImoStyle* pStyle=NULL);
    virtual ~ButtonCtrl() {}

    //Control mandatory overrides
    USize measure();
    GmoBoxControl* layout(LibraryScope& libraryScope, UPoint pos);
    void on_draw(Drawer* pDrawer, RenderOptions& opt);
    void handle_event(SpEventInfo pEvent);
    LUnits width() { return m_width; }
    LUnits height() { return m_height; }
    LUnits top() { return m_pos.y; }
    LUnits bottom() { return m_pos.y + m_height; }
    LUnits left() { return m_pos.x; }
    LUnits right() { return m_pos.x + m_width; }

    //specific methods
    void set_text(const string& text);
    void set_tooltip(const string& text);
    inline void set_bg_color(Color color) { m_bgColor = color; }

protected:
    GmoBoxControl* m_pMainBox;
    LUnits  m_xLabel;
    LUnits  m_yLabel;

    void select_font();
    ImoStyle* create_default_style();
    void center_text();

};

////---------------------------------------------------------------------------------------
//class GmoShapeButton : public GmoSimpleShape
//{
//protected:
//    FontStorage* m_pFontStorage;
//    LibraryScope& m_libraryScope;
//    ImoButton* m_pButton;
//    LUnits m_xLabel;
//    LUnits m_yLabel;
//    Color m_bgColor;
//
//    friend class ButtonEngrouter;
//    GmoShapeButton(ImoObj* pCreatorImo, UPoint pos, USize size,
//                   LibraryScope& libraryScope);
//
//public:
//    void on_draw(Drawer* pDrawer, RenderOptions& opt);
//
//    //modifiers
//    void change_color(Color color);
//
//protected:
//    void select_font();
//    void center_text();
//
//    //Color get_normal_color();
//};


} //namespace lomse

#endif    //_LOMSE_BUTTON_CTRL_H__
