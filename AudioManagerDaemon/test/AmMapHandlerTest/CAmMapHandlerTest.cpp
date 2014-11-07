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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmMapHandlerTest.h"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <ios>
#include "shared/CAmDltWrapper.h"

using namespace am;
using namespace testing;


extern bool equalMainSoundProperty(const am_MainSoundProperty_s a, const am_MainSoundProperty_s b);
extern bool equalNotificationConfiguration(const am_NotificationConfiguration_s a, const am_NotificationConfiguration_s b);
extern bool equalClassProperties(const am_ClassProperty_s a, const am_ClassProperty_s b);
extern std::string int2string(int i);

int16_t const TEST_MAX_CONNECTION_ID = 20;
int16_t const TEST_MAX_MAINCONNECTION_ID = 20;
int16_t const TEST_MAX_SINK_ID = 40;

CAmMapHandlerTest::CAmMapHandlerTest() :
        plistRoutingPluginDirs(), //
        plistCommandPluginDirs(), //
        pSocketHandler(),//
        pDatabaseHandler(), //
        pRoutingSender(plistRoutingPluginDirs, pSocketHandler), //
        pCommandSender(plistCommandPluginDirs), //
        pMockInterface(), //
        pRoutingInterfaceBackdoor(), //
        pCommandInterfaceBackdoor(), //
        pControlSender(), //
        pRouter(&pDatabaseHandler, &pControlSender), //
        pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender,  &pSocketHandler, &pRouter), //
        pObserver(&pCommandSender,&pRoutingSender, &pSocketHandler)
{
    pDatabaseHandler.registerObserver(&pObserver);
    pDatabaseHandler.setConnectionIDRange(1, TEST_MAX_CONNECTION_ID);
    pDatabaseHandler.setMainConnectionIDRange(1, TEST_MAX_MAINCONNECTION_ID);
    pDatabaseHandler.setSinkIDRange(DYNAMIC_ID_BOUNDARY, DYNAMIC_ID_BOUNDARY+TEST_MAX_SINK_ID);
    pCommandInterfaceBackdoor.injectInterface(&pCommandSender, &pMockInterface);
}

CAmMapHandlerTest::~CAmMapHandlerTest()
{
}

void CAmMapHandlerTest::createMainConnectionSetup()
{
    //fill the connection database
    am_Connection_s connection;
    am_Source_s source;
    am_Sink_s sink;
    std::vector<am_connectionID_t> connectionList;

    //we create 9 sources and sinks:

    for (uint16_t i = 1; i < 10; i++)
    {
        am_sinkID_t forgetSink;
        am_sourceID_t forgetSource;
        am_connectionID_t connectionID;

        pCF.createSink(sink);
        sink.sinkID = i;
        sink.name = "sink" + int2string(i);
        sink.domainID = 4;
        pCF.createSource(source);
        source.sourceID = i;
        source.name = "source" + int2string(i);
        source.domainID = 4;

        connection.sinkID = i;
        connection.sourceID = i;
        connection.delay = -1;
        connection.connectionFormat = CF_GENIVI_ANALOG;
        connection.connectionID = 0;

        ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,forgetSink));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,forgetSource));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
        ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
        connectionList.push_back(connectionID);

    }

    //create a mainConnection
    am_MainConnection_s mainConnection;
    am_mainConnectionID_t mainConnectionID;
    std::vector<am_MainConnection_s> mainConnectionList;
    mainConnection.listConnectionID = connectionList;
    mainConnection.mainConnectionID = 0;
    mainConnection.sinkID = 1;
    mainConnection.sourceID = 1;
    mainConnection.connectionState = CS_CONNECTED;
    mainConnection.delay = -1;

    //enter mainconnection in database


    ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
    ASSERT_NE(0, mainConnectionID);

    //read out the mainconnections and check if they are equal to the data written.
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(mainConnectionList));
    bool equal = true;
    std::vector<am_MainConnection_s>::iterator listIterator = mainConnectionList.begin();
    for (; listIterator < mainConnectionList.end(); ++listIterator)
    {
        if (listIterator->mainConnectionID == mainConnectionID)
        {
            equal = equal && (listIterator->connectionState == mainConnection.connectionState) && (listIterator->sinkID == mainConnection.sinkID) && (listIterator->sourceID == mainConnection.sourceID) && (listIterator->delay == mainConnection.delay) && (std::equal(listIterator->listConnectionID.begin(), listIterator->listConnectionID.end(), connectionList.begin()));
        }
    }
//    pDatabaseHandler.dump();

    ASSERT_EQ(true, equal);
}

void CAmMapHandlerTest::SetUp()
{
	::testing::FLAGS_gmock_verbose = "error";
}

void CAmMapHandlerTest::TearDown()
{
	::testing::FLAGS_gmock_verbose = "warning";
}

TEST_F(CAmMapHandlerTest,getMainConnectionInfo)
{
    //fill the connection database
    am_Connection_s connection;
    am_Source_s source;
    am_Sink_s sink;
    std::vector<am_connectionID_t> connectionList;

    //we create 9 sources and sinks:
    uint16_t i = 1;
    for (; i < 10; i++)
    {
        am_sinkID_t forgetSink;
        am_sourceID_t forgetSource;
        am_connectionID_t connectionID;

        pCF.createSink(sink);
        sink.sinkID = i;
        sink.name = "sink" + int2string(i);
        sink.domainID = 4;
        pCF.createSource(source);
        source.sourceID = i;
        source.name = "source" + int2string(i);
        source.domainID = 4;

        connection.sinkID = i;
        connection.sourceID = i;
        connection.delay = -1;
        connection.connectionFormat = CF_GENIVI_ANALOG;
        connection.connectionID = 0;

        ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,forgetSink));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,forgetSource));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
        ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
        connectionList.push_back(connectionID);

    }

    //create a mainConnection
    am_MainConnection_s mainConnection;
    am_mainConnectionID_t mainConnectionID;
    std::vector<am_MainConnection_s> mainConnectionList;
    mainConnection.listConnectionID = connectionList;
    mainConnection.mainConnectionID = 0;
    mainConnection.sinkID = 1;
    mainConnection.sourceID = 1;
    mainConnection.connectionState = CS_CONNECTED;
    mainConnection.delay = -1;

    //enter mainconnection in database


    ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
    ASSERT_NE(0, mainConnectionID);

    //read out the mainconnections and check if they are equal to the data written.
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(mainConnectionList));
    bool equal = true;
    std::vector<am_MainConnection_s>::iterator listIterator = mainConnectionList.begin();
    for (; listIterator < mainConnectionList.end(); ++listIterator)
    {
        if (listIterator->mainConnectionID == mainConnectionID)
        {
            equal = equal && (listIterator->connectionState == mainConnection.connectionState) && (listIterator->sinkID == mainConnection.sinkID) && (listIterator->sourceID == mainConnection.sourceID) && (listIterator->delay == mainConnection.delay) && (std::equal(listIterator->listConnectionID.begin(), listIterator->listConnectionID.end(), connectionList.begin()));
        }
    }ASSERT_EQ(true, equal);

    am_MainConnection_s mainConnectionT;
    ASSERT_EQ(E_OK, pDatabaseHandler.getMainConnectionInfoDB(mainConnectionID,mainConnectionT));
    ASSERT_TRUE( (mainConnection.connectionState==mainConnectionT.connectionState) && (mainConnection.delay==mainConnectionT.delay) && (std::equal(mainConnection.listConnectionID.begin(),mainConnection.listConnectionID.end(),mainConnectionT.listConnectionID.begin())) && (mainConnection.sinkID==mainConnectionT.sinkID) && (mainConnection.sourceID==mainConnectionT.sourceID) && (mainConnectionID==mainConnectionT.mainConnectionID));

}

TEST_F(CAmMapHandlerTest,getSinkInfo)
{
    //fill the connection database
    am_Sink_s staticSink, firstDynamicSink, secondDynamicSink;
    am_sinkID_t staticSinkID, firstDynamicSinkID, secondDynamicSinkID;
    std::vector<am_Sink_s> sinkList;

    pCF.createSink(staticSink);
    staticSink.sinkID = 4;

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(staticSink,staticSinkID))
        << "ERROR: database error";
    ASSERT_EQ(staticSink.sinkID,staticSinkID)
        << "ERROR: ID not the one given in staticSink";

    pCF.createSink(firstDynamicSink);
    firstDynamicSink.name = "firstdynamic";
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(firstDynamicSink,firstDynamicSinkID))
        << "ERROR: database error";
    ASSERT_EQ(firstDynamicSinkID,DYNAMIC_ID_BOUNDARY)
        << "ERROR: ID not the one given in firstDynamicSink";

    pCF.createSink(secondDynamicSink);
    secondDynamicSink.name = "seconddynamic";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(secondDynamicSink,secondDynamicSinkID))
        << "ERROR: database error";
    ASSERT_NEAR(secondDynamicSinkID,DYNAMIC_ID_BOUNDARY,10)
        << "ERROR: ID not the one given in secondDynamicSink";

    //now read back and check the returns agains the given values
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(sinkList))
        << "ERROR: database error";
    bool equal = true;

    std::vector<am_Sink_s>::iterator listIterator = sinkList.begin();
    for (; listIterator < sinkList.end(); ++listIterator)
    {
        if (listIterator->sinkID == staticSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, staticSink);
        }

        if (listIterator->sinkID == firstDynamicSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, firstDynamicSink);
        }

        if (listIterator->sinkID == secondDynamicSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, secondDynamicSink);
        }
    }ASSERT_EQ(true, equal);

    am_Sink_s sinkData;
    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkInfoDB(secondDynamicSinkID,sinkData));
    ASSERT_TRUE( (secondDynamicSink.available.availability == sinkData.available.availability) && //
            (secondDynamicSink.available.availabilityReason == sinkData.available.availabilityReason) && //
            (secondDynamicSink.sinkClassID == sinkData.sinkClassID) && //
            (secondDynamicSink.domainID == sinkData.domainID) && //
            (secondDynamicSink.visible == sinkData.visible) && //
            (secondDynamicSink.name.compare(sinkData.name) == 0) && //
            (secondDynamicSink.volume == sinkData.volume) && //
            std::equal(secondDynamicSink.listConnectionFormats.begin(), secondDynamicSink.listConnectionFormats.end(), sinkData.listConnectionFormats.begin()) && //
            std::equal(secondDynamicSink.listMainSoundProperties.begin(), secondDynamicSink.listMainSoundProperties.end(), sinkData.listMainSoundProperties.begin(), equalMainSoundProperty) && //
            std::equal(secondDynamicSink.listNotificationConfigurations.begin(), secondDynamicSink.listNotificationConfigurations.end(), sinkData.listNotificationConfigurations.begin(), equalNotificationConfiguration) && //
            std::equal(secondDynamicSink.listMainNotificationConfigurations.begin(), secondDynamicSink.listMainNotificationConfigurations.end(), sinkData.listMainNotificationConfigurations.begin(), equalNotificationConfiguration) //
            );

}

