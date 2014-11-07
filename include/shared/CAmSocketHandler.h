/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  \file CAmSocketHandler.h
 *  For further information see http://www.genivi.org/.
 */

#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_

#include "audiomanagertypes.h"
#include <sys/socket.h>
#include <stdint.h>
#include <sys/poll.h>
#include <list>
#include <map>
#include <signal.h>

#include <memory>

#include <glib.h>

//#include <iostream> //todo: remove me
namespace am
{

class GLibTimeOut;
class GLibFileDescriptorWatch;

typedef GLibTimeOut* sh_timerHandle_t; //!<this is a handle for a timer to be used with the SocketHandler
typedef GLibFileDescriptorWatch* sh_pollHandle_t; //!<this is a handle for a filedescriptor to be used with the SocketHandler

/**
 * prototype for poll prepared callback
 */
class IAmShPollPrepare
{
public:
    virtual void Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~IAmShPollPrepare()
    {
    }
};

/**
 * prototype for poll fired callback
 */
class IAmShPollFired
{
public:
    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollFired()
    {
    }
};

/**
 * prototype for poll check callback
 */
class IAmShPollCheck
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollCheck()
    {
    }
};

/**
 * prototype for dispatch callback
 */
class IAmShPollDispatch
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollDispatch()
    {
    }
};

/**
 * prototype for the timer callback
 */
class IAmShTimerCallBack
{
public:
    virtual void Call(const sh_timerHandle_t handle, void* userData)=0;
    virtual ~IAmShTimerCallBack()
    {
    }
};

struct WatchFunctions {
	IAmShPollPrepare* prepare;
	IAmShPollFired *fired;
	IAmShPollCheck *check;
	IAmShPollDispatch *dispatch;
};


/**
 * The am::CAmSocketHandler implements a mainloop for the AudioManager. Plugins and different parts of the AudioManager add their filedescriptors to the handler
 * to get called on communication of the filedescriptors.\n
 * More information can be found here : \ref mainl
 */
class CAmSocketHandler
{
public:
    template<class TClass> class TAmShPollFired: public IAmShPollFired
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);

    public:
        TAmShPollFired(TClass* instance, void (TClass::*function)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)) :
                mInstance(instance), //
                mFunction(function) {};

        virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
        {
            (*mInstance.*mFunction)(pollfd, handle, userData);
        }
    };

    template<class TClass> class TAmShPollCheck: public IAmShPollCheck
    {
    private:
        TClass* mInstance;
        bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

    public:
        TAmShPollCheck(TClass* instance, bool (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
                mInstance(instance), //
                mFunction(function) {};

        virtual bool Call(const sh_pollHandle_t handle, void* userData)
        {
            return ((*mInstance.*mFunction)(handle, userData));
        }
    };

    CAmSocketHandler();
    ~CAmSocketHandler();

    /**
     * Returns the GLIB main context used by the main loop.
     * That context can be used to directly add sources of events to wake up the main loop.
     */
    GMainContext* getMainGlibContext() {
    	return m_mainContext;
    }

    am_Error_e addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void* userData, sh_pollHandle_t& handle);
    am_Error_e removeFDPoll(const sh_pollHandle_t handle);
    am_Error_e updateEventFlags(const sh_pollHandle_t handle, const short events);
    am_Error_e addTimer(const timespec timeouts, IAmShTimerCallBack* callback, sh_timerHandle_t& handle, void* userData);
    am_Error_e removeTimer(const sh_timerHandle_t handle);
    am_Error_e restartTimer(const sh_timerHandle_t handle);
    am_Error_e updateTimer(const sh_timerHandle_t handle, const timespec timeouts);
    am_Error_e stopTimer(const sh_timerHandle_t handle);
    void start_listenting();
    void stop_listening();
    void exit_mainloop();
    void receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
        {
            (void) pollfd;
            (void) handle;
            (void) userData;
        }
    bool checkerCallback(const sh_pollHandle_t handle, void* userData)
        {
            (void) handle;
            (void) userData;
            return (false);
        }

    TAmShPollFired<CAmSocketHandler> receiverCallbackT;
    TAmShPollCheck<CAmSocketHandler> checkerCallbackT;

private:

    int mPipe[2];

    bool fdIsValid(const int fd) const;

    GMainContext* m_mainContext = nullptr;
	GMainLoop* m_mainLoop = nullptr;

};

/**
 * template to create the functor for a class
 */
template<class TClass> class TAmShTimerCallBack: public IAmShTimerCallBack
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(sh_timerHandle_t handle, void* userData);

public:
    TAmShTimerCallBack(TClass* instance, void (TClass::*function)(sh_timerHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {
    }

    virtual void Call(sh_timerHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    }
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollPrepare: public IAmShPollPrepare
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollPrepare(TClass* instance, void (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {
    }

    virtual void Call(const sh_pollHandle_t handle, void* userData) override
    {
        (*mInstance.*mFunction)(handle, userData);
    }
};

/**make private, not public
 * template for a callback
 */
template<class TClass> class TAmShPollFired: public IAmShPollFired
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollFired(TClass* instance, void (TClass::*function)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {
    }

    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(pollfd, handle, userData);
    }
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollCheck: public IAmShPollCheck
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollCheck(TClass* instance, bool (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {
    }

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    }
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollDispatch: public IAmShPollDispatch
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollDispatch(TClass* instance, bool (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {
    }

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    }
};

} /* namespace am */
#endif /* SOCKETHANDLER_H_ */
