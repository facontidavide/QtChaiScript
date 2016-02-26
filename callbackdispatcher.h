#ifndef CALLBACKDISPATCHER_H
#define CALLBACKDISPATCHER_H

#include <memory>
#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>

/**
 * Dispatches function on a number of callbacks and cleans up callbacks when
 * they are dead.
 */
template <typename C> class CallbackDispatcher final {
private:
    std::vector<std::weak_ptr<C>> callbacks;
    int32_t concurrent_dispatcher_count = 0;
public:
    std::shared_ptr<C> add(C && callback)
    {
        auto shared = std::make_shared<C>(callback);
        this->callbacks.push_back(shared);
        return shared;
    }

    template <typename ...A>
    void invoke(A && ... args)
    {
        this->concurrent_dispatcher_count++;

        // Go over all callbacks and dispatch on those that are still available.
        typename std::vector<std::weak_ptr<C>>::iterator iter;

        for (iter = this->callbacks.begin(); iter != this->callbacks.end(); ++iter)
        {
            if (auto callback = iter->lock())
            {
                 std::cout << "invoking callback " << callbacks.size() << std::endl;
                (*callback)(std::forward<A>(args)...);
            }
        }

        this->concurrent_dispatcher_count--;

        // Remove all callbacks that are gone, only if we are not dispatching.

        if (0 == this->concurrent_dispatcher_count)
        {
            std::cout << "callback count before erase " << callbacks.size() << std::endl;

            this->callbacks.erase(
                std::remove_if(
                    this->callbacks.begin(),
                    this->callbacks.end(),
                    [] (std::weak_ptr<C> callback) { return callback.expired(); }
                ),
                this->callbacks.end()
            );

            std::cout << "callback count after erase " << callbacks.size() << std::endl;
        }
    }
};

typedef CallbackDispatcher<std::function<void (int)> > SimpleCallbackDispatcher ;
typedef std::shared_ptr< std::function<void (int)> > CallbackSubscriber;



#endif // CALLBACKDISPATCHER_H