TEST_F(CAmMapHandlerTest,getSourceInfo)
{
    //fill the connection database
    am_Source_s staticSource, firstDynamicSource, secondDynamicSource;
    am_sourceID_t staticSourceID, firstDynamicSourceID, secondDynamicSourceID;
    std::vector<am_Source_s> sourceList;

    pCF.createSource(staticSource);
    staticSource.sourceID = 4;
    staticSource.name = "Static";


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(staticSource,staticSourceID))
        << "ERROR: database error";
    ASSERT_EQ(staticSource.sourceID,staticSourceID)
        << "ERROR: ID not the one given in staticSource";

    pCF.createSource(firstDynamicSource);
    firstDynamicSource.name = "firstDynamicSource";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(firstDynamicSource,firstDynamicSourceID))
        << "ERROR: database error";
    ASSERT_EQ(firstDynamicSourceID,DYNAMIC_ID_BOUNDARY)
        << "ERROR: ID not the one given in firstDynamicSink";


    pCF.createSource(secondDynamicSource);
    secondDynamicSource.name = "secondDynamicSource";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(secondDynamicSource,secondDynamicSourceID))
        << "ERROR: database error";
    ASSERT_NEAR(secondDynamicSourceID,DYNAMIC_ID_BOUNDARY,10)
        << "ERROR: ID not the one given in secondDynamicSink";

    //now read back and check the returns agains the given values
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSources(sourceList))
        << "ERROR: database error";

    bool equal = true;

    std::vector<am_Source_s>::iterator listIterator = sourceList.begin();
    for (; listIterator < sourceList.end(); ++listIterator)
    {
        if (listIterator->sourceID == staticSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, staticSource);
        }

        if (listIterator->sourceID == firstDynamicSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, firstDynamicSource);
        }

        if (listIterator->sourceID == secondDynamicSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, secondDynamicSource);
        }
    }

    ASSERT_EQ(true, equal);

    am_Source_s sourceData;
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceInfoDB(secondDynamicSourceID,sourceData));
    ASSERT_TRUE((secondDynamicSource.available.availability == sourceData.available.availability) && //
            (secondDynamicSource.available.availabilityReason == sourceData.available.availabilityReason) && //
            (secondDynamicSource.sourceClassID == sourceData.sourceClassID) && (secondDynamicSource.domainID == sourceData.domainID) && //
            (secondDynamicSource.interruptState == sourceData.interruptState) && (secondDynamicSource.visible == sourceData.visible) && //
            (secondDynamicSource.name.compare(sourceData.name) == 0) && (secondDynamicSource.volume == sourceData.volume) && //
            std::equal(secondDynamicSource.listConnectionFormats.begin(), secondDynamicSource.listConnectionFormats.end(), sourceData.listConnectionFormats.begin()) && //
            std::equal(secondDynamicSource.listMainSoundProperties.begin(), secondDynamicSource.listMainSoundProperties.end(), sourceData.listMainSoundProperties.begin(), equalMainSoundProperty) && //
            std::equal(secondDynamicSource.listMainNotificationConfigurations.begin(), secondDynamicSource.listMainNotificationConfigurations.end(), sourceData.listMainNotificationConfigurations.begin(), equalNotificationConfiguration) && //
            std::equal(secondDynamicSource.listNotificationConfigurations.begin(), secondDynamicSource.listNotificationConfigurations.end(), sourceData.listNotificationConfigurations.begin(), equalNotificationConfiguration) //
    );

}

TEST_F(CAmMapHandlerTest, peekSourceID)
{

    std::string sourceName("myClassID");
    am_sourceClass_t sourceClassID, peekID;
    am_SourceClass_s sourceClass;
    am_ClassProperty_s classProperty;
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 13;
    sourceClass.name = sourceName;
    sourceClass.sourceClassID = 0;
    sourceClass.listClassProperties.push_back(classProperty);

    //first we peek without an existing class
    ASSERT_EQ(E_NON_EXISTENT, pDatabaseHandler.peekSourceClassID(sourceName,sourceClassID));

    //now we enter the class into the database
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));

    //first we peek without an existing class
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSourceClassID(sourceName,peekID));
    ASSERT_EQ(sourceClassID, peekID);
}

TEST_F(CAmMapHandlerTest, peekSinkID)
{

    std::string sinkName("myClassID");
    am_sinkClass_t sinkClassID, peekID;
    am_SinkClass_s sinkClass;
    am_ClassProperty_s classProperty;
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 13;
    sinkClass.name = sinkName;
    sinkClass.sinkClassID = 0;
    sinkClass.listClassProperties.push_back(classProperty);

    //first we peek without an existing class
    ASSERT_EQ(E_NON_EXISTENT, pDatabaseHandler.peekSinkClassID(sinkName,sinkClassID));

    //now we enter the class into the database
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));

    //first we peek without an existing class
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSinkClassID(sinkName,peekID));
    ASSERT_EQ(sinkClassID, peekID);
}

TEST_F(CAmMapHandlerTest,crossfaders)
{
    am_Crossfader_s crossfader;
    am_crossfaderID_t crossfaderID;
    am_Sink_s sinkA, sinkB;
    am_Source_s source;
    am_sourceID_t sourceID;
    am_sinkID_t sinkAID, sinkBID;
    pCF.createSink(sinkA);
    pCF.createSink(sinkB);
    sinkB.name = "sinkB";
    pCF.createSource(source);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkA,sinkAID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkB,sinkBID));

    crossfader.crossfaderID = 0;
    crossfader.hotSink = HS_SINKA;
    crossfader.sinkID_A = sinkAID;
    crossfader.sinkID_B = sinkBID;
    crossfader.sourceID = sourceID;
    crossfader.name = "Crossfader";
    crossfader.hotSink = HS_UNKNOWN;

    std::vector<am_Crossfader_s> listCrossfaders;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterCrossfaderDB(crossfader,crossfaderID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListCrossfaders(listCrossfaders));
    ASSERT_EQ(crossfader.sinkID_A, listCrossfaders[0].sinkID_A);
    ASSERT_EQ(crossfader.sinkID_B, listCrossfaders[0].sinkID_B);
    ASSERT_EQ(crossfader.sourceID, listCrossfaders[0].sourceID);
    ASSERT_EQ(crossfader.hotSink, listCrossfaders[0].hotSink);
    ASSERT_EQ(100, listCrossfaders[0].crossfaderID);
    ASSERT_EQ(crossfader.name.compare(listCrossfaders[0].name), 0);
}

TEST_F(CAmMapHandlerTest,crossfadersGetFromDomain)
{



    am_Crossfader_s crossfader;
    am_crossfaderID_t crossfaderID;
    am_Sink_s sinkA, sinkB;
    am_Source_s source;
    am_sourceID_t sourceID;
    am_sinkID_t sinkAID, sinkBID;
    am_domainID_t domainID;
    am_Domain_s domain;
    pCF.createSink(sinkA);
    pCF.createSink(sinkB);
    pCF.createDomain(domain);
    sinkB.name = "sinkB";
    pCF.createSource(source);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    source.domainID = domainID;
    sinkA.domainID = domainID;
    sinkB.domainID = domainID;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkA,sinkAID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkB,sinkBID));

    crossfader.crossfaderID = 0;
    crossfader.hotSink = HS_SINKA;
    crossfader.sinkID_A = sinkAID;
    crossfader.sinkID_B = sinkBID;
    crossfader.sourceID = sourceID;
    crossfader.name = "Crossfader";
    crossfader.hotSink = HS_UNKNOWN;

    std::vector<am_crossfaderID_t> listCrossfaders;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterCrossfaderDB(crossfader,crossfaderID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListCrossfadersOfDomain(source.domainID,listCrossfaders));
    ASSERT_EQ(100, listCrossfaders[0]);

}

TEST_F(CAmMapHandlerTest,sourceState)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    pCF.createSource(source);
    source.sourceState = SS_OFF;

    //prepare the test

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //change the source state
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSourceState(sourceID,SS_ON));

    //read out the changed values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(listSources[0].sourceState, SS_ON);
}

TEST_F(CAmMapHandlerTest,sinkVolumeChange)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;
    pCF.createSink(sink);
    sink.volume = 23;

    //prepare the test

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    //change the volume and check the read out
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkVolume(sinkID,34));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(listSinks[0].volume, 34);
}

TEST_F(CAmMapHandlerTest,sourceVolumeChange)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    pCF.createSource(source);
    source.volume = 23;

    //prepare test

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //change the volume and check the read out
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSourceVolume(sourceID,34));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(listSources[0].volume, 34);
}

TEST_F(CAmMapHandlerTest, peekSource)
{
    std::vector<am_Source_s> listSources;
    am_sourceID_t sourceID, source2ID, source3ID;
    am_Source_s source;
    pCF.createSource(source);

    //peek a source that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(std::string("newsource"),sourceID));

    //make sure it is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_TRUE(listSources.empty());
    ASSERT_EQ(sourceID, DYNAMIC_ID_BOUNDARY);

    //now enter the source with the same name and make sure it does not get a new ID
    source.name = "newsource";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source2ID));

    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(sourceID, source2ID);
    ASSERT_TRUE(listSources.at(0).sourceID==sourceID);

    //now we peek again. This time, the source exists
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(source.name,source3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_TRUE(listSources.size()==1);
    ASSERT_EQ(source3ID, source2ID);
}

TEST_F(CAmMapHandlerTest, peekSourceDouble)
{
    std::vector<am_Source_s> listSources;
    am_sourceID_t sourceID;
    am_sourceID_t source2ID;
    am_sourceID_t source3ID;
    am_Source_s source;
    pCF.createSource(source);

    //peek a source that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(std::string("newsource"),sourceID));

    //peek a second source that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(std::string("newsource2"),source2ID));

    //make sure they are is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_TRUE(listSources.empty());
    ASSERT_EQ(sourceID, 100);
    source.name = "newsource";

    //now enter the source with the same name than the first peek and make sure it does not get a new ID

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(sourceID, source3ID);
    ASSERT_TRUE(listSources[0].sourceID==sourceID);
}

TEST_F(CAmMapHandlerTest, peekSink)
{
    std::vector<am_Sink_s> listSinks;
    am_sinkID_t sinkID, sink2ID, sink3ID;
    am_Sink_s sink;
    pCF.createSink(sink);

    //peek a sink that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(std::string("newsink"),sinkID));

    //make sure it is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_TRUE(listSinks.empty());
    ASSERT_EQ(sinkID, 100);
    sink.name = "newsink";

    //now enter the source with the same name and make sure it does not get a new ID

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sink2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(sinkID, sink2ID);
    ASSERT_TRUE(listSinks[0].sinkID==sinkID);

    //now we peek again, this time, the sink exists
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(sink.name,sink3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_TRUE(listSinks.size()==1);
    ASSERT_EQ(sink3ID, sink2ID);
}

TEST_F(CAmMapHandlerTest, peekSinkDouble)
{
    std::vector<am_Sink_s> listSinks;
    am_sinkID_t sinkID;
    am_sinkID_t sink2ID;
    am_sinkID_t sink3ID;
    am_Sink_s sink;
    pCF.createSink(sink);

    //peek a sink that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(std::string("newsink"),sinkID));

    //peek again

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(std::string("nextsink"),sink2ID));

    //make sure they are is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_TRUE(listSinks.empty());
    ASSERT_EQ(sinkID, 100);
    sink.name = "newsink";

    //now enter the sink with the same name than the first peek and make sure it does not get a new ID

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sink3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(sinkID, sink3ID);
    ASSERT_TRUE(listSinks[0].sinkID==sinkID);
}

