/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __DIALOG_INTERFACE_H__
#define __DIALOG_INTERFACE_H__
#include <view/view.h>
namespace cdroid{
class Dialog;
class DialogInterface{
public:
    enum{
       /** The identifier for the positive button. */
       BUTTON_POSITIVE = -1,
       /** The identifier for the negative button. */
       BUTTON_NEGATIVE = -2,
       /** The identifier for the neutral button. */
       BUTTON_NEUTRAL  = -3
    };
    DECLARE_UIEVENT(void,OnCancelListener,Dialog&);
    DECLARE_UIEVENT(void,OnDismissListener,Dialog&);
    DECLARE_UIEVENT(void,OnShowListener,Dialog&);
    DECLARE_UIEVENT(void,OnClickListener,Dialog&,int);
    DECLARE_UIEVENT(void,OnMultiChoiceClickListener,Dialog&,int ,bool);
    DECLARE_UIEVENT(void,OnKeyListener,Dialog&,int, KeyEvent&);
public:
    virtual void cancel()=0;
    virtual void dismiss()=0;
};

}//endof namespace
#endif
