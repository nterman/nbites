/**
 * MObject.hpp
 *
 * @class MObject : short for Memory Object, is intended to be the interface
 * to the generic wrapper class to the protobuffer subsystem we employ to
 * manage important data
 *
 *      Author: Octavian Neamtu
 */

#pragma once

namespace memory {

/**
 * @enum MObject_ID
 *
 * This enum is written as an int in the log head in order to identify it
 *
 */
enum MObject_ID {
    MVISION_ID = 1,//!< MVISION_ID
    MSENSORS_ID = 2//!< MSENSORS_ID
};

class MObject {

public:
    /**
     * method update - this should be overwritten by a method that sets all of
     * the proto message fields with values from its respective man counterpart
     */
    virtual void update() = 0;
    /**
     * method log - this should be overwritten by a method that does some
     * sort of logging, or does nothing
     */
    virtual void log() const = 0;

};
}
