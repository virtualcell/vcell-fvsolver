/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <activemq/wireformat/openwire/marshal/generated/BrokerInfoMarshaller.h>

#include <activemq/commands/BrokerInfo.h>
#include <activemq/exceptions/ActiveMQException.h>
#include <decaf/lang/Pointer.h>

//
//     NOTE!: This file is autogenerated - do not modify!
//            if you need to make a change, please see the Java Classes in the
//            activemq-core module
//

using namespace std;
using namespace activemq;
using namespace activemq::exceptions;
using namespace activemq::commands;
using namespace activemq::wireformat;
using namespace activemq::wireformat::openwire;
using namespace activemq::wireformat::openwire::marshal;
using namespace activemq::wireformat::openwire::utils;
using namespace activemq::wireformat::openwire::marshal::generated;
using namespace decaf;
using namespace decaf::io;
using namespace decaf::lang;

///////////////////////////////////////////////////////////////////////////////
DataStructure* BrokerInfoMarshaller::createObject() const {
    return new BrokerInfo();
}

///////////////////////////////////////////////////////////////////////////////
unsigned char BrokerInfoMarshaller::getDataStructureType() const {
    return BrokerInfo::ID_BROKERINFO;
}

///////////////////////////////////////////////////////////////////////////////
void BrokerInfoMarshaller::tightUnmarshal( OpenWireFormat* wireFormat, DataStructure* dataStructure, DataInputStream* dataIn, BooleanStream* bs ) {

    try {

        BaseCommandMarshaller::tightUnmarshal( wireFormat, dataStructure, dataIn, bs );

        BrokerInfo* info =
            dynamic_cast<BrokerInfo*>( dataStructure );

        int wireVersion = wireFormat->getVersion();

        info->setBrokerId( Pointer<BrokerId>( dynamic_cast< BrokerId* >(
            tightUnmarshalCachedObject( wireFormat, dataIn, bs ) ) ) );
        info->setBrokerURL( tightUnmarshalString( dataIn, bs ) );

        if( bs->readBoolean() ) {
            short size = dataIn->readShort();
            info->getPeerBrokerInfos().reserve( size );
            for( int i = 0; i < size; i++ ) {
                info->getPeerBrokerInfos().push_back( Pointer<BrokerInfo>( dynamic_cast< BrokerInfo* >(
                    tightUnmarshalNestedObject( wireFormat, dataIn, bs ) ) ) );
            }
        } else {
            info->getPeerBrokerInfos().clear();
        }
        info->setBrokerName( tightUnmarshalString( dataIn, bs ) );
        info->setSlaveBroker( bs->readBoolean() );
        info->setMasterBroker( bs->readBoolean() );
        info->setFaultTolerantConfiguration( bs->readBoolean() );
        if( wireVersion >= 2 ) {
            info->setDuplexConnection( bs->readBoolean() );
        }
        if( wireVersion >= 2 ) {
            info->setNetworkConnection( bs->readBoolean() );
        }
        if( wireVersion >= 2 ) {
            info->setConnectionId( tightUnmarshalLong( wireFormat, dataIn, bs ) );
        }
        if( wireVersion >= 3 ) {
            info->setBrokerUploadUrl( tightUnmarshalString( dataIn, bs ) );
        }
        if( wireVersion >= 3 ) {
            info->setNetworkProperties( tightUnmarshalString( dataIn, bs ) );
        }
    }
    AMQ_CATCH_RETHROW( decaf::io::IOException )
    AMQ_CATCH_EXCEPTION_CONVERT( exceptions::ActiveMQException, decaf::io::IOException )
    AMQ_CATCHALL_THROW( decaf::io::IOException )
}

