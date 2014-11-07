/**
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmSocketHandler.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "shared/CAmSocketHandler.h"
#include <config.h>
#include <cassert>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <time.h>
#include <algorithm>
#include <features.h>
#include <csignal>
#include <unistd.h>
#include "shared/CAmDltWrapper.h"

namespace am
{

inline GIOCondition operator|(const GIOCondition c1, const GIOCondition c2) {
	return static_cast<GIOCondition>( static_cast<int>(c1) | static_cast<int>(c2) );
}

inline GIOCondition& operator|=(GIOCondition& c1, const GIOCondition c2) {
	c1 = c1 | c2;
	return c1;
}

class GLibFileDescriptorWatch : private GSource {
public:

	GLibFileDescriptorWatch(const pollfd& fd, WatchFunctions& functions, void* userData,
				GMainContext* context) : m_fd(fd), m_mainContext(context) {
		m_functions = functions;
		m_userData = userData;
	}

	~GLibFileDescriptorWatch() {
		disable();
	}

	bool isEnabled() const {
		return (m_source != nullptr);
	}

	void setEventFlags(short events) {
		bool bEnabled = isEnabled();
		disable();
		m_fd.events = events;
		if (bEnabled)
			enable();
	}

	struct GSourceWrapper : public GSource {
		GLibFileDescriptorWatch* m_watch;
	};

	void enable() {
		if (m_source == nullptr) {
			GIOCondition condition = static_cast<GIOCondition>(0);
			if (m_fd.events & POLLIN)
				condition |= G_IO_IN;
			if (m_fd.events & POLLOUT)
				condition |= G_IO_OUT;
			if (m_fd.events & POLLHUP)
				condition |= G_IO_HUP;
			if (m_fd.events & POLLERR)
				condition |= G_IO_ERR;

			// Let GLib allocate a new GSourceWrapper object
			m_source = g_source_new( &s_callbackFunctions, sizeof(GSourceWrapper) );
			reinterpret_cast<GSourceWrapper*>(m_source)->m_watch = this;

			m_fdTag = g_source_add_unix_fd(m_source, m_fd.fd, condition);

			g_source_attach(m_source, m_mainContext);
		}
	}

	void disable() {
		if (m_source != nullptr) {
			g_source_destroy(m_source);
			m_source = nullptr;
		}
	}

	static short int conditionToREvents(GIOCondition condition) {
		short int revents = 0;

		if (condition & G_IO_IN)
			revents |= POLLIN;
		if (condition & G_IO_OUT)
			revents |= POLLOUT;
		if (condition & G_IO_ERR)
			revents |= POLLERR;
		if (condition & G_IO_HUP)
			revents |= POLLHUP;

		return revents;
	}

	static GLibFileDescriptorWatch* getThis(GSource* source) {
		auto watch = static_cast<GSourceWrapper*>(source)->m_watch;
		return watch;
	}

	static gboolean dispatchPrepare(GSource* source, gint* timeout) {
		*timeout = -1;
		auto thiz = getThis(source);
		if (thiz->m_functions.prepare != nullptr) {
			thiz->m_functions.prepare->Call(thiz, thiz->m_userData);
		}

		return false;
	}

	static gboolean dispatchCheck(GSource* source) {
		auto thiz = getThis(source);

		GIOCondition condition = g_source_query_unix_fd(source, thiz->m_fdTag);
		if (condition) {
			thiz->m_fd.revents = conditionToREvents(condition);
			thiz->m_functions.fired->Call(thiz->m_fd, thiz, thiz->m_userData);
		}

		if (thiz->m_functions.check != nullptr)
			return thiz->m_functions.check->Call(thiz, thiz->m_userData);
		else
			return false;
	}

	static gboolean dispatchExecute(GSource* source, GSourceFunc callback, gpointer userData) {
		(void) callback;
		(void) userData;

		auto thiz = getThis(source);
		if (thiz->m_functions.dispatch != nullptr)
			thiz->m_functions.dispatch->Call(thiz, thiz->m_userData);

		// Always return true, otherwise our source gets unregistered
		return true;
	}

private:

	pollfd m_fd;
	GSource* m_source = nullptr;
	GMainContext* m_mainContext;
	gpointer m_fdTag = nullptr;
	void* m_userData;
	WatchFunctions m_functions;

	static GSourceFuncs s_callbackFunctions;
};

GSourceFuncs GLibFileDescriptorWatch::s_callbackFunctions = { .prepare = dispatchPrepare, .check = dispatchCheck, .dispatch = dispatchExecute, .finalize = nullptr};

class GLibTimeOut {
public:

	GLibTimeOut(int durationInMilliseconds, GMainContext* context) {
		m_mainContext = context;
		setDuration(durationInMilliseconds);
	}

	~GLibTimeOut() {
		stop();
	}

	void start() {
		m_source = g_timeout_source_new(m_duration);
		g_source_set_callback(m_source, onTimerCallback, this, nullptr);
		g_source_attach(m_source, m_mainContext);
	}

	void stop() {
		if (m_source != nullptr) {
			g_source_destroy(m_source);
			g_source_unref(m_source);
			m_source = nullptr;
		}
	}

	static gboolean onTimerCallback(gpointer data) {
		GLibTimeOut* timer = (GLibTimeOut*) data;
		timer->m_func->Call(timer, timer->m_userData);

		// return true so that the timer keeps going
		return true;
	}

	bool isEnabled() const {
		return (m_source != nullptr);
	}

	void setCallBackFunction(IAmShTimerCallBack* callback, void * userData) {
		m_func = callback;
		m_userData = userData;
	}

	void setDuration(int durationInMilliseconds) {
		bool bEnabled = isEnabled();
		stop();
		m_duration = durationInMilliseconds;
		if (bEnabled)
			start();
	}

private:
	GMainContext* m_mainContext;
	GSource* m_source = nullptr;
	int m_duration;
	IAmShTimerCallBack* m_func = nullptr;
	void* m_userData = nullptr;
};



CAmSocketHandler::CAmSocketHandler() :
        receiverCallbackT(this, &CAmSocketHandler::receiverCallback),//
        checkerCallbackT(this, &CAmSocketHandler::checkerCallback),//
        mPipe()
{
    if (pipe(mPipe) == -1)
    {
        logError("CAmSerializer could not create pipe!");
    }

    //add the pipe to the poll - nothing needs to be proccessed here we just need the pipe to trigger the ppoll
    short event = 0;
    sh_pollHandle_t handle;
    event |= POLLIN;
    addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, NULL, NULL, handle);
}

CAmSocketHandler::~CAmSocketHandler()
{
}

//todo: maybe have some: give me more time returned?
/**
 * start the block listening for filedescriptors. This is the mainloop.
 */