TEST_F(CAmMapHandlerTest,changeConnectionTimingInformationCheckMainConnection)
{
    std::vector<am_Connection_s> connectionList;
    std::vector<am_MainConnectionType_s> mainList;

    //prepare the test, it is one mainconnection, so we expect one callback
    createMainConnectionSetup();

    //first get all visible mainconnections and make sure, the delay is set to -1 for the first entry
    ASSERT_EQ(E_OK, pDatabaseHandler.getListVisibleMainConnections(mainList));
    ASSERT_EQ(mainList[0].delay, -1);

    //no go through all connections and set the delay time to 24 for each connection
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    std::vector<am_Connection_s>::iterator iteratorConnectionList = connectionList.begin();
    for (; iteratorConnectionList < connectionList.end(); ++iteratorConnectionList)
    {
        ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionTimingInformation(iteratorConnectionList->connectionID,24));
    }

    //we read the result again and expect that the value is now different from -1
    ASSERT_EQ(E_OK, pDatabaseHandler.getListVisibleMainConnections(mainList));
    ASSERT_EQ(mainList[0].delay, 216);
}

TEST_F(CAmMapHandlerTest,changeConnectionTimingInformation)
{
    am_Connection_s connection;
    am_connectionID_t connectionID;
    std::vector<am_Connection_s> connectionList;
    pCF.createConnection(connection);

    //enter a connection
    ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));

    //change the timing and check it
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionTimingInformation(connectionID,24));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(connectionList[0].delay==24);
}

TEST_F(CAmMapHandlerTest,getSinkClassOfSink)
{
    std::vector<am_SinkClass_s> sinkClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SinkClass_s sinkClass, returnClass;
    am_ClassProperty_s classProperty;
    am_sinkClass_t sinkClassID;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sinkClass.name = "test";
    sinkClass.sinkClassID = 4;
    sinkClass.listClassProperties = classPropertyList;
    pCF.createSink(sink);
    sink.sinkClassID = 4;

    //prepare test

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    //enter a new sinkclass, read out again and check

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 4);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));

    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkClassInfoDB(sinkID,returnClass));
    ASSERT_EQ(sinkClassList[0].name, returnClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, returnClass.sinkClassID);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),returnClass.listClassProperties.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,getSourceClassOfSource)
{
    std::vector<am_SourceClass_s> sourceClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SourceClass_s sourceClass, sinkSourceClass;
    am_ClassProperty_s classProperty;
    am_sourceClass_t sourceClassID;
    am_Source_s source;
    am_sourceID_t sourceID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sourceClass.name = "test";
    sourceClass.sourceClassID = 1;
    sourceClass.listClassProperties = classPropertyList;
    pCF.createSource(source);


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 1);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceClassInfoDB(sourceID,sinkSourceClass));
    ASSERT_EQ(sourceClassList[0].name, sinkSourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, sinkSourceClass.sourceClassID);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),sinkSourceClass.listClassProperties.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,removeSourceClass)
{
    std::vector<am_SourceClass_s> sourceClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SourceClass_s sourceClass;
    am_ClassProperty_s classProperty;
    am_sourceClass_t sourceClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sourceClass.name = "test";
    sourceClass.sourceClassID = 3;
    sourceClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 3);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.removeSourceClassDB(3));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_TRUE(sourceClassList.empty());
}

TEST_F(CAmMapHandlerTest,updateSourceClass)
{
    std::vector<am_SourceClass_s> sourceClassList;
    std::vector<am_ClassProperty_s> classPropertyList, changedPropertyList;
    am_SourceClass_s sourceClass, changedClass;
    am_ClassProperty_s classProperty;
    am_sourceClass_t sourceClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sourceClass.name = "test";
    sourceClass.sourceClassID = 0;
    sourceClass.listClassProperties = classPropertyList;
    changedClass = sourceClass;
    changedClass.listClassProperties[1].value = 6;
    changedPropertyList = changedClass.listClassProperties;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));
    changedClass.sourceClassID = sourceClassID;
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 100);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSourceClassInfoDB(changedClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 100);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),changedPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,enterSourceClass)
{
    std::vector<am_SourceClass_s> sourceClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SourceClass_s sourceClass;
    am_ClassProperty_s classProperty;
    am_sourceClass_t sourceClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sourceClass.name = "test";
    sourceClass.sourceClassID = 0;
    sourceClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 100);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,enterSourceClassStatic)
{
    std::vector<am_SourceClass_s> sourceClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SourceClass_s sourceClass;
    am_ClassProperty_s classProperty;
    am_sourceClass_t sourceClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sourceClass.name = "test";
    sourceClass.sourceClassID = 3;
    sourceClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSourceClasses(sourceClassList));
    ASSERT_EQ(sourceClassList[0].name, sourceClass.name);
    ASSERT_EQ(sourceClassList[0].sourceClassID, 3);
    ASSERT_TRUE(std::equal(sourceClassList[0].listClassProperties.begin(),sourceClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,removeSinkClass)
{
    std::vector<am_SinkClass_s> sinkClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SinkClass_s sinkClass;
    am_ClassProperty_s classProperty;
    am_sinkClass_t sinkClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sinkClass.name = "test";
    sinkClass.sinkClassID = 0;
    sinkClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 100);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.removeSinkClassDB(sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_TRUE(sinkClassList.empty());
}

TEST_F(CAmMapHandlerTest,updateSinkClass)
{
    std::vector<am_SinkClass_s> sinkClassList;
    std::vector<am_ClassProperty_s> classPropertyList, changedPropertyList;
    am_SinkClass_s sinkClass, changedClass;
    am_ClassProperty_s classProperty;
    am_sinkClass_t sinkClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sinkClass.name = "test";
    sinkClass.sinkClassID = 0;
    sinkClass.listClassProperties = classPropertyList;
    changedClass = sinkClass;
    changedClass.listClassProperties[1].value = 6;
    changedPropertyList = changedClass.listClassProperties;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));
    changedClass.sinkClassID = sinkClassID;
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 100);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkClassInfoDB(changedClass));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 100);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),changedPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,enterSinkClass)
{
    std::vector<am_SinkClass_s> sinkClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SinkClass_s sinkClass;
    am_ClassProperty_s classProperty;
    am_sinkClass_t sinkClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sinkClass.name = "test";
    sinkClass.sinkClassID = 0;
    sinkClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 100);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest,enterSinkClassStatic)
{
    std::vector<am_SinkClass_s> sinkClassList;
    std::vector<am_ClassProperty_s> classPropertyList;
    am_SinkClass_s sinkClass;
    am_ClassProperty_s classProperty;
    am_sinkClass_t sinkClassID;
    classProperty.classProperty = CP_GENIVI_SINK_TYPE;
    classProperty.value = 1;
    classPropertyList.push_back(classProperty);
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 4;
    classPropertyList.push_back(classProperty);
    sinkClass.name = "test";
    sinkClass.sinkClassID = 4;
    sinkClass.listClassProperties = classPropertyList;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinkClasses(sinkClassList));
    ASSERT_EQ(sinkClassList[0].name, sinkClass.name);
    ASSERT_EQ(sinkClassList[0].sinkClassID, 4);
    ASSERT_TRUE(std::equal(sinkClassList[0].listClassProperties.begin(),sinkClassList[0].listClassProperties.end(),classPropertyList.begin(),equalClassProperties));
}

TEST_F(CAmMapHandlerTest, changeSystemProperty)
{
    std::vector<am_SystemProperty_s> listSystemProperties, listReturn;
    am_SystemProperty_s systemProperty;

    systemProperty.type = SYP_UNKNOWN;
    systemProperty.value = 33;
    listSystemProperties.push_back(systemProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSystemProperties(listSystemProperties));
    systemProperty.value = 444;
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSystemPropertyDB(systemProperty));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSystemProperties(listReturn));
    ASSERT_EQ(listReturn[0].type, systemProperty.type);
    ASSERT_EQ(listReturn[0].value, systemProperty.value);
}

TEST_F(CAmMapHandlerTest, systemProperties)
{
    std::vector<am_SystemProperty_s> listSystemProperties, listReturn;
    am_SystemProperty_s systemProperty;

    systemProperty.type = SYP_UNKNOWN;
    systemProperty.value = 33;
    listSystemProperties.push_back(systemProperty);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSystemProperties(listSystemProperties));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSystemProperties(listReturn));
    ASSERT_EQ(listReturn[0].type, systemProperty.type);
    ASSERT_EQ(listReturn[0].value, systemProperty.value);
}

