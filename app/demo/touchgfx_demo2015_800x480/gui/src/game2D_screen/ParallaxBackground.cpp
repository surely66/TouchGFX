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
#include <gui/game2D_screen/ParallaxBackground.hpp>
#include <BitmapDatabase.hpp>


ParallaxBackground::ParallaxBackground(int16_t width, int16_t height) :
    currentAnimationState(NO_ANIMATION),
    tickCounter(0)
{
    setWidth(width);
    setHeight(height);


    initializeLayer(background, BITMAP_GAME2D_BACKGROUND_ID, 1, 2, 35);
    initializeLayer(treeTops, BITMAP_GAME2D_TREETOPS_ID, 1, 3, 20);
    initializeLayer(ground, BITMAP_GAME2D_GROUND_ID, 1, 5, getHeight() - Bitmap(BITMAP_GAME2D_GROUND_ID).getHeight());
    initializeLayer(leaves, BITMAP_GAME2D_TOPLEAVES_ID, 1, 6, 0);
}

ParallaxBackground::~ParallaxBackground()
{
}

void ParallaxBackground::initializeLayer(Layer& layer, BitmapId bmp, int animationUpdateSpeed, int animationWidth, int y)
{
    layer.image0.setBitmap(Bitmap(bmp));
    layer.image1.setBitmap(Bitmap(bmp));
    layer.image2.setBitmap(Bitmap(bmp));

    layer.image0.setXY(0, y);
    layer.image1.setXY(layer.image0.getRect().right(), y);
    layer.image2.setXY(layer.image1.getRect().right(), y);

    add(layer.image0);
    add(layer.image1);
    add(layer.image2);

    layer.animationUpdateSpeed = animationUpdateSpeed;
    layer.animationWidth = animationWidth;
}

void ParallaxBackground::startAnimation()
{
    currentAnimationState = ANIMATION_RUNNING;

    Application::getInstance()->registerTimerWidget(this);
}

void ParallaxBackground::stopAnimation()
{
    currentAnimationState = NO_ANIMATION;

    Application::getInstance()->unregisterTimerWidget(this);
}

void ParallaxBackground::handleTickEvent()
{
    tickCounter++;

    if (currentAnimationState == ANIMATION_RUNNING)
    {
        moveLayer(background, tickCounter);
        moveLayer(ground, tickCounter);
        moveLayer(leaves, tickCounter);
        moveLayer(treeTops, tickCounter);
    }
}

void ParallaxBackground::moveLayer(Layer& layer, int tickCount)
{
    if (tickCount % layer.animationUpdateSpeed == 0)
    {
        layer.image0.moveTo(layer.image0.getX() - layer.animationWidth, layer.image0.getY());
        layer.image1.moveTo(layer.image1.getX() - layer.animationWidth, layer.image1.getY());
        layer.image2.moveTo(layer.image2.getX() - layer.animationWidth, layer.image2.getY());

        if (layer.image0.getRect().right() < 0)
        {
            layer.image0.moveTo(layer.image2.getRect().right(), layer.image0.getY());
        }

        if (layer.image1.getRect().right() < 0)
        {
            layer.image1.moveTo(layer.image0.getRect().right(), layer.image1.getY());
        }

        if (layer.image2.getRect().right() < 0)
        {
            layer.image2.moveTo(layer.image1.getRect().right(), layer.image2.getY());
        }
    }
}

void ParallaxBackground::addPlayerCharacter(AnimatedImage& player)
{
    insert(&(ground.image2), player);
}

void ParallaxBackground::addCollectibleItem(CollectibleItem& item)
{
    insert(&(ground.image2), item);
}