///////////////////////////////////////////////////////////////////////////////
int BrokerInfoMarshaller::tightMarshal1( OpenWireFormat* wireFormat, DataStructure* dataStructure, BooleanStream* bs ) {

    try {

        BrokerInfo* info =
            dynamic_cast<BrokerInfo*>( dataStructure );

        int rc = BaseCommandMarshaller::tightMarshal1( wireFormat, dataStructure, bs );

        int wireVersion = wireFormat->getVersion();

        rc += tightMarshalCachedObject1( wireFormat, info->getBrokerId().get(), bs );
        rc += tightMarshalString1( info->getBrokerURL(), bs );
        rc += tightMarshalObjectArray1( wireFormat, info->getPeerBrokerInfos(), bs );
        rc += tightMarshalString1( info->getBrokerName(), bs );
        bs->writeBoolean( info->isSlaveBroker() );
        bs->writeBoolean( info->isMasterBroker() );
        bs->writeBoolean( info->isFaultTolerantConfiguration() );
        if( wireVersion >= 2 ) {
            bs->writeBoolean( info->isDuplexConnection() );
        }
        if( wireVersion >= 2 ) {
            bs->writeBoolean( info->isNetworkConnection() );
        }
        if( wireVersion >= 2 ) {
            rc += tightMarshalLong1( wireFormat, info->getConnectionId(), bs );
        }
        if( wireVersion >= 3 ) {
            rc += tightMarshalString1( info->getBrokerUploadUrl(), bs );
        }
        if( wireVersion >= 3 ) {
            rc += tightMarshalString1( info->getNetworkProperties(), bs );
        }

        return rc + 0;
    }
    AMQ_CATCH_RETHROW( decaf::io::IOException )
    AMQ_CATCH_EXCEPTION_CONVERT( exceptions::ActiveMQException, decaf::io::IOException )
    AMQ_CATCHALL_THROW( decaf::io::IOException )
}

///////////////////////////////////////////////////////////////////////////////
void BrokerInfoMarshaller::tightMarshal2( OpenWireFormat* wireFormat, DataStructure* dataStructure, DataOutputStream* dataOut, BooleanStream* bs ) {

    try {

        BaseCommandMarshaller::tightMarshal2( wireFormat, dataStructure, dataOut, bs );

        BrokerInfo* info =
            dynamic_cast<BrokerInfo*>( dataStructure );

        int wireVersion = wireFormat->getVersion();

        tightMarshalCachedObject2( wireFormat, info->getBrokerId().get(), dataOut, bs );
        tightMarshalString2( info->getBrokerURL(), dataOut, bs );
        tightMarshalObjectArray2( wireFormat, info->getPeerBrokerInfos(), dataOut, bs );
        tightMarshalString2( info->getBrokerName(), dataOut, bs );
        bs->readBoolean();
        bs->readBoolean();
        bs->readBoolean();
        if( wireVersion >= 2 ) {
            bs->readBoolean();
        }
        if( wireVersion >= 2 ) {
            bs->readBoolean();
        }
        if( wireVersion >= 2 ) {
            tightMarshalLong2( wireFormat, info->getConnectionId(), dataOut, bs );
        }
        if( wireVersion >= 3 ) {
            tightMarshalString2( info->getBrokerUploadUrl(), dataOut, bs );
        }
        if( wireVersion >= 3 ) {
            tightMarshalString2( info->getNetworkProperties(), dataOut, bs );
        }
    }
    AMQ_CATCH_RETHROW( decaf::io::IOException )
    AMQ_CATCH_EXCEPTION_CONVERT( exceptions::ActiveMQException, decaf::io::IOException )
    AMQ_CATCHALL_THROW( decaf::io::IOException )
}