TEST_F(CAmMapHandlerTest,enterSourcesCorrect)
{
    //fill the connection database
    am_Source_s staticSource, firstDynamicSource, secondDynamicSource;
    am_sourceID_t staticSourceID, firstDynamicSourceID, secondDynamicSourceID;
    std::vector<am_Source_s> sourceList;

    pCF.createSource(staticSource);
    staticSource.sourceID = 4;
    staticSource.name = "Static";


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(staticSource,staticSourceID))
        << "ERROR: database error";
    ASSERT_EQ(staticSource.sourceID,staticSourceID)
        << "ERROR: ID not the one given in staticSource";

    pCF.createSource(firstDynamicSource);
    firstDynamicSource.name = "firstDynamicSource";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(firstDynamicSource,firstDynamicSourceID))
        << "ERROR: database error";
    ASSERT_EQ(firstDynamicSourceID,DYNAMIC_ID_BOUNDARY)
        << "ERROR: ID not the one given in firstDynamicSink";

    pCF.createSource(secondDynamicSource);
    secondDynamicSource.name = "secondDynamicSource";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(secondDynamicSource,secondDynamicSourceID))
        << "ERROR: database error";
    ASSERT_NEAR(secondDynamicSourceID,DYNAMIC_ID_BOUNDARY,10)
        << "ERROR: ID not the one given in secondDynamicSink";

    //now read back and check the returns agains the given values
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSources(sourceList))
        << "ERROR: database error";
    bool equal = true;

    std::vector<am_Source_s>::iterator listIterator = sourceList.begin();
    for (; listIterator < sourceList.end(); ++listIterator)
    {
        if (listIterator->sourceID == staticSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, staticSource);
        }

        if (listIterator->sourceID == firstDynamicSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, firstDynamicSource);
        }

        if (listIterator->sourceID == secondDynamicSourceID)
        {
            equal = equal && pCF.compareSource(listIterator, secondDynamicSource);
        }

    }
    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest, changeSourceMainSoundProperty)
{
    std::vector<am_Source_s> listSources;
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    am_MainSoundProperty_s property;
    property.type = MSP_UNKNOWN;
    property.value = 33;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    ASSERT_EQ(E_OK, pDatabaseHandler.changeMainSourceSoundPropertyDB(property,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    std::vector<am_MainSoundProperty_s>::iterator listIterator = listSources[0].listMainSoundProperties.begin();
    for (; listIterator < listSources[0].listMainSoundProperties.end(); ++listIterator)
    {
        if (listIterator->type == property.type)
        {
            ASSERT_EQ(listIterator->value, property.value);
        }
    }
}

TEST_F(CAmMapHandlerTest, changeSinkMuteState)
{
    std::vector<am_Sink_s> listSinks;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    am_MuteState_e muteState = MS_MUTED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkMuteStateDB(muteState,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(muteState, listSinks[0].muteState);
}

TEST_F(CAmMapHandlerTest, changeSinkMainSoundProperty)
{
    std::vector<am_Sink_s> listSinks;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    am_MainSoundProperty_s property;
    property.type = MSP_UNKNOWN;
    property.value = 33;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    ASSERT_EQ(E_OK, pDatabaseHandler.changeMainSinkSoundPropertyDB(property,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    std::vector<am_MainSoundProperty_s>::iterator listIterator = listSinks[0].listMainSoundProperties.begin();
    for (; listIterator < listSinks[0].listMainSoundProperties.end(); ++listIterator)
    {
        if (listIterator->type == property.type)
        {
            ASSERT_EQ(listIterator->value, property.value);
        }
    }
}

TEST_F(CAmMapHandlerTest, peekDomain)
{
    std::vector<am_Domain_s> listDomains;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_domainID_t domain2ID;
    pCF.createDomain(domain);
    ASSERT_EQ(E_OK, pDatabaseHandler.peekDomain(std::string("newdomain"),domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_TRUE(listDomains.empty());
    ASSERT_EQ(domainID, 1);
    domain.name = "newdomain";
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domain2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(domainID, domain2ID);
    ASSERT_TRUE(listDomains[0].domainID==domainID);
}

TEST_F(CAmMapHandlerTest, peekDomainFirstEntered)
{
    std::vector<am_Domain_s> listDomains;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_domainID_t domain2ID;
    pCF.createDomain(domain);
    domain.name = "newdomain";
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.peekDomain(std::string("newdomain"),domain2ID));
    ASSERT_EQ(domainID, domain2ID);
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_TRUE(listDomains.size()==1);
}

TEST_F(CAmMapHandlerTest, changeDomainState)
{
    std::vector<am_Domain_s> listDomains;
    am_Domain_s domain;
    am_domainID_t domainID;
    pCF.createDomain(domain);
    am_DomainState_e newState = DS_INDEPENDENT_STARTUP;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.changDomainStateDB(newState,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(newState, listDomains[0].state);
}

TEST_F(CAmMapHandlerTest, changeMainConnectionState)
{
    std::vector<am_MainConnection_s> listMainConnections;
    createMainConnectionSetup();

    ASSERT_EQ(E_OK, pDatabaseHandler.changeMainConnectionStateDB(1,CS_DISCONNECTING));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(listMainConnections));
    ASSERT_EQ(CS_DISCONNECTING, listMainConnections[0].connectionState);
}

TEST_F(CAmMapHandlerTest, changeSinkAvailability)
{
    std::vector<am_Sink_s> listSinks;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    am_Availability_s availability;
    availability.availability = A_UNKNOWN;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkAvailabilityDB(availability,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(availability.availability, listSinks[0].available.availability);
    ASSERT_EQ(availability.availabilityReason, listSinks[0].available.availabilityReason);
}

TEST_F(CAmMapHandlerTest, changeSourceAvailability)
{
    std::vector<am_Source_s> listSources;
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    am_Availability_s availability;
    availability.availability = A_UNKNOWN;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    source.visible = true;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSourceAvailabilityDB(availability,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(availability.availability, listSources[0].available.availability);
    ASSERT_EQ(availability.availabilityReason, listSources[0].available.availabilityReason);
}

TEST_F(CAmMapHandlerTest,changeMainConnectionRoute)
{
    std::vector<am_MainConnection_s> originalList;
    std::vector<am_MainConnection_s> newList;
    createMainConnectionSetup();
    //fill the connection database
    am_Connection_s connection;
    am_Source_s source;
    am_Sink_s sink;
    std::vector<am_connectionID_t> listConnectionID;

    uint16_t i = 1;
    for (; i < 10; i++)
    {
        am_sinkID_t forgetSink;
        am_sourceID_t forgetSource;
        am_connectionID_t connectionID;

        connection.sinkID = i + 20;
        connection.sourceID = i + 20;
        connection.delay = -1;
        connection.connectionFormat = CF_GENIVI_ANALOG;
        connection.connectionID = 0;

        pCF.createSink(sink);
        sink.sinkID = i + 20;
        sink.name = "sink" + int2string(i + 20);
        sink.domainID = 4;
        pCF.createSource(source);
        source.sourceID = i + 20;
        source.name = "source" + int2string(i + 30);
        source.domainID = 4;

        ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,forgetSink));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,forgetSource));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
        listConnectionID.push_back(connectionID);
        ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(originalList));

    }

    ASSERT_EQ(E_OK, pDatabaseHandler.changeMainConnectionRouteDB(1,listConnectionID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(newList));
    ASSERT_FALSE(std::equal(newList[0].listConnectionID.begin(),newList[0].listConnectionID.end(),originalList[0].listConnectionID.begin()));
}

TEST_F(CAmMapHandlerTest,changeMainSinkVolume)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_mainVolume_t newVol = 20;
    std::vector<am_Sink_s> listSinks;
    pCF.createSink(sink);


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkMainVolumeDB(newVol,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(listSinks[0].mainVolume, newVol);
}

TEST_F(CAmMapHandlerTest,getMainSourceSoundProperties)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    std::vector<am_MainSoundProperty_s> mainSoundProperties = source.listMainSoundProperties;
    std::vector<am_MainSoundProperty_s> listMainSoundProperties;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainSourceSoundProperties(sourceID,listMainSoundProperties));
    ASSERT_TRUE(std::equal(mainSoundProperties.begin(),mainSoundProperties.end(),listMainSoundProperties.begin(),equalMainSoundProperty));
}

TEST_F(CAmMapHandlerTest,getMainSinkSoundProperties)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    std::vector<am_MainSoundProperty_s> mainSoundProperties = sink.listMainSoundProperties;
    std::vector<am_MainSoundProperty_s> listMainSoundProperties;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainSinkSoundProperties(sinkID,listMainSoundProperties));
    ASSERT_TRUE(std::equal(mainSoundProperties.begin(),mainSoundProperties.end(),listMainSoundProperties.begin(),equalMainSoundProperty));
}

TEST_F(CAmMapHandlerTest,getMainSources)
{
    am_Source_s source, source1, source2;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    pCF.createSource(source1);
    pCF.createSource(source2);
    source1.name = "source1";
    source2.name = "source2";
    bool equal = true;
    source1.visible = false;
    std::vector<am_SourceType_s> listMainSources;
    std::vector<am_Source_s> listSources;



    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    source.sourceID = sourceID;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source1,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source2,sourceID));
    source2.sourceID = sourceID;
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainSources(listMainSources));
    listSources.push_back(source);
    listSources.push_back(source2);
    std::vector<am_SourceType_s>::iterator listIterator = listMainSources.begin();
    for (; listIterator < listMainSources.end(); ++listIterator)
    {
        equal = equal && pCF.compareSinkMainSource(listIterator, listSources);
    }
    ASSERT_TRUE(equal);
}

TEST_F(CAmMapHandlerTest,getMainSinks)
{
	am_Sink_s sink, sink1, sink2;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    pCF.createSink(sink1);
    pCF.createSink(sink2);
    sink1.name = "sink1";
    sink2.name = "sink2";
    bool equal = true;
    sink1.visible = false;
    std::vector<am_SinkType_s> listMainSinks;
    std::vector<am_Sink_s> listSinks;


    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    sink.sinkID = sinkID;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink1,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink2,sinkID));
    sink2.sinkID = sinkID;
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainSinks(listMainSinks));
    listSinks.push_back(sink);
    listSinks.push_back(sink2);

    std::vector<am_SinkType_s>::iterator listIterator = listMainSinks.begin();
    for (; listIterator < listMainSinks.end(); ++listIterator)
    {
        equal = equal && pCF.compareSinkMainSink(listIterator, listSinks);
    }
    ASSERT_TRUE(equal);
}

TEST_F(CAmMapHandlerTest,getVisibleMainConnections)
{
    createMainConnectionSetup();
    std::vector<am_MainConnectionType_s> listVisibleMainConnections;
    std::vector<am_MainConnection_s> listMainConnections;
    ASSERT_EQ(E_OK, pDatabaseHandler.getListVisibleMainConnections(listVisibleMainConnections));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListMainConnections(listMainConnections));
    ASSERT_EQ(listMainConnections[0].mainConnectionID, listVisibleMainConnections[0].mainConnectionID);
    ASSERT_EQ(listMainConnections[0].connectionState, listVisibleMainConnections[0].connectionState);
    ASSERT_EQ(listMainConnections[0].delay, listVisibleMainConnections[0].delay);
    ASSERT_EQ(listMainConnections[0].sinkID, listVisibleMainConnections[0].sinkID);
    ASSERT_EQ(listMainConnections[0].sourceID, listVisibleMainConnections[0].sourceID);
}

TEST_F(CAmMapHandlerTest,getListSourcesOfDomain)
{
    am_Source_s source, source2;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_sourceID_t sourceID;
    std::vector<am_sourceID_t> sourceList, sourceCheckList;
    pCF.createSource(source);
    source.sourceID = 1;
    source.name = "testSource";
    source.domainID = 1;
    pCF.createSource(source2);
    source2.sourceID = 0;
    source2.name = "testSource2";
    source2.domainID = 5;
    pCF.createDomain(domain);
    sourceCheckList.push_back(1); //sink.sinkID);


    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source2,sourceID))
        << "ERROR: database error";
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.getListSourcesOfDomain(2,sourceList))
        << "ERROR: database error";ASSERT_TRUE(sourceList.empty());
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSourcesOfDomain(1,sourceList))
        << "ERROR: database error";
    ASSERT_TRUE(std::equal(sourceList.begin(),sourceList.end(),sourceCheckList.begin()) && !sourceList.empty());
}

TEST_F(CAmMapHandlerTest,getListSinksOfDomain)
{
    am_Sink_s sink, sink2;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_sinkID_t sinkID;
    std::vector<am_sinkID_t> sinkList, sinkCheckList;
    pCF.createSink(sink);
    sink.sinkID = 1;
    sink.domainID = 1;
    pCF.createSink(sink2);
    sink2.domainID = 5;
    sink2.name = "sink2";
    pCF.createDomain(domain);
    sinkCheckList.push_back(1); //sink.sinkID);


    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink2,sinkID))
        << "ERROR: database error";
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.getListSinksOfDomain(2,sinkList))
        << "ERROR: database error";ASSERT_TRUE(sinkList.empty());
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinksOfDomain(1,sinkList))
        << "ERROR: database error";
    ASSERT_TRUE(std::equal(sinkList.begin(),sinkList.end(),sinkCheckList.begin()) && !sinkList.empty());
}

