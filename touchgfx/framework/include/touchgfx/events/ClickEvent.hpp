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
 * Copyright (C) 2014-2016 Draupner Graphics A/S <http://www.touchgfx.com>.
 * All rights reserved.
 *
 * TouchGFX is protected by international copyright laws and the knowledge of
 * this source code may not be used to write a similar product. This file may
 * only be used in accordance with a license and should not be re-
 * distributed in any way without the prior permission of Draupner Graphics.
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
#ifndef CLICKEVENT_HPP
#define CLICKEVENT_HPP

#include <touchgfx/Event.hpp>
#include <touchgfx/hal/Types.hpp>

namespace touchgfx
{
/**
 * @class ClickEvent ClickEvent.hpp touchgfx/events/ClickEvent.hpp
 *
 * @brief A click event.
 *
 *        A click event. The semantics of this event is slightly depending on hardware
 *        platform. ClickEvents are generated by the HAL layer.
 *
 * @see Event
 */
class ClickEvent : public Event
{
public:

    /**
     * @typedef enum ClickEventType
     *
     * @brief The click event types.
     */
    typedef enum
    {
        PRESSED,  ///< An enum constant representing the pressed option
        RELEASED, ///< An enum constant representing the released option
        CANCEL    ///< An enum constant representing the cancel option
    } ClickEventType;

    /**
     * @fn ClickEvent::ClickEvent(ClickEventType type, int16_t x, int16_t y, int16_t force = 0)
     *
     * @brief Constructor.
     *
     *        Constructor.
     *
     * @param type  The type of the click event.
     * @param x     The x coordinate of the click event.
     * @param y     The y coordinate of the click event.
     * @param force The force of the click. On touch displays this usually means how hard the
     *              user pressed on the display. On the windows platform, this will always be
     *              zero.
     */
    ClickEvent(ClickEventType type, int16_t x, int16_t y, int16_t force = 0) :
        _type(type), _x(x), _y(y), _force(force) { }

    /**
     * @fn virtual ClickEvent::~ClickEvent()
     *
     * @brief Destructor.
     *
     *        Destructor.
     */
    virtual ~ClickEvent() { }

    /**
     * @fn int16_t ClickEvent::getX() const
     *
     * @brief Gets the x coordinate of this event.
     *
     *        Gets the x coordinate of this event.
     *
     * @return The x coordinate of this event.
     */
    int16_t getX() const
    {
        return _x;
    }

    /**
     * @fn int16_t ClickEvent::getY() const
     *
     * @brief Gets the y coordinate of this event.
     *
     *        Gets the y coordinate of this event.
     *
     * @return The y coordinate of this event.
     */
    int16_t getY() const
    {
        return _y;
    }

    /**
     * @fn void ClickEvent::setX(int16_t x)
     *
     * @brief Sets the x coordinate of this event.
     *
     *        Sets the x coordinate of this event.
     *
     * @param x The x coordinate of this event.
     */
    void setX(int16_t x)
    {
        _x = x;
    }

    /**
     * @fn void ClickEvent::setY(int16_t y)
     *
     * @brief Sets the y coordinate of this event.
     *
     *        Sets the y coordinate of this event.
     *
     * @param y The y coordinate of this event.
     */
    void setY(int16_t y)
    {
        _y = y;
    }

    /**
     * @fn void ClickEvent::setType(ClickEventType type)
     *
     * @brief Sets the click type of this event.
     *
     *        Sets the click type of this event.
     *
     * @param type The type to set.
     */
    void setType(ClickEventType type)
    {
        _type = type;
    }

    /**
     * @fn ClickEventType ClickEvent::getType() const
     *
     * @brief Gets the click type of this event.
     *
     *        Gets the click type of this event.
     *
     * @return The click type of this event.
     */
    ClickEventType getType() const
    {
        return _type;
    }

    /**
     * @fn int16_t ClickEvent::getForce() const
     *
     * @brief Gets the force of the click.
     *
     *        Gets the force of the click. On touch displays this usually means how hard the
     *        user pressed on the display. On the windows platform, this will always be zero.
     *
     * @return The force of the click.
     */
    int16_t getForce() const
    {
        return _force;
    }

    /**
     * @fn virtual Event::EventType ClickEvent::getEventType()
     *
     * @brief Gets event type.
     *
     *        Gets event type.
     *
     * @return The type of this event.
     */
    virtual Event::EventType getEventType()
    {
        return Event::EVENT_CLICK;
    }

private:
    ClickEventType _type;
    int16_t _x;
    int16_t _y;
    int16_t _force;
};

} // namespace touchgfx

#endif // CLICKEVENT_HPP
