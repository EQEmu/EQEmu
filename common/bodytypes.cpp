#include "bodytypes.h"

std::string BodyType::GetName(uint8 body_type_id)
{
	return IsValid(body_type_id) ? body_type_names[body_type_id] : "UNKNOWN BODY TYPE";
}

bool BodyType::IsValid(uint8 body_type_id)
{
	return body_type_names.find(body_type_id) != body_type_names.end();
}