TEST_F(CAmMapHandlerTest,getListGatewaysOfDomain)
{
    am_Gateway_s gateway, gateway2;
    am_gatewayID_t gatewayID1, gatewayID2;
    am_domainID_t domainID;
    am_Domain_s domain;
    std::vector<am_gatewayID_t> gatewayList, gatewayCheckList;

    pCF.createDomain(domain);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));

    pCF.createGateway(gateway);
    gateway.gatewayID = 1;
    gateway.name = "testGateway";
    gateway.controlDomainID = domainID;
    gateway.sourceID = 1;
    gateway.sinkID = 1;
    gateway.domainSinkID = 1;
    gateway.domainSourceID = 1;
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway,gatewayID1))
        << "ERROR: database error";
    ASSERT_EQ(true, gatewayID1==1);

    pCF.createGateway(gateway2);
    gateway2.gatewayID = 2;
    gateway2.name = "testGateway2";
    gateway2.controlDomainID = 4;
    gateway2.sourceID = 1;
    gateway2.sinkID = 1;
    gateway2.domainSinkID = 1;
    gateway2.domainSourceID = 1;
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2))
        << "ERROR: database error";
    ASSERT_EQ(true, gatewayID2==2);
    gatewayCheckList.push_back(gatewayID1);

    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    sink.sinkID = sinkID;

    pCF.createSource(source);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    source.sourceID = sourceID;

    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.getListGatewaysOfDomain(2,gatewayList))
        << "ERROR: database error";
    ASSERT_TRUE(gatewayList.empty());
    ASSERT_EQ(E_OK,pDatabaseHandler.getListGatewaysOfDomain(domainID,gatewayList))
        << "ERROR: database error";
    ASSERT_TRUE(std::equal(gatewayList.begin(),gatewayList.end(),gatewayCheckList.begin()) && !gatewayList.empty());
}

TEST_F(CAmMapHandlerTest,getListConvertersOfDomain)
{
    am_Converter_s converter, converter2;
    am_converterID_t converterID1, converterID2;
    am_domainID_t domainID;
    am_Domain_s domain;
    std::vector<am_converterID_t> converterList, converterCheckList;

    pCF.createDomain(domain);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));

    pCF.createConverter(converter);
    converter.converterID = 1;
    converter.name = "testGateway";
    converter.sourceID = 1;
    converter.sinkID = 1;
    converter.domainID = domainID;
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(converter,converterID1))
        << "ERROR: database error";
    ASSERT_EQ(true, converterID1==1);

    pCF.createConverter(converter2);
    converter2.converterID = 2;
    converter2.name = "testGateway2";
    converter2.domainID = 4;
    converter2.sourceID = 1;
    converter2.sinkID = 1;
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(converter2,converterID2))
        << "ERROR: database error";
    ASSERT_EQ(true, converterID2==2);
    converterCheckList.push_back(converterID1);

    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    sink.sinkID = sinkID;

    pCF.createSource(source);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    source.sourceID = sourceID;

    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.getListConvertersOfDomain(4,converterList))
        << "ERROR: database error";
    ASSERT_TRUE(converterList.empty());
    ASSERT_EQ(E_OK,pDatabaseHandler.getListConvertersOfDomain(domainID,converterList))
        << "ERROR: database error";
    ASSERT_TRUE(std::equal(converterList.begin(),converterList.end(),converterCheckList.begin()) && !converterList.empty());
}

TEST_F(CAmMapHandlerTest,removeDomain)
{
    am_Domain_s domain;
    am_domainID_t domainID;
    std::vector<am_Domain_s> listDomains;
    pCF.createDomain(domain);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.removeDomainDB(domainID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.getListDomains(listDomains))
        << "ERROR: database error";
    ASSERT_TRUE(listDomains.empty());
}

TEST_F(CAmMapHandlerTest,removeGateway)
{
    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;
    std::vector<am_Gateway_s> listGateways;
    pCF.createGateway(gateway);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway,gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.removeGatewayDB(gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.getListGateways(listGateways))
        << "ERROR: database error";
    ASSERT_TRUE(listGateways.empty());
}

TEST_F(CAmMapHandlerTest,removeConverter)
{
    am_Converter_s converter;
    am_converterID_t converterID;
    std::vector<am_Converter_s> listConverters;
    pCF.createConverter(converter);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(converter,converterID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.removeConverterDB(converterID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.getListConverters(listConverters))
        << "ERROR: database error";
    ASSERT_TRUE(listConverters.empty());
}

TEST_F(CAmMapHandlerTest,removeSink)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;
    pCF.createSink(sink);

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSinkDB(sinkID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(listSinks))
        << "ERROR: database error";
    ASSERT_TRUE(listSinks.empty());
}

TEST_F(CAmMapHandlerTest,removeSource)
{
    //fill the connection database
    am_Source_s source;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    pCF.createSource(source);


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSourceDB(sourceID))
        << "ERROR: database error";
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSources(listSources))
        << "ERROR: database error";
    ASSERT_TRUE(listSources.empty());
}

TEST_F(CAmMapHandlerTest, removeMainConnection)
{
    createMainConnectionSetup();


    ASSERT_EQ(E_OK,pDatabaseHandler.removeMainConnectionDB(1))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,removeNonexistentMainConnectionFail)
{
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.removeMainConnectionDB(34))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,removeNonexistentSource)
{
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.removeSourceDB(3))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,removeNonexistentSink)
{
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.removeSinkDB(2))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,removeNonexistentGateway)
{
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.removeGatewayDB(12))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,removeNonexistentConverter)
{
    ASSERT_EQ(E_NON_EXISTENT,pDatabaseHandler.removeConverterDB(12))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,registerGatewayCorrect)
{
    //initialize gateway
    std::vector<am_Gateway_s> returnList;
    am_Gateway_s gateway, gateway1, gateway2;
    am_gatewayID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;

    pCF.createGateway(gateway);
    pCF.createGateway(gateway1);
    gateway1.gatewayID = 20;
    pCF.createGateway(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway,gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(100,gatewayID)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1))
        << "ERROR: database error";
    ASSERT_EQ(gateway1.gatewayID,gatewayID1)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2))
        << "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)
        << "ERROR: domainID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListGateways(returnList));
    bool equal = true;
    std::vector<am_Gateway_s>::iterator listIterator = returnList.begin();

    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->gatewayID == gatewayID)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway);
        }

        if (listIterator->gatewayID == gatewayID1)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway1);
        }

        if (listIterator->gatewayID == gatewayID2)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway2);
        }
    }

    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest,registerConverterCorrect)
{
    //initialize gateway
    std::vector<am_Converter_s> returnList;
    am_Converter_s gateway, gateway1, gateway2;
    am_converterID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;

    pCF.createConverter(gateway);
    pCF.createConverter(gateway1);
    gateway1.converterID = 20;
    pCF.createConverter(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway,gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(100,gatewayID)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway1,gatewayID1))
        << "ERROR: database error";
    ASSERT_EQ(gateway1.converterID,gatewayID1)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway2,gatewayID2))
        << "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)
        << "ERROR: domainID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConverters(returnList));
    bool equal = true;
    std::vector<am_Converter_s>::iterator listIterator = returnList.begin();

    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->converterID == gatewayID)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway);
        }

        if (listIterator->converterID == gatewayID1)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway1);
        }

        if (listIterator->converterID == gatewayID2)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway2);
        }
    }

    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest,getGatewayInfo)
{


    //initialize gateway
    std::vector<am_Gateway_s> returnList;
    am_Gateway_s gateway, gateway1, gateway2;
    am_gatewayID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;

    pCF.createGateway(gateway);
    pCF.createGateway(gateway1);
    gateway1.gatewayID = 20;
    pCF.createGateway(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway,gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(100,gatewayID)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1))
        << "ERROR: database error";
    ASSERT_EQ(gateway1.gatewayID,gatewayID1)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2))
        << "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)
        << "ERROR: domainID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListGateways(returnList));
    bool equal = true;
    std::vector<am_Gateway_s>::iterator listIterator = returnList.begin();

    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->gatewayID == gatewayID)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway);
        }

        if (listIterator->gatewayID == gatewayID1)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway1);
        }

        if (listIterator->gatewayID == gatewayID2)
        {
            equal = equal && pCF.compareGateway(listIterator, gateway2);
        }
    }

    ASSERT_EQ(true, equal);

    am_Gateway_s gatewayInfo;
    ASSERT_EQ(E_OK, pDatabaseHandler.getGatewayInfoDB(20,gatewayInfo));
    ASSERT_TRUE(pCF.compareGateway1(gateway1,gatewayInfo));

}

TEST_F(CAmMapHandlerTest,getConverterInfo)
{
    //initialize gateway
    std::vector<am_Converter_s> returnList;
    am_Converter_s gateway, gateway1, gateway2;
    am_converterID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;

    pCF.createConverter(gateway);
    pCF.createConverter(gateway1);
    gateway1.converterID = 20;
    pCF.createConverter(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway,gatewayID))
        << "ERROR: database error";
    ASSERT_EQ(100,gatewayID)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway1,gatewayID1))
        << "ERROR: database error";
    ASSERT_EQ(gateway1.converterID,gatewayID1)
        << "ERROR: domainID zero";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway2,gatewayID2))
        << "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)
        << "ERROR: domainID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConverters(returnList));
    bool equal = true;
    std::vector<am_Converter_s>::iterator listIterator = returnList.begin();

    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->converterID == gatewayID)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway);
        }

        if (listIterator->converterID == gatewayID1)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway1);
        }

        if (listIterator->converterID == gatewayID2)
        {
            equal = equal && pCF.compareConverter(listIterator, gateway2);
        }
    }

    ASSERT_EQ(true, equal);

    am_Converter_s gatewayInfo;
    ASSERT_EQ(E_OK, pDatabaseHandler.getConverterInfoDB(20,gatewayInfo));
    ASSERT_TRUE(pCF.compareConverter1(gateway1,gatewayInfo));

}

TEST_F(CAmMapHandlerTest,enterSinkThatAlreadyExistFail)
{
    //fill the connection database
    am_Sink_s staticSink, SecondSink;
    am_sinkID_t staticSinkID, SecondSinkID;

    pCF.createSink(staticSink);
    staticSink.sinkID = 43;
    staticSink.name = "Static";


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(staticSink,staticSinkID))
        << "ERROR: database error";
    ASSERT_EQ(staticSink.sinkID,staticSinkID)
        << "ERROR: ID not the one given in staticSink";

    pCF.createSink(SecondSink);
    SecondSink.sinkID = 43;
    SecondSink.name = "SecondSink";

    ASSERT_EQ(E_ALREADY_EXISTS,pDatabaseHandler.enterSinkDB(SecondSink,SecondSinkID))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,enterSourcesThatAlreadyExistFail)
{
    //fill the connection database
    am_Source_s staticSource, SecondSource;
    am_sourceID_t staticSourceID, SecondSourceID;
    pCF.createSource(staticSource);
    staticSource.sourceID = 4;


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(staticSource,staticSourceID))
        << "ERROR: database error";
    ASSERT_EQ(staticSource.sourceID,staticSourceID)
        << "ERROR: ID not the one given in staticSource";

    pCF.createSource(SecondSource);
    SecondSource.sourceID = 4;

    ASSERT_EQ(E_ALREADY_EXISTS,pDatabaseHandler.enterSourceDB(SecondSource,SecondSourceID))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerTest,registerDomainCorrect)
{
    //initialize domain
    std::vector<am_Domain_s> returnList;
    am_Domain_s domain;
    am_domainID_t domainID = 0;
    pCF.createDomain(domain);

    ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID))
        << "ERROR: database error";
    ASSERT_NE(0,domainID)
        << "ERROR: domainID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(returnList));
    bool equal = true;
    std::vector<am_Domain_s>::iterator listIterator = returnList.begin();
    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->domainID == domainID)
        {
            equal = equal && (listIterator->name.compare(domain.name) == 0) && (listIterator->busname.compare(domain.busname) == 0) && (listIterator->complete == domain.complete) && (listIterator->early == domain.early) && (listIterator->state == domain.state);
        }
    }
    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest,registerConnectionCorrect)
{
    am_Connection_s connection;
    am_connectionID_t connectionID;
    std::vector<am_Connection_s> returnList;
    pCF.createConnection(connection);

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConnectionDB(connection,connectionID))
        << "ERROR: database error";;
    ASSERT_NE(0,connectionID)
        << "ERROR: connectionID zero";

    //now check if we read out the correct values
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(returnList));
    bool equal = false;
    std::vector<am_Connection_s>::iterator listIterator = returnList.begin();
    for (; listIterator < returnList.end(); ++listIterator)
    {
        if (listIterator->connectionID == connectionID)
        {
            equal = (listIterator->sourceID == connection.sourceID) && (listIterator->sinkID == connection.sinkID) && (listIterator->delay == connection.delay) && (listIterator->connectionFormat = connection.connectionFormat);
        }
    }
    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest,enterMainConnectionCorrect)
{
    createMainConnectionSetup();
}

