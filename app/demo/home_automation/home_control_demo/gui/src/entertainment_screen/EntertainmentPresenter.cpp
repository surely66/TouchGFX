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
#include <gui/entertainment_screen/EntertainmentPresenter.hpp>
#include <gui/entertainment_screen/EntertainmentView.hpp>

EntertainmentPresenter::EntertainmentPresenter(EntertainmentView& v)
    : BasePresenter(v), view(v)
{
}

void EntertainmentPresenter::activate()
{
    BasePresenter::activate();
    view.setAlbumManager(model->getAlbumManager());
    view.setCurrentCenteredAlbum(model->getMusicPlayer().getCurrentAlbum()->getAlbumId());
}

void EntertainmentPresenter::deactivate()
{
    BasePresenter::deactivate();
}

void EntertainmentPresenter::setCurrentAlbum(uint8_t currentAlbum)
{
    model->setCurrentAlbum(currentAlbum);
    model->setPlaying();
}

void EntertainmentPresenter::setCurrentNumber(uint8_t currentSelectedNumber)
{
    // Selected number is not used
}
