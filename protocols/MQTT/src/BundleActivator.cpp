//
// BundleActivator.cpp
//
// $Id$
//
// Copyright (c) 2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "MQTTClientImpl.h"
#include "IoT/MQTT/MQTTClientServerHelper.h"
#include "Poco/OSP/BundleActivator.h"
#include "Poco/OSP/BundleContext.h"
#include "Poco/OSP/Bundle.h"
#include "Poco/OSP/ServiceRegistry.h"
#include "Poco/OSP/ServiceRef.h"
#include "Poco/OSP/ServiceFinder.h"
#include "Poco/OSP/PreferencesService.h"
#include "Poco/ClassLibrary.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Environment.h"


using Poco::OSP::BundleContext;
using Poco::OSP::ServiceRegistry;
using Poco::OSP::ServiceRef;
using Poco::OSP::ServiceFinder;
using Poco::OSP::Properties;
using Poco::OSP::PreferencesService;


namespace IoT {
namespace MQTT {


class BundleActivator: public Poco::OSP::BundleActivator
{
public:
	BundleActivator()
	{
	}
	
	~BundleActivator()
	{
	}
	
	void createClient(const std::string& baseConfig)
	{
		typedef Poco::RemotingNG::ServerHelper<IoT::MQTT::MQTTClient> ServerHelper;

		std::string serverURI = getStringConfig(baseConfig + ".serverURI", "");
		std::string clientId = getStringConfig(baseConfig + ".id", "");
		std::string persistencePath = getStringConfig(baseConfig + ".persistence.path", "");
		MQTTClientImpl::Persistence persistence = persistencePath.empty() ? MQTTClientImpl::MQTT_PERSISTENCE_NONE : MQTTClientImpl::MQTT_PERSISTENCE_FILE;

		if (!serverURI.empty())
		{
			MQTTClientImpl::ConnectOptions options;
			options.keepAliveInterval = getIntConfig(baseConfig + ".keepAliveInterval", 60);
			options.retryInterval = getIntConfig(baseConfig + ".retryInterval", 30);
			options.connectTimeout = getIntConfig(baseConfig + ".connectTimeout", 20);
			options.cleanSession = getBoolConfig(baseConfig + ".cleanSession", true);
			options.reliable = getBoolConfig(baseConfig + ".reliable", false);
			options.username = getStringConfig(baseConfig + ".username", "");
			options.password = getStringConfig(baseConfig + ".password", "");
			options.willQoS = getIntConfig(baseConfig + ".will.qos", 0);
			options.mqttVersion = getIntConfig(baseConfig + ".mqttVersion", 0);
			Poco::StringTokenizer tok(getStringConfig(baseConfig + ".serverURIs", ""), ";,", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			options.serverURIs.assign(tok.begin(), tok.end());

			options.willTopic = getStringConfig(baseConfig + ".will.topic", "");
			options.willMessage = getStringConfig(baseConfig + ".will.message", "");
			options.willRetained = getBoolConfig(baseConfig + ".will.retained", false);
		
			options.sslTrustStore = getStringConfig(baseConfig + ".ssl.trustStore", "");
			options.sslKeyStore = getStringConfig(baseConfig + ".ssl.keystore", "");
			options.sslPrivateKey = getStringConfig(baseConfig + ".ssl.privateKey", "");
			options.sslPrivateKeyPassword = getStringConfig(baseConfig + ".ssl.privateKeyPassword", "");
			options.sslEnabledCipherSuites = getStringConfig(baseConfig + ".ssl.enabledCipherSuites", "");
			options.sslEnableServerCertAuth = getBoolConfig(baseConfig + ".ssl.enableServerCertAuth", "");
		
			MQTTClientImpl::Ptr pMQTTClient = new MQTTClientImpl(serverURI, clientId, persistence, persistencePath, options);
			std::string oid(Poco::format("io.macchina.mqtt.client#%z", _clients.size()));
			ServerHelper::RemoteObjectPtr pMQTTClientRemoteObject = ServerHelper::createRemoteObject(pMQTTClient, oid);
			Poco::OSP::Properties props;
			props.set("io.macchina.protocol", "io.macchina.mqtt");
			props.set("io.macchina.mqtt.id", clientId);	
			props.set("io.macchina.mqtt.serverURI", serverURI);	
			Poco::OSP::ServiceRef::Ptr pServiceRef = _pContext->registry().registerService(oid, pMQTTClientRemoteObject, props);
			
			_clients.push_back(pMQTTClient);
			_serviceRefs.push_back(pServiceRef);
		}
	}
	
	void start(BundleContext::Ptr pContext)
	{
		_pContext = pContext;
		_pPrefs = ServiceFinder::find<PreferencesService>(pContext);
		
		Poco::Util::AbstractConfiguration::Keys keys;
		_pPrefs->configuration()->keys("mqtt.clients", keys);
		for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
		{
			std::string baseKey = "mqtt.clients.";
			baseKey += *it;
			createClient(baseKey);
		}
	}
		
	void stop(BundleContext::Ptr pContext)
	{
		for (std::vector<Poco::OSP::ServiceRef::Ptr>::iterator it = _serviceRefs.begin(); it != _serviceRefs.end(); ++it)
		{
			pContext->registry().unregisterService(*it);
		}
		_serviceRefs.clear();
		for (std::vector<MQTTClientImpl::Ptr>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			try
			{
				(*it)->disconnect(200);
			}
			catch (Poco::Exception& exc)
			{
				pContext->logger().log(exc);
			}
		}
		_clients.clear();
	}

protected:
	bool getBoolConfig(const std::string& key)
	{
		return _pPrefs->configuration()->getBool(key, _pContext->thisBundle()->properties().getBool(key));
	}

	bool getBoolConfig(const std::string& key, bool deflt)
	{
		return _pPrefs->configuration()->getBool(key, _pContext->thisBundle()->properties().getBool(key, deflt));
	}

	int getIntConfig(const std::string& key)
	{
		return _pPrefs->configuration()->getInt(key, _pContext->thisBundle()->properties().getInt(key));
	}

	int getIntConfig(const std::string& key, int deflt)
	{
		return _pPrefs->configuration()->getInt(key, _pContext->thisBundle()->properties().getInt(key, deflt));
	}
	
	std::string getStringConfig(const std::string& key)
	{
		return _pPrefs->configuration()->getString(key, _pContext->thisBundle()->properties().getString(key));
	}
	
	std::string getStringConfig(const std::string& key, const std::string& deflt)
	{
		return _pPrefs->configuration()->getString(key, _pContext->thisBundle()->properties().getString(key, deflt));
	}
	
private:
	BundleContext::Ptr _pContext;
	PreferencesService::Ptr _pPrefs;
	std::vector<MQTTClientImpl::Ptr> _clients;
	std::vector<Poco::OSP::ServiceRef::Ptr> _serviceRefs;
};


} } // namespace IoT::MQTT


POCO_BEGIN_MANIFEST(Poco::OSP::BundleActivator)
	POCO_EXPORT_CLASS(IoT::MQTT::BundleActivator)
POCO_END_MANIFEST