TEST_F(CAmMapHandlerTest,enterSinksCorrect)
{
    //fill the connection database
    am_Sink_s staticSink, firstDynamicSink, secondDynamicSink;
    am_sinkID_t staticSinkID, firstDynamicSinkID, secondDynamicSinkID;
    std::vector<am_Sink_s> sinkList;

    pCF.createSink(staticSink);
    staticSink.sinkID = 4;


    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(staticSink,staticSinkID))
        << "ERROR: database error";
    ASSERT_EQ(staticSink.sinkID,staticSinkID)
        << "ERROR: ID not the one given in staticSink";

    pCF.createSink(firstDynamicSink);
    firstDynamicSink.name = "firstdynamic";
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(firstDynamicSink,firstDynamicSinkID))
        << "ERROR: database error";
    ASSERT_EQ(firstDynamicSinkID,DYNAMIC_ID_BOUNDARY)
        << "ERROR: ID not the one given in firstDynamicSink";

    pCF.createSink(secondDynamicSink);
    secondDynamicSink.name = "seconddynamic";

    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(secondDynamicSink,secondDynamicSinkID))
        << "ERROR: database error";
    ASSERT_NEAR(secondDynamicSinkID,DYNAMIC_ID_BOUNDARY,10)
        << "ERROR: ID not the one given in secondDynamicSink";

    //now read back and check the returns agains the given values
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(sinkList))
        << "ERROR: database error";
    bool equal = true;

    std::vector<am_Sink_s>::iterator listIterator = sinkList.begin();
    for (; listIterator < sinkList.end(); ++listIterator)
    {
        if (listIterator->sinkID == staticSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, staticSink);
        }

        if (listIterator->sinkID == firstDynamicSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, firstDynamicSink);
        }

        if (listIterator->sinkID == secondDynamicSinkID)
        {
            equal = equal && pCF.compareSink(listIterator, secondDynamicSink);
        }
    }
    ASSERT_EQ(true, equal);
}

TEST_F(CAmMapHandlerTest,enterNotificationConfigurationCorrect)
{
    am_Sink_s testSinkData, readoutData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;
    testSinkData.listNotificationConfigurations.push_back(notify);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(listSinks))
        << "ERROR: database error";

    ASSERT_EQ(listSinks.begin()->listNotificationConfigurations[2].parameter,notify.parameter);
    ASSERT_EQ(listSinks.begin()->listNotificationConfigurations[2].status,notify.status);
    ASSERT_EQ(listSinks.begin()->listNotificationConfigurations[2].type,notify.type);

    ASSERT_EQ(E_OK,pDatabaseHandler.getSinkInfoDB(testSinkData.sinkID,readoutData))
            << "ERROR: database error";

    ASSERT_EQ(readoutData.listNotificationConfigurations[2].parameter,notify.parameter);
    ASSERT_EQ(readoutData.listNotificationConfigurations[2].status,notify.status);
    ASSERT_EQ(readoutData.listNotificationConfigurations[2].type,notify.type);

}

TEST_F(CAmMapHandlerTest,enterMainNotificationConfigurationCorrect)
{
    am_Sink_s testSinkData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSinkData.listMainNotificationConfigurations.push_back(notify);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(listSinks))
        << "ERROR: database error";

    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].parameter,notify.parameter);
    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].status,notify.status);
    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].type,notify.type);
}

TEST_F(CAmMapHandlerTest,removeNotificationsSink)
{
    am_Sink_s testSinkData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSinkData.listMainNotificationConfigurations.push_back(notify);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSinks(listSinks))
        << "ERROR: database error";

    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].parameter,notify.parameter);
    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].status,notify.status);
    ASSERT_EQ(listSinks.begin()->listMainNotificationConfigurations[2].type,notify.type);

    //now we remove the sink
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSinkDB(sinkID));
}

TEST_F(CAmMapHandlerTest,removeNotificationsSource)
{
    am_Source_s testSourceData;
    pCF.createSource(testSourceData);
    testSourceData.sourceID = 4;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSourceData.listMainNotificationConfigurations.push_back(notify);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(testSourceData,sourceID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListSources(listSources))
        << "ERROR: database error";

    ASSERT_EQ(listSources.begin()->listMainNotificationConfigurations[2].parameter,notify.parameter);
    ASSERT_EQ(listSources.begin()->listMainNotificationConfigurations[2].status,notify.status);
    ASSERT_EQ(listSources.begin()->listMainNotificationConfigurations[2].type,notify.type);

    //now we remove the sink
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSourceDB(sourceID));
}

TEST_F(CAmMapHandlerTest,getMainNotificationsSink)
{
    am_Sink_s testSinkData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;
    std::vector<am_NotificationConfiguration_s>returnList;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSinkData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    testSinkData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSinkNotificationConfigurations(sinkID,returnList))
        << "ERROR: database error";

    std::equal(testSinkData.listMainNotificationConfigurations.begin(),testSinkData.listMainNotificationConfigurations.end(),returnList.begin(),equalNotificationConfiguration);

}

TEST_F(CAmMapHandlerTest,getMainNotificationsSources)
{
    am_Source_s testSourceData;
    pCF.createSource(testSourceData);
    testSourceData.sourceID = 4;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    std::vector<am_NotificationConfiguration_s>returnList;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSourceData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    testSourceData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(testSourceData,sourceID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSourceNotificationConfigurations(sourceID,returnList))
        << "ERROR: database error";

    std::equal(testSourceData.listMainNotificationConfigurations.begin(),testSourceData.listMainNotificationConfigurations.end(),returnList.begin(),equalNotificationConfiguration);

}

TEST_F(CAmMapHandlerTest,changeMainNotificationsSources)
{
    am_Source_s testSourceData;
    pCF.createSource(testSourceData);
    testSourceData.sourceID = 4;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    std::vector<am_NotificationConfiguration_s>returnList,returnList1;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSourceData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    am_NotificationConfiguration_s notify2;
    notify2.type=NT_UNKNOWN;
    notify2.status=NS_CHANGE;
    notify2.parameter=10;

    testSourceData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(testSourceData,sourceID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSourceNotificationConfigurations(sourceID,returnList))
        << "ERROR: database error";

    ASSERT_EQ(true, std::equal(testSourceData.listMainNotificationConfigurations.begin(),
    				 testSourceData.listMainNotificationConfigurations.end(),
    				 returnList.begin(),
    				 equalNotificationConfiguration));

    //change a setting
    ASSERT_EQ(E_OK,pDatabaseHandler.changeMainSourceNotificationConfigurationDB(sourceID,notify2));

    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSourceNotificationConfigurations(sourceID,returnList1))
        << "ERROR: database error";

    ASSERT_EQ(returnList1[3].parameter,notify2.parameter);
    ASSERT_EQ(returnList1[3].status,notify2.status);
    ASSERT_EQ(returnList1[3].type,notify2.type);

}

TEST_F(CAmMapHandlerTest,changeMainNotificationsSink)
{
    am_Sink_s testSinkData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;
    std::vector<am_NotificationConfiguration_s>returnList,returnList1;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSinkData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    am_NotificationConfiguration_s notify2;
    notify2.type=NT_UNKNOWN;
    notify2.status=NS_CHANGE;
    notify2.parameter=27;

    testSinkData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";

    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSinkNotificationConfigurations(sinkID,returnList))
        << "ERROR: database error";

    std::equal(testSinkData.listMainNotificationConfigurations.begin(),testSinkData.listMainNotificationConfigurations.end(),returnList.begin(),equalNotificationConfiguration);

    ASSERT_EQ(E_OK,pDatabaseHandler.changeMainSinkNotificationConfigurationDB(sinkID,notify2))
        << "ERROR: database error";

    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSinkNotificationConfigurations(sinkID,returnList1))
        << "ERROR: database error";

    ASSERT_EQ(returnList1[3].parameter,notify2.parameter);
    ASSERT_EQ(returnList1[3].status,notify2.status);
    ASSERT_EQ(returnList1[3].type,notify2.type);
}

TEST_F(CAmMapHandlerTest, peekDomain_2)
{
    std::vector<am_Domain_s> listDomains;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_domainID_t domain2ID;
    pCF.createDomain(domain);
    ASSERT_EQ(E_OK,pDatabaseHandler.peekDomain(std::string("newdomain"),domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_TRUE(listDomains.empty());
    ASSERT_EQ(domainID, 1);

    domain.name = "anotherdomain";
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domain2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(domain2ID, 2);

    domain.name = "newdomain";
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domain2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(domainID, domain2ID);               // FAILS, ID is 2 instead of 1
	bool containsDomainID = std::find_if(listDomains.begin(), listDomains.end(), [&](const am_Domain_s & ref) {
		return ref.domainID==domainID;
	})!=listDomains.end();
    ASSERT_TRUE(containsDomainID);
}

TEST_F(CAmMapHandlerTest, connectionIDBoundary)
{
	am_Sink_s sink;
	am_Source_s source;
	am_Connection_s connection;
	connection.delay = -1;
	connection.connectionFormat = CF_GENIVI_ANALOG;
	connection.connectionID = 0;
	am_sinkID_t forgetSink;
	am_sourceID_t forgetSource;
	am_connectionID_t connectionID;
	for (uint16_t i = 1; i < TEST_MAX_SINK_ID; i++)
	{
		pCF.createSink(sink);
		sink.sinkID = 0;
		sink.name = "sink" + int2string(i);
		sink.domainID = 4;
		pCF.createSource(source);
		source.sourceID = 0;
		source.name = "source" + int2string(i);
		source.domainID = 4;
		ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, forgetSink));
		ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, forgetSource));
		connection.sinkID = forgetSink;
		connection.sourceID = forgetSource;
		if( i < TEST_MAX_CONNECTION_ID )
		{
			ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
			ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
			ASSERT_EQ(i, connectionID);
		}
	}
	std::vector<am_Connection_s> connectionList;
	ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
	ASSERT_EQ(TEST_MAX_CONNECTION_ID-1, static_cast<int>(connectionList.size()));
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterConnectionDB(connection,connectionID));
	logInfo("here");
	ASSERT_EQ(0, connectionID);

	ASSERT_EQ(E_OK, pDatabaseHandler.removeConnection(10));
	ASSERT_EQ(E_OK, pDatabaseHandler.removeConnection(12));

	ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
	ASSERT_EQ(10, connectionID);
	connection.sinkID = 77;
	connection.sourceID = 77;
	ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
	ASSERT_EQ(12, connectionID);
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterConnectionDB(connection,connectionID));
	ASSERT_EQ(0, connectionID);
}

