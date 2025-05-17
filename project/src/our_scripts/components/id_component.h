
#include "../../ecs/Component.h"
#include "../../ecs/ecs.h"
#include <bitset>
class id_component : public ecs::Component {
private:
	inline static std::bitset<256> ocuppied_ids;
	inline static uint8_t last_given_bit;
	uint8_t _id;
public:
	inline static void reset() {
		ocuppied_ids.reset();
		last_given_bit = 0;
	}
	id_component() : _id(get_next_free_id()) { ocuppied_ids[_id] = true; }
	id_component(uint8_t _new_id) : _id(_new_id) {}
	~id_component() {
		ocuppied_ids[_id] = false;
	}
	__CMPID_DECL__(ecs::cmp::ID_COMPONENT);

	static inline int get_next_free_id() {
		int aux = last_given_bit;
		++last_given_bit;
		while (ocuppied_ids[last_given_bit] && last_given_bit!=aux) last_given_bit = last_given_bit >= ocuppied_ids.size() ? 0 : ++last_given_bit;
		//Si esto peta, se han llenado los ids y no se pueden dar m�s
		assert(aux != last_given_bit);
		return last_given_bit;
	}

	inline uint8_t getId() { return _id; }
};