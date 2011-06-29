
#include <cmath>

#include "BoardError.h"

BoardError::BoardError(float _cutoff)
    : cutoff(_cutoff)
{
    Filter::TransientCount(2); // only takes two measurements to find a jump
}

BoardError::~BoardError() {
    // no dynamically allocated memory to manage
}

double BoardError::X(double input) {
    if (Steady() && std::abs(Y() - input) > cutoff) {
	//printf("Board Error: %f jumped to %f\n", Y(), input);
	input = Y();
    }

    return Y(input);
}