TEST_F(CAmMapHandlerTest, mainConnectionIDBoundary)
{
	am_Sink_s sink;
	am_Source_s source;
	am_Connection_s connection;
	connection.delay = -1;
	connection.connectionFormat = CF_GENIVI_ANALOG;
	connection.connectionID = 0;
	am_sinkID_t forgetSink;
	am_sourceID_t forgetSource;
	am_connectionID_t connectionID;
	std::vector<am_connectionID_t> connectionIDList;
	for (uint16_t i = 1; i < TEST_MAX_SINK_ID; i++)
	{
		pCF.createSink(sink);
		sink.sinkID = 0;
		sink.name = "sink" + int2string(i);
		sink.domainID = 4;
		pCF.createSource(source);
		source.sourceID = 0;
		source.name = "source" + int2string(i);
		source.domainID = 4;
		ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, forgetSink));
		ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, forgetSource));
		connection.sinkID = forgetSink;
		connection.sourceID = forgetSource;
		if( i < TEST_MAX_CONNECTION_ID )
		{
			ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
			ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
			ASSERT_EQ(i, connectionID);
			connectionIDList.push_back(i);
		}
	}
	std::vector<am_Connection_s> connectionList;
	ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
	ASSERT_EQ(TEST_MAX_CONNECTION_ID-1, static_cast<int>(connectionList.size()));

	//create a mainConnection

	am_MainConnection_s mainConnection;
	am_mainConnectionID_t mainConnectionID;
	mainConnection.listConnectionID = connectionIDList;
	mainConnection.mainConnectionID = 0;
	mainConnection.connectionState = CS_CONNECTED;
	mainConnection.delay = -1;

	for (uint16_t i = 1; i < TEST_MAX_MAINCONNECTION_ID; i++)
	{
		mainConnection.sinkID = DYNAMIC_ID_BOUNDARY + i;
		mainConnection.sourceID = DYNAMIC_ID_BOUNDARY + i;
		ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
		ASSERT_EQ(i, mainConnectionID);
	}
	ASSERT_EQ(E_OK, pDatabaseHandler.removeMainConnectionDB(10));
	ASSERT_EQ(E_OK, pDatabaseHandler.removeMainConnectionDB(12));
	ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
	ASSERT_EQ(10, mainConnectionID);
	mainConnection.sinkID = 77;
	mainConnection.sourceID = 77;
	ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
	ASSERT_EQ(12, mainConnectionID);
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
	ASSERT_EQ(0, mainConnectionID);
}

TEST_F(CAmMapHandlerTest, increaseID)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	for (uint16_t i = 0; i < TEST_MAX_SINK_ID; i++)
	{
		pCF.createSink(sink);
		sink.sinkID = 0;
		sink.name = "sink" + int2string(i);
		sink.domainID = 4;
		ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
		ASSERT_EQ(DYNAMIC_ID_BOUNDARY+i, sinkID);
	}
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterSinkDB(sink, sinkID));

	ASSERT_EQ(E_OK, pDatabaseHandler.removeSinkDB(DYNAMIC_ID_BOUNDARY+10));
	ASSERT_EQ(E_OK, pDatabaseHandler.removeSinkDB(DYNAMIC_ID_BOUNDARY+12));

	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterSinkDB(sink, sinkID));
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterSinkDB(sink, sinkID));
	ASSERT_EQ(E_UNKNOWN, pDatabaseHandler.enterSinkDB(sink, sinkID));
}


CAmMapHandlerObserverCallbacksTest::CAmMapHandlerObserverCallbacksTest() :
                plistRoutingPluginDirs(),
                plistCommandPluginDirs(),
				pSocketHandler(),
				pRoutingSender(plistRoutingPluginDirs, pSocketHandler),
				pCommandSender(plistCommandPluginDirs),
				pControlSender(),
				mMockObserver(&pCommandSender, &pRoutingSender, &pSocketHandler),
				pDatabaseHandler(),
				pRouter(&pDatabaseHandler, &pControlSender),
				pCF()
{
	pDatabaseHandler.registerObserver(&mMockObserver);
}

void CAmMapHandlerObserverCallbacksTest::SetUp()
{

}

void CAmMapHandlerObserverCallbacksTest::TearDown()
{

}

CAmMapHandlerObserverCallbacksTest::~CAmMapHandlerObserverCallbacksTest()
{

}

MATCHER_P(IsDomainDataEqualTo, value, "") {
	auto lh = arg;
	return lh.domainID == value.domainID &&
			lh.name == value.name &&
			lh.nodename == value.nodename &&
			lh.early == value.early &&
			lh.complete == value.complete &&
			lh.state == value.state;
}

TEST_F(CAmMapHandlerObserverCallbacksTest, peek_enter_removeDomain)
{
    std::vector<am_Domain_s> listDomains;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_domainID_t domain2ID;
    pCF.createDomain(domain);
    ASSERT_EQ(E_OK,pDatabaseHandler.peekDomain(std::string("newdomain"), domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_TRUE(listDomains.empty());
    ASSERT_EQ(domainID, 1);

    domain.name = "anotherdomain";
    const am_Domain_s expDomain1 = {2, domain.name, domain.busname, domain.nodename, domain.early, domain.complete, domain.state};
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newDomain(IsDomainDataEqualTo(expDomain1))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domain2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(domain2ID, 2);
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(MockDatabaseObserver::getMockObserverObject()));
    domain.name = "newdomain";
    const am_Domain_s expDomain2 = {1, domain.name, domain.busname, domain.nodename, domain.early, domain.complete, domain.state};
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newDomain(IsDomainDataEqualTo(expDomain2))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domain2ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListDomains(listDomains));
    ASSERT_EQ(domainID, domain2ID);               // FAILS, ID is 2 instead of 1
	bool containsDomainID = std::find_if(listDomains.begin(), listDomains.end(), [&](const am_Domain_s & ref) {
		return ref.domainID==domainID;
	})!=listDomains.end();
    ASSERT_TRUE(containsDomainID);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removeDomain(domainID)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeDomainDB(domainID))<< "ERROR: database error";
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(MockDatabaseObserver::getMockObserverObject()));
}

TEST_F(CAmMapHandlerObserverCallbacksTest, peek_enter_update_removeSource)
{
    std::vector<am_Source_s> listSources;
    am_sourceID_t sourceID;
    am_sourceID_t source2ID;
    am_sourceID_t source3ID;
    am_Source_s source;
    pCF.createSource(source);

    //peek a source that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(std::string("newsource"),sourceID));

    //peek a second source that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSource(std::string("newsource2"),source2ID));

    //make sure they are is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_TRUE(listSources.empty());
    ASSERT_EQ(sourceID, 100);

    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(Field(&am_Source_s::sourceID, 102))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source3ID));
    ASSERT_EQ(source3ID, 102);

    source.name = "newsource";
    //now enter the source with the same name than the first peek and make sure it does not get a new ID
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(Field(&am_Source_s::sourceID, 100))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(sourceID, source3ID);
    bool containsSourceID = std::find_if(listSources.begin(), listSources.end(), [&](const am_Source_s & ref) {
    		return ref.sourceID==sourceID;
    	})!=listSources.end();
    ASSERT_TRUE(containsSourceID);
    std::vector<am_SoundProperty_s> listSoundProperties;
    std::vector<am_CustomAvailabilityReason_t> listConnectionFormats;
    std::vector<am_MainSoundProperty_s> listMainSoundProperties;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sourceUpdated(sourceID, _, _, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.changeSourceDB(sourceID, 1, listSoundProperties, listConnectionFormats, listMainSoundProperties))<< "ERROR: database error";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removedSource(sourceID, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSourceDB(sourceID))<< "ERROR: database error";
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(MockDatabaseObserver::getMockObserverObject()));
}

TEST_F(CAmMapHandlerObserverCallbacksTest, peek_enter_update_removeSink)
{
    std::vector<am_Sink_s> listSinks;
    am_sinkID_t sinkID;
    am_sinkID_t sink2ID;
    am_sinkID_t sink3ID;
    am_Sink_s sink;
    pCF.createSink(sink);

    //peek a sink that does not exits

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(std::string("newsink"),sinkID));

    //peek again

    ASSERT_EQ(E_OK, pDatabaseHandler.peekSink(std::string("nextsink"),sink2ID));

    //make sure they are is not in the list
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_TRUE(listSinks.empty());
    ASSERT_EQ(sinkID, 100);

    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(Field(&am_Sink_s::sinkID, 102))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sink3ID));
    ASSERT_EQ(sink3ID, 102);

    sink.name = "newsink";
    //now enter the sink with the same name than the first peek and make sure it does not get a new ID
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(Field(&am_Sink_s::sinkID, 100))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sink3ID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(sinkID, sink3ID);
    bool containsSourceID = std::find_if(listSinks.begin(), listSinks.end(), [&](const am_Sink_s & ref) {
			return ref.sinkID==sinkID;
	})!=listSinks.end();
	ASSERT_TRUE(containsSourceID);
    std::vector<am_SoundProperty_s> listSoundProperties;
    std::vector<am_CustomAvailabilityReason_t> listConnectionFormats;
    std::vector<am_MainSoundProperty_s> listMainSoundProperties;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sinkUpdated(sinkID, _, _, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.changeSinkDB(sinkID, 1, listSoundProperties, listConnectionFormats, listMainSoundProperties))<< "ERROR: database error";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removedSink(sinkID, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeSinkDB(sinkID))<< "ERROR: database error";
    EXPECT_TRUE(Mock::VerifyAndClearExpectations(MockDatabaseObserver::getMockObserverObject()));
}

