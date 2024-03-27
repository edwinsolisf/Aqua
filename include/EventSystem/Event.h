#pragma once

#include <deque>
#include <mutex>
#include <variant>
#include <functional>

#include "WindowEvents.h"
#include "KeyboardEvents.h"
#include "MouseEvents.h"
#include "Debug/Debug.h"

namespace Aqua
{
    class Event
    {
    public:
        // Types and Data should match in index
        enum class Types
        {
            Any,

            Window_Resized,
            Window_Closed,

            Key_Released,
            Key_Pressed,
            Key_Repeated,

            Mouse_Moved,
            Mouse_Clicked,
        };


        using Data = std::variant<
            NullEventData,

            WindowResizeEventData,
            WindowCloseEventData,

            KeyReleaseEventData,
            KeyPressEventData,
            KeyRepeatEventData,

            MouseMoveEventData,
            MouseClickEventData
        >;

        Types get_type() const noexcept
        {
            return static_cast<Types>(data.index());
        }

        Data data;

        static Event WindowResizeEvent(WindowResizeEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Window_Resized>, event_data };
            Event event{ data };

            return event;
        }

        static Event WindowCloseEvent()
        {
            Data data{ std::in_place_index<(size_t)Types::Window_Closed>, WindowCloseEventData{} };
            Event event{ data };

            return event;
        }

        static Event MouseMoveEvent(MouseMoveEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Mouse_Moved>, event_data };
            Event event{ data };

            return event;
        }

        static Event MouseClickEvent(MouseClickEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Mouse_Clicked>, event_data };
            Event event{ data };

            return event;
        }

        static Event KeyPressEvent(KeyPressEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Key_Pressed>, event_data };
            Event event{ data };

            return event;
        }

        static Event KeyReleaseEvent(KeyReleaseEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Key_Released>, event_data };
            Event event{ data };

            return event;
        }

        static Event KeyRepeatEventData(KeyRepeatEventData event_data)
        {
            Data data{ std::in_place_index<(size_t)Types::Key_Repeated>, event_data };
            Event event{ data };

            return event;
        }
    };

    class EventQueue
    {
    public:
        void enqueue(Event event)
        {
            std::lock_guard lock{ mutex_ };

            AQUA_INFO("Added Event: " + std::to_string((int)event.get_type()));
            
            events_.push_back(std::move(event));
        }

        template<typename FUNC>
        requires requires (FUNC&& func) { std::is_same_v<decltype(std::function(std::forward<FUNC>(func))), std::function<bool(const Event&)>>; }
        bool handle(FUNC&& handle_event)
        {
            std::lock_guard lock{ mutex_ };

            if (!is_empty())
            {
                const auto& event = events_.back();
                
                bool result = handle_event(event);
                AQUA_INFO("Added Event Handle: " + std::to_string((int)event.get_type()) + " " + std::to_string(result));
                if (result)
                    events_.pop_front();

                return result;
            }
            return true;
        }

        bool is_empty() const noexcept { return events_.empty(); }

    private:
        std::mutex mutex_{};
        std::deque<Event> events_;
    };
}