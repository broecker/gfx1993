#ifndef ENGINE_INCLUDED
#define ENGINE_INCLUDED

#include "Message.h"

class Engine : public MessageHandler
{
public:

	void update(float dt);
	void draw();

	void handleMessage(const Message& msg);


};

#endif