///////////////////////////////////////////////////////////////////////////////
void BrokerInfoMarshaller::looseUnmarshal( OpenWireFormat* wireFormat, DataStructure* dataStructure, DataInputStream* dataIn ) {

    try {

        BaseCommandMarshaller::looseUnmarshal( wireFormat, dataStructure, dataIn );
        BrokerInfo* info =
            dynamic_cast<BrokerInfo*>( dataStructure );

        int wireVersion = wireFormat->getVersion();

        info->setBrokerId( Pointer<BrokerId>( dynamic_cast< BrokerId* >( 
            looseUnmarshalCachedObject( wireFormat, dataIn ) ) ) );
        info->setBrokerURL( looseUnmarshalString( dataIn ) );

        if( dataIn->readBoolean() ) {
            short size = dataIn->readShort();
            info->getPeerBrokerInfos().reserve( size );
            for( int i = 0; i < size; i++ ) {
                info->getPeerBrokerInfos().push_back( Pointer<BrokerInfo>( dynamic_cast<BrokerInfo* >(
                    looseUnmarshalNestedObject( wireFormat, dataIn ) ) ) );
            }
        } else {
            info->getPeerBrokerInfos().clear();
        }
        info->setBrokerName( looseUnmarshalString( dataIn ) );
        info->setSlaveBroker( dataIn->readBoolean() );
        info->setMasterBroker( dataIn->readBoolean() );
        info->setFaultTolerantConfiguration( dataIn->readBoolean() );
        if( wireVersion >= 2 ) {
            info->setDuplexConnection( dataIn->readBoolean() );
        }
        if( wireVersion >= 2 ) {
            info->setNetworkConnection( dataIn->readBoolean() );
        }
        if( wireVersion >= 2 ) {
            info->setConnectionId( looseUnmarshalLong( wireFormat, dataIn ) );
        }
        if( wireVersion >= 3 ) {
            info->setBrokerUploadUrl( looseUnmarshalString( dataIn ) );
        }
        if( wireVersion >= 3 ) {
            info->setNetworkProperties( looseUnmarshalString( dataIn ) );
        }
    }
    AMQ_CATCH_RETHROW( decaf::io::IOException )
    AMQ_CATCH_EXCEPTION_CONVERT( exceptions::ActiveMQException, decaf::io::IOException )
    AMQ_CATCHALL_THROW( decaf::io::IOException )
}

///////////////////////////////////////////////////////////////////////////////
void BrokerInfoMarshaller::looseMarshal( OpenWireFormat* wireFormat, DataStructure* dataStructure, DataOutputStream* dataOut ) {

    try {

        BrokerInfo* info =
            dynamic_cast<BrokerInfo*>( dataStructure );
        BaseCommandMarshaller::looseMarshal( wireFormat, dataStructure, dataOut );

        int wireVersion = wireFormat->getVersion();

        looseMarshalCachedObject( wireFormat, info->getBrokerId().get(), dataOut );
        looseMarshalString( info->getBrokerURL(), dataOut );
        looseMarshalObjectArray( wireFormat, info->getPeerBrokerInfos(), dataOut );
        looseMarshalString( info->getBrokerName(), dataOut );
        dataOut->writeBoolean( info->isSlaveBroker() );
        dataOut->writeBoolean( info->isMasterBroker() );
        dataOut->writeBoolean( info->isFaultTolerantConfiguration() );
        if( wireVersion >= 2 ) {
            dataOut->writeBoolean( info->isDuplexConnection() );
        }
        if( wireVersion >= 2 ) {
            dataOut->writeBoolean( info->isNetworkConnection() );
        }
        if( wireVersion >= 2 ) {
            looseMarshalLong( wireFormat, info->getConnectionId(), dataOut );
        }
        if( wireVersion >= 3 ) {
            looseMarshalString( info->getBrokerUploadUrl(), dataOut );
        }
        if( wireVersion >= 3 ) {
            looseMarshalString( info->getNetworkProperties(), dataOut );
        }
    }
    AMQ_CATCH_RETHROW( decaf::io::IOException )
    AMQ_CATCH_EXCEPTION_CONVERT( exceptions::ActiveMQException, decaf::io::IOException )
    AMQ_CATCHALL_THROW( decaf::io::IOException )
}

