#ifndef __SIGNAL_H__
#define __SIGNAL_H__


#include <functional>
#include <unordered_map>
#include <vector>


namespace signal
{


    template <typename R, typename... Args>
        class Signal
        {
            public:
                using ListenerID = uint;
                using Listener = std::function<R(Args...)>;

            private:
                mutable uint loopers = 0;
                ListenerID next_id = 0;
                std::unordered_map<ListenerID, Listener> listeners{};

            public:
                Signal() {}


                Signal(const Signal &src) :
                    loopers(0),
                    next_id(src.next_id),
                    listeners(src.listeners)
            {}


                const Signal &operator=(const Signal &src)
                {
                    loopers = 0;
                    next_id = src.next_id;
                    listeners = src.listeners;
                }


                ListenerID add_listener(const Listener &listener)
                {
                    listeners[next_id] = listener;
                    return next_id++;
                }


                void remove_listener(ListenerID id)
                {
                    if (loopers) throw std::runtime_error("reentrant use of Signal::remove_listener()");
                    listeners.erase(id);
                }


                void signal(Args... args) const
                {
                    ++loopers;

                    for (const auto &listener : listeners)
                        listener.second(args...);

                    --loopers;
                }


                std::vector<R> signal_return(Args... args) const
                {
                    ++loopers;

                    std::vector<R> result(listeners.size());
                    for (const auto &listener : listeners) {
                        static int i = 0;
                        result[i++] = listener.second(args...);
                    }

                    --loopers;
                    return result;
                }
        };
}


#endif
