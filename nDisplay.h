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
 * @file        nDisplay.h
 * @summary     Generic Display interface
 * @version     3.3
 * @author      nitacku
 * @data        15 July 2018
 */


#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>

typedef uint8_t type_item;
typedef const __FlashStringHelper* const type_array;

class CDisplay
{
    public:
    
    enum status_t : bool
    {
        STATUS_OK = 0,
        STATUS_ERROR,
    };
    
    enum class Event : uint8_t
    {
        DECREMENT,
        INCREMENT,
        SELECTION,
        TIMEOUT,
    };
    
    enum class Direction : uint8_t
    {
        LEFT,
        RIGHT,
    };

    enum class Mode : uint8_t
    {
        STATIC,
        SCROLL,
    };
    
    enum class Brightness : uint8_t
    {
        AUTO = 0,
        L1 = 1,
        L2 = 2,
        L3 = 3,
        L4 = 4,
        L5 = 5,
        L6 = 6,
        L7 = 7,
        L8 = 8,
        MIN = 0,
        MAX = L8,
    };

    struct PromptSelectStruct
    {
        PromptSelectStruct()
            : item_count{0}
            , initial_selection{0}
            , display_mode{Mode::STATIC}
            , title{nullptr}
            , item_array{nullptr}
        {
            // empty
        }
        
        uint8_t item_count;
        uint8_t initial_selection;
        Mode display_mode;
        const __FlashStringHelper* title;
        const type_array* item_array;
    };
    
    struct PromptValueStruct
    {
        PromptValueStruct() 
            : alphabetic{false}
            , item_count{0}
            , brightness_min{Brightness::L1}
            , brightness_max{Brightness::MAX}
            , item_position{nullptr}
            , item_digit_count{nullptr}
            , item_lower_limit{nullptr}
            , item_upper_limit{nullptr}
            , item_value{nullptr}
            , initial_display{nullptr}
            , title{nullptr}
        {
            // empty
        }
        
        bool alphabetic;
        uint8_t item_count;
        Brightness brightness_min;
        Brightness brightness_max;
        const uint8_t* item_position;
        const uint8_t* item_digit_count;
        const type_item* item_lower_limit;
        const type_item* item_upper_limit;
        type_item* item_value;
        const char* initial_display;
        const __FlashStringHelper* title;
    };
    
    protected:
    
    typedef struct UnitStruct
    {
        char value;
        Brightness brightness;
    } Unit;
    
    typedef struct DisplayStruct
    {
        DisplayStruct()
            : unit_count{0}
            , unit{nullptr}
        {
            // empty
        }
        
        uint8_t unit_count;
        Unit* unit;
    } Display;
    
    protected:
    Display m_display;
    bool (*m_callback_is_increment)();
    bool (*m_callback_is_select)();
    bool (*m_callback_is_update)();
    
    static constexpr auto default_parameter = [](Event event, uint8_t value) -> bool
    {
        (void)event;
        (void)value;
        return false;
    };
    
    public:
    // Constructor
    CDisplay(const uint8_t unit_count);
    ~CDisplay(void);
    
    // Set methods
    status_t SetUnitValue(const uint8_t unit, const char character);
    status_t SetUnitIndicator(const uint8_t unit, const bool state);
    status_t SetUnitBrightness(const uint8_t unit, const Brightness brightness);
    status_t SetDisplayValue(const char* string);
    status_t SetDisplayValue(const __FlashStringHelper* string);
    status_t SetDisplayValue(const uint32_t value);
    status_t SetDisplayIndicator(const bool state);
    status_t SetDisplayBrightness(const Brightness brightness);
    
    void SetCallbackIsIncrement(bool (*function_ptr)(void)) { m_callback_is_increment = function_ptr; }
    void SetCallbackIsSelect(bool (*function_ptr)(void)) { m_callback_is_select = function_ptr; }
    void SetCallbackIsUpdate(bool (*function_ptr)(void)) { m_callback_is_update = function_ptr; }
    
    // Get methods
    uint8_t GetUnitCount(void);
    char GetUnitValue(const uint8_t unit);
    bool GetUnitIndicator(const uint8_t unit);
    Brightness GetUnitBrightness(const uint8_t unit);
    status_t GetDisplayValue(char* string);

