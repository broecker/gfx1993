#ifndef MESSAGE_INCLUDED
#define MESSAGE_INCLUDED

#include <list>


enum MessageType
{
	MESSAGE_NOOP = 0,

	MESSAGE_COUNT
};

struct Message
{
	MessageType 	type;

	union Payload
	{

	} payload;
};


class MessageHandler
{
public:
	MessageHandler();
	virtual ~MessageHandler();

	virtual void handleMessage(const Message& msg) = 0;

	static void sendMessage(Message msg);
	static void propagateMessages();

private:
	typedef std::list<MessageHandler*> HandlerList;
	static HandlerList		sHandlers;

	typedef std::list<Message> MessageQueue;
	static MessageQueue		sMessages;

};


#endif