void CAmSocketHandler::start_listenting()
{
	m_mainLoop = g_main_loop_new(getMainGlibContext(), FALSE);
	g_main_loop_run(m_mainLoop);
	g_main_loop_unref(m_mainLoop);
	m_mainLoop = nullptr;
}

/**
 * exits the loop
 */
void CAmSocketHandler::stop_listening()
{
	g_main_loop_quit(m_mainLoop);
}

/**
 * Adds a filedescriptor to the polling loop
 * @param fd the filedescriptor
 * @param event the event flags
 * @param prepare a callback that is called before the loop is entered
 * @param fired a callback that is called when the filedescriptor needs to be read
 * @param check a callback that is called to check if further actions are neccessary
 * @param dispatch a callback that is called to dispatch the received data
 * @param userData a pointer to userdata that is always passed around
 * @param handle the handle of this poll
 * @return E_OK if the descriptor was added, E_NON_EXISTENT if the fd is not valid
 */
am_Error_e CAmSocketHandler::addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void *userData, sh_pollHandle_t & handle)
{
    if (!fdIsValid(fd))
        return (E_NON_EXISTENT);

    pollfd f;
    f.events = event;
    f.fd = fd;
    WatchFunctions functions;
    functions.prepare = prepare;
    functions.check = check;
    functions.fired = fired;
    functions.dispatch = dispatch;

    handle = new GLibFileDescriptorWatch(f, functions, userData, m_mainContext);
    handle->enable();

    return E_OK;
}

/**
 * removes a filedescriptor from the poll loop
 * @param handle
 * @return
 */
am_Error_e CAmSocketHandler::removeFDPoll(const sh_pollHandle_t handle)
{
	delete handle;
	return E_OK;
}

int timespec2Milliseconds(const timespec& timeouts) {
	return (int) (timeouts.tv_sec) * 1000 + (int)(timeouts.tv_nsec) / 1000000;
}

/**
 * adds a timer to the list of timers. The callback will be fired when the timer is up.
 * This is not a high precise timer, it is very coarse. It is meant to be used for timeouts when waiting
 * for an answer via a filedescriptor.
 * One time timer. If you need again a timer, you need to add a new timer in the callback of the old one.
 * @param timeouts timeouts time until the callback is fired
 * @param callback callback the callback
 * @param handle handle the handle that is created for the timer is returned. Can be used to remove the timer
 * @param userData pointer always passed with the call
 * @return E_OK in case of success
 */
am_Error_e CAmSocketHandler::addTimer(const timespec timeouts, IAmShTimerCallBack* callback, sh_timerHandle_t& handle, void * userData)
{
	handle = new GLibTimeOut(timespec2Milliseconds(timeouts), m_mainContext);
	handle->setCallBackFunction(callback, userData);
	handle->start();
    return E_OK;
}

/**
 * removes a timer from the list of timers
 * @param handle the handle to the timer
 * @return E_OK in case of success, E_UNKNOWN if timer was not found.
 */
am_Error_e CAmSocketHandler::removeTimer(const sh_timerHandle_t handle)
{
	delete handle;
	return E_OK;
}

/**
 * restarts a timer and updates with a new interva
 * @param handle handle to the timer
 * @param timeouts new timout time
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::updateTimer(const sh_timerHandle_t handle, const timespec timeouts)
{
	handle->setDuration(timespec2Milliseconds(timeouts));
    return E_OK;
}

/**
 * restarts a timer with the original value
 * @param handle
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::restartTimer(const sh_timerHandle_t handle)
{
	handle->start();
    return E_OK;
}

/**
 * stops a timer
 * @param handle
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::stopTimer(const sh_timerHandle_t handle)
{
	handle->stop();
	return E_OK;
}

/**
 * updates the eventFlags of a poll
 * @param handle
 * @param events
 * @return @return E_OK on succsess, E_NON_EXISTENT if fd was not found
 */
am_Error_e CAmSocketHandler::updateEventFlags(const sh_pollHandle_t handle, const short events)
{
	handle->setEventFlags(events);
	return E_OK;
}

/**
 * checks if a filedescriptor is validCAmShSubstractTime
 * @param fd the filedescriptor
 * @return true if the fd is valid
 */
bool CAmSocketHandler::fdIsValid(const int fd) const
{
    return (fcntl(fd, F_GETFL) != -1 || errno != EBADF);
}


void CAmSocketHandler::exit_mainloop()
{
    //end the main loop
    stop_listening();

    //fire the ending filedescriptor
    int p(1);
    write(mPipe[1], &p, sizeof(p));
}

}