    // Effect methods
    void EffectScroll(const char* string, const Direction direction, const uint32_t delay_ms = 50);
    void EffectScroll(const __FlashStringHelper* string, const Direction direction, const uint32_t delay_ms = 50);
    void EffectScroll(const uint32_t value, const Direction direction, const uint32_t delay_ms = 50);
    void EffectSlotMachine(const uint32_t delay_ms = 10);
    void EffectStrobe(const uint8_t iteration = 10, const uint32_t delay_ms = 40);
        
    // Prompt methods
    template<typename Functor = decltype(default_parameter)>
    int8_t PromptSelect(const PromptSelectStruct &prompt, const uint32_t timeout = 500, Functor functor = default_parameter)
    {
        uint32_t timeout_count = timeout * 3000;
        //uint32_t timeout_resolution = timeout_count / m_display.unit_count;
        
        Brightness brightness_copy[m_display.unit_count];

        if (prompt.title != nullptr)
        {
            SetDisplayValue(prompt.title);
            EffectSlotMachine(10);
            delay(1000);
        }

        Direction initial_direction = (prompt.display_mode == Mode::SCROLL) ? \
            ((IsInputIncrement() ? Direction::LEFT : Direction::RIGHT)) : Direction::LEFT;
        
        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            EffectScroll(" ", initial_direction, 25);
            brightness_copy[index] = GetUnitBrightness(index);
        }

        uint8_t selection = prompt.initial_selection;
        EffectScroll(prompt.item_array[selection], initial_direction, 25);

        uint32_t count = 0;
        IsInputUpdate(); // Clear any pending update

        do
        {
            if (IsInputUpdate())
            {
                if (IsInputIncrement())
                {
                    selection += 1;

                    if (selection > prompt.item_count - 1)
                    {
                        selection = 0;
                    }
                    
                    functor(Event::INCREMENT, selection);
                }
                else
                {
                    if (selection > 0)
                    {
                        selection -= 1;
                    }
                    else
                    {
                        selection = prompt.item_count - 1;
                    }
                    
                    functor(Event::DECREMENT, selection);
                }

                if (prompt.display_mode == Mode::SCROLL)
                {
                    Direction direction = (IsInputIncrement() ? Direction::LEFT : Direction::RIGHT);

                    for (uint8_t index = 0; index < m_display.unit_count; index++)
                    {
                        EffectScroll(" ", direction, 25);
                    }

                    EffectScroll(prompt.item_array[selection], direction, 25);
                }
                else
                {
                    SetDisplayValue(prompt.item_array[selection]);
                }

                count = 0;
            }
            else
            {
                /*
                // Display "progress bar"
                if (!(count % timeout_resolution))
                {
                    uint8_t position = (count / timeout_resolution);
                    
                    for (uint8_t index = 0; index < m_display.unit_count; index++)
                    {
                        SetUnitIndicator(index, (index >= position));
                    }
                }
                */
                
                count++;
                
                if (count > timeout_count)
                {
                    if (functor(Event::TIMEOUT, selection)) //Check if we should reset timeout
                    {
                        count = 0;
                        continue;
                    }
                    
                    return -1; // Timeout
                }
            }
        } while (IsInputSelect() == false);

