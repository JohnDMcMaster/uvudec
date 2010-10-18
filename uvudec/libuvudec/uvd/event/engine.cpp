/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#include "uvd/event/engine.h"
#include "uvd/event/events.h"

/*
UVDRegisteredHandler
*/

bool UVDRegisteredHandler::operator==(const UVDRegisteredHandler &r) const
{
	return m_handler == r.m_handler && m_data == r.m_data;
}

/*
UVDEventEngine
*/

UVDEventEngine::UVDEventEngine()
{
}

UVDEventEngine::~UVDEventEngine()
{
	UV_DEBUG(deinit());
}

uv_err_t UVDEventEngine::init()
{
	m_nextEventKey = UVD_EVENT_DYNAMIC_BASE;
	m_eventTypes[UVD_EVENT_FUNCTION_CHANGED] = "function.changed";
	
	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::deinit()
{
	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::registerHandler(UVDEventHandler handler, void *data, uint32_t priority)
{
	UVDRegisteredHandler registeredHandler;
	
	registeredHandler.m_handler = handler;
	registeredHandler.m_data = data;
	if( m_handlers.find(priority) == m_handlers.end() )
	{
		m_handlers[priority] = HandlerBucket();
	}
	m_handlers[priority].push_back(registeredHandler);
	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::unregisterHandler(UVDEventHandler handler, void *data)
{
	UVDRegisteredHandler toMatch;
	
	toMatch.m_handler = handler;
	toMatch.m_data = data;
	
	for( Handlers::iterator handlersIter = m_handlers.begin(); handlersIter != m_handlers.end(); ++handlersIter )
	{
		HandlerBucket &bucket = (*handlersIter).second;
		
		for( HandlerBucket::iterator bucketIter = bucket.begin(); bucketIter != bucket.end(); ++bucketIter )
		{
			UVDRegisteredHandler &registeredHandler = *bucketIter;
			
			if( registeredHandler == toMatch )
			{
				bucket.erase(bucketIter);
				if( bucket.empty() )
				{
					m_handlers.erase(handlersIter);
				} 
				return UV_ERR_OK;
			}
		}
	}
	return UV_DEBUG(UV_ERR_GENERAL);
}

uv_err_t UVDEventEngine::unregisterHandlerAll(UVDEventHandler handler)
{
	for( Handlers::iterator handlersIter = m_handlers.begin(); handlersIter != m_handlers.end(); )
	{
		HandlerBucket &bucket = (*handlersIter).second;
		
		for( HandlerBucket::iterator bucketIter = bucket.begin(); bucketIter != bucket.end();  )
		{
			UVDRegisteredHandler &registeredHandler = *bucketIter;
			
			if( registeredHandler.m_handler == handler )
			{
				HandlerBucket::iterator toErase = bucketIter;
				++bucketIter;
				bucket.erase(toErase);
			}
			else
			{
				++bucketIter;
			}
		}
		
		if( bucket.empty() )
		{
			Handlers::iterator toErase = handlersIter;
			++handlersIter;
			m_handlers.erase(toErase);
		}
		else
		{
			++handlersIter;
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::emitEvent(const UVDEvent *event)
{
	for( Handlers::iterator handlersIter = m_handlers.begin(); handlersIter != m_handlers.end(); ++handlersIter )
	{
		HandlerBucket &bucket = (*handlersIter).second;
		
		for( HandlerBucket::iterator bucketIter = bucket.begin(); bucketIter != bucket.end(); ++bucketIter )
		{
			UVDRegisteredHandler registeredHandler = *bucketIter;
			uv_err_t handlerRet = UV_ERR_GENERAL;
		
			uv_assert_ret(registeredHandler.m_handler);
			handlerRet = registeredHandler.m_handler(event, registeredHandler.m_data);
			uv_assert_err_ret(handlerRet);
			if( handlerRet == UV_ERR_DONE )
			{
				break;
			}
		}
	}

	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::registerEventType(const std::string &name, uint32_t *out)
{
	uv_assert_ret(out);
	*out = m_nextEventKey;
	uv_assert_err_ret(registerEventTypeCore(name, m_nextEventKey));
	//Successful, advance
	++m_nextEventKey;
	return UV_ERR_OK;
}

uv_err_t UVDEventEngine::registerEventTypeCore(const std::string &name, uint32_t type)
{
	m_eventTypes[type] = name;
	return UV_ERR_OK;
}

