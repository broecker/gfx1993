#ifndef ENTITY_INCLUDED
#define ENTITY_INCLUDED

#include <glm/glm.hpp>

#include "Message.h"

class Entity : public MessageHandler
{
public:
	typedef unsigned int ID_type; 
	virtual ~Entity();

	virtual void handleMessage(const Message& msg);

 	virtual void update(float dt);
 	virtual void draw();

 	inline ID_type getId() const { return id; }

 	glm::vec2	position;
 	float		rotation;

protected:

private:
	ID_type		id;

};


#endif