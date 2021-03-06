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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmDatabaseHandlerMap.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include "CAmDatabaseHandlerMap.h"
#include "CAmDatabaseObserver.h"
#include "CAmRouter.h"
#include "shared/CAmDltWrapper.h"


namespace am
{

/*
 * Returns an object for given key
 */
template <typename TMapKeyType, class TMapObjectType> TMapObjectType const * objectForKeyIfExistsInMap(const TMapKeyType & key, const std::unordered_map<TMapKeyType,TMapObjectType> & map)
{
	typename std::unordered_map<TMapKeyType,TMapObjectType>::const_iterator iter = map.find(key);
	if( iter!=map.end() )
		return &iter->second;
	return NULL;
}

/*
 * Checks whether any object with key exists in a given map
 */
template <typename TMapKeyType, class TMapObjectType> bool existsObjectWithKeyInMap(const TMapKeyType & key, const std::unordered_map<TMapKeyType,TMapObjectType> & map)
{
	return objectForKeyIfExistsInMap(key, map)!=NULL;
}

/*
 * Comparators
 */
template <class TObject>
struct CAmComparator
{
    bool operator()(const TObject & anObject, const std::string & aValue, void *)
    {
    	return aValue.compare(anObject.name)==0;
    }
};

struct CAmComparatorFlag
{
    bool operator()(const CAmDatabaseHandlerMap::am_Sink_Database_s & anObject, const std::string & aValue, void *contextOrNull)
    {
		bool flag = (contextOrNull!=NULL)?*((bool*)contextOrNull):true;
		return flag==anObject.reserved &&
				aValue.compare(anObject.name)==0;

    }
    bool operator()(const CAmDatabaseHandlerMap::am_Connection_Database_s &anObject, const am_Connection_s &aValue, void *)
    {
		return 0==anObject.reserved &&
				aValue.sinkID == anObject.sinkID &&
				aValue.sourceID == anObject.sourceID &&
				aValue.connectionFormat == anObject.connectionFormat;
    }
    bool operator()(const CAmDatabaseHandlerMap::am_Source_Database_s & anObject, const std::string & aValue, void *contextOrNull)
    {
		bool flag = (contextOrNull!=NULL)?*((bool*)contextOrNull):true;
		return flag==anObject.reserved &&
				aValue.compare(anObject.name)==0;

    }
};

/**
 * \brief Returns an object matching predicate.
 *
 * Convenient method for searching in a given map.
 *
 * @param map Map reference.
 * @param comparator Search predicate.
 * @return NULL or pointer to the found object.
 */
template <class TReturn, typename TIdentifier> const TReturn *  objectMatchingPredicate(const std::unordered_map<TIdentifier, TReturn> & map,
																								  std::function<bool(const TReturn & refObject)> & comparator)
{
	typename std::unordered_map<TIdentifier, TReturn>::const_iterator elementIterator = map.begin();
	for (;elementIterator != map.end(); ++elementIterator)
	{
		if( comparator(elementIterator->second) )
			return &elementIterator->second;
	}
    return NULL;
}

/**
 * \brief Returns the first object matching criteria.
 *
 * A common method for searching in a given map.
 *
 * @param aMap A map reference.
 * @param aComparisonArgument An object which will be used in the comparison.
 * @param aComparator A structure which defines comparison operations.
 * @param context An optional user info passed to the comparators.
 * @return NULL or pointer to the found object.
 */
template <typename TMapKeyType, class TMapObjectType, class TSearchObjectType, class TComparator>
TMapObjectType const * findFirstObjectMatchingCriteria(const std::unordered_map<TMapKeyType, TMapObjectType> & aMap,
															  const TSearchObjectType & aComparisonArgument,
															  TComparator & aComparator,
															  void *context = NULL)
{
	std::function<bool(const TMapObjectType & refObject)> comparator = [&](const TMapObjectType & object)->bool{
		return aComparator(object, aComparisonArgument, context);
	};
	return objectMatchingPredicate(aMap, comparator);
}

/* Domain */

void CAmDatabaseHandlerMap::CAmDomain::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Domain(" << name.c_str() << ") id(" << domainID << ")" << std::endl <<
			"bus name(" << busname.c_str() <<
			") node name(" << nodename.c_str() <<
			") early(" << early <<
			") domainID(" << domainID <<
			") complete(" << complete <<
			") state(" << state <<
			") reserved(" << reserved << ")" << std::endl;
	outString = fmt.str();
}

/* Source */

void CAmDatabaseHandlerMap::CAmSource::getSourceType(am_SourceType_s & sourceType) const
{
	sourceType.name = name;
	sourceType.sourceClassID = sourceClassID;
	sourceType.availability = available;
	sourceType.sourceID = sourceID;
}

void CAmDatabaseHandlerMap::CAmSource::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Source(" << name.c_str() << ") id(" << sourceID << ")" << std::endl <<
			"sourceClassID(" << sourceClassID <<
			") domainID(" << domainID <<
			") visible(" << visible <<
			") volume(" << volume <<
			") interruptState(" << interruptState <<
			") sourceState(" << sourceState <<
			") reserved(" << reserved << ")" <<
			") available([availability:" << available.availability << " availabilityReason:" << available.availabilityReason << "]"
			") listSoundProperties (";
			std::for_each(listSoundProperties.begin(), listSoundProperties.end(), [&](const am_SoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listConnectionFormats (";
			std::for_each(listConnectionFormats.begin(), listConnectionFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listMainSoundProperties (";
			std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listMainNotificationConfigurations (";
			std::for_each(listMainNotificationConfigurations.begin(), listMainNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt << ") listNotificationConfigurations (";
			std::for_each(listNotificationConfigurations.begin(), listNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt <<  ")" << std::endl;
	outString = fmt.str();
}

/* Sink */

void CAmDatabaseHandlerMap::CAmSink::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Sink(" << name.c_str() << ") id(" << sinkID << ")" << std::endl <<
			"sinkClassID(" << sinkClassID <<
			") domainID(" << domainID <<
			") visible(" << visible <<
			") volume(" << volume <<
			") muteState(" << muteState <<
			") mainVolume(" << mainVolume <<
			") reserved(" << reserved << ")" <<
			") available([availability:" << available.availability << " availabilityReason:" << available.availabilityReason << "]"
			") listSoundProperties (";
			std::for_each(listSoundProperties.begin(), listSoundProperties.end(), [&](const am_SoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listConnectionFormats (";
			std::for_each(listConnectionFormats.begin(), listConnectionFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listMainSoundProperties (";
			std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listMainNotificationConfigurations (";
			std::for_each(listMainNotificationConfigurations.begin(), listMainNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt << ") listNotificationConfigurations (";
			std::for_each(listNotificationConfigurations.begin(), listNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt <<  ")" << std::endl;
	outString = fmt.str();
}

void CAmDatabaseHandlerMap::CAmSink::getSinkType(am_SinkType_s & sinkType) const
{
	sinkType.name = name;
	sinkType.sinkID = sinkID;
	sinkType.availability = available;
	sinkType.muteState = muteState;
	sinkType.volume = mainVolume;
	sinkType.sinkClassID = sinkClassID;
}

/* Connection */

void CAmDatabaseHandlerMap::CAmConnection::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Connection id(" << connectionID << ") " << std::endl <<
			"sourceID(" << sourceID <<
			") sinkID(" << sinkID <<
			") delay(" << delay <<
			") connectionFormat(" << connectionFormat <<
			") reserved(" << reserved << ")" << std::endl;
	outString = fmt.str();
}

/* Main Connection */

void CAmDatabaseHandlerMap::CAmMainConnection::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "MainConnection id(" << mainConnectionID << ") " << std::endl <<
			"connectionState(" << connectionState <<
			") sinkID(" << sinkID <<
			") sourceID(" << sourceID <<
			") delay(" << delay <<
			") listConnectionID (";
			std::for_each(listConnectionID.begin(), listConnectionID.end(), [&](const am_connectionID_t & connID) {
				fmt << "["<< connID << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

void CAmDatabaseHandlerMap::am_MainConnection_Database_s::getMainConnectionType(am_MainConnectionType_s & connectionType) const
{
	connectionType.mainConnectionID = mainConnectionID;
	connectionType.sourceID = sourceID;
	connectionType.sinkID = sinkID;
	connectionType.connectionState = connectionState;
	connectionType.delay = delay;
}

/* Source Class */

void CAmDatabaseHandlerMap::CAmSourceClass::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Source class(" << name.c_str() << ") id(" << sourceClassID << ")\n" <<
			") listClassProperties (";
			std::for_each(listClassProperties.begin(), listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				fmt << "[classProperty:" << ref.classProperty << " value:" << ref.value << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

/* Sink Class */

void CAmDatabaseHandlerMap::CAmSinkClass::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Sink class(" << name.c_str() << ") id(" << sinkClassID << ")\n" <<
			") listClassProperties (";
			std::for_each(listClassProperties.begin(), listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				fmt << "[classProperty:" << ref.classProperty << " value:" << ref.value << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}


/* Gateway */

void CAmDatabaseHandlerMap::CAmGateway::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Gateway(" << name.c_str() << ") id(" << gatewayID << ")\n" <<
			"sinkID(" << sinkID <<
			") sourceID(" << sourceID <<
			") domainSinkID(" << domainSinkID <<
			") domainSourceID(" << domainSourceID <<
			") controlDomainID(" << controlDomainID <<
			") listSourceFormats (";
			std::for_each(listSourceFormats.begin(), listSourceFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listSinkFormats (";
			std::for_each(listSinkFormats.begin(), listSinkFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") convertionMatrix (";
			std::for_each(convertionMatrix.begin(), convertionMatrix.end(), [&](const bool & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

/* Crossfader */

void CAmDatabaseHandlerMap::CAmCrossfader::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Crossfader(" << name.c_str() << ") id(" << crossfaderID << ")\n" <<
			"sinkID_A(" << sinkID_A <<
			") sinkID_B(" << sinkID_B <<
			") sourceID(" << sourceID <<
			") hotSink(" << hotSink <<
			")" << std::endl;
	outString = fmt.str();
}

bool CAmDatabaseHandlerMap::CAmMappedData::increaseID(int16_t & resultID, am_Identifier_s & sourceID,
															 int16_t const desiredStaticID = 0)
{
	if( desiredStaticID > 0 && desiredStaticID < sourceID.mMin )
	{
		resultID = desiredStaticID;
		return true;
	}
	else if( sourceID.mCurrentValue < sourceID.mMax ) //The last used value is 'limit' - 1. e.g. SHRT_MAX - 1, SHRT_MAX is reserved.
	{
		resultID = sourceID.mCurrentValue++;
		return true;
	}
	else
	{
		resultID = -1;
		return false;
	}
 }

template <typename TMapKey,class TMapObject> bool CAmDatabaseHandlerMap::CAmMappedData::getNextConnectionID(int16_t & resultID, am_Identifier_s & sourceID,
																			  	  	  	  	  	  	  	  	  	  	  	  const std::unordered_map<TMapKey, TMapObject> & map)
{
	TMapKey nextID;
	int16_t const lastID = sourceID.mCurrentValue;
	if( sourceID.mCurrentValue < sourceID.mMax )
		nextID = sourceID.mCurrentValue++;
	else
		nextID = sourceID.mCurrentValue = sourceID.mMin;

	bool notFreeIDs = false;
	while( existsObjectWithKeyInMap(nextID, map) )
	{

		if( sourceID.mCurrentValue < sourceID.mMax )
			nextID = sourceID.mCurrentValue++;
		else
		{
			sourceID.mCurrentValue = sourceID.mMin;
			nextID = sourceID.mCurrentValue++;
		}

		if( sourceID.mCurrentValue == lastID )
		{
			notFreeIDs = true;
			break;
		}
	}
	if(notFreeIDs)
	{
		resultID = -1;
		return false;
	}
	resultID = nextID;
	return true;
}

bool CAmDatabaseHandlerMap::CAmMappedData::increaseMainConnectionID(int16_t & resultID)
{
	return getNextConnectionID(resultID, mCurrentMainConnectionID, mMainConnectionMap);
}

bool CAmDatabaseHandlerMap::CAmMappedData::increaseConnectionID(int16_t & resultID)
{
	return getNextConnectionID(resultID, mCurrentConnectionID, mConnectionMap);
}


CAmDatabaseHandlerMap::CAmDatabaseHandlerMap():	mFirstStaticSink(true), //
														mFirstStaticSource(true), //
														mFirstStaticGateway(true), //
														mFirstStaticConverter(true), //
														mFirstStaticSinkClass(true), //
														mFirstStaticSourceClass(true), //
														mFirstStaticCrossfader(true), //
														mpDatabaseObserver(NULL), //
														mListConnectionFormat(), //
														mMappedData()
{
	logInfo(__PRETTY_FUNCTION__,"Init ");
}

CAmDatabaseHandlerMap::~CAmDatabaseHandlerMap()
{
    logInfo(__PRETTY_FUNCTION__,"Destroy");
    mpDatabaseObserver = NULL;
}

am_Error_e CAmDatabaseHandlerMap::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    assert(domainData.domainID==0);
    assert(!domainData.name.empty());
    assert(!domainData.busname.empty());
    assert(domainData.state>=DS_UNKNOWN && domainData.state<=DS_MAX);
    //first check for a reserved domain
    CAmComparator<am_Domain_s> comparator;
    am_Domain_s const *reservedDomain = findFirstObjectMatchingCriteria(mMappedData.mDomainMap, domainData.name, comparator);

    int16_t nextID = 0;

    if( NULL != reservedDomain )
    {
    	nextID = reservedDomain->domainID;
    	domainID = nextID;
    	mMappedData.mDomainMap[nextID] = domainData;
    	mMappedData.mDomainMap[nextID].domainID = nextID;
    	mMappedData.mDomainMap[nextID].reserved = 0;
    	logInfo("DatabaseHandler::enterDomainDB entered reserved domain with name=", domainData.name, "busname=", domainData.busname, "nodename=", domainData.nodename, "reserved ID:", domainID);
        if (mpDatabaseObserver)
            mpDatabaseObserver->newDomain(mMappedData.mDomainMap[nextID]);
    	return (E_OK);
    }
    else
    {
		if(mMappedData.increaseID(nextID, mMappedData.mCurrentDomainID))
		{
			domainID = nextID;
			mMappedData.mDomainMap[nextID] = domainData;
			mMappedData.mDomainMap[nextID].domainID = nextID;
			logInfo("DatabaseHandler::enterDomainDB entered new domain with name=", domainData.name, "busname=", domainData.busname, "nodename=", domainData.nodename, "assigned ID:", domainID);
			if (mpDatabaseObserver)
				mpDatabaseObserver->newDomain(mMappedData.mDomainMap[nextID]);
			return (E_OK);
		}
		else
		{
			domainID = 0;
			logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
			return (E_UNKNOWN);
		}
    }
}

int16_t CAmDatabaseHandlerMap::calculateDelayForRoute(const std::vector<am_connectionID_t>& listConnectionID)
{
	int16_t delay = 0;
	std::vector<am_connectionID_t>::const_iterator elementIterator = listConnectionID.begin();
	for (; elementIterator < listConnectionID.end(); ++elementIterator)
	{
		am_connectionID_t key = *elementIterator;
		std::unordered_map<am_connectionID_t, am_Connection_Database_s>::const_iterator it = mMappedData.mConnectionMap.find(key);
		if (it!=mMappedData.mConnectionMap.end())
		{
			int16_t temp_delay = it->second.delay;
			if (temp_delay != -1 && delay != -1)
				delay += temp_delay;
			else
				delay = -1;
		}
	}
	return delay;
}

am_Error_e CAmDatabaseHandlerMap::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    assert(mainConnectionData.mainConnectionID==0);
    assert(mainConnectionData.connectionState>=CS_UNKNOWN && mainConnectionData.connectionState<=CS_MAX);
    assert(mainConnectionData.sinkID!=0);
    assert(mainConnectionData.sourceID!=0);

    int16_t delay = 0;
    int16_t nextID = 0;
	if(mMappedData.increaseMainConnectionID(nextID))
	{
		connectionID = nextID;
		mMappedData.mMainConnectionMap[nextID] = mainConnectionData;
		mMappedData.mMainConnectionMap[nextID].mainConnectionID = nextID;
	}
	else
	{
		connectionID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (E_UNKNOWN);
	}

    //now check the connectionTable for all connections in the route. IF connectionID exist
     delay = calculateDelayForRoute(mainConnectionData.listConnectionID);
     mMappedData.mMainConnectionMap[nextID].delay = delay;
    logInfo("DatabaseHandler::enterMainConnectionDB entered new mainConnection with sourceID", mainConnectionData.sourceID, "sinkID:", mainConnectionData.sinkID, "delay:", delay, "assigned ID:", connectionID);

    if (mpDatabaseObserver)
    {
        am_MainConnectionType_s mainConnection;
        mMappedData.mMainConnectionMap[nextID].getMainConnectionType(mainConnection);
        mpDatabaseObserver->newMainConnection(mainConnection);
        mpDatabaseObserver->mainConnectionStateChanged(connectionID, mMappedData.mMainConnectionMap[nextID].connectionState);
    }

    //finally, we update the delay value for the maintable
    if (delay == 0)
        delay = -1;
    return (changeDelayMainConnection(delay, connectionID));
}

/**
 * Helper method, that inserts a new struct in the map and copies the given into it.
 * This method uses the increaseID function to secure the global id is properly increased.
 **/
bool CAmDatabaseHandlerMap::insertSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    int16_t nextID = 0;
	if(  mMappedData.increaseID(nextID, mMappedData.mCurrentSinkID, sinkData.sinkID) )
	{
		sinkID = nextID;
		mMappedData.mSinkMap[nextID] = sinkData;
		mMappedData.mSinkMap[nextID].sinkID = nextID;
		return (true);
	}
	else
	{
		sinkID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached!");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    assert(sinkData.sinkID<DYNAMIC_ID_BOUNDARY);
    assert(sinkData.domainID!=0);
    assert(!sinkData.name.empty());
    assert(sinkData.sinkClassID!=0);
    //todo: need to check if class exists?,
    assert(!sinkData.listConnectionFormats.empty());
    assert(sinkData.muteState>=MS_UNKNOWN && sinkData.muteState<=MS_MAX);

    am_sinkID_t temp_SinkID = 0;
    am_sinkID_t temp_SinkIndex = 0;
	//if sinkID is zero and the first Static Sink was already entered, the ID is created
    CAmComparatorFlag comparator;
    bool checkForFlag = true;
    am_Sink_s const *reservedDomain = findFirstObjectMatchingCriteria(mMappedData.mSinkMap, sinkData.name, comparator, &checkForFlag);
    if( NULL!=reservedDomain )
    {
    	am_sinkID_t oldSinkID = reservedDomain->sinkID;
    	mMappedData.mSinkMap[oldSinkID] = sinkData;
    	mMappedData.mSinkMap[oldSinkID].reserved = 0;
    	temp_SinkID = oldSinkID;
    	temp_SinkIndex = oldSinkID;
    }
    else
    {
		bool result;
		if ( sinkData.sinkID != 0 || mFirstStaticSink )
		{
			//check if the ID already exists
			if (existSinkNameOrID(sinkData.sinkID, sinkData.name))
			{
				sinkID = sinkData.sinkID;
				return (E_ALREADY_EXISTS);
			}
		}
		result = insertSinkDB(sinkData, temp_SinkID);
		if( false == result )
			return (E_UNKNOWN);
		temp_SinkIndex = temp_SinkID;
    }
    //if the first static sink is entered, we need to set it onto the boundary
    if (sinkData.sinkID == 0 && mFirstStaticSink)
    {
        mFirstStaticSink = false;
    }
    mMappedData.mSinkMap[temp_SinkIndex].sinkID = temp_SinkID;
    sinkID = temp_SinkID;

    am_Sink_s & sink = mMappedData.mSinkMap[temp_SinkID];
    logInfo("DatabaseHandler::enterSinkDB entered new sink with name", sink.name, "domainID:", sink.domainID, "classID:", sink.sinkClassID, "volume:", sink.volume, "assigned ID:", sink.sinkID);

    if (mpDatabaseObserver != NULL)
        sink.sinkID=sinkID;
        mpDatabaseObserver->newSink(sink);
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentCrossfaderID, crossfaderData.crossfaderID))
	{
		crossfaderID = nextID;
		mMappedData.mCrossfaderMap[nextID] = crossfaderData;
		mMappedData.mCrossfaderMap[nextID].crossfaderID = nextID;
		return (true);
	}
	else
	{
		crossfaderID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    assert(crossfaderData.crossfaderID<DYNAMIC_ID_BOUNDARY);
    assert(crossfaderData.hotSink>=HS_UNKNOWN && crossfaderData.hotSink<=HS_MAX);
    assert(!crossfaderData.name.empty());
    assert(existSink(crossfaderData.sinkID_A));
    assert(existSink(crossfaderData.sinkID_B));
    assert(existSource(crossfaderData.sourceID));

    am_crossfaderID_t temp_CrossfaderID = 0;
    am_crossfaderID_t temp_CrossfaderIndex = 0;

    bool result;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    if (crossfaderData.crossfaderID != 0 || mFirstStaticCrossfader)
    {
        //check if the ID already exists
        if (existCrossFader(crossfaderData.crossfaderID))
        {
        	crossfaderID = crossfaderData.crossfaderID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertCrossfaderDB(crossfaderData, temp_CrossfaderID);
	if( false == result )
		return (E_UNKNOWN);
	temp_CrossfaderIndex = temp_CrossfaderID;

    //if the first static sink is entered, we need to set it onto the boundary
    if ( 0==crossfaderData.crossfaderID && mFirstStaticCrossfader)
    {
        mFirstStaticCrossfader = false;
    }

   mMappedData.mCrossfaderMap[temp_CrossfaderIndex].crossfaderID = temp_CrossfaderID;
   crossfaderID = temp_CrossfaderID;
   logInfo("DatabaseHandler::enterCrossfaderDB entered new crossfader with name=", crossfaderData.name, "sinkA= ", crossfaderData.sinkID_A, "sinkB=", crossfaderData.sinkID_B, "source=", crossfaderData.sourceID, "assigned ID:", crossfaderID);

    if (mpDatabaseObserver)
        mpDatabaseObserver->newCrossfader(mMappedData.mCrossfaderMap[temp_CrossfaderIndex]);
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentGatewayID, gatewayData.gatewayID))
	{
		gatewayID = nextID;
		mMappedData.mGatewayMap[nextID] = gatewayData;
		mMappedData.mGatewayMap[nextID].gatewayID = nextID;
		return (true);
	}
	else
	{
		gatewayID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    assert(gatewayData.gatewayID<DYNAMIC_ID_BOUNDARY);
    assert(gatewayData.sinkID!=0);
    assert(gatewayData.sourceID!=0);
    assert(gatewayData.controlDomainID!=0);
    assert(gatewayData.domainSinkID!=0);
    assert(gatewayData.domainSourceID!=0);
    assert(!gatewayData.name.empty());
    assert(!gatewayData.convertionMatrix.empty());
    assert(!gatewayData.listSinkFormats.empty());
    assert(!gatewayData.listSourceFormats.empty());

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    am_gatewayID_t temp_GatewayID = 0;
    am_gatewayID_t temp_GatewayIndex = 0;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    bool result;
    if (gatewayData.gatewayID != 0 || mFirstStaticGateway)
    {
        //check if the ID already exists
        if (existGateway(gatewayData.gatewayID))
        {
        	gatewayID = gatewayData.gatewayID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertGatewayDB(gatewayData, temp_GatewayID);
	if( false == result )
		return (E_UNKNOWN);

	temp_GatewayIndex = temp_GatewayID;
    //if the ID is not created, we add it to the query
    if (gatewayData.gatewayID == 0 && mFirstStaticGateway)
    {
        mFirstStaticGateway = false;
    }
    mMappedData.mGatewayMap[temp_GatewayIndex].gatewayID = temp_GatewayID;
    gatewayID = temp_GatewayID;

    logInfo("DatabaseHandler::enterGatewayDB entered new gateway with name", gatewayData.name, "sourceID:", gatewayData.sourceID, "sinkID:", gatewayData.sinkID, "assigned ID:", gatewayID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->newGateway(mMappedData.mGatewayMap[temp_GatewayIndex]);
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertConverterDB(const am_Converter_s & converteData, am_converterID_t & converterID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentConverterID, converteData.converterID))
	{
		converterID = nextID;
		mMappedData.mConverterMap[nextID] = converteData;
		mMappedData.mConverterMap[nextID].converterID = nextID;
		return (true);
	}
	else
	{
		converterID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterConverterDB(const am_Converter_s & converterData, am_converterID_t & converterID)
{
    assert(converterData.converterID<DYNAMIC_ID_BOUNDARY);
    assert(converterData.sinkID!=0);
    assert(converterData.sourceID!=0);
    assert(converterData.domainID!=0);
    assert(!converterData.name.empty());
    assert(!converterData.convertionMatrix.empty());
    assert(!converterData.listSinkFormats.empty());
    assert(!converterData.listSourceFormats.empty());

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    am_converterID_t tempID = 0;
    am_converterID_t tempIndex = 0;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    bool result;
    if (converterData.converterID != 0 || mFirstStaticConverter)
    {
        //check if the ID already exists
        if (existGateway(converterData.converterID))
        {
        	converterID = converterData.converterID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertConverterDB(converterData, tempID);
	if( false == result )
		return (E_UNKNOWN);

	tempIndex = tempID;
    //if the ID is not created, we add it to the query
    if (converterData.converterID == 0 && mFirstStaticConverter)
    {
    	mFirstStaticConverter = false;
    }
    mMappedData.mConverterMap[tempIndex].converterID = tempID;
    converterID = tempID;

    logInfo("DatabaseHandler::enterConverterDB entered new converter with name", converterData.name, "sourceID:", converterData.sourceID, "sinkID:", converterData.sinkID, "assigned ID:", converterID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->newConverter(mMappedData.mConverterMap[tempIndex]);
    return (E_OK);
}

void CAmDatabaseHandlerMap::dump( std::ostream & output ) const
{
	output << std::endl << "****************** DUMP START ******************" << std::endl;
	CAmMappedData::printMap(mMappedData.mDomainMap, output);
	CAmMappedData::printMap(mMappedData.mSourceMap, output);
	CAmMappedData::printMap(mMappedData.mSinkMap, output);
	CAmMappedData::printMap(mMappedData.mSourceClassesMap, output);
	CAmMappedData::printMap(mMappedData.mSinkClassesMap, output);
	CAmMappedData::printMap(mMappedData.mConnectionMap, output);
	CAmMappedData::printMap(mMappedData.mMainConnectionMap, output);
	CAmMappedData::printMap(mMappedData.mCrossfaderMap, output);
	CAmMappedData::printMap(mMappedData.mGatewayMap, output);
	CAmVectorSystemProperties::const_iterator iter = mMappedData.mSystemProperties.begin();
	output << "System properties" << "\n";
	for(; iter!=mMappedData.mSystemProperties.end(); iter++)
		output << "[type:" << iter->type << " value:" << iter->value << "]";
	output << std::endl << "****************** DUMP END ******************" << std::endl;
}

bool CAmDatabaseHandlerMap::insertSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceID, sourceData.sourceID))
	{
		sourceID = nextID;
		mMappedData.mSourceMap[nextID] = sourceData;
		mMappedData.mSourceMap[nextID].sourceID = nextID;
		return (true);
	}
	else
	{
		sourceID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    assert(sourceData.sourceID<DYNAMIC_ID_BOUNDARY);
    assert(sourceData.domainID!=0);
    assert(!sourceData.name.empty());
    assert(sourceData.sourceClassID!=0);
    // \todo: need to check if class exists?
    assert(!sourceData.listConnectionFormats.empty());
    assert(sourceData.sourceState>=SS_UNKNNOWN && sourceData.sourceState<=SS_MAX);

    bool isFirstStatic = sourceData.sourceID == 0 && mFirstStaticSource;
    am_sourceID_t temp_SourceID = 0;
    am_sourceID_t temp_SourceIndex = 0;
    bool checkForFlag = true;
    CAmComparatorFlag comparator;
    am_Source_Database_s const *reservedSource = findFirstObjectMatchingCriteria(mMappedData.mSourceMap, sourceData.name, comparator, &checkForFlag);
	if( NULL != reservedSource )
	{
		am_sourceID_t oldSourceID = reservedSource->sourceID;
		mMappedData.mSourceMap[oldSourceID] = sourceData;
		mMappedData.mSourceMap[oldSourceID].reserved = 0;
		temp_SourceID = oldSourceID;
		temp_SourceIndex = oldSourceID;
	}
	else
	{
	    bool result;
	    if ( !isFirstStatic )
	    {
	        //check if the ID already exists
	    	 if (existSourceNameOrID(sourceData.sourceID, sourceData.name))
	    	 {
	    		sourceID = sourceData.sourceID;
	            return (E_ALREADY_EXISTS);
	    	 }
	    }
	    result = insertSourceDB(sourceData, temp_SourceID);
		if( false == result )
			return (E_UNKNOWN);
		temp_SourceIndex = temp_SourceID;
    }

    if ( isFirstStatic )
    {
      //if the first static sink is entered, we need to set it onto the boundary if needed
        mFirstStaticSource = false;
    }
    mMappedData.mSourceMap[temp_SourceIndex].sourceID = temp_SourceID;
    sourceID = temp_SourceID;

    logInfo("DatabaseHandler::enterSourceDB entered new source with name", sourceData.name, "domainID:", sourceData.domainID, "classID:", sourceData.sourceClassID, "visible:", sourceData.visible, "assigned ID:", sourceID);

    if (mpDatabaseObserver)
        mpDatabaseObserver->newSource(mMappedData.mSourceMap[temp_SourceIndex]);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID)
{
    assert(connection.connectionID==0);
    assert(connection.sinkID!=0);
    assert(connection.sourceID!=0);
    //connection format is not checked, because it's project specific
    int16_t nextID = 0;
    if(mMappedData.increaseConnectionID(nextID))
	{
		connectionID = nextID;
		mMappedData.mConnectionMap[nextID] = connection;
		mMappedData.mConnectionMap[nextID].connectionID = nextID;
		mMappedData.mConnectionMap[nextID].reserved = true;
	}
	else
	{
		connectionID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (E_UNKNOWN);
	}

    logInfo("DatabaseHandler::enterConnectionDB entered new connection sourceID=", connection.sourceID, "sinkID=", connection.sinkID, "sourceID=", connection.sourceID, "connectionFormat=", connection.connectionFormat, "assigned ID=", connectionID);
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSinkClassesID, sinkClass.sinkClassID))
	{
		sinkClassID = nextID;
		mMappedData.mSinkClassesMap[nextID] = sinkClass;
		mMappedData.mSinkClassesMap[nextID].sinkClassID = nextID;
		return (true);
	}
	else
	{
		sinkClassID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    assert(sinkClass.sinkClassID<DYNAMIC_ID_BOUNDARY);
    assert(!sinkClass.name.empty());

    am_sinkClass_t temp_SinkClassID = 0;
    am_sinkClass_t temp_SinkClassIndex = 0;

	bool result;
	if (sinkClass.sinkClassID != 0 || mFirstStaticSinkClass)
	{
		//check if the ID already exists
		 if (existSinkClass(sinkClass.sinkClassID))
		 {
			 sinkClassID = sinkClass.sinkClassID;
			return (E_ALREADY_EXISTS);
		 }
	}
	result = insertSinkClassDB(sinkClass, temp_SinkClassID);
	if( false == result )
		return (E_UNKNOWN);

	temp_SinkClassIndex = temp_SinkClassID;
	//if the ID is not created, we add it to the query
	if (sinkClass.sinkClassID == 0 && mFirstStaticSinkClass)
	{
		mFirstStaticSinkClass = false;
	}
	mMappedData.mSinkClassesMap[temp_SinkClassIndex].sinkClassID = temp_SinkClassID;
	sinkClassID = temp_SinkClassID;

    //todo:change last_insert implementations for multithreaded usage...
    logInfo("DatabaseHandler::enterSinkClassDB entered new sinkClass");
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSinkClassesChanged();
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceClassesID, sourceClass.sourceClassID))
	{
		sourceClassID = nextID;
		mMappedData.mSourceClassesMap[nextID] = sourceClass;
		mMappedData.mSourceClassesMap[nextID].sourceClassID = nextID;
		return (true);
	}
	else
	{
		sourceClassID = 0;
		logInfo(__PRETTY_FUNCTION__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    assert(sourceClass.sourceClassID<DYNAMIC_ID_BOUNDARY);
    assert(!sourceClass.name.empty());

    am_sourceClass_t temp_SourceClassID = 0;
    am_sourceClass_t temp_SourceClassIndex = 0;

	bool result;
	if (sourceClass.sourceClassID != 0 || mFirstStaticSourceClass)
	{
		//check if the ID already exists
		if (existSourceClass(sourceClass.sourceClassID))
		{
			sourceClassID = sourceClass.sourceClassID;
			return (E_ALREADY_EXISTS);
		}
	}
	result = insertSourceClassDB(temp_SourceClassID, sourceClass);
	if( false == result )
		return (E_UNKNOWN);

	temp_SourceClassIndex = temp_SourceClassID;
	//if the ID is not created, we add it to the query
	if (sourceClass.sourceClassID == 0 && mFirstStaticSourceClass)
	{
		mFirstStaticSinkClass = false;
	}
	mMappedData.mSourceClassesMap[temp_SourceClassIndex].sourceClassID = temp_SourceClassID;
	sourceClassID = temp_SourceClassID;

	//todo:change last_insert implementations for multithread usage...

    logInfo("DatabaseHandler::enterSourceClassDB entered new sourceClass");

    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSourceClassesChanged();
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::enterSystemProperties(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
    assert(!listSystemProperties.empty());

    mMappedData.mSystemProperties = listSystemProperties;

    logInfo("DatabaseHandler::enterSystemProperties entered system properties");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID)
{
    assert(mainconnectionID!=0);
    if (!existMainConnection(mainconnectionID))
    {
        return (E_NON_EXISTENT);
    }

    int16_t delay = calculateDelayForRoute(listConnectionID);

    //now we replace the data in the main connection object with the new one
    mMappedData.mMainConnectionMap[mainconnectionID].listConnectionID = listConnectionID;

    if (changeDelayMainConnection(delay,mainconnectionID)!=E_OK)
        logError("DatabaseHandler::changeMainConnectionRouteDB error while changing mainConnectionDelay to ", delay);

    logInfo("DatabaseHandler::changeMainConnectionRouteDB entered new route:", mainconnectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
    assert(mainconnectionID!=0);
    assert(connectionState>=CS_UNKNOWN && connectionState<=CS_MAX);

    if (!existMainConnection(mainconnectionID))
    {
        return (E_NON_EXISTENT);
    }
    mMappedData.mMainConnectionMap[mainconnectionID].connectionState = connectionState;

    logInfo("DatabaseHandler::changeMainConnectionStateDB changed mainConnectionState of MainConnection:", mainconnectionID, "to:", connectionState);
    if (mpDatabaseObserver)
        mpDatabaseObserver->mainConnectionStateChanged(mainconnectionID, connectionState);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkMap[sinkID].mainVolume = mainVolume;

    logInfo("DatabaseHandler::changeSinkMainVolumeDB changed mainVolume of sink:", sinkID, "to:", mainVolume);

    if (mpDatabaseObserver)
        mpDatabaseObserver->volumeChanged(sinkID, mainVolume);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);
    assert(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkMap[sinkID].available = availability;

    logInfo("DatabaseHandler::changeSinkAvailabilityDB changed sinkAvailability of sink:", sinkID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (mpDatabaseObserver && sinkVisible(sinkID))
        mpDatabaseObserver->sinkAvailabilityChanged(sinkID, availability);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{
    assert(domainID!=0);
    assert(domainState>=DS_UNKNOWN && domainState<=DS_MAX);

    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mDomainMap[domainID].state = domainState;

    logInfo("DatabaseHandler::changDomainStateDB changed domainState of domain:", domainID, "to:", domainState);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);
    assert(muteState>=MS_UNKNOWN && muteState<=MS_MAX);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkMap[sinkID].muteState = muteState;

    logInfo("DatabaseHandler::changeSinkMuteStateDB changed sinkMuteState of sink:", sinkID, "to:", muteState);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sinkMuteStateChanged(sinkID, muteState);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    std::vector<am_MainSoundProperty_s>::iterator elementIterator = mMappedData.mSinkMap[sinkID].listMainSoundProperties.begin();
	for (;elementIterator != mMappedData.mSinkMap[sinkID].listMainSoundProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == soundProperty.type)
			elementIterator->value = soundProperty.value;
	}

    logInfo("DatabaseHandler::changeMainSinkSoundPropertyDB changed MainSinkSoundProperty of sink:", sinkID, "type:", soundProperty.type, "to:", soundProperty.value);
    if (mpDatabaseObserver)
        mpDatabaseObserver->mainSinkSoundPropertyChanged(sinkID, soundProperty);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    std::vector<am_MainSoundProperty_s>::iterator elementIterator = mMappedData.mSourceMap[sourceID].listMainSoundProperties.begin();
	for (;elementIterator != mMappedData.mSourceMap[sourceID].listMainSoundProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == soundProperty.type)
			elementIterator->value = soundProperty.value;
	}

    logInfo("DatabaseHandler::changeMainSourceSoundPropertyDB changed MainSinkSoundProperty of source:", sourceID, "type:", soundProperty.type, "to:", soundProperty.value);

    if (mpDatabaseObserver)
        mpDatabaseObserver->mainSourceSoundPropertyChanged(sourceID, soundProperty);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);
    assert(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSourceMap[sourceID].available = availability;

    logInfo("DatabaseHandler::changeSourceAvailabilityDB changed changeSourceAvailabilityDB of source:", sourceID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (mpDatabaseObserver && sourceVisible(sourceID))
        mpDatabaseObserver->sourceAvailabilityChanged(sourceID, availability);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
    std::vector<am_SystemProperty_s>::iterator elementIterator = mMappedData.mSystemProperties.begin();
	for (;elementIterator != mMappedData.mSystemProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == property.type)
			elementIterator->value = property.value;
	}

    logInfo("DatabaseHandler::changeSystemPropertyDB changed system property");

    if (mpDatabaseObserver)
        mpDatabaseObserver->systemPropertyChanged(property);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{
    assert(mainConnectionID!=0);

    if (!existMainConnection(mainConnectionID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mMainConnectionMap.erase(mainConnectionID);

    logInfo("DatabaseHandler::removeMainConnectionDB removed:", mainConnectionID);
    if (mpDatabaseObserver)
    {
        mpDatabaseObserver->mainConnectionStateChanged(mainConnectionID, CS_DISCONNECTED);
        mpDatabaseObserver->removedMainConnection(mainConnectionID);
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSinkDB(const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    bool visible = sinkVisible(sinkID);

    mMappedData.mSinkMap.erase(sinkID);
    // todo: Check the tables SinkMainSoundProperty and SinkMainNotificationConfiguration with 'visible' set to true
    //if visible is true then delete SinkMainSoundProperty and SinkMainNotificationConfiguration ????
    logInfo("DatabaseHandler::removeSinkDB removed:", sinkID);

    if (mpDatabaseObserver != NULL)
        mpDatabaseObserver->removedSink(sinkID, visible);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSourceDB(const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    bool visible = sourceVisible(sourceID);

    mMappedData.mSourceMap.erase(sourceID);

    // todo: Check the tables SourceMainSoundProperty and SourceMainNotificationConfiguration with 'visible' set to true
    //if visible is true then delete SourceMainSoundProperty and SourceMainNotificationConfiguration ????

    logInfo("DatabaseHandler::removeSourceDB removed:", sourceID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removedSource(sourceID, visible);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeGatewayDB(const am_gatewayID_t gatewayID)
{
    assert(gatewayID!=0);

    if (!existGateway(gatewayID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mGatewayMap.erase(gatewayID);

    logInfo("DatabaseHandler::removeGatewayDB removed:", gatewayID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeGateway(gatewayID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeConverterDB(const am_converterID_t converterID)
{
    assert(converterID!=0);

    if (!existConverter(converterID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mConverterMap.erase(converterID);

    logInfo("DatabaseHandler::removeConverterDB removed:", converterID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeConverter(converterID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{
    assert(crossfaderID!=0);

    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }
    mMappedData.mCrossfaderMap.erase(crossfaderID);

    logInfo("DatabaseHandler::removeCrossfaderDB removed:", crossfaderID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeCrossfader(crossfaderID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeDomainDB(const am_domainID_t domainID)
{
    assert(domainID!=0);

    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
    mMappedData.mDomainMap.erase(domainID);

    logInfo("DatabaseHandler::removeDomainDB removed:", domainID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->removeDomain(domainID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{
    assert(sinkClassID!=0);

    if (!existSinkClass(sinkClassID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkClassesMap.erase(sinkClassID);

    logInfo("DatabaseHandler::removeSinkClassDB removed:", sinkClassID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSinkClassesChanged();

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{
    assert(sourceClassID!=0);

    if (!existSourceClass(sourceClassID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSourceClassesMap.erase(sourceClassID);
    logInfo("DatabaseHandler::removeSourceClassDB removed:", sourceClassID);
    if (mpDatabaseObserver)
        mpDatabaseObserver->numberOfSourceClassesChanged();
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeConnection(const am_connectionID_t connectionID)
{
    assert(connectionID!=0);

    mMappedData.mConnectionMap.erase(connectionID);

    logInfo("DatabaseHandler::removeConnection removed:", connectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    am_Source_Database_s source = mMappedData.mSourceMap.at(sourceID);
    classInfo.sourceClassID  = source.sourceClassID;

    if (!existSourceClass(classInfo.sourceClassID))
    {
        return (E_NON_EXISTENT);
    }
    am_SourceClass_s tmpClass = mMappedData.mSourceClassesMap.at(classInfo.sourceClassID);
    classInfo = tmpClass;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s & sinkData) const
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
        return (E_NON_EXISTENT);

    am_Sink_Database_s mappedSink = mMappedData.mSinkMap.at(sinkID);
    if( true == mappedSink.reserved )
    	return (E_NON_EXISTENT);
	sinkData = mappedSink;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s & sourceData) const
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
        return (E_NON_EXISTENT);

    am_Source_Database_s mappedSource = mMappedData.mSourceMap.at(sourceID);
    if( true == mappedSource.reserved )
    	return (E_NON_EXISTENT);

    sourceData = mappedSource;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s & mainConnectionData) const
{
    assert(mainConnectionID!=0);
    if (!existMainConnection(mainConnectionID))
    {
        return (E_NON_EXISTENT);
    }
    am_MainConnection_s temp = mMappedData.mMainConnectionMap.at(mainConnectionID);
    mainConnectionData = temp;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkClassInfoDB(const am_SinkClass_s& sinkClass)
{
    assert(sinkClass.sinkClassID!=0);
    assert(!sinkClass.listClassProperties.empty());

    //check if the ID already exists
    if (!existSinkClass(sinkClass.sinkClassID))
        return (E_NON_EXISTENT);

    mMappedData.mSinkClassesMap[sinkClass.sinkClassID].listClassProperties = sinkClass.listClassProperties;

    logInfo("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceClassInfoDB(const am_SourceClass_s& sourceClass)
{
    assert(sourceClass.sourceClassID!=0);
    assert(!sourceClass.listClassProperties.empty());

    //check if the ID already exists
    if (!existSourceClass(sourceClass.sourceClassID))
        return (E_NON_EXISTENT);

    mMappedData.mSourceClassesMap[sourceClass.sourceClassID].listClassProperties = sourceClass.listClassProperties;

    logInfo("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    am_Sink_Database_s sink = mMappedData.mSinkMap.at(sinkID);
    sinkClass.sinkClassID  = sink.sinkClassID;

    if (!existSinkClass(sinkClass.sinkClassID))
    {
        return (E_NON_EXISTENT);
    }
    am_SinkClass_s tmpSinkClass = mMappedData.mSinkClassesMap.at(sinkClass.sinkClassID);
    sinkClass = tmpSinkClass;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
    assert(gatewayID!=0);
    if (!existGateway(gatewayID))
    {
        return (E_NON_EXISTENT);
    }

    gatewayData = mMappedData.mGatewayMap.at(gatewayID);

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getConverterInfoDB(const am_converterID_t converterID, am_Converter_s& converterData) const
{
    assert(converterID!=0);
    if (!existConverter(converterID))
    {
        return (E_NON_EXISTENT);
    }

    converterData = mMappedData.mConverterMap.at(converterID);

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
    assert(crossfaderID!=0);
    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }

    crossfaderData = mMappedData.mCrossfaderMap.at(crossfaderID);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
    assert(domainID!=0);
    listSinkID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }

    std::unordered_map<am_sinkID_t, am_Sink_Database_s>::const_iterator elementIterator = mMappedData.mSinkMap.begin();
	for (;elementIterator != mMappedData.mSinkMap.end(); ++elementIterator)
	{
		if (0==elementIterator->second.reserved && domainID==elementIterator->second.domainID)
			listSinkID.push_back(elementIterator->second.sinkID);
	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
    assert(domainID!=0);
    listSourceID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }
     CAmMapSource::const_iterator elementIterator = mMappedData.mSourceMap.begin();
	for (;elementIterator != mMappedData.mSourceMap.end(); ++elementIterator)
	{
		if (0==elementIterator->second.reserved && domainID==elementIterator->second.domainID)
			listSourceID.push_back(elementIterator->second.sourceID);
	}

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listCrossfader) const
{
    assert(domainID!=0);
    listCrossfader.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }

    CAmMapSource::const_iterator sourceIterator = mMappedData.mSourceMap.begin();
	for (;sourceIterator != mMappedData.mSourceMap.end(); ++sourceIterator)
	{
		if (domainID==sourceIterator->second.domainID)
		{
			CAmMapCrossfader::const_iterator elementIterator = mMappedData.mCrossfaderMap.begin();
			for (;elementIterator != mMappedData.mCrossfaderMap.end(); ++elementIterator)
			{
				if ( sourceIterator->second.sourceID==elementIterator->second.sourceID )
					listCrossfader.push_back(elementIterator->second.crossfaderID);
			}
		}
	}

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
    assert(domainID!=0);
    listGatewaysID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }

    CAmMapGateway::const_iterator elementIterator = mMappedData.mGatewayMap.begin();
 	for (;elementIterator != mMappedData.mGatewayMap.end(); ++elementIterator)
 	{
 		if (domainID==elementIterator->second.controlDomainID)
 			listGatewaysID.push_back(elementIterator->second.gatewayID);
 	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConvertersOfDomain(const am_domainID_t domainID, std::vector<am_converterID_t>& listConvertersID) const
{
    assert(domainID!=0);
    listConvertersID.clear();
    if (!existDomain(domainID))
    {
        return (E_NON_EXISTENT);
    }

    CAmMapConverter::const_iterator elementIterator = mMappedData.mConverterMap.begin();
 	for (;elementIterator != mMappedData.mConverterMap.end(); ++elementIterator)
 	{
 		if (domainID==elementIterator->second.domainID)
 			listConvertersID.push_back(elementIterator->second.converterID);
 	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
    listMainConnections.clear();

    CAmMapMainConnection::const_iterator elementIterator = mMappedData.mMainConnectionMap.begin();
    for (;elementIterator != mMappedData.mMainConnectionMap.end(); ++elementIterator)
    {
    	listMainConnections.push_back(elementIterator->second);
    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
    listDomains.clear();

    CAmMapDomain::const_iterator elementIterator = mMappedData.mDomainMap.begin();
     for (;elementIterator != mMappedData.mDomainMap.end(); ++elementIterator)
     {
    	 if( 0==elementIterator->second.reserved )
    		 listDomains.push_back(elementIterator->second);
     }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
    listConnections.clear();

    CAmMapConnection::const_iterator elementIterator = mMappedData.mConnectionMap.begin();
	for (;elementIterator != mMappedData.mConnectionMap.end(); ++elementIterator)
	{
		if( 0==elementIterator->second.reserved )
			listConnections.push_back(elementIterator->second);
	}

      return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
    listSinks.clear();

	std::for_each(mMappedData.mSinkMap.begin(), mMappedData.mSinkMap.end(), [&](const std::pair<am_sinkID_t, am_Sink_Database_s>& ref) {
		if( 0==ref.second.reserved )
			listSinks.push_back(ref.second);
	});

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSources(std::vector<am_Source_s> & listSources) const
{
    listSources.clear();

    std::for_each(mMappedData.mSourceMap.begin(), mMappedData.mSourceMap.end(), [&](const std::pair<am_sourceID_t, am_Source_Database_s>& ref) {
    		if( 0==ref.second.reserved )
    		{
    			listSources.push_back(ref.second);
    		}
    	});
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    listSourceClasses.clear();

    std::for_each(mMappedData.mSourceClassesMap.begin(), mMappedData.mSourceClassesMap.end(), [&](const std::pair<am_sourceClass_t, am_SourceClass_s>& ref) {
    	listSourceClasses.push_back(ref.second);
     });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
    listCrossfaders.clear();

    std::for_each(mMappedData.mCrossfaderMap.begin(), mMappedData.mCrossfaderMap.end(), [&](const std::pair<am_crossfaderID_t, am_Crossfader_s>& ref) {
    	listCrossfaders.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
    listGateways.clear();

    std::for_each(mMappedData.mGatewayMap.begin(), mMappedData.mGatewayMap.end(), [&](const std::pair<am_gatewayID_t, am_Gateway_s>& ref) {
    	listGateways.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConverters(std::vector<am_Converter_s> & listConverters) const
{
	listConverters.clear();

    std::for_each(mMappedData.mConverterMap.begin(), mMappedData.mConverterMap.end(), [&](const std::pair<am_converterID_t, am_Converter_s>& ref) {
    	listConverters.push_back(ref.second);
    });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    listSinkClasses.clear();

    std::for_each(mMappedData.mSinkClassesMap.begin(), mMappedData.mSinkClassesMap.end(), [&](const std::pair<am_gatewayID_t, am_SinkClass_s>& ref) {
    	listSinkClasses.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListVisibleMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
    listConnections.clear();
    am_MainConnectionType_s temp;
    std::for_each(mMappedData.mMainConnectionMap.begin(), mMappedData.mMainConnectionMap.end(), [&](const std::pair<am_mainConnectionID_t, am_MainConnection_Database_s>& ref) {
    	ref.second.getMainConnectionType(temp);
    	listConnections.push_back(temp);
      });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinks(std::vector<am_SinkType_s> & listMainSinks) const
{
    listMainSinks.clear();
    am_SinkType_s sinkType;
    std::for_each(mMappedData.mSinkMap.begin(), mMappedData.mSinkMap.end(), [&](const std::pair<am_sinkID_t, am_Sink_Database_s>& ref) {
    	if( 0==ref.second.reserved && 1==ref.second.visible )
    	{
    		ref.second.getSinkType(sinkType);
    		listMainSinks.push_back(sinkType);
    	}
      });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSources(std::vector<am_SourceType_s> & listMainSources) const
{
    listMainSources.clear();
    am_SourceType_s temp;
    std::for_each(mMappedData.mSourceMap.begin(), mMappedData.mSourceMap.end(), [&](const std::pair<am_sourceID_t, am_Source_Database_s>& ref) {
    	if( 1==ref.second.visible )
    	{
    		ref.second.getSourceType(temp);
			listMainSources.push_back(temp);
    	}
      });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
    	return E_NON_EXISTENT;

    am_Sink_s sink = mMappedData.mSinkMap.at(sinkID);
    listSoundProperties = sink.listMainSoundProperties;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
    	return E_NON_EXISTENT;

    am_Source_s source = mMappedData.mSourceMap.at(sourceID);
    listSourceProperties = source.listMainSoundProperties;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
     listSystemProperties = mMappedData.mSystemProperties;
    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListSinkConnectionFormats(const am_sinkID_t sinkID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
   if (!existSink(sinkID))
	   return E_NON_EXISTENT;
	am_Sink_s sink = mMappedData.mSinkMap.at(sinkID);
	listConnectionFormats = sink.listConnectionFormats;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListSourceConnectionFormats(const am_sourceID_t sourceID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
   if (!existSource(sourceID))
	   return E_NON_EXISTENT;
    am_Source_s source = mMappedData.mSourceMap.at(sourceID);
    listConnectionFormats = source.listConnectionFormats;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListGatewayConnectionFormats(const am_gatewayID_t gatewayID, std::vector<bool> & listConnectionFormat) const
{
    ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
    iter = mListConnectionFormat.find(gatewayID);
    if (iter == mListConnectionFormat.end())
    {
        logError("DatabaseHandler::getListGatewayConnectionFormats database error with convertionFormat");

        return E_NON_EXISTENT;
    }
    listConnectionFormat = iter->second;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
    assert(mainConnectionID!=0);
    if (!existMainConnection(mainConnectionID))
    	return E_NON_EXISTENT;
    delay = -1;

    am_MainConnection_s mainConnection = mMappedData.mMainConnectionMap.at(mainConnectionID);
    delay = mainConnection.delay;

    if (delay == -1)
        return (E_NOT_POSSIBLE);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID)
{
    assert(connectionID!=0);
    if (!existMainConnection(connectionID))
 		return (E_NON_EXISTENT);
    mMappedData.mMainConnectionMap[connectionID].delay = delay;
    if (mpDatabaseObserver)
        mpDatabaseObserver->timingInformationChanged(connectionID, delay);
    return (E_OK);
}

/**
 * checks for a certain mainConnection
 * @param mainConnectionID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existMainConnection(const am_mainConnectionID_t mainConnectionID) const
{
	return existsObjectWithKeyInMap(mainConnectionID, mMappedData.mMainConnectionMap);
}

/**
 * checks for a certain Source
 * @param sourceID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSource(const am_sourceID_t sourceID) const
{
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
		return (0==source->reserved);

	return false;
}

/**
 * checks if a source name or ID exists
 * @param sourceID the sourceID
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerMap::existSourceNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
    return sourceWithNameOrID(sourceID, name);
}

/**
 * checks if a name exits
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerMap::existSourceName(const std::string & name) const
{
    return existSourceNameOrID(mMappedData.mCurrentSourceID.mMax, name);
}

/**
 * checks for a certain Sink
 * @param sinkID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSink(const am_sinkID_t sinkID) const
{
	bool returnVal = false;
	CAmMapSink::const_iterator elementIterator = mMappedData.mSinkMap.begin();
	for (;elementIterator != mMappedData.mSinkMap.end(); ++elementIterator)
	{
		if( 0==elementIterator->second.reserved &&
			sinkID==elementIterator->second.sinkID)
		{
			returnVal = true;
			break;
		}
	}
    return (returnVal);
}

/**
 * returns source with given ID or the name if exists
 * @param sourceID the ID
 * @param name the name
 * @return source structure if exists.
 */
const CAmDatabaseHandlerMap::am_Source_Database_s *  CAmDatabaseHandlerMap::sourceWithNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
	std::function<bool(const CAmDatabaseHandlerMap::am_Source_Database_s & refObject)> comparator = [&](const CAmDatabaseHandlerMap::am_Source_Database_s & source)->bool{
			return ( 0==source.reserved &&
					(sourceID==source.sourceID || name.compare(source.name)==0));
	};
	return objectMatchingPredicate(mMappedData.mSourceMap, comparator);
}

/**
 * returns sink with given ID or the name if exists
 * @param sinkID the ID
 * @param name the name
 * @return sink structure if exists.
 */
const CAmDatabaseHandlerMap::am_Sink_Database_s * CAmDatabaseHandlerMap::sinkWithNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
	std::function<bool(const CAmDatabaseHandlerMap::am_Sink_Database_s & refObject)> comparator = [&](const CAmDatabaseHandlerMap::am_Sink_Database_s & sink)->bool{
			return ( 0==sink.reserved &&
					(sinkID==sink.sinkID || name.compare(sink.name)==0));
	};
	return objectMatchingPredicate(mMappedData.mSinkMap, comparator);
}

/**
 * checks if a sink with the ID or the name exists
 * @param sinkID the ID
 * @param name the name
 * @return true if it exists.
 */
bool CAmDatabaseHandlerMap::existSinkNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
    return sinkWithNameOrID( sinkID,  name)!=NULL;
}

/**
 * checks if a sink with the name exists
 * @param name the name
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSinkName(const std::string & name) const
{
    return existSinkNameOrID(mMappedData.mCurrentSinkID.mMax, name);
}

/**
 * checks for a certain domain
 * @param domainID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existDomain(const am_domainID_t domainID) const
{
	am_Domain_Database_s const * source = objectForKeyIfExistsInMap(domainID, mMappedData.mDomainMap);
	if( NULL!=source )
		return (0==source->reserved);

	return false;
}

/**
 * checks for certain gateway
 * @param gatewayID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existGateway(const am_gatewayID_t gatewayID) const
{
	return existsObjectWithKeyInMap(gatewayID, mMappedData.mGatewayMap);
}

bool CAmDatabaseHandlerMap::existConverter(const am_converterID_t converterID) const
{
	return existsObjectWithKeyInMap(converterID, mMappedData.mConverterMap);
}

am_Error_e CAmDatabaseHandlerMap::getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t & domainID) const
{
    assert(sourceID!=0);

    am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
    if( NULL!=source )
    {
    	domainID = source->domainID;
    	return E_OK;
    }
    return E_NON_EXISTENT;
}

am_Error_e am::CAmDatabaseHandlerMap::getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t & domainID) const
{
    assert(sinkID!=0);

    am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		domainID = source->domainID;
		return E_OK;
	}
	return E_NON_EXISTENT;
}

/**
 * checks for certain SinkClass
 * @param sinkClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSinkClass(const am_sinkClass_t sinkClassID) const
{
	return existsObjectWithKeyInMap(sinkClassID, mMappedData.mSinkClassesMap);
}

/**
 * checks for certain sourceClass
 * @param sourceClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSourceClass(const am_sourceClass_t sourceClassID) const
{
	return existsObjectWithKeyInMap(sourceClassID, mMappedData.mSourceClassesMap);
}

am_Error_e CAmDatabaseHandlerMap::changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    assert(connectionID!=0);
    if(!existConnectionID(connectionID))
    	return E_NON_EXISTENT;

    mMappedData.mConnectionMap[connectionID].delay = delay;

    //now we need to find all mainConnections that use the changed connection and update their timing

    //first get all route tables for all mainconnections

    CAmMapMainConnection::const_iterator iter = mMappedData.mMainConnectionMap.begin();
    for(; iter != mMappedData.mMainConnectionMap.end(); ++iter)
    {
    	am_MainConnection_s mainConnection = iter->second;
    	if (std::find(mainConnection.listConnectionID.begin(), mainConnection.listConnectionID.end(), connectionID) != mainConnection.listConnectionID.end())
    	{
    	  // Got it.
    		changeDelayMainConnection(calculateMainConnectionDelay(mainConnection.mainConnectionID), mainConnection.mainConnectionID);
    	}

    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeConnectionFinal(const am_connectionID_t connectionID)
{
    assert(connectionID!=0);
    am_Connection_Database_s const * connection = objectForKeyIfExistsInMap(connectionID, mMappedData.mConnectionMap);
    if( NULL!=connection )
    {
    	mMappedData.mConnectionMap.at(connectionID).reserved = false;
    	return E_OK;
    }
    return (E_NON_EXISTENT);
}

am_timeSync_t CAmDatabaseHandlerMap::calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const
{
    assert(mainConnectionID!=0);
    if (!existMainConnection(mainConnectionID))
  		return -1;
    am_MainConnection_s mainConnection = mMappedData.mMainConnectionMap.at(mainConnectionID);
    am_timeSync_t delay = 0;
	am_timeSync_t min = SHRT_MAX;
    std::vector<am_connectionID_t>::const_iterator iter = mainConnection.listConnectionID.begin();
	for(;iter<mainConnection.listConnectionID.end(); ++iter)
	{
		am_Connection_Database_s const * source = objectForKeyIfExistsInMap(*iter, mMappedData.mConnectionMap);
		if( NULL!=source )
		{
			delay += source->delay;
			min = std::min(min,source->delay);
		}
	}
    if (min < 0)
        delay = -1;
    return (delay);

}

/**
 * registers the Observer at the Database
 * @param iObserver pointer to the observer
 */
void CAmDatabaseHandlerMap::registerObserver(CAmDatabaseObserver *iObserver)
{
    assert(iObserver!=NULL);
    mpDatabaseObserver = iObserver;
}

/**
 * gives information about the visibility of a source
 * @param sourceID the sourceID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerMap::sourceVisible(const am_sourceID_t sourceID) const
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
    	return false;
    am_Source_Database_s source = mMappedData.mSourceMap.at(sourceID);
    return source.visible;
}

/**
 * gives information about the visibility of a sink
 * @param sinkID the sinkID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerMap::sinkVisible(const am_sinkID_t sinkID) const
{
	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		if(0==source->reserved)
			return source->visible;
	}
	return false;
}

/**
 * checks if a connection already exists.
 * Only takes sink, source and format information for search!
 * @param connection the connection to be checked
 * @return true if connections exists
 */
bool CAmDatabaseHandlerMap::existConnection(const am_Connection_s & connection) const
{
	CAmComparatorFlag comparator;
	am_Connection_Database_s const * connectionObject = findFirstObjectMatchingCriteria(mMappedData.mConnectionMap, connection, comparator);
	return ( NULL!=connectionObject );
}

/**
 * checks if a connection with the given ID exists
 * @param connectionID
 * @return true if connection exits
 */
bool CAmDatabaseHandlerMap::existConnectionID(const am_connectionID_t connectionID) const
{
	am_Connection_Database_s const * connection = objectForKeyIfExistsInMap(connectionID, mMappedData.mConnectionMap);
	if( NULL!=connection )
	{
		return (0==connection->reserved);
	}
	return false;
}

/**
 * checks if a CrossFader exists
 * @param crossfaderID the ID of the crossfader to be checked
 * @return true if exists
 */
bool CAmDatabaseHandlerMap::existCrossFader(const am_crossfaderID_t crossfaderID) const
{
     return existsObjectWithKeyInMap(crossfaderID, mMappedData.mCrossfaderMap);
}

am_Error_e CAmDatabaseHandlerMap::getSoureState(const am_sourceID_t sourceID, am_SourceState_e & sourceState) const
{
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
	{
		sourceState = source->sourceState;
		return (E_OK);
	}
	else
	{
		sourceState =  SS_UNKNNOWN;
		return (E_NON_EXISTENT);
	}
}

am_Error_e CAmDatabaseHandlerMap::changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState)
{
    assert(sourceID!=0);
    assert(sourceState>=SS_UNKNNOWN && sourceState<=SS_MAX);
    if(existSource(sourceID))
    {
    	mMappedData.mSourceMap.at(sourceID).sourceState = sourceState;
		return (E_OK);
	}
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSinkVolume(const am_sinkID_t sinkID, am_volume_t & volume) const
{
    assert(sinkID!=0);

	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		volume = source->volume;
		return (E_OK);
	}
	volume = -1;
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSourceVolume(const am_sourceID_t sourceID, am_volume_t & volume) const
{
    assert(sourceID!=0);
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
	{
		volume = source->volume;
		return (E_OK);
	}
	volume = -1;
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{
    assert(sinkID!=0);

	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		std::vector<am_SoundProperty_s>::const_iterator iter = source->listSoundProperties.begin();
		for(; iter<source->listSoundProperties.end(); ++iter)
		{
			if( propertyType == iter->type )
			{
				value = iter->value;
				return (E_OK);
			}
		}
	}
	value = -1;
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{
    assert(sourceID!=0);

	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
	{
		std::vector<am_SoundProperty_s>::const_iterator iter = source->listSoundProperties.begin();
		for(; iter<source->listSoundProperties.end(); ++iter)
		{
			if( propertyType == iter->type )
			{
				value = iter->value;
				return (E_OK);
			}
		}
	}
	value = -1;
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getDomainState(const am_domainID_t domainID, am_DomainState_e& state) const
{
    assert(domainID!=0);

    am_Domain_Database_s const * source = objectForKeyIfExistsInMap(domainID, mMappedData.mDomainMap);
	if( NULL!=source )
	{
		state = source->state;
		return (E_OK);
	}
	state = DS_UNKNOWN;
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::peekDomain(const std::string & name, am_domainID_t & domainID)
{
    domainID=0;

    CAmComparator<am_Domain_Database_s> comparator;
    am_Domain_Database_s const *reservedDomain = findFirstObjectMatchingCriteria(mMappedData.mDomainMap, name, comparator);

     if( NULL != reservedDomain )
    {
    	domainID = reservedDomain->domainID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if( mMappedData.increaseID( nextID, mMappedData.mCurrentDomainID) )
    	{
    		domainID = nextID;
    		am_Domain_Database_s domain;
    		domain.domainID = nextID;
    		domain.name = name;
    		domain.reserved = 1;
    		mMappedData.mDomainMap[nextID] = domain;
    		return E_OK;
    	}
    	return E_UNKNOWN;
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::peekSink(const std::string & name, am_sinkID_t & sinkID)
{
	CAmComparator<am_Sink_Database_s> comparator;
	am_Sink_Database_s const *reservedSink = findFirstObjectMatchingCriteria(mMappedData.mSinkMap, name, comparator);
	if( NULL!=reservedSink )
    {
		sinkID = reservedSink->sinkID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSinkID))
    	{
    		if(mFirstStaticSink)
    		{
    			nextID = DYNAMIC_ID_BOUNDARY;
    			mFirstStaticSink = false;
    		}
    		sinkID = nextID;
    		am_Sink_Database_s object;
    		object.sinkID = nextID;
    		object.name = name;
    		object.reserved = 1;
    		mMappedData.mSinkMap[nextID] = object;
    		return E_OK;
    	}
   		return E_UNKNOWN;
    }
}

am_Error_e CAmDatabaseHandlerMap::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
	CAmComparator<am_Source_Database_s> comparator;
	am_Source_Database_s const *reservedDomain = findFirstObjectMatchingCriteria(mMappedData.mSourceMap, name, comparator);
	if( NULL!=reservedDomain )
    {
		sourceID = reservedDomain->sourceID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceID))
    	{
    		if(mFirstStaticSource)
    		{
//    			nextID = DYNAMIC_ID_BOUNDARY;
    			mFirstStaticSource = false;
    		}
    		sourceID = nextID;
    		am_Source_Database_s object;
    		object.sourceID = nextID;
    		object.name = name;
    		object.reserved = 1;
    		mMappedData.mSourceMap[nextID] = object;
    		return E_OK;
    	}
    	else
    		return E_UNKNOWN;
    }
}

am_Error_e CAmDatabaseHandlerMap::changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkMap[sinkID].volume = volume;
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume)
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    mMappedData.mSourceMap[sourceID].volume = volume;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    std::vector<am_SoundProperty_s>::iterator iter = mMappedData.mSourceMap[sourceID].listSoundProperties.begin();
	for(; iter<mMappedData.mSourceMap[sourceID].listSoundProperties.end(); ++iter)
	{
		if( soundProperty.type == iter->type )
		{
			iter->value = soundProperty.value;
			return (E_OK);
		}
	}
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

    std::vector<am_SoundProperty_s>::iterator iter = mMappedData.mSinkMap[sinkID].listSoundProperties.begin();
 	for(; iter<mMappedData.mSinkMap[sinkID].listSoundProperties.end(); ++iter)
 	{
 		if( soundProperty.type == iter->type )
 		{
 			iter->value = soundProperty.value;
 			return (E_OK);
 		}
 	}
 	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink)
{
    assert(crossfaderID!=0);
    assert(hotsink!=HS_UNKNOWN);

    if (!existCrossFader(crossfaderID))
    {
        return (E_NON_EXISTENT);
    }

    mMappedData.mCrossfaderMap[crossfaderID].hotSink = hotsink;
    return (E_OK);
}

bool CAmDatabaseHandlerMap::isComponentConnected(const am_Gateway_s & gateway) const
{
	bool ret = isConnected(gateway);
	return ret;
}

bool CAmDatabaseHandlerMap::isComponentConnected(const am_Converter_s & converter) const
{
	bool ret = isConnected(converter);
	return ret;
}

am_Error_e am::CAmDatabaseHandlerMap::peekSinkClassID(const std::string & name, am_sinkClass_t & sinkClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);
    CAmComparator<am_SinkClass_Database_s> comparator;
    am_SinkClass_Database_s const *reservedDomain = findFirstObjectMatchingCriteria(mMappedData.mSinkClassesMap, name, comparator);
	if( NULL!=reservedDomain )
	{
		sinkClassID = reservedDomain->sinkClassID;
		return E_OK;
	}
	return (E_NON_EXISTENT);
}

am_Error_e am::CAmDatabaseHandlerMap::peekSourceClassID(const std::string & name, am_sourceClass_t & sourceClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);
    CAmComparator<am_SourceClass_Database_s> comparator;
    am_SourceClass_Database_s const *ptrSource = findFirstObjectMatchingCriteria(mMappedData.mSourceClassesMap, name, comparator);
  	if( NULL!=ptrSource )
  	{
  		sourceClassID = ptrSource->sourceClassID;
  		return E_OK;
  	}
  	return (E_NON_EXISTENT);
}


am_Error_e CAmDatabaseHandlerMap::changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }
    am_sourceClass_t sourceClassOut(sourceClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);
    //check if sinkClass needs to be changed

	std::unordered_map<am_sourceID_t, am_Source_Database_s>::iterator iter = mMappedData.mSourceMap.begin();
	for(; iter!=mMappedData.mSourceMap.end(); ++iter)
	{
		if( iter->second.sourceID == sourceID )
		{
			if( sourceClassID!=0 )
				iter->second.sourceClassID = sourceClassID;
			else if( 0 == iter->second.reserved )
				sourceClassOut = iter->second.sourceClassID;
		}
	}

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
    	mMappedData.mSourceMap.at(sourceID).listSoundProperties = listSoundProperties;
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
    	mMappedData.mSourceMap.at(sourceID).listConnectionFormats = listConnectionFormats;
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (!listMainSoundProperties.empty() && sourceVisible(sourceID))
    {

    	mMappedData.mSourceMap.at(sourceID).listMainSoundProperties = listMainSoundProperties;
    }
    else //read out the properties
    {
        getListMainSourceSoundProperties(sourceID,listMainSoundPropertiesOut);
    }

    logInfo("DatabaseHandler::changeSource changed changeSink of source:", sourceID);

    if (mpDatabaseObserver != NULL)
    {
        mpDatabaseObserver->sourceUpdated(sourceID,sourceClassOut,listMainSoundPropertiesOut,sourceVisible(sourceID));
    }

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    assert(sinkID!=0);

    am_sinkClass_t sinkClassOut(sinkClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }

	std::unordered_map<am_sinkID_t, am_Sink_Database_s>::iterator iter = mMappedData.mSinkMap.begin();
	for(; iter!=mMappedData.mSinkMap.end(); ++iter)
	{
		if( iter->second.sinkID == sinkID )
		{
			if( sinkClassID!=0 )
				iter->second.sinkClassID = sinkClassID;
			else if( 0 == iter->second.reserved )
				sinkClassOut = iter->second.sinkClassID;
		}
	}

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
    	mMappedData.mSinkMap.at(sinkID).listSoundProperties = listSoundProperties;
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
    	mMappedData.mSinkMap.at(sinkID).listConnectionFormats = listConnectionFormats;
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (!listMainSoundProperties.empty() && sinkVisible(sinkID))
    {
    	mMappedData.mSinkMap.at(sinkID).listMainSoundProperties = listMainSoundProperties;
    }
    else //read out the properties
    {
        getListMainSinkSoundProperties(sinkID,listMainSoundPropertiesOut);
    }

    logInfo("DatabaseHandler::changeSink changed changeSink of sink:", sinkID);

    if (mpDatabaseObserver != NULL)
    {
        mpDatabaseObserver->sinkUpdated(sinkID,sinkClassOut,listMainSoundPropertiesOut,sinkVisible(sinkID));
    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{
    assert(sinkID!=0);
    if (!existSink(sinkID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences
    listMainNotificationConfigurations.clear();

    listMainNotificationConfigurations = mMappedData.mSinkMap.at(sinkID).listMainNotificationConfigurations;

     return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{
    assert(sourceID!=0);
    if (!existSource(sourceID))
        return (E_DATABASE_ERROR); // todo: here we could change to non existen, but not shown in sequences

    listMainNotificationConfigurations = mMappedData.mSourceMap.at(sourceID).listMainNotificationConfigurations;

    return (E_OK);
}

bool changeMainNotificationConfiguration(std::vector<am_NotificationConfiguration_s> & listMainNotificationConfigurations,
											  const am_NotificationConfiguration_s & mainNotificationConfiguration)
{
	bool changed = false;
    std::vector<am_NotificationConfiguration_s>::iterator iter = listMainNotificationConfigurations.begin();
 	for(; iter<listMainNotificationConfigurations.end(); ++iter)
 	{
 		if( mainNotificationConfiguration.type == iter->type )
 		{
 			iter->status = mainNotificationConfiguration.status;
 			iter->parameter = mainNotificationConfiguration.parameter;
 			changed |= true;
 		}
 	}
 	return changed;
}

am_Error_e CAmDatabaseHandlerMap::changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    if(!changeMainNotificationConfiguration(mMappedData.mSinkMap.at(sinkID).listMainNotificationConfigurations, mainNotificationConfiguration))
    	return (E_NO_CHANGE);

    logInfo("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sinkMainNotificationConfigurationChanged(sinkID, mainNotificationConfiguration);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    if(!changeMainNotificationConfiguration(mMappedData.mSourceMap.at(sourceID).listMainNotificationConfigurations, mainNotificationConfiguration))
    	return (E_NO_CHANGE);

    logInfo("DatabaseHandler::changeMainSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    if (mpDatabaseObserver)
        mpDatabaseObserver->sourceMainNotificationConfigurationChanged(sourceID, mainNotificationConfiguration);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    assert(gatewayID!=0);

   if (!existGateway(gatewayID))
   {
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
	   mMappedData.mGatewayMap.at(gatewayID).listSourceFormats = listSourceConnectionFormats;
   }

   if (!listSinkConnectionFormats.empty())
   {
	   mMappedData.mGatewayMap.at(gatewayID).listSinkFormats = listSinkConnectionFormats;
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(gatewayID, convertionMatrix));
   }

   logInfo("DatabaseHandler::changeGatewayDB changed Gateway with ID", gatewayID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    assert(converterID!=0);

   if (!existConverter(converterID))
   {
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
	   mMappedData.mConverterMap.at(converterID).listSourceFormats = listSourceConnectionFormats;
   }

   if (!listSinkConnectionFormats.empty())
   {
	   mMappedData.mConverterMap.at(converterID).listSinkFormats = listSinkConnectionFormats;
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(converterID, convertionMatrix));
   }

   logInfo("DatabaseHandler::changeConverterDB changed Gateway with ID", converterID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

bool changeNotificationConfiguration(std::vector<am_NotificationConfiguration_s> & listNotificationConfigurations, const am_NotificationConfiguration_s & notificationConfiguration)
{
	bool changed = false;
    std::vector<am_NotificationConfiguration_s>::iterator iter = listNotificationConfigurations.begin();
 	for(; iter<listNotificationConfigurations.end(); ++iter)
 	{
 		if( notificationConfiguration.type == iter->type )
 		{
 			iter->status = notificationConfiguration.status;
 			iter->parameter = notificationConfiguration.parameter;
 			changed |= true;
 		}
 	}
 	return changed;
}

am_Error_e CAmDatabaseHandlerMap::changeSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s notificationConfiguration)
{
    assert(sinkID!=0);

    if (!existSink(sinkID))
    {
        return (E_NON_EXISTENT);
    }
    if(!changeNotificationConfiguration(mMappedData.mSinkMap.at(sinkID).listNotificationConfigurations, notificationConfiguration))
    	return (E_NO_CHANGE);

    logInfo("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: inform obsever here...
    return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s notificationConfiguration)
{
    assert(sourceID!=0);

    if (!existSource(sourceID))
    {
        return (E_NON_EXISTENT);
    }

    if(!changeNotificationConfiguration(mMappedData.mSourceMap.at(sourceID).listNotificationConfigurations, notificationConfiguration))
    	return (E_NO_CHANGE);

    logInfo("DatabaseHandler::changeSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: implement observer function
    return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::enumerateSources(std::function<void(const am_Source_s & element)> cb) const
{
	for(auto it = mMappedData.mSourceMap.begin(); it!=mMappedData.mSourceMap.end(); it++)
	{
		const am_Source_Database_s *pObject = &it->second;
		if( 0==pObject->reserved )
			cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const
{
	for(auto it = mMappedData.mSinkMap.begin(); it!=mMappedData.mSinkMap.end(); it++)
	{
		const am_Sink_Database_s *pObject = &it->second;
		if( 0==pObject->reserved )
			cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const
{
	for(auto it = mMappedData.mGatewayMap.begin(); it!=mMappedData.mGatewayMap.end(); it++)
	{
		const am_Gateway_s *pObject = &it->second;
		cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const
{
	for(auto it = mMappedData.mConverterMap.begin(); it!=mMappedData.mConverterMap.end(); it++)
	{
		const am_Converter_s *pObject =  &it->second;
		cb(*pObject);
	}
	return E_OK;
}

}
