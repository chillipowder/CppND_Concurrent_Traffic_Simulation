#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulock(_mtx);
    _cond.wait(ulock, [this] {return !_queue.empty(); });
    T msg = std::move(_queue.back());

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mtx);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase color)
{
    _currentPhase = color;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) 
    {
        TrafficLightPhase tlp = _msgQueue.receive();
        if(tlp == TrafficLightPhase::green) 
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate() // simulate is called when intersection simulate is called 
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;     
    std::mt19937 rng(rd());    
    std::uniform_int_distribution<int> uni(4,6); 
    auto rnd_int = uni(rng);
    
    //first timestamp
    auto t0 = std::chrono::high_resolution_clock::now();

    while(true)
    {
        // second timestamp
        auto t1 = std::chrono::high_resolution_clock::now();
        auto dif = std::chrono::duration_cast<std::chrono::seconds> (t1 - t0).count();
        
        // wait for 4-6 seconds to switch lights
        if(dif >= rnd_int)
        {
            //std::cout << dif << std::endl;
            if(this->getCurrentPhase() == TrafficLightPhase::green){
                this->setCurrentPhase(TrafficLightPhase::red);
                // send update method to the message queue
                _msgQueue.send(std::move(TrafficLightPhase::red));
            } else {
                this->setCurrentPhase(TrafficLightPhase::green);
                // send update method to the message queue
                _msgQueue.send(std::move(TrafficLightPhase::green));
            }
            
            // update first timestamp
            t0 = std::chrono::high_resolution_clock::now();
            auto t0_sec = std::chrono::time_point_cast<std::chrono::seconds>(t0);
            rnd_int = uni(rng);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