        functor(Event::SELECTION, selection);
        SetDisplayBrightness(Brightness::MAX);
        EffectStrobe(10, 36);
        delay(250);

        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            SetUnitBrightness(index, brightness_copy[index]);
        }

        return selection;
    }
    
    template<typename Functor = decltype(default_parameter)>
    int8_t PromptValue(const PromptValueStruct &prompt, const uint32_t timeout = 4000, Functor functor = default_parameter)
    {
       uint8_t item = 0;
        char s[m_display.unit_count];
        Brightness brightness_copy[m_display.unit_count];

        if (prompt.title != nullptr)
        {
            SetDisplayValue(prompt.title);
            EffectSlotMachine(10);
            delay(1000);
        }

        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            EffectScroll(" ", Direction::LEFT, 25);
            brightness_copy[index] = GetUnitBrightness(index);
        }

        EffectScroll(prompt.initial_display, Direction::LEFT, 25);
        SetDisplayBrightness(prompt.brightness_min);

        do
        {
            uint32_t count = 0;

            if (prompt.alphabetic == true)
            {
                s[m_display.unit_count-1] = prompt.item_value[item];
            }
            else
            {
                itoa(s, prompt.item_value[item]);
            }

            for (uint8_t index = 0; index < prompt.item_digit_count[item]; index++)
            {
                SetUnitValue(prompt.item_position[item] + index, s[m_display.unit_count - prompt.item_digit_count[item] + index]);
                SetUnitBrightness(prompt.item_position[item] + index, prompt.brightness_max);
            }

            IsInputUpdate(); // Clear any pending update
            
            do
            {
                if (IsInputUpdate())
                {
                    if (IsInputIncrement())
                    {
                        prompt.item_value[item] += 1;

                        if (prompt.item_value[item] > prompt.item_upper_limit[item])
                        {
                            prompt.item_value[item] = prompt.item_lower_limit[item];
                        }
                        
                        functor(Event::INCREMENT, prompt.item_value[item]);
                    }
                    else
                    {
                        if (prompt.item_value[item] > 0)
                        {
                            prompt.item_value[item] -= 1;

                            if (prompt.item_value[item] < prompt.item_lower_limit[item])
                            {
                                prompt.item_value[item] = prompt.item_upper_limit[item];
                            }
                        }
                        else
                        {
                            prompt.item_value[item] = prompt.item_upper_limit[item];
                        }
                        
                        functor(Event::DECREMENT, prompt.item_value[item]);
                    }

                    if (prompt.alphabetic == true)
                    {
                        s[m_display.unit_count-1] = prompt.item_value[item];
                    }
                    else
                    {
                        itoa(s, prompt.item_value[item]);
                    }

                    for (uint8_t index = 0; index < prompt.item_digit_count[item]; index++)
                    {
                        SetUnitValue(prompt.item_position[item] + index, s[m_display.unit_count - prompt.item_digit_count[item] + index]);
                        SetUnitBrightness(prompt.item_position[item] + index, prompt.brightness_max);
                    }

                    count = 0;
                }
                else
                {
                    count++;

                    if ((count % timeout) == 0)
                    {
                        for (uint8_t index = 0; index < prompt.item_digit_count[item]; index++)
                        {
                            SetUnitBrightness(prompt.item_position[item] + index, (((count / timeout) % 2) ? prompt.brightness_max : prompt.brightness_min));
                        }
                    }

                    if (count > (timeout * 62))
                    {
                        if (functor(Event::TIMEOUT, prompt.item_value[item])) //Check if we should reset timeout
                        {
                            count = 0;
                            continue;
                        }
                        
                        for (uint8_t index = 0; index < m_display.unit_count; index++)
                        {
                            SetUnitBrightness(index, brightness_copy[index]);
                        }

                        return -1; // Timeout
                    }
                }
            } while (IsInputSelect() == false);
            
            while (IsInputSelect() == true); // Wait while button pressed
            functor(Event::SELECTION, prompt.item_value[item]);

            for (uint8_t index = 0; index < prompt.item_digit_count[item]; index++)
            {
                SetUnitBrightness(prompt.item_position[item] + index, prompt.brightness_min);
            }
        }
        while (++item < prompt.item_count);

        SetDisplayBrightness(Brightness::MAX);
        EffectStrobe(10, 36);
        delay(250);

        for (uint8_t index = 0; index < m_display.unit_count; index++)
        {
            SetUnitBrightness(index, brightness_copy[index]);
        }

        return 0; // OK
    }
    
    protected:
    void itoa(char* s, uint32_t value);
    bool IsInputIncrement(void);
    bool IsInputSelect(void);
    bool IsInputUpdate(void);
    
    // Choose either Arduino or FastLED random implementation
    static uint8_t random_fast(const uint8_t min, const uint8_t max);
    
    private:
    // Initialize display
    void Initialize(const uint8_t unit_count);
};

#endif
