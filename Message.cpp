#include "Message.h"

MessageHandler::HandlerList MessageHandler::sHandlers;
MessageHandler::MessageQueue MessageHandler::sMessages;

MessageHandler::MessageHandler()
{
	sHandlers.push_back( this );
}

MessageHandler::~MessageHandler()
{
	sHandlers.remove( this );
}

void MessageHandler::sendMessage(Message msg)
{

}

void MessageHandler::propagateMessages()
{
	while (!sMessages.empty())
	{
		const Message& msg = sMessages.front();

		for (HandlerList::iterator handler = sHandlers.begin(); handler != sHandlers.end(); ++handler )
		{
			(*handler)->handleMessage( msg );
		}

		sMessages.pop_front();

	}
}