TEST_F(CAmMapHandlerObserverCallbacksTest, peekSourceClassID)
{
    std::string sourceName("myClassID");
    am_sourceClass_t sourceClassID, peekID;
    am_SourceClass_s sourceClass;
    am_ClassProperty_s classProperty;
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 13;
    sourceClass.name = sourceName;
    sourceClass.sourceClassID = 0;
    sourceClass.listClassProperties.push_back(classProperty);

    //first we peek without an existing class
    ASSERT_EQ(E_NON_EXISTENT, pDatabaseHandler.peekSourceClassID(sourceName,sourceClassID));

    //now we enter the class into the database
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), numberOfSourceClassesChanged()).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceClassID,sourceClass));

    //first we peek without an existing class
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSourceClassID(sourceName,peekID));
    ASSERT_EQ(sourceClassID, peekID);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, peekSinkClassID)
{
    std::string sinkName("myClassID");
    am_sinkClass_t sinkClassID, peekID;
    am_SinkClass_s sinkClass;
    am_ClassProperty_s classProperty;
    classProperty.classProperty = CP_GENIVI_SOURCE_TYPE;
    classProperty.value = 13;
    sinkClass.name = sinkName;
    sinkClass.sinkClassID = 0;
    sinkClass.listClassProperties.push_back(classProperty);

    //first we peek without an existing class
    ASSERT_EQ(E_NON_EXISTENT, pDatabaseHandler.peekSinkClassID(sinkName,sinkClassID));

    //now we enter the class into the database
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), numberOfSinkClassesChanged()).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass,sinkClassID));

    //first we peek without an existing class
    ASSERT_EQ(E_OK, pDatabaseHandler.peekSinkClassID(sinkName,peekID));
    ASSERT_EQ(sinkClassID, peekID);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, enter_removeGateway)
{
    //initialize gateway
    std::vector<am_Gateway_s> returnList;
    am_Gateway_s gateway, gateway1, gateway2;
    am_gatewayID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;
    pCF.createGateway(gateway);
    pCF.createGateway(gateway1);
    gateway1.gatewayID = 20;
    pCF.createGateway(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newGateway(Field(&am_Gateway_s::gatewayID, 100))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway,gatewayID))<< "ERROR: database error";
    ASSERT_EQ(100,gatewayID)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newGateway(Field(&am_Gateway_s::gatewayID, 20))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1))<< "ERROR: database error";
    ASSERT_EQ(gateway1.gatewayID,gatewayID1)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newGateway(Field(&am_Gateway_s::gatewayID, 101))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2))<< "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removeGateway(gatewayID2)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeGatewayDB(gatewayID2))<< "ERROR: database error";
}

TEST_F(CAmMapHandlerObserverCallbacksTest, enter_removeConverter)
{
    //initialize gateway
    std::vector<am_Converter_s> returnList;
    am_Converter_s gateway, gateway1, gateway2;
    am_converterID_t gatewayID = 0, gatewayID1 = 0, gatewayID2 = 0;
    pCF.createConverter(gateway);
    pCF.createConverter(gateway1);
    gateway1.converterID = 20;
    pCF.createConverter(gateway2);
    am_Sink_s sink;
    am_Source_s source;
    am_sinkID_t sinkID;
    am_sourceID_t sourceID;
    pCF.createSink(sink);
    pCF.createSource(source);
    sink.sinkID = 1;
    source.sourceID = 2;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newConverter(Field(&am_Converter_s::converterID, 100))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway,gatewayID))<< "ERROR: database error";
    ASSERT_EQ(100,gatewayID)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newConverter(Field(&am_Converter_s::converterID, 20))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway1,gatewayID1))<< "ERROR: database error";
    ASSERT_EQ(gateway1.converterID,gatewayID1)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newConverter(Field(&am_Converter_s::converterID, 101))).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConverterDB(gateway2,gatewayID2))<< "ERROR: database error";
    ASSERT_EQ(101,gatewayID2)<< "ERROR: domainID zero";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removeConverter(gatewayID2)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeConverterDB(gatewayID2))<< "ERROR: database error";
}

TEST_F(CAmMapHandlerObserverCallbacksTest, enter_removeCrossfader)
{
    am_Crossfader_s crossfader;
    am_crossfaderID_t crossfaderID;
    am_Sink_s sinkA, sinkB;
    am_Source_s source;
    am_sourceID_t sourceID;
    am_sinkID_t sinkAID, sinkBID;
    pCF.createSink(sinkA);
    pCF.createSink(sinkB);
    sinkB.name = "sinkB";
    pCF.createSource(source);

    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkA,sinkAID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sinkB,sinkBID));

    crossfader.crossfaderID = 0;
    crossfader.hotSink = HS_SINKA;
    crossfader.sinkID_A = sinkAID;
    crossfader.sinkID_B = sinkBID;
    crossfader.sourceID = sourceID;
    crossfader.name = "Crossfader";
    crossfader.hotSink = HS_UNKNOWN;

    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newCrossfader(Field(&am_Crossfader_s::crossfaderID, 100))).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterCrossfaderDB(crossfader,crossfaderID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removeCrossfader(crossfaderID)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeCrossfaderDB(crossfaderID))<< "ERROR: database error";
}

TEST_F(CAmMapHandlerObserverCallbacksTest, enter_removeMainConnection)
{
    //fill the connection database
    am_Connection_s connection;
    am_Source_s source;
    am_Sink_s sink;
    std::vector<am_connectionID_t> connectionList;

    //we create 9 sources and sinks:
    uint16_t i = 1;
    for (; i < 10; i++)
    {
        am_sinkID_t forgetSink;
        am_sourceID_t forgetSource;
        am_connectionID_t connectionID;

        pCF.createSink(sink);
        sink.sinkID = i;
        sink.name = "sink" + int2string(i);
        sink.domainID = 4;
        pCF.createSource(source);
        source.sourceID = i;
        source.name = "source" + int2string(i);
        source.domainID = 4;

        connection.sinkID = i;
        connection.sourceID = i;
        connection.delay = -1;
        connection.connectionFormat = CF_GENIVI_ANALOG;
        connection.connectionID = 0;

        EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
        ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,forgetSink));
        EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
        ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,forgetSource));
        ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection,connectionID));
        ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
        connectionList.push_back(connectionID);
    }
    //create a mainConnection
    am_MainConnection_s mainConnection;
    am_mainConnectionID_t mainConnectionID;
    std::vector<am_MainConnection_s> mainConnectionList;
    mainConnection.listConnectionID = connectionList;
    mainConnection.mainConnectionID = 0;
    mainConnection.sinkID = 1;
    mainConnection.sourceID = 1;
    mainConnection.connectionState = CS_CONNECTED;
    mainConnection.delay = -1;

    //enter mainconnection in database
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newMainConnection(Field(&am_MainConnectionType_s::mainConnectionID, 1))).Times(1);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), mainConnectionStateChanged(1, CS_CONNECTED)).Times(1);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), timingInformationChanged(1, _)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterMainConnectionDB(mainConnection,mainConnectionID));
    ASSERT_NE(0, mainConnectionID);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), removedMainConnection(1)).Times(1);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), mainConnectionStateChanged(1, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.removeMainConnectionDB(1)) << "ERROR: database error";
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeSinkAvailability)
{
    std::vector<am_Sink_s> listSinks;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    am_Availability_s availability;
    availability.availability = A_UNKNOWN;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sinkAvailabilityChanged(_, _)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkAvailabilityDB(availability,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(availability.availability, listSinks[0].available.availability);
    ASSERT_EQ(availability.availabilityReason, listSinks[0].available.availabilityReason);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeSourceAvailability)
{
    std::vector<am_Source_s> listSources;
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    am_Availability_s availability;
    availability.availability = A_UNKNOWN;
    availability.availabilityReason = AR_GENIVI_TEMPERATURE;
    source.visible = true;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sourceAvailabilityChanged(_, _)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSourceAvailabilityDB(availability,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSources(listSources));
    ASSERT_EQ(availability.availability, listSources[0].available.availability);
    ASSERT_EQ(availability.availabilityReason, listSources[0].available.availabilityReason);
}

TEST_F(CAmMapHandlerObserverCallbacksTest,changeMainSinkVolume)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_mainVolume_t newVol = 20;
    std::vector<am_Sink_s> listSinks;
    pCF.createSink(sink);
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), volumeChanged(sinkID, newVol)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkMainVolumeDB(newVol,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(listSinks[0].mainVolume, newVol);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeSinkMuteState)
{
    std::vector<am_Sink_s> listSinks;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);
    am_MuteState_e muteState = MS_MUTED;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sinkMuteStateChanged(sinkID, muteState)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSinkMuteStateDB(muteState,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSinks(listSinks));
    ASSERT_EQ(muteState, listSinks[0].muteState);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeSystemProperty)
{
    std::vector<am_SystemProperty_s> listSystemProperties, listReturn;
    am_SystemProperty_s systemProperty;

    systemProperty.type = SYP_UNKNOWN;
    systemProperty.value = 33;
    listSystemProperties.push_back(systemProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSystemProperties(listSystemProperties));
    systemProperty.value = 444;
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), systemPropertyChanged(_)).Times(1);
    ASSERT_EQ(E_OK, pDatabaseHandler.changeSystemPropertyDB(systemProperty));
    ASSERT_EQ(E_OK, pDatabaseHandler.getListSystemProperties(listReturn));
    ASSERT_EQ(listReturn[0].type, systemProperty.type);
    ASSERT_EQ(listReturn[0].value, systemProperty.value);
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeMainNotificationsSink)
{
    am_Sink_s testSinkData;
    pCF.createSink(testSinkData);
    testSinkData.sinkID = 4;
    am_sinkID_t sinkID;
    std::vector<am_Sink_s> listSinks;
    std::vector<am_NotificationConfiguration_s>returnList,returnList1;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSinkData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    am_NotificationConfiguration_s notify2;
    notify2.type=NT_UNKNOWN;
    notify2.status=NS_CHANGE;
    notify2.parameter=27;

    testSinkData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSink(_)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(testSinkData,sinkID))
        << "ERROR: database error";
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sinkMainNotificationConfigurationChanged(sinkID, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.changeMainSinkNotificationConfigurationDB(sinkID,notify2))
        << "ERROR: database error";
}

TEST_F(CAmMapHandlerObserverCallbacksTest, changeMainNotificationsSources)
{

    am_Source_s testSourceData;
    pCF.createSource(testSourceData);
    testSourceData.sourceID = 4;
    am_sourceID_t sourceID;
    std::vector<am_Source_s> listSources;
    std::vector<am_NotificationConfiguration_s>returnList,returnList1;

    am_NotificationConfiguration_s notify;
    notify.type=NT_UNKNOWN;
    notify.status=NS_CHANGE;
    notify.parameter=25;

    testSourceData.listMainNotificationConfigurations.push_back(notify);

    am_NotificationConfiguration_s notify1;
    notify1.type=NT_UNKNOWN;
    notify1.status=NS_PERIODIC;
    notify1.parameter=5;

    am_NotificationConfiguration_s notify2;
    notify2.type=NT_UNKNOWN;
    notify2.status=NS_CHANGE;
    notify2.parameter=10;

    testSourceData.listMainNotificationConfigurations.push_back(notify1);

    //enter the sink in the database
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), newSource(_)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(testSourceData,sourceID))
        << "ERROR: database error";
    //read it again
    ASSERT_EQ(E_OK,pDatabaseHandler.getListMainSourceNotificationConfigurations(sourceID,returnList))
        << "ERROR: database error";
    //change a setting
    EXPECT_CALL(*MockDatabaseObserver::getMockObserverObject(), sourceMainNotificationConfigurationChanged(sourceID, _)).Times(1);
    ASSERT_EQ(E_OK,pDatabaseHandler.changeMainSourceNotificationConfigurationDB(sourceID,notify2));
}

int main(int argc, char **argv)
{
    CAmDltWrapper::instance()->registerApp("databse", "databasetest");
    logInfo("Database Test started ");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


