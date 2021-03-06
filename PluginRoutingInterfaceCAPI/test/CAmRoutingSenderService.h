/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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
 *  For further information see http://www.genivi.org/.
 */

#ifndef CAMROUTINGSENDERSERVICE_H_
#define CAMROUTINGSENDERSERVICE_H_

#include "audiomanagertypes.h"
#include <org/genivi/am/RoutingControlStubDefault.h>
#include <org/genivi/am/RoutingControlObserverProxy.h>

/** 
 * GENERATED FROM CMAKE. PLEASE USE cmake/CAmRoutingSenderService.h.in IF YOU WANT TO MAKE CHANGES.
 */

namespace am {

using namespace CommonAPI;

class CAmCommonAPIWrapper;

/** 
 * THE FOLLOWING DEFINES ARE POPULATED FROM THE CMAKELISTS.TXT .
 */
#define CAPI_SENDER_INTERFACE  "org.genivi.audiomanager.routingsender" 
#define CAPI_SENDER_INSTANCE  	"org.genivi.routingsender"  
#define CAPI_SENDER_PATH 		"/org/genivi/audiomanager/routingsender"  

#define CAPI_ROUTING_INTERFACE  "org.genivi.audiomanager.routinginterface"
#define CAPI_ROUTING_INSTANCE   "org.genivi.audiomanager"
#define CAPI_ROUTING_PATH 		 "/org/genivi/audiomanager/routinginterface"


/** Test class implementing the routing sender service.
*
*	Please see cmake/CAmRoutingSenderService.h.in	
*/
class CAmRoutingSenderService : public org::genivi::am::RoutingControlStubDefault {

	org::genivi::am::am_Domain_s mDomainData;
	org::genivi::am::am_Handle_s mAbortedHandle;
	bool mIsServiceAvailable;
	bool mIsDomainRegistred;
	bool mIsReady;
	CAmCommonAPIWrapper *mpWrapper;
	std::shared_ptr<org::genivi::am::RoutingControlObserverProxy<> > mRoutingInterfaceProxy;

public:

	CAmRoutingSenderService();
	CAmRoutingSenderService(CAmCommonAPIWrapper * aWrapper);
	CAmRoutingSenderService(CAmCommonAPIWrapper * aWrapper, std::shared_ptr<org::genivi::am::RoutingControlObserverProxy<> > aProxy);
	virtual ~CAmRoutingSenderService();

	void onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus);
	void onRoutingReadyRundown();
	void onRoutingReadyEvent();
	void registerDomain();
	void setAbortHandle(org::genivi::am::am_Handle_s handle);
	org::genivi::am::am_Error_e errorForHandle(const org::genivi::am::am_Handle_s & handle);

	virtual void asyncSetSourceState(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SourceState_e sourceState);

	virtual void setDomainState(org::genivi::am::am_domainID_t domainID, org::genivi::am::am_DomainState_e domainState, org::genivi::am::am_Error_e& error);

	virtual void asyncSetSourceVolume(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_volume_t volume, org::genivi::am::am_CustomRampType_t ramp, org::genivi::am::am_time_t time);

	virtual void asyncSetSinkVolume(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_volume_t volume, org::genivi::am::am_CustomRampType_t ramp, org::genivi::am::am_time_t time);

	virtual void asyncConnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_CustomConnectionFormat_t connectionFormat);

	virtual void asyncDisconnect(org::genivi::am::am_Handle_s handle, org::genivi::am::am_connectionID_t connectionID);

	virtual void asyncAbort(org::genivi::am::am_Handle_s handle);

	virtual void asyncSetSinkSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_SoundProperty_L listSoundProperties);

	virtual void asyncSetSinkSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_SoundProperty_s soundProperty);

	virtual void asyncSetSourceSoundProperties(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SoundProperty_L listSoundProperties);

	virtual void asyncSetSourceSoundProperty(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_SoundProperty_s soundProperty);

	virtual void asyncCrossFade(org::genivi::am::am_Handle_s handle, org::genivi::am::am_crossfaderID_t crossfaderID, org::genivi::am::am_HotSink_e hotSink, org::genivi::am::am_CustomRampType_t rampType, org::genivi::am::am_time_t time);

	virtual void asyncSetVolumes(org::genivi::am::am_Handle_s handle, org::genivi::am::am_Volumes_L volumes);

	virtual void asyncSetSinkNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_NotificationConfiguration_s notificationConfiguration);

	virtual void asyncSetSourceNotificationConfiguration(org::genivi::am::am_Handle_s handle, org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_NotificationConfiguration_s notificationConfiguration);

	static const char * ROUTING_SENDER_SERVICE;
	static const char * ROUTING_INTERFACE_SERVICE;
};


} /* namespace am */
#endif /* CAMROUTINGSENDERSERVICE_H_ */
