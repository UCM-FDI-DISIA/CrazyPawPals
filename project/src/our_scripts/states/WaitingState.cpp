
#include "WaitingState.h"


WaitingState::WaitingState() {
}

void 
WaitingState::enter() {
	// No hay nada que hacer al entrar de este estado
}

void 
WaitingState::update(uint32_t delta_time) {
	(void)delta_time;
}

void 
WaitingState::exit() {
	// No hay nada que hacer al salir de este estado
}
