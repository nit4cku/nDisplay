/*
 * Copyright (c) 2018 nitacku
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * @file        nDisplay.cpp
 * @summary     Generic Display interface
 * @version     3.3
 * @author      nitacku
 * @data        15 July 2018
 */


#include "nDisplay.h"

#ifdef USE_FASTLED
#define FASTLED_INTERNAL
#include <FastLED.h>
#endif

//---------------------------------------------------------------------
// Implicit Function Prototypes
//---------------------------------------------------------------------
// delay()
// strlen()
// strncpy()
// strncpy_P()


CDisplay::CDisplay(const uint8_t unit_count)
    : m_callback_is_increment{nullptr}
    , m_callback_is_select{nullptr}
    , m_callback_is_update{nullptr}
{
    Initialize(unit_count);
}


CDisplay::~CDisplay(void)
{
    delete[] m_display.unit;
}


void CDisplay::Initialize(const uint8_t unit_count)
{
    // Allocate memory
    m_display.unit = new Unit[unit_count](); // Initialize to 0

    if (m_display.unit != nullptr)
    {
        m_display.unit_count = unit_count;
    }
}


uint8_t CDisplay::GetUnitCount(void)
{
    return m_display.unit_count;
}


CDisplay::status_t CDisplay::SetUnitValue(const uint8_t unit, const char character)
{
    if (unit < m_display.unit_count)
    {
        uint8_t indicator = (m_display.unit[unit].value & 0x80);
        m_display.unit[unit].value = (character & 0x7F) | indicator; // Preserve indicator
        return STATUS_OK;
    }

    return STATUS_ERROR;
}


CDisplay::status_t CDisplay::SetUnitIndicator(const uint8_t unit, const bool state)
{
    if (unit < m_display.unit_count)
    {
        if (state == true)
        {
            m_display.unit[unit].value |= 0x80;
        }
        else
        {
            m_display.unit[unit].value &= ~0x80;            
        }

        return STATUS_OK;
    }

    return STATUS_ERROR;
}


CDisplay::status_t CDisplay::SetUnitBrightness(const uint8_t unit, const Brightness brightness)
{
    if (unit < m_display.unit_count)
    {
        if (brightness <= Brightness::MAX)
        {
            m_display.unit[unit].brightness = brightness;
            return STATUS_OK;
        }
    }

    return STATUS_ERROR;
}


CDisplay::status_t CDisplay::SetDisplayValue(const char* string)
{
    if (string != nullptr)
    {
        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            SetUnitValue(index, string[index]);
        }

        return STATUS_OK;
    }

    return STATUS_ERROR;
}


CDisplay::status_t CDisplay::SetDisplayValue(const __FlashStringHelper* string)
{
    char s[m_display.unit_count];
    PGM_P ptr = reinterpret_cast<PGM_P>(string);
    
    memcpy_P(s, ptr, m_display.unit_count);
    return SetDisplayValue(s);
}


CDisplay::status_t CDisplay::SetDisplayValue(const uint32_t value)
{
    char s[m_display.unit_count];

    itoa(s, value);
    return SetDisplayValue(s);
}


CDisplay::status_t CDisplay::SetDisplayIndicator(const bool state)
{
    for (uint8_t index = 0; index < m_display.unit_count; index++)
    {
        SetUnitIndicator(index, state);
    }

    return STATUS_ERROR;
}


CDisplay::status_t CDisplay::SetDisplayBrightness(const Brightness brightness)
{
    if (brightness <= Brightness::MAX)
    {
        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            m_display.unit[index].brightness = brightness;
        }

        return STATUS_OK;
    }
    
    return STATUS_ERROR;
}


char CDisplay::GetUnitValue(const uint8_t unit)
{
    if (unit < m_display.unit_count)
    {
        return m_display.unit[unit].value & 0x7F; // Mask indicator
    }

    return 0;
}


bool CDisplay::GetUnitIndicator(const uint8_t unit)
{
    if (unit < m_display.unit_count)
    {
        return m_display.unit[unit].value & 0x80;
    }

    return false;
}


CDisplay::Brightness CDisplay::GetUnitBrightness(const uint8_t unit)
{
    if (unit < m_display.unit_count)
    {
        return m_display.unit[unit].brightness;
    }

    return Brightness::MIN;
}


