/******************************************************************************
 *
 * @brief     This file is part of the TouchGFX 4.5.0 evaluation distribution.
 *
 * @author    Draupner Graphics A/S <http://www.touchgfx.com>
 *
 ******************************************************************************
 *
 * @section Copyright
 *
 * This file is free software and is provided for example purposes. You may
 * use, copy, and modify within the terms and conditions of the license
 * agreement.
 *
 * This is licensed software for evaluation use, any use must strictly comply
 * with the evaluation license agreement provided with delivery of the
 * TouchGFX software.
 *
 * The evaluation license agreement can be seen on www.touchgfx.com
 *
 * @section Disclaimer
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Draupner Graphics A/S has
 * no obligation to support this software. Draupner Graphics A/S is providing
 * the software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Draupner Graphics A/S can not be held liable for any consequential,
 * incidental, or special damages, or any other relief, or for any claim by
 * any third party, arising from your use of this software.
 *
 *****************************************************************************/
#include <gui/right_screen/RightView.hpp>
#include <BitmapDatabase.hpp>

void RightView::setupScreen()
{
    img.setBitmap(Bitmap(BITMAP_BG_TRANSITION_RIGHT_ID));
    img.setXY(0, 0);

    btnLeft.setBitmaps(Bitmap(BITMAP_LEFT_BTN_ID), Bitmap(BITMAP_LEFT_BTN_PRESSED_ID));
    btnLeft.setXY(20, 71);

    // Setup callback
    btnLeft.setAction(buttonClickedCallback);

    // Remember to add widgets to container.
    // They must be added from bottom and out, or else upper layer(s)
    // may block view of elements below.
    add(img);
    add(btnLeft);
}

void RightView::buttonClicked(const AbstractButton& source)
{
    presenter->leftPressed();
}