CDisplay::status_t CDisplay::GetDisplayValue(char* string)
{
    if (string != nullptr)
    {
        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            string[index] = m_display.unit[index].value & 0x7F; // Mask indicator
        }
        
        return STATUS_OK;
    }
    
    return STATUS_ERROR;
}


void CDisplay::EffectScroll(const char* string, const Direction direction, const uint32_t delay_ms)
{
    uint8_t string_length = strlen(string);
    char s[m_display.unit_count + string_length];

    if (direction == Direction::LEFT)
    {
        GetDisplayValue(s);
        strncpy(s + m_display.unit_count, string, string_length);
    }
    else
    {
        strncpy(s, string, sizeof(s));
        GetDisplayValue(s + string_length);
    }

    for (uint8_t index = 0; index < string_length; index++)
    {
        if (direction == Direction::LEFT)
        {
            SetDisplayValue(1 + s + index);
        }
        else
        {
            SetDisplayValue(s + string_length - index - 1);
        }

        delay(delay_ms);
    }
}


void CDisplay::EffectScroll(const __FlashStringHelper* string, const Direction direction, const uint32_t delay_ms)
{
    PGM_P ptr = reinterpret_cast<PGM_P>(string);
    char s[strlen_P(ptr) + 1]; // Leave byte for null terminator
    
    strcpy_P(s, ptr); // Copy string including null terminator
    EffectScroll(s, direction, delay_ms);
}


void CDisplay::EffectScroll(const uint32_t value, const Direction direction, const uint32_t delay_ms)
{
    char s[m_display.unit_count + 1] = {0}; // Add null terminator

    itoa(s, value);
    EffectScroll(s, direction, delay_ms);
}


void CDisplay::EffectSlotMachine(const uint32_t delay_ms)
{
    uint8_t array[m_display.unit_count];
    char s[m_display.unit_count];

    GetDisplayValue(s);

    for (uint8_t index = 0; index < m_display.unit_count; index++)
    {
        array[index] = 0;
    }

    for (uint8_t count = 0; count < m_display.unit_count + 3; count++)
    {
        // Iterate at least 3 times before latching values
        if (count > 2)
        {
            // Randomly select next unit to latch
            while (array[random_fast(0, m_display.unit_count)]++);
        }

        // Display random values for 5 cycles
        for (uint8_t repeat = 0; repeat < 5; repeat++)
        {
            for (uint8_t index = 0; index < m_display.unit_count; index++)
            {
                if (array[index] || ((index == 1) && (s[index] == ':')))
                {
                    SetUnitValue(index, s[index]);
                }
                else
                {
                    SetUnitValue(index, '0' + random_fast(0, 10));
                }
            }

            delay(delay_ms);
        }
    }
}


void CDisplay::EffectStrobe(const uint8_t iteration, const uint32_t delay_ms)
{
    char s[m_display.unit_count];

    GetDisplayValue(s);

    for (uint8_t count = 0; count < iteration; count++)
    {
        if (count % 2)
        {
            SetDisplayValue(s);
        }
        else
        {
            for (uint8_t index = 0; index < m_display.unit_count; index++)
            {
                SetUnitValue(index, ' ');
            }
        }

        delay(delay_ms);
    }

    SetDisplayValue(s);
}


void CDisplay::itoa(char* s, uint32_t value)
{
    uint8_t index = 0;

    while (index < m_display.unit_count)
    {
        s[m_display.unit_count - index - 1] = '0' + (value % 10);
        value /= 10;
        index++;
    }
}


bool CDisplay::IsInputIncrement(void)
{
    if (m_callback_is_increment != nullptr)
    {
        return m_callback_is_increment();
    }
    
    return false;
}


bool CDisplay::IsInputSelect(void)
{
    if (m_callback_is_select != nullptr)
    {
        return m_callback_is_select();
    }

    return false;
}


bool CDisplay::IsInputUpdate(void)
{
    if (m_callback_is_update != nullptr)
    {
        return m_callback_is_update();
    }

    return false;
}


uint8_t CDisplay::random_fast(const uint8_t min, const uint8_t max)
{
#ifdef USE_FASTLED
    return random8(min, max); // FastLED implementation
#else
    return random(min, max); // Arduino implementation
#endif